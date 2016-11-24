#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "list.h"
#include "ftree.h"

#define ARR_END (FTREE_BS - 1)

enum ftree_type { EMPTY = 0, SINGLE = 1 };

#define NONE (SINGLE + 1)

static void
bnch_inc_refs(Nit_fbnch *bnch)
{
	Nit_fbnch **bnchs = (Nit_fbnch **) bnch->elems;
	int i = 0;

	for (; i < bnch->cnt; ++i)
		++bnchs[i]->refs;
}

static Nit_fbnch *
fbnch_new(void **arr, int cnt)
{
	size_t elems = cnt * sizeof(void *);
	Nit_fbnch *bnch = malloc(sizeof(*bnch) + elems);

	pcheck(bnch, NULL);
	bnch->ano = NULL;
	bnch->refs = 1;
	bnch->cnt = cnt;
	memcpy(bnch->elems, arr, elems);

	return bnch;
}

Nit_ftree *
ftree_new(void)
{
	Nit_ftree *tree = palloc(tree);

	pcheck(tree, NULL);
	memset(tree, 0, sizeof(*tree));
	tree->refs = 1;

	return tree;
}

static void
ftree_inc_bnch_refs(Nit_ftree *tree)
{
	int i = 0;
	Nit_fbnch **bnch = (Nit_fbnch **) tree->pre;

	for (; i < tree->precnt; ++i)
		++bnch[i]->refs;

	i = 0;
	bnch = (Nit_fbnch **) tree->suf;

	for (; i < tree->sufcnt; ++i)
		++bnch[i]->refs;
}

Nit_ftree *
ftree_copy(Nit_ftree *tree, int depth)
{
	Nit_ftree *copy = palloc(copy);
	Nit_ftree *next = LIST_NEXT(tree);

	pcheck(copy, NULL);
	memcpy(copy, tree, sizeof(*tree));
	copy->refs = 1;

	if (next)
		++next->refs;

	if (depth > 0)
		ftree_inc_bnch_refs(copy);

	return copy;
}

static void
reduce_branches(Nit_fbnch *b, int depth)
{
	int i = 0;

	if (!depth || !b || --b->refs > 0)
		return;

	if (depth == 1) {
		free(b);
		return;
	}

	for (; i < b->cnt; ++i)
		reduce_branches(b->elems[i], depth - 1);

	free(b);
}

void
ftree_reduce(Nit_ftree *tree, int depth)
{
	int i = 0;

	if (unlikely(!tree))
		return;

	if (--tree->refs)
		return;

	ftree_reduce(LIST_NEXT(tree), depth + 1);

	for (; i < tree->precnt; ++i)
		reduce_branches(tree->pre[i], depth);

	i = 0;

	for (; i < tree->sufcnt; ++i)
		reduce_branches(tree->suf[i], depth);

	free(tree);
}

static Nit_ftree *
mut_usable(Nit_ftree *tree, int depth)
{
	Nit_ftree *copy;

	if (tree->refs == 1)
		return tree;

	pcheck(copy = ftree_copy(tree, depth), NULL);
	ftree_reduce(tree, depth);

	return copy;
}

