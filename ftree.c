#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <stdio.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "list.h"
#include "fbnch.h"
#include "ftree.h"

/* #define ARR_END (FTREE_BS - 1) */

enum { EMPTY = 0, SINGLE = 1 };

#define NEITHER (SINGLE + 1)

static inline int
reset_ano(Nit_fnat nat, union nit_ano *ano, void *extra)
{
	return nat(FT_RESET, ano, NULL, extra);
}

static Nit_ftree *
fmem_get_tree(Nit_fmem *mem)
{
	Nit_ftree *tree = mem->trees;

	if (!tree)
	        tree = palloc(tree);

	return tree;
}

static void
fmem_put_tree(Nit_fmem *mem, Nit_ftree *tree)
{
	LIST_CONS(tree, mem->trees);
	mem->trees = tree;
}

Nit_ftree *
ftree_new(Nit_fdat *dat)
{
	Nit_ftree *tree = fmem_get_tree(dat->mem);

	pcheck(tree, NULL);
	memset(tree, 0, sizeof(*tree));
	tree->refs = 1;

	return tree;
}

static int
precnt(const Nit_ftree *tree)
{
	if (!tree->pre)
		return 0;

	return tree->pre->cnt;
}

static int
sufcnt(const Nit_ftree *tree)
{
	if (!tree->suf)
		return 0;

	return tree->suf->cnt;
}

static void
incpre(Nit_ftree *tree)
{
	if (tree->pre)
		++tree->pre->refs;
}

static void
incsuf(Nit_ftree *tree)
{
	if (tree->suf)
		++tree->suf->refs;
}

static void
incsides(Nit_ftree *tree)
{
	incpre(tree);
	incsuf(tree);
}

Nit_ftree *
ftree_copy(Nit_fdat *dat, Nit_ftree *tree)
{
	Nit_ftree *copy = fmem_get_tree(dat->mem);
	Nit_ftree *next = LIST_NEXT(tree);

	pcheck(copy, NULL);
	memcpy(copy, tree, sizeof(*tree));
	copy->refs = 1;

	if (!dat->nat(FT_COPY, &tree->ano, NULL, dat->ext)) {
		free(copy);
		return NULL;
	}

	if (next)
		++next->refs;

	incsides(tree);

	return copy;
}

void
ftree_reduce(Nit_fdat *dat, Nit_ftree *tree, int depth)
{
	if (unlikely(!tree))
		return;

	if (--tree->refs)
		return;

	ftree_reduce(dat, LIST_NEXT(tree), depth + 1);
	fbnch_reduce(dat, tree->pre, depth);
	fbnch_reduce(dat, tree->suf, depth);
	dat->nat(FT_DEC, &tree->ano, NULL, dat->ext);
	free(tree);
}

static Nit_ftree *
mut_usable(Nit_fdat *dat, Nit_ftree *tree, int depth)
{
	Nit_ftree *copy;

	if (tree->refs == 1)
		return tree;

	pcheck(copy = ftree_copy(dat, tree), NULL);
	ftree_reduce(dat, tree, depth);

	return copy;
}

static Nit_ftree *
mut_next(Nit_fdat *dat, Nit_ftree *tree, int depth)
{
	Nit_ftree *next = LIST_NEXT(tree);
	Nit_ftree *tmp = next ?
		mut_usable(dat, next, depth + 1) : ftree_new(dat);

	pcheck(tmp, NULL);

	if (unlikely(next != tmp))
		LIST_CONS(tree, tmp);

	return tmp;
}

static inline int
ftree_type(const Nit_ftree *tree)
{
	return precnt(tree) + sufcnt(tree);
}

static inline void *
single_elem_get(const Nit_ftree *tree)
{
	return fbnch_first(tree->pre);
}

static inline int
single_elem_set(Nit_fdat *dat, Nit_ftree *tree, void *elem, int depth)
{
	if (!tree->pre)
		pcheck(tree->pre = fbnch_new(dat, elem, depth), 0);

	tree->pre->elems[0] = elem;

	return 1;
}

