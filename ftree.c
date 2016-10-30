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
	void *elems[HALF + 1];
} Branch;

enum ftree_type { EMPTY=0, SINGLE=1 };

Nit_ftree *
ftree_new(void)
{
	Nit_ftree *tree = palloc(tree);

	pcheck(tree, NULL);
        memset(tree, 0, sizeof(*tree));

	return tree;
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

void *
ftree_first(const Nit_ftree *tree)
{
	switch (ftree_type(tree)) {
	case EMPTY:
		return NULL;
	case SINGLE:
		return single_elem_get(tree);
	}

	return tree->pre[tree->precnt - 1];
}

void *
ftree_last(const Nit_ftree *tree)
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

int
ftree_prepend(Nit_ftree *tree, void *elem)
{
	Branch *b;
	Nit_ftree *next;

	for (;; tree = next, elem = b) {
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
			tree->pre[tree->precnt] = elem;
			goto success;
		}

	        b = palloc(b);
	        next = LIST_NEXT(tree);

		if (unlikely(!next)) {
			pcheck_c((next = ftree_new()), 0, free(b));
			LIST_CONS(tree, next);
		}

		tree->precnt = FTREE_BS / 2;
		digit_set(tree->pre, b, elem);
	}

success:
	++tree->precnt;
	return 1;
}

int
ftree_append(Nit_ftree *tree, void *elem)
{
	Branch *b;
	Nit_ftree *next;

	for (;; tree = next, elem = b) {
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
			tree->suf[tree->sufcnt] = elem;
			goto success;
		}

	        b = palloc(b);
	        next = LIST_NEXT(tree);

		if (unlikely(!next)) {
			pcheck_c((next = ftree_new()), 0, free(b));
			LIST_CONS(tree, next);
		}

		tree->sufcnt = FTREE_BS / 2;
		digit_set(tree->suf, b, elem);
	}

success:
	++tree->sufcnt;
	return 1;
}

void *
ftree_pop(Nit_ftree *tree)
{
	void *val;
	Branch *b;
	Nit_ftree *next;

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
	if (likely(tree->precnt > 1)) {
		val = tree->pre[tree->precnt - 1];
		tree->pre[tree->precnt - 1] = NULL;
		--tree->precnt;
		return val;
	}

	val = tree->pre[0];
	next = LIST_NEXT(tree);

	/* [A___] + [[BCD] [EFG] [HIJ]] -> [BCD_] */
        if (likely(next && (b = ftree_pop(next)))) {
		if (unlikely(ftree_type(next) == EMPTY)) {
			free(next);
			LIST_CONS(tree, NULL);
		}


		tree->precnt = ARRAY_UNITS(b->elems);
		memcpy(tree->pre, b->elems, sizeof(b->elems));
		free(b);
		return val;
	}

	/* Suffix to prefix */
	if (likely(tree->sufcnt != 1)) {
		memcpy(tree->pre, tree->suf, sizeof(val) * (tree->sufcnt - 1));
		tree->suf[0] = tree->suf[tree->sufcnt - 1];
		memset(tree->suf + 1, 0, sizeof(val) * (tree->sufcnt - 1));
		tree->suf[tree->sufcnt - 1] = NULL;
		tree->precnt = tree->sufcnt;
		tree->sufcnt = 1;
		return val;
	}

	/* Makes single */
	tree->sufcnt = 0;
	single_elem_set(tree, tree->suf[0]);
	tree->suf[0] = NULL;
	return val;
}
