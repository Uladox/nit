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

enum ftree_type { EMPTY = 0, SINGLE = 1 };

static void
bnch_inc_refs(Nit_fbnch *bnch)
{
	Nit_fbnch **bnchs = (Nit_fbnch **) bnch->elems;
	int i = 0;

	for (; i < bnch->cnt; ++i)
		++bnchs[i]->refs;
}

static Nit_fbnch *
fbnch_new_arr(void **arr, int cnt)
{
	size_t elems = cnt * sizeof(void *);
	Nit_fbnch *bnch = malloc(sizeof(*bnch) + elems);

	pcheck(bnch, NULL);
	bnch->refs = 1;
	bnch->cnt = cnt;
	memcpy(bnch->elems, arr, elems);

	/* if (depth > 0) */
	/* 	bnch_inc_refs(bnch, cnt); */

	return bnch;
}

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

static void
ftree_bnch_inc_refs(Nit_ftree *tree)
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
ftree_copy(Nit_ftree *tree)
{
	Nit_ftree *copy = palloc(copy);
	Nit_ftree *next = LIST_NEXT(tree);

	pcheck(copy, NULL);
        memcpy(copy, tree, sizeof(*tree));
	copy->refs = 1;

	if (next)
		++next->refs;

	if (tree->depth > 0)
		ftree_bnch_inc_refs(copy);

	return copy;
}

static void
reduce_branches(Nit_fbnch *b, short depth)
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

#include <stdio.h>
#include <inttypes.h>