static inline int
single_to_multi_prepend(Nit_fdat *dat, Nit_ftree *tree, void *elem, int depth)
{
	tree->suf = tree->pre;

	return single_elem_set(dat, tree, elem, depth);
}

static inline int
single_to_multi_append(Nit_fdat *dat, Nit_ftree *tree, void *elem, int depth)
{
	pcheck(tree->suf = fbnch_new(dat, elem, depth), 0);

	return 1;
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

	return fbnch_first(tree->pre);
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

	return fbnch_last(tree->suf);
}

static int
ftree_set_ano(Nit_fdat *dat, Nit_ftree *tree)
{
	reset_ano(dat->nat, &tree->ano, dat->ext);

	if (tree->pre &&
	    !dat->nat(FT_MES_ANO, &tree->ano, &tree->pre->ano, dat->ext))
			return 0;

	if (LIST_NEXT(tree) &&
	    !dat->nat(FT_MES_ANO, &tree->ano,
		     &((Nit_ftree *) LIST_NEXT(tree))->ano, dat->ext))
			return 0;

	if (tree->suf &&
	    !dat->nat(FT_MES_ANO, &tree->ano, &tree->suf->ano, dat->ext))
			return 0;
	return 1;
}

static int
tree_ano_set(Nit_fdat *dat, Nit_ftree *tree, void *elem, int depth)
{
	if (!depth)
		return dat->nat(FT_MES_DAT, &tree->ano, elem, dat->ext);

	return dat->nat(FT_MES_ANO, &tree->ano,
		       &((Nit_fbnch *) elem)->ano, dat->ext);
}

int
ftree_prepend(Nit_fdat *dat, Nit_ftree *tree, void *elem, int depth)
{
	Nit_fbnch *tmp;

	for (;; ++depth) {
		if (!tree_ano_set(dat, tree, elem, depth))
			return 0;

		switch (ftree_type(tree)) {
		case EMPTY:
			single_elem_set(dat, tree, elem, depth);
			return 1;
		case SINGLE:
			single_to_multi_prepend(dat, tree, elem, depth);
			return 1;
		}

		if (precnt(tree) < dat->max_elems)
			return fbnch_prepend(dat, &tree->pre, elem, depth);

		tmp = tree->pre;

		if (!(tree->pre = fbnch_new(dat, elem, depth)))
			return 0;

		elem = tmp;

		if (!(tree = mut_next(dat, tree, depth)))
			return 0;
	}

	return 0;
}

int
ftree_append(Nit_fdat *dat, Nit_ftree *tree, void *elem, int depth)
{
	Nit_fbnch *tmp;

        for (;; ++depth) {
		if (!tree_ano_set(dat, tree, elem, depth))
			return 0;

		switch (ftree_type(tree)) {
		case EMPTY:
			single_elem_set(dat, tree, elem, depth);
			return 1;
		case SINGLE:
			single_to_multi_append(dat, tree, elem, depth);
			return 1;
		}

		if (sufcnt(tree) < dat->max_elems)
			return fbnch_append(dat, &tree->suf, elem, depth);

		tmp = tree->suf;

		if (!(tree->suf = fbnch_new(dat, elem, depth)))
			return 0;

		elem = tmp;

		if (!(tree = mut_next(dat, tree, depth)))
			return 0;
	}

	return 0;
}

void *
ftree_pop(Nit_fdat *dat, Nit_ftree *tree, int depth)
{
	void *val;
	Nit_ftree *next;

	/* [] [X] */
	switch (ftree_type(tree)) {
		case EMPTY:
			return NULL;
		case SINGLE:
			reset_ano(dat->nat, &tree->ano, dat->ext);
		        return fbnch_pop(dat, &tree->pre, depth);
	}

	val = fbnch_pop(dat, &tree->pre, depth);

	if (tree->pre)
		goto end;

	if (!LIST_NEXT(tree)) {
		printf("\nwot?!\n");
		tree->pre = tree->suf;
		tree->suf = NULL;

		if (precnt(tree) > 1)
			tree->suf = fbnch_new(dat,
					     fbnch_rpop(dat, &tree->pre, depth),
					     depth);

		goto end;
	}

	if (!(next = mut_next(dat, tree, depth)))
		return NULL;

        tree->pre = ftree_pop(dat, next, depth + 1);

	if (ftree_type(next) == EMPTY) {
		ftree_reduce(dat, next, depth + 1);
		LIST_CONS(tree, NULL);
	}

end:
	if (!ftree_set_ano(dat, tree))
		return NULL;

	return val;
}

