#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "list.h"
#include "ftree.h"

#define ARR_END (FTREE_BS - 1)
#define HALF (FTREE_BS / 2)

typedef struct {
	void *elems[3];
} Branch3;

enum ftree_type { EMPTY, SINGLE };

Nit_ftree *
ftree_new(void)
{
	Nit_ftree *tree = palloc(tree);

	pcheck(tree, NULL);
        memset(tree, 0, sizeof(*tree));

	return tree;
}

static inline enum ftree_type
ftree_type(Nit_ftree *tree)
{
	return tree->precnt + tree->sufcnt;
}

static inline void *
single_elem_get(const Nit_ftree *tree)
{
	return tree->suf[0];
}

static inline void
single_elem_set(Nit_ftree *tree, void *elem)
{
        tree->suf[0] = elem;
}

static inline void
single_to_multi_prepend(Nit_ftree *tree, void *elem)
{
	tree->pre[ARR_END] = elem;
}

static inline void
single_to_multi_append(Nit_ftree *tree, void *elem)
{
	tree->pre[ARR_END] = single_elem_get(tree);
        single_elem_set(tree, elem);
}

void *
ftree_first(Nit_ftree *tree)
{
	switch (ftree_type(tree)) {
	case EMPTY:
		return NULL;
	case SINGLE:
		return single_elem_get(tree);
	}

	return tree->pre[FTREE_BS - tree->precnt];
}

void *
ftree_last(Nit_ftree *tree)
{
	switch (ftree_type(tree)) {
	case EMPTY:
		return NULL;
	case SINGLE:
		return single_elem_get(tree);
	}

	return tree->suf[tree->sufcnt - 1];
}

static void
prepend_set(void **pre, Branch3 *b, void *elem)
{
	/* [___] + [ABCD] -> [BCD] */
	memcpy(b->elems, pre + HALF - 1, sizeof(elem) * HALF + 1);
	/* [ABCD] -> [ABCA] */
        memcpy(pre + HALF + 1, pre, sizeof(elem) * HALF - 1);
	/* [ABCA] + X -> [ABXA] */
        pre[HALF] = elem;
	/* [ABXA] -> [__XA] */
	memset(pre, 0, sizeof(elem) * HALF);
}

int
ftree_prepend(Nit_ftree *tree, void *elem)
{
	Branch3 *b;
	Nit_ftree *next;

	for (;; tree = next, elem = b) {
		switch (ftree_type(tree)) {
		case EMPTY:
			single_elem_set(tree, elem);
			goto success;
		case SINGLE:
			single_to_multi_prepend(tree, elem);
			goto success;
		}

		if (likely(!tree->pre[0])) {
			tree->pre[ARR_END - tree->precnt] = elem;
			goto success;
		}

	        b = palloc(b);
	        next = LIST_NEXT(tree);

		if (unlikely(!next))
			pcheck_c((next = ftree_new()), 0, free(b));

		tree->precnt = FTREE_BS / 2;
		prepend_set(tree->pre, b, elem);
	}

success:
	++tree->precnt;
	return 1;
}

static void
prepend_set(void **pre, Branch3 *b, void *elem)
{
	/* [___] + [ABCD] -> [BCD] */
	memcpy(b->elems, pre + HALF - 1, sizeof(elem) * HALF + 1);
	/* [ABCD] -> [ABCA] */
        memcpy(pre + HALF + 1, pre, sizeof(elem) * HALF - 1);
	/* [ABCA] + X -> [ABXA] */
        pre[HALF] = elem;
	/* [ABXA] -> [__XA] */
	memset(pre, 0, sizeof(elem) * HALF);
}

/* int */
/* ftree_append(Nit_ftree *tree, void *elem) */
/* { */
/* 	Branch3 *b; */
/* 	Nit_ftree *next; */

/* 	for (;; tree = next, elem = b) { */
/* 		switch (ftree_type(tree)) { */
/* 		case EMPTY: */
/* 			single_elem_set(tree, elem); */
/* 			goto success; */
/* 		case SINGLE: */
/* 			single_to_multi_append(tree, elem); */
/* 			goto success; */
/* 		} */

/* 		if (likely(!tree->pre[0])) { */
/* 			tree->suf[tree->sufcnt - 1] = elem; */
/* 			goto success; */
/* 		} */

/* 	        b = palloc(b); */
/* 	        next = LIST_NEXT(tree); */

/* 		if (unlikely(!next)) */
/* 			pcheck_c((next = ftree_new()), 0, free(b)); */

/* 		tree->sufcnt = FTREE_BS / 2; */
/* 		append_set(tree->suf, b, elem); */
/* 	} */

/* success: */
/* 	++tree->precnt; */
/* 	return 1; */
/* } */