void
ftree_reduce(Nit_ftree *tree)
{
	int i = 0;

	if (unlikely(!tree))
		return;

	if (--tree->refs)
		return;

	ftree_reduce(LIST_NEXT(tree));

	for (; i < tree->precnt; ++i)
		reduce_branches(tree->pre[i], tree->depth);

	i = 0;

	for (; i < tree->sufcnt; ++i)
		reduce_branches(tree->suf[i], tree->depth);

	free(tree);
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

/* static Nit_fbnch * */
/* fbnch_new_elem(void *elem, int max) */
/* { */
/* 	size_t elems = max * sizeof(elem); */
/* 	Nit_fbnch *bnch = malloc(sizeof(*bnch) + elems); */

/* 	pcheck(bnch, NULL); */
/* 	bnch->refs = 1; */
/* 	bnch->cnt = 1; */
/* 	bnch->max = max; */
/* 	bnch->elems[0] = elem; */
/* 	memset(bnch->elems + 1, 0, elems - sizeof(elem)); */

/* 	return bnch; */
/* } */

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

/* static void */
/* digit_set(void **digit, Nit_fbnch *b, void *elem) */
/* { */
/* 	/\* [___] + [ABCD] -> [ABC] *\/ */
/* 	memcpy(b->elems, digit, sizeof(b->elems)); */
/* 	/\* [ABCD] -> [DBCD] *\/ */
/*         memcpy(digit, digit + HALF + 1, sizeof(elem) * (HALF - 1)); */
/* 	/\* [DBCD] + X -> [DXCD] *\/ */
/*         digit[HALF - 1] = elem; */
/* 	/\* [DXCD] -> [DX__] *\/ */
/* 	memset(digit + HALF, 0, sizeof(elem) * HALF); */
/* } */

static inline int
next_tree_add(Nit_ftree **tree, uint8_t *fixcnt, void **fix, void **elem)
{
	Nit_fbnch *b = fbnch_new_arr(fix, FTREE_BS);
	Nit_ftree *next = mut_next(*tree);

	pcheck_c(next, 0, free(b));

	*fixcnt = 1;
	fix[0] = *elem;
	memset(fix + 1, 0, sizeof(*elem) * (FTREE_BS - 1));

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
	Nit_fbnch *b;

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

	*fixcnt = b->cnt;

	memcpy(fix, b->elems, b->cnt * sizeof(void *));

	/* printf("hi %" PRIu32 "\n", b->refs); */

	if (b->refs-- == 1)
		free(b);
	else if (tree->depth > 0)
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
		/* fromfix[last_entry] = NULL; */
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

static Nit_ftree *
concat_top(Nit_ftree *left, Nit_ftree *right)
{
	Nit_ftree *tree = ftree_new(left->depth);

	pcheck(tree, NULL);
	memcpy(tree->pre, left->pre, sizeof(left->pre));
	tree->precnt = left->precnt;
	memcpy(tree->suf, right->suf, sizeof(right->suf));
	tree->sufcnt = right->sufcnt;

	if (tree->depth > 0)
		ftree_bnch_inc_refs(tree);

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
		mid[new_elems++] = fbnch_new_arr(arr_ptr, FTREE_BS);

	if (cnt)
		mid[new_elems++] = fbnch_new_arr(arr_ptr, cnt);

	/* for (; cnt > FTREE_BS; */
	/*      ++new_elems, cnt -= FTREE_BS, arr_ptr += FTREE_BS) */
	/*         if (!(mid[new_elems] = fbnch_new_arr(arr_ptr, FTREE_BS))) { */
	/* 		while (new_elems >= 0) */
	/* 			free(arr[new_elems--]); */
	/* 		printf("error!\n"); */
	/* 		return 0; */
	/* 	} */

	/* if (cnt) */
	/*         if(!(mid[new_elems++] = fbnch_new_arr(arr_ptr, cnt))) { */
	/* 		 while (new_elems >= 0) */
	/* 			 free(arr[new_elems--]); */
	/* 		 printf("error!\n"); */
	/* 		 return 0; */
	/* 	} */

	*elems = new_elems;
	printf("%i what?!\n", *elems);

	return 1;
}

/* static inline int */
/* concat_simp_types(Nit_ftree **layer, Nit_ftree *left, Nit_ftree *right) */
/* { */
/* 	switch (ftree_type(right)) { */
/* 	case EMPTY: */
/* 		pcheck((*layer = ftree_copy(left)), -1); */
/* 		return 1; */
/* 	case SINGLE: */
/* 		pcheck((*layer = ftree_copy(left)), -1); */
/* 		ftree_append(*layer, single_elem_get(right)); */
/* 		printf("layer: %i %i!\n", (*layer)->precnt, */
/* 		       (*layer)->sufcnt); */
/* 	        return 1; */
/* 	} */

/* 	switch (ftree_type(left)) { */
/* 	case EMPTY: */
/* 		pcheck((*layer = ftree_copy(right)), -1); */
/* 	        return 1; */
/* 	case SINGLE: */
/* 		pcheck((*layer = ftree_copy(right)), -1); */
/* 		ftree_append(*layer, single_elem_get(left)); */
/* 	        return 1; */
/* 	} */

/* 	return 0; */
/* } */

static int
concat_mid_append(Nit_ftree *layer, Nit_fbnch **mid, int elems)
{
	while (elems--)
		if (!ftree_append(layer, mid[elems]))
			return 0;
	return 1;
}

static int
concat_mid_prepend(Nit_ftree *layer, Nit_fbnch **mid, int elems)
{
	int i = 0;

	while (i < elems)
		if (!ftree_prepend(layer, mid[i++]))
			return 0;
	return 1;
}

Nit_ftree *
ftree_concat(Nit_ftree *left, Nit_ftree *right)
{
	Nit_ftree *top = NULL;
	Nit_ftree **layer = &top;
	int depth = 0;
	int elems = 0;
	Nit_fbnch *mid[3];

	for (; right || left;
	     LIST_INC(right), LIST_INC(left),
		     (layer = NEXT_REF(*layer), ++depth)) {
		/* printf("\nagain\n"); */
		if (!left) {
				printf("how!\n");
			pcheck((*layer = ftree_copy(right)), NULL);

		        if (!concat_mid_append(*layer, mid, elems))
				return NULL;

			return top;
		}

		if (!right) {
			pcheck((*layer = ftree_copy(left)), NULL);

			if (!concat_mid_prepend(*layer, mid, elems))
					return NULL;

		        return top;
		}

		switch (ftree_type(right)) {
		case EMPTY:
			pcheck((*layer = ftree_copy(left)), NULL);

		        if (!concat_mid_append(*layer, mid, elems))
				return NULL;

			return top;
		case SINGLE:
			printf("%i how\n", elems);
			pcheck((*layer = ftree_copy(left)), NULL);
			ftree_append(*layer, single_elem_get(right));

		        if (!concat_mid_append(*layer, mid, elems))
				return NULL;
			/* printf("layer: %i %i!\n", (*layer)->precnt, */
			/*        (*layer)->sufcnt); */

			return top;
		}

		switch (ftree_type(left)) {
		case EMPTY:
			pcheck((*layer = ftree_copy(right)), NULL);

			if (!concat_mid_prepend(*layer, mid, elems))
					return NULL;

			return top;
		case SINGLE:
			pcheck((*layer = ftree_copy(right)), NULL);
			ftree_prepend(*layer, single_elem_get(left));

			if (!concat_mid_prepend(*layer, mid, elems))
					return NULL;

			return top;
		}



		pcheck((*layer = concat_top(left, right)), NULL);


		if (!nodes(&elems, depth, mid, left->sufcnt, left->suf,
			   right->precnt, right->pre))
			return NULL;
	}

	if (elems) {
		printf("elems!\n");
		printf("%i\n", elems);
		pcheck(*layer = ftree_new(depth), NULL);
		if (!concat_mid_prepend(*layer, mid, elems))
			return NULL;
	}

	return top;
}