void *
ftree_rpop(Nit_fdat *dat, Nit_ftree *tree, int depth)
{
	void *val;
	Nit_ftree *next;

	switch (ftree_type(tree)) {
		case EMPTY:
			return NULL;
		case SINGLE:
			reset_ano(dat->nat, &tree->ano, dat->ext);
			return fbnch_pop(dat, &tree->pre, depth);
	}

	val = fbnch_rpop(dat, &tree->suf, depth);

	if (tree->suf)
		goto end;

	if (!LIST_NEXT(tree)) {
		printf("\nwot?!\n");

		if (precnt(tree) == 1)
			goto end;

		tree->suf = tree->pre;
		tree->pre = fbnch_new(dat,
				     fbnch_pop(dat, &tree->pre, depth), depth);
		goto end;
	}

	if (!(next = mut_next(dat, tree, depth)))
		return NULL;

	tree->pre = ftree_rpop(dat, next, depth + 1);

	if (ftree_type(next) == EMPTY) {
		ftree_reduce(dat, next, depth + 1);
		LIST_CONS(tree, NULL);
	}

end:
	if (!ftree_set_ano(dat, tree))
		return NULL;

	return val;
}

/* static Nit_ftree * */
/* concat_top(Nit_ftree *left, Nit_ftree *right, int depth) */
/* { */
/* 	Nit_ftree *tree = ftree_new(); */

/* 	pcheck(tree, NULL); */
/* 	memcpy(tree->pre, left->pre, sizeof(left->pre)); */
/* 	tree->precnt = left->precnt; */
/* 	memcpy(tree->suf, right->suf, sizeof(right->suf)); */
/* 	tree->sufcnt = right->sufcnt; */

/* 	if (depth > 0) */
/* 		ftree_inc_bnch_refs(tree); */

/* 	return tree; */
/* } */

/* static void */
/* nodes_set_arr(void **arr, Nit_fbnch **mid, int elems, */
/* 	      int lsufcnt, void **lsuf, */
/* 	      int rprecnt, void **rpre, int depth) */
/* { */
/* 	int i; */

/* 	if (depth > 0) { */
/* 		for (i = 0; i < rprecnt; ++i) */
/* 			++((Nit_fbnch *) rpre[i])->refs; */

/* 		for (i = 0; i < lsufcnt; ++i) */
/* 			++((Nit_fbnch *) lsuf[i])->refs; */
/* 	} */

/* 	memcpy(arr, lsuf, lsufcnt * sizeof(*lsuf)); */
/* 	memcpy(arr + lsufcnt, mid, elems * sizeof(*mid)); */
/* 	memcpy(arr + lsufcnt + elems, rpre, rprecnt * sizeof(*rpre)); */

/* } */

/* static int */
/* nodes(Nit_fnat nat, Nit_fbnch **mid, int *elems, */
/*       Nit_ftree *left, Nit_ftree *right, int depth, void *extra) */
/* { */
/* 	int new_elems = 0; */
/* 	int cnt = *elems + left->sufcnt + right->precnt; */
/* 	void *arr[3 * FTREE_BS]; */
/* 	void **arr_ptr = arr; */

/* 	nodes_set_arr(arr, mid, *elems, left->sufcnt, left->suf, right->precnt, */
/* 		      right->pre, depth); */

