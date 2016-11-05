#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "list.h"
#include "ftree.h"

#define ARR_END (FTREE_BS - 1)
#define HALF (FTREE_BS / 2)

typedef struct {
	void *elems[HALF + 1];
} Branch;

enum ftree_type { EMPTY = 0, SINGLE = 1 };

Nit_ftree *
ftree_new(short depth)
{
	Nit_ftree *tree = palloc(tree);

	pcheck(tree, NULL);
        memset(tree, 0, sizeof(*tree));
	tree->refs = 1;
	tree->depth = depth;

	return tree;
}

Nit_ftree *
ftree_copy(Nit_ftree *tree)
{
	Nit_ftree *copy = palloc(copy);
	Nit_ftree *next = LIST_NEXT(tree);

	pcheck(copy, NULL);
        memcpy(copy, tree, sizeof(*tree));
	copy->refs = 1;

	if (next)
		++next->refs;

	return copy;
}

static void
free_branches(Branch *b, short depth)
{
	int i = 0;

	if (!b)
		return;

	if (depth == 1) {
		free(b);
		return;
	}

	for (; i <= HALF; ++i)
		free_branches(b->elems[i], depth - 1);

	free(b);
}

void
ftree_reduce(Nit_ftree *tree)
{
	int i = 0;

	if (unlikely(!tree))
		return;

	if (!--tree->refs) {
		ftree_reduce(LIST_NEXT(tree));

		if (!tree->depth) {
			free(tree);
			return;
		}

		for (; i < FTREE_BS; ++i)
			free_branches(tree->pre[i], tree->depth);

		i = 0;

		for (; i < FTREE_BS; ++i)
			free_branches(tree->suf[i], tree->depth);

		free(tree);
	}
}

static Nit_ftree *
mut_usable(Nit_ftree *tree)
{
	Nit_ftree *copy;

	if (tree->refs == 1)
		return tree;

        pcheck(copy = ftree_copy(tree), NULL);
	ftree_reduce(tree);

	return copy;
}

static Nit_ftree *
mut_next(Nit_ftree *tree)
{
	Nit_ftree *next = LIST_NEXT(tree);
	Nit_ftree *tmp = next ? mut_usable(next) :
		ftree_new(tree->depth + 1);

	pcheck(tmp, NULL);

	if (unlikely(next != tmp))
		LIST_CONS(tree, tmp);

	return tmp;
}

static inline enum ftree_type
ftree_type(const Nit_ftree *tree)
{
	return tree->precnt + tree->sufcnt;
}

static inline void *
single_elem_get(const Nit_ftree *tree)
{
	return tree->pre[0];
}

static inline void
single_elem_set(Nit_ftree *tree, void *elem)
{
        tree->pre[0] = elem;
}

static inline void
single_to_multi_prepend(Nit_ftree *tree, void *elem)
{
	tree->sufcnt = 1;
	tree->suf[0] = single_elem_get(tree);
        single_elem_set(tree, elem);
}

static inline void
single_to_multi_append(Nit_ftree *tree, void *elem)
{
	tree->sufcnt = 1;
	tree->suf[0] = elem;
}

static inline void *
ftree_get(const Nit_ftree *tree, const uint8_t *fixcnt, void *const *fix)
{
	switch (ftree_type(tree)) {
	case EMPTY:
		return NULL;
	case SINGLE:
		return single_elem_get(tree);
	}

	return fix[*fixcnt - 1];
}

void *
ftree_first(const Nit_ftree *tree)
{
	return ftree_get(tree, &tree->precnt, tree->pre);
}

void *
ftree_last(const Nit_ftree *tree)
{
	return ftree_get(tree, &tree->sufcnt, tree->suf);
}

static void
digit_set(void **digit, Branch *b, void *elem)
{
	/* [___] + [ABCD] -> [ABC] */
	memcpy(b->elems, digit, sizeof(b->elems));
	/* [ABCD] -> [DBCD] */
        memcpy(digit, digit + HALF + 1, sizeof(elem) * (HALF - 1));
	/* [DBCD] + X -> [DXCD] */
        digit[HALF - 1] = elem;
	/* [DXCD] -> [DX__] */
	memset(digit + HALF, 0, sizeof(elem) * HALF);
}

static inline int
next_tree_add(Nit_ftree **tree, uint8_t *fixcnt, void **fix, void **elem)
{
	Branch *b = palloc(b);
	Nit_ftree *next = mut_next(*tree);

	pcheck_c(next, 0, free(b));

        *fixcnt = FTREE_BS / 2;
	digit_set(fix, b, *elem);

	*tree = next;
	*elem = b;

	return 1;
}

int
ftree_prepend(Nit_ftree *tree, void *elem)
{
	while (1) {
		switch (ftree_type(tree)) {
		case EMPTY:
			single_elem_set(tree, elem);
			tree->precnt = 1;
			return 1;
		case SINGLE:
			single_to_multi_prepend(tree, elem);
			return 1;
		}

		if (likely(!tree->pre[ARR_END])) {
			tree->pre[tree->precnt++] = elem;
			return 1;
		}

		if (!next_tree_add(&tree, &tree->precnt, tree->pre, &elem))
			return 0;
	}

	/* Should never reach here */

	return 0;
}