static Nit_ftree *
mut_next(Nit_ftree *tree, int depth)
{
	Nit_ftree *next = LIST_NEXT(tree);
	Nit_ftree *tmp = next ? mut_usable(next, depth + 1) :
		ftree_new();

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

static inline int
next_tree_add(Nit_ftree **tree, uint8_t *fixcnt, void **fix, void **elem,
	      int depth)
{
	Nit_fbnch *b = fbnch_new(fix, FTREE_BS);
	Nit_ftree *next = mut_next(*tree, depth);

	pcheck_c(next, 0, free(b));

	*fixcnt = 1;
	fix[0] = *elem;
	memset(fix + 1, 0, sizeof(*elem) * (FTREE_BS - 1));

	*tree = next;
	*elem = b;

	return 1;
}

int
ftree_prepend(Nit_ftree *tree, void *elem, int depth)
{
	for (;; ++depth) {
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

		if (!next_tree_add(&tree, &tree->precnt, tree->pre, &elem, depth))
			return 0;
	}

	return 0;
}

int
ftree_append(Nit_ftree *tree, void *elem, int depth)
{
        for (;; ++depth) {
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

		if (!next_tree_add(&tree, &tree->sufcnt, tree->suf, &elem, depth))
			return 0;
	}

	return 0;
}

static inline int
promote_node(Nit_ftree *tree, uint8_t *fixcnt, void **fix, int depth)
{
	Nit_ftree *next = LIST_NEXT(tree);
	Nit_fbnch *b;

	if (unlikely(!next))
		return 0;

	next = mut_next(tree, depth);
	pcheck(next, 0);

	if (!(b = ftree_pop(next, depth + 1)))
		return 0;

	if (unlikely(ftree_type(next) == EMPTY)) {
		ftree_reduce(next, depth + 1);
		LIST_CONS(tree, NULL);
	}

	*fixcnt = b->cnt;
	memcpy(fix, b->elems, b->cnt * sizeof(void *));

	if (b->refs-- == 1)
		free(b);
	else if (depth > 0)
		bnch_inc_refs(b);

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
		*tocnt = *fromcnt - 1;
		*fromcnt = 1;
		return;
	}

	/* Makes single */
	tree->precnt = 1;
	tree->sufcnt = 0;
	single_elem_set(tree, fromfix[0]);
	tree->suf[0] = NULL;
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
ftree_pop(Nit_ftree *tree, int depth)
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
	if (promote_node(tree, &tree->precnt, tree->pre, depth))
		return val;

	/* Suffix to prefix */
	fix_to_fix(tree, &tree->precnt, tree->pre, &tree->sufcnt, tree->suf);

	return val;
}

void *
ftree_rpop(Nit_ftree *tree, int depth)
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
	if (promote_node(tree, &tree->sufcnt, tree->suf, depth))
		return val;

	/* Suffix to prefix */
	fix_to_fix(tree, &tree->sufcnt, tree->suf, &tree->precnt, tree->pre);

	return val;
}

static Nit_ftree *
concat_top(Nit_ftree *left, Nit_ftree *right, int depth)
{
	Nit_ftree *tree = ftree_new();

	pcheck(tree, NULL);
	memcpy(tree->pre, left->pre, sizeof(left->pre));
	tree->precnt = left->precnt;
	memcpy(tree->suf, right->suf, sizeof(right->suf));
	tree->sufcnt = right->sufcnt;

	if (depth > 0)
		ftree_inc_bnch_refs(tree);

	return tree;
}

static int
nodes(int *elems, int depth, Nit_fbnch **mid, int lsufcnt, void **lsuf,
      int rprecnt, void **rpre)
{
	int new_elems = 0;
	int cnt = *elems + lsufcnt + rprecnt;
	void *arr[3 * FTREE_BS];
	void **arr_ptr = arr;
	int i;

	if (depth > 0) {
		for (i = 0; i < rprecnt; ++i)
			++((Nit_fbnch *) rpre[i])->refs;

		for (i = 0; i < lsufcnt; ++i)
			++((Nit_fbnch *) lsuf[i])->refs;
	}

	memcpy(arr, lsuf, lsufcnt * sizeof(*lsuf));
	memcpy(arr + lsufcnt, mid, *elems * sizeof(*mid));
	memcpy(arr + lsufcnt + *elems, rpre, rprecnt * sizeof(*rpre));

	for (; cnt > FTREE_BS; cnt -= FTREE_BS, arr_ptr += FTREE_BS)
		if (!(mid[new_elems++] = fbnch_new(arr_ptr, FTREE_BS)))
			goto error;

	if (cnt)
		if (!(mid[new_elems++] = fbnch_new(arr_ptr, cnt)))
			goto error;

	*elems = new_elems;

	return 1;

error:
	while (new_elems--)
		free(arr[new_elems]);

	return 0;
}