/* 	for (; cnt >= FTREE_BS; */
/* 	     cnt -= FTREE_BS, arr_ptr += FTREE_BS, ++new_elems) */
/* 	        if (!(mid[new_elems] = */
/* 		      fbnch_new(nat, arr_ptr, FTREE_BS, depth, extra))) */
/* 			goto error; */

/* 	if (cnt && */
/* 	    !(mid[new_elems++] = fbnch_new(nat, arr_ptr, cnt, depth, extra))) */
/* 		goto error; */

/* 	*elems = new_elems; */

/* 	return 1; */

/* error: */
/* 	while (new_elems--) */
/* 		free(arr[new_elems]); */

/* 	return 0; */
/* } */

/* static int */
/* concat_mid_append(Nit_fnat nat, Nit_ftree *layer, Nit_fbnch **mid, */
/* 		  int elems, int depth, void *extra) */
/* { */
/* 	while (elems--) */
/* 		if (!ftree_append(nat, layer, mid[elems], depth, extra)) */
/* 			return 0; */
/* 	return 1; */
/* } */

/* static int */
/* concat_mid_prepend(Nit_fnat nat, Nit_ftree *layer, Nit_fbnch **mid, */
/* 		   int elems, int depth, void *extra) */
/* { */
/* 	int i = 0; */

/* 	while (i < elems) */
/* 		if (!ftree_prepend(nat, layer, mid[i++], depth, extra)) */
/* 			return 0; */
/* 	return 1; */
/* } */

/* static inline Nit_ftree * */
/* concat_error(Nit_fnat nat, Nit_ftree *top, int depth, void *extra) */
/* { */
/* 	ftree_reduce(nat, top, depth, extra); */

/* 	return NULL; */
/* } */


/* static int */
/* concat_finals(Nit_fnat nat, Nit_ftree **layer, */
/* 	      Nit_ftree *left, Nit_ftree *right, Nit_fbnch **mid, */
/* 	      int elems, int depth, void *extra) */
/* { */
/* 	enum { RIGHT, LEFT, NULLED = EMPTY - 1 }; */

/* 	int side; */
/*         int single = 0; */
/*         int ltype = (left)  ? ftree_type(left)  : NULLED; */
/*         int rtype = (right) ? ftree_type(right) : NULLED; */

/* 	if (ltype > SINGLE && rtype > SINGLE) */
/* 		return 0; */

/* 	if (ltype <= EMPTY && rtype <= EMPTY) */
/* 		return 1; */

/* 	/\* One of them is single or empty *\/ */
/* 	if (ltype > SINGLE) { */
/* 		side = LEFT; */

/* 		if (rtype == SINGLE) */
/* 			single = 1; */
/* 	} else if (ltype == SINGLE) { */
/* 		if (rtype >= SINGLE) { */
/* 			side = RIGHT; */
/* 			single = 1; */
/* 		} else { */
/* 			side = LEFT; */
/* 		} */
/* 	} else { */
/* 		side = RIGHT; */
/* 	} */

/* 	if (side == RIGHT) { */
/* 		pcheck((*layer = ftree_copy(nat, right, depth, extra)), -1); */

/* 		if (single) */
/* 			ftree_prepend(nat, *layer, */
/* 				      single_elem_get(left), depth, extra); */

/* 		if (!concat_mid_append(nat, *layer, mid, elems, depth, extra)) */
/* 			return -1; */
/* 	} else { */
/* 		pcheck((*layer = ftree_copy(nat, left, depth, extra)), -1); */

/* 		if (single) */
/* 			ftree_append(nat, *layer, */
/* 				     single_elem_get(right), depth, extra); */

/* 		if (!concat_mid_prepend(nat, *layer, mid, elems, depth, extra)) */
/* 			return -1; */
/* 	} */

/* 	return 1; */
/* } */

/* Nit_ftree * */
/* ftree_concat(Nit_fnat nat, Nit_ftree *left, Nit_ftree *right, void *extra) */
/* { */
/* 	Nit_ftree *top = NULL; */
/* 	Nit_ftree **layer = &top; */
/* 	Nit_fbnch *mid[3]; */
/* 	int depth = 0; */
/* 	int elems = 0; */