int
ftree_append(Nit_ftree *tree, void *elem)
{
	while (1) {
		switch (ftree_type(tree)) {
		case EMPTY:
			single_elem_set(tree, elem);
			tree->precnt = 1;
			return 1;
		case SINGLE:
			single_to_multi_append(tree, elem);
		        return 1;
		}

		if (likely(!tree->suf[ARR_END])) {
			tree->suf[tree->sufcnt++] = elem;
		        return 1;
		}

		if (!next_tree_add(&tree, &tree->sufcnt, tree->suf, &elem))
			return 0;
	}

	return 0;
}

static inline int
promote_node(Nit_ftree *tree, uint8_t *fixcnt, void **fix)
{
	Nit_ftree *next = LIST_NEXT(tree);
	Branch *b;

	if (unlikely(!next))
		return 0;

	next = mut_next(tree);
	pcheck(next, 0);

	if (!(b = ftree_pop(next)))
		return 0;

	if (unlikely(ftree_type(next) == EMPTY)) {
		ftree_reduce(next);
		LIST_CONS(tree, NULL);
	}

	*fixcnt = ARRAY_UNITS(b->elems);
	memcpy(fix, b->elems, sizeof(b->elems));

	free(b);

	return 1;
}

static inline void
fix_to_fix(Nit_ftree *tree, uint8_t *tocnt, void **tofix,
	   uint8_t *fromcnt, void **fromfix)
{
	if (likely(*fromcnt != 1)) {
		int last_entry = *fromcnt - 1;

		memcpy(tofix, fromfix, sizeof(*fromfix) * last_entry);
		fromfix[0] = fromfix[last_entry];
		memset(fromfix + 1, 0, sizeof(*fromfix) * last_entry);
		fromfix[last_entry] = NULL;
	        *tocnt = *fromcnt;
		*tocnt = 1;
		return;
	}

	/* Makes single */
	*fromcnt = 0;
	single_elem_set(tree, fromfix[0]);
	fromfix[0] = NULL;
}

static inline void *
take_val_off(uint8_t *fixcnt, void **fix)
{
	void *val = fix[*fixcnt - 1];

        fix[*fixcnt - 1] = NULL;
	--*fixcnt;

	return val;
}

void *
ftree_pop(Nit_ftree *tree)
{
	void *val;

	/* [] [X] */
	switch (ftree_type(tree)) {
		case EMPTY:
			return NULL;
		case SINGLE:
		        val = single_elem_get(tree);
			single_elem_set(tree, NULL);
			tree->precnt = tree->sufcnt = 0;
			return val;
	}

	/* [ABCD] -> [ABC_] */
	if (likely(tree->precnt > 1))
		return take_val_off(&tree->precnt, tree->pre);

	val = tree->pre[0];

	/* [A___] + [[BCD] [EFG] [HIJ]] -> [BCD_] */
        if (promote_node(tree, &tree->precnt, tree->pre))
		return val;

	/* Suffix to prefix */
	fix_to_fix(tree, &tree->precnt, tree->pre, &tree->sufcnt, tree->suf);

	return val;
}

void *
ftree_rpop(Nit_ftree *tree)
{
	void *val;

	/* [] [X] */
	switch (ftree_type(tree)) {
		case EMPTY:
			return NULL;
		case SINGLE:
		        val = single_elem_get(tree);
			single_elem_set(tree, NULL);
			tree->precnt = tree->sufcnt = 0;
			return val;
	}

	/* [ABCD] -> [ABC_] */
	if (likely(tree->sufcnt > 1))
		return take_val_off(&tree->sufcnt, tree->suf);

	val = tree->suf[0];

	/* [A___] + [[BCD] [EFG] [HIJ]] -> [BCD_] */
        if (promote_node(tree, &tree->sufcnt, tree->suf))
		return val;

	/* Suffix to prefix */
	fix_to_fix(tree, &tree->sufcnt, tree->suf, &tree->precnt, tree->pre);

	return val;
}

typedef struct {
	Nit_dlist list;
	Branch *branch;
} Node_list;

typedef struct {
	Node_list *front;
        Node_list *end;
	void *remain[HALF - 1];
} Nodes;

static Nit_ftree *
concat_middle(Nit_ftree *left, Nit_dlist *mid, Nit_ftree *right, int *error)
{
	Nit_ftree *result = ftree_new(0);
	Nit_ftree *level = result;

	/* just right, keep prepending, deal with single first */
	/* just left, keep appending, deal with single first */

	for (; right || left; LIST_INC(right), LIST_INC(left)) {
		if (!left) {
			if (!mid)
				return right;

			for (; LIST_NEXT(mid); mid = LIST_NEXT(mid));

			preveach (mid)
				if (!ftree_prepend(right, mid)) {
					*error = 1;
					return NULL;
				}

			return right;
		}
	}


	if (!right) {
		if (!mid)
			return left;

		foreach (mid)
			if (!ftree_append(left, mid)) {
				*error = 1;
				return NULL;
			}

		return left;
	}

	switch (ftree_type(left)) {
	case EMPTY:
		if (!mid)
			return right;

		result = concat_middle(
	}
		return

	return concat_middle(left, NULL, right);
}

/* Nit_ftree * */
/* ftree_concat(Nit_ftree *left, Nit_ftree *right) */
/* { */
/* 	return concat_middle(left, NULL, right); */
/* } */