static int
concat_mid_append(Nit_ftree *layer, Nit_fbnch **mid, int elems, int depth)
{
	while (elems--)
		if (!ftree_append(layer, mid[elems], depth))
			return 0;
	return 1;
}

static int
concat_mid_prepend(Nit_ftree *layer, Nit_fbnch **mid, int elems, int depth)
{
	int i = 0;

	while (i < elems)
		if (!ftree_prepend(layer, mid[i++], depth))
			return 0;
	return 1;
}

static inline Nit_ftree *
concat_error(Nit_ftree *top, int depth)
{
	ftree_reduce(top, depth);

	return NULL;
}

static int
concat_rfinal(Nit_ftree **layer, Nit_ftree *right,
	      Nit_fbnch **mid, int elems, int depth)
{
	pcheck((*layer = ftree_copy(right, depth)), -1);

	if (!concat_mid_append(*layer, mid, elems, depth))
		return -1;

	return 1;
}

static int
concat_lfinal(Nit_ftree **layer, Nit_ftree *left,
	      Nit_fbnch **mid, int elems, int depth)
{
	pcheck((*layer = ftree_copy(left, depth)), -1);

	if (!concat_mid_prepend(*layer, mid, elems, depth))
		return -1;

	return 1;
}

static int
concat_srfinal(Nit_ftree **layer, Nit_ftree *right,
	       Nit_ftree *left, Nit_fbnch **mid, int elems, int depth)
{
	pcheck((*layer = ftree_copy(right, depth)), -1);
	ftree_prepend(*layer, single_elem_get(left), depth);


	if (!concat_mid_append(*layer, mid, elems, depth))
		return -1;

	return 1;
}

static int
concat_slfinal(Nit_ftree **layer, Nit_ftree *left,
	       Nit_ftree *right, Nit_fbnch **mid, int elems, int depth)
{
	pcheck((*layer = ftree_copy(left, depth)), -1);
	ftree_append(*layer, single_elem_get(right), depth);

	if (!concat_mid_prepend(*layer, mid, elems, depth))
		return -1;

	return 1;
}

static int
concat_finals(Nit_ftree **layer, Nit_ftree *left,
	      Nit_ftree *right, Nit_fbnch **mid, int elems, int depth)
{
	enum ftree_type type = NONE;

	if (!left || (type = ftree_type(left)) == EMPTY)
		return concat_rfinal(layer, right, mid, elems, depth);

	if (type == SINGLE)
		return concat_srfinal(layer, right, left, mid, elems, depth);

	if (!right || (type = ftree_type(right)) == EMPTY)
		return concat_lfinal(layer, left, mid, elems, depth);

	if (type == SINGLE)
		return concat_slfinal(layer, left, right, mid, elems, depth);

	return 0;
}

Nit_ftree *
ftree_concat(Nit_ftree *left, Nit_ftree *right)
{
	Nit_ftree *top = NULL;
	Nit_ftree **layer = &top;
	Nit_fbnch *mid[3];
	int depth = 0;
	int elems = 0;

	for (; right || left;
	     (LIST_INC(right), LIST_INC(left),
	      layer = NEXT_REF(*layer), ++depth)) {

		switch (concat_finals(layer, left, right, mid, elems, depth)) {
		case -1:
			return concat_error(top, depth);
		case 0:
			break;
		case 1:
			return top;
		}


		pcheck((*layer = concat_top(left, right, depth)), NULL);


		if (!nodes(&elems, depth, mid,
			   left->sufcnt, left->suf, right->precnt, right->pre))
		        return concat_error(top, depth);
	}

	if (elems) {
		pcheck(*layer = ftree_new(), NULL);

		if (!concat_mid_prepend(*layer, mid, elems, depth))
			return concat_error(top, depth);
	}

	return top;
}