/* 	for (; right || left; */
/* 	     (LIST_INC(right), LIST_INC(left), */
/* 	      layer = NEXT_REF(*layer), ++depth)) { */

/* 		switch (concat_finals(nat, layer, left, right, mid, */
/* 				      elems, depth, extra)) { */
/* 		case -1: */
/* 			return concat_error(nat, top, depth, extra); */
/* 		case 0: */
/* 			break; */
/* 		case 1: */
/* 			return top; */
/* 		} */


/* 		pcheck((*layer = concat_top(left, right, depth)), NULL); */


/* 		if (!nodes(nat, mid, &elems, left, right, depth, extra)) */
/* 		        return concat_error(nat, top, depth, extra); */
/* 	} */

/* 	if (elems) { */
/* 		pcheck(*layer = ftree_new(), NULL); */

/* 		if (!concat_mid_prepend(nat, *layer, mid, elems, depth, extra)) */
/* 			return concat_error(nat, top, depth, extra); */
/* 	} */

/* 	return top; */
/* } */

/* #define FBNCH(PTR) ((Nit_fbnch *) (PTR)) */

/* static void * */
/* fbnch_search(Nit_fsrch srch, Nit_fbnch *bnch, void *acc, */
/* 	     int depth, void *extra) */
/* { */
/* 	int i; */

/* 	for (;; bnch = bnch->elems[i], --depth) { */
/* 		if (!depth) { */
/* 			for (i = 0; i < bnch->cnt; ++i) */
/* 				if (srch(FT_DAT, acc, bnch->elems[i], extra)) */
/* 					return bnch->elems[i]; */

/* 			/\* fprintf(stderr, "Your ftree search makes no sense!\n"); *\/ */

/* 			return NULL; */
/* 		} */

/* 		for (i = 0; i < bnch->cnt; ++i) */
/* 			if (srch(FT_ANO, acc, */
/* 				 &FBNCH(bnch->elems[i])->ano, extra)) */
/* 				break; */
/* 	} */

/* 	return NULL; */
/* } */

/* void * */
/* ftree_search(Nit_fsrch srch, Nit_ftree *tree, void *acc, void *extra) */
/* { */
/* 	int depth = 0; */

/* 	if (!srch(FT_ANO, acc, &tree->ano, extra)) */
/* 		return NULL; */

/* 	while (1) { */
/* 		int i; */

/* 		if (!depth) { */
/* 			i = tree->precnt; */

/* 			while (i--) */
/* 				if (srch(FT_DAT, acc, tree->pre[i], extra)) */
/* 					return tree->pre[i]; */

/* 			if (LIST_NEXT(tree)) */
/* 				if (srch(FT_ANO, acc, */
/* 					 &LIST_NEXT(tree)->ano, extra)) { */
/* 					++depth; */
/* 					continue; */
/* 				} */

/* 			for (i = 0; i < tree->sufcnt; ++i) */
/* 				if (srch(FT_DAT, acc, tree->suf[i], extra)) */
/* 					return tree->suf[i]; */
/* 		} */

/* 		i = tree->precnt; */

/* 		while (i--) */
/* 			if (srch(FT_ANO, acc, &FBNCH(tree->pre[i])->ano, extra)) */
/* 				return fbnch_search(srch, tree->pre[i], */
/* 						    acc, depth - 1, extra); */

/* 		if (LIST_NEXT(tree)) */
/* 			if (srch(FT_ANO, acc, &LIST_NEXT(tree)->ano, extra)) { */
/* 				++depth; */
/* 				continue; */
/* 			} */

/* 		for (i = 0; i < tree->sufcnt; ++i) */
/* 			if (srch(FT_ANO, acc, &FBNCH(tree->suf[i])->ano, extra)) */
/* 				return fbnch_search(srch, tree->suf[i], */
/* 						    acc, depth - 1, extra); */
/* 	} */
/* } */
