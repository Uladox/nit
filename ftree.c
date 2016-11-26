#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "list.h"
#include "ftree.h"

#define ARR_END (FTREE_BS - 1)

enum { EMPTY = 0, SINGLE = 1 };

#define NEITHER (SINGLE + 1)

static inline int
reset_ano(Nit_fnat nat, union nit_ano *ano, void *extra)
{
	return nat(FT_RESET, ano, NULL, extra);
}

static void
bnch_inc_refs(Nit_fbnch *bnch)
{
	Nit_fbnch **bnchs = (Nit_fbnch **) bnch->elems;
	int i = 0;

	for (; i < bnch->cnt; ++i)
		++bnchs[i]->refs;
}

static Nit_fbnch *
fbnch_new(Nit_fnat nat, void **arr, int cnt, int depth, void *extra)
{
	size_t elems = cnt * sizeof(void *);
	Nit_fbnch *bnch = malloc(sizeof(*bnch) + elems);
	enum nit_ftop op = (depth > 0) ? FT_MES_ANO : FT_MES_DAT;
	int i = 0;

	pcheck(bnch, NULL);
	bnch->ano.ptr = NULL;
	bnch->refs = 1;
	bnch->cnt = cnt;
	memcpy(bnch->elems, arr, elems);

	for (; i < cnt; ++i)
		if (!nat(op, &bnch->ano, arr[i], extra)) {
			free(bnch);
			return NULL;
		}

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
ftree_copy(Nit_fnat nat, Nit_ftree *tree, int depth, void *extra)
{
	Nit_ftree *copy = palloc(copy);
	Nit_ftree *next = LIST_NEXT(tree);

	pcheck(copy, NULL);
	memcpy(copy, tree, sizeof(*tree));
	copy->refs = 1;

	if (!nat(FT_COPY, &tree->ano, NULL, extra)) {
		free(copy);
		return NULL;
	}

	if (next)
		++next->refs;

	if (depth > 0)
		ftree_inc_bnch_refs(copy);

	return copy;
}

static void
reduce_branches(Nit_fnat nat, Nit_fbnch *bnch, int depth, void *extra)
{
	int i = 0;

	if (!depth || !bnch || --bnch->refs > 0)
		return;

	if (depth > 1)
		for (; i < bnch->cnt; ++i)
			reduce_branches(nat, bnch->elems[i], depth - 1, extra);

	nat(FT_DEC, &bnch->ano, NULL, extra);
	free(bnch);
}

void
ftree_reduce(Nit_fnat nat, Nit_ftree *tree, int depth, void *extra)
{
	int i = 0;

	if (unlikely(!tree))
		return;

	if (--tree->refs)
		return;

	ftree_reduce(nat, LIST_NEXT(tree), depth + 1, extra);

	for (; i < tree->precnt; ++i)
		reduce_branches(nat, tree->pre[i], depth, extra);

	i = 0;

	for (; i < tree->sufcnt; ++i)
		reduce_branches(nat, tree->suf[i], depth, extra);

	nat(FT_DEC, &tree->ano, NULL, extra);
	free(tree);
}

static Nit_ftree *
mut_usable(Nit_fnat nat, Nit_ftree *tree, int depth, void *extra)
{
	Nit_ftree *copy;

	if (tree->refs == 1)
		return tree;

	pcheck(copy = ftree_copy(nat, tree, depth, extra), NULL);
	ftree_reduce(nat, tree, depth, extra);

	return copy;
}

static Nit_ftree *
mut_next(Nit_fnat nat, Nit_ftree *tree, int depth, void *extra)
{
	Nit_ftree *next = LIST_NEXT(tree);
	Nit_ftree *tmp = next ? mut_usable(nat, next, depth + 1, extra) :
		ftree_new();

	pcheck(tmp, NULL);

	if (unlikely(next != tmp))
		LIST_CONS(tree, tmp);

	return tmp;
}

static inline int
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
next_tree_add(Nit_fnat nat, Nit_ftree **tree, uint8_t *fixcnt, void **fix,
	      void **elem, int depth, void *extra)
{
	Nit_fbnch *bnch = fbnch_new(nat, fix, FTREE_BS, depth, extra);
	Nit_ftree *next = mut_next(nat, *tree, depth, extra);

	pcheck_c(next, 0, free(bnch));

	*fixcnt = 1;
	fix[0] = *elem;
	memset(fix + 1, 0, sizeof(*elem) * (FTREE_BS - 1));

	*tree = next;
	*elem = bnch;

	return 1;
}

static int
mes_fix_ano(Nit_fnat nat, union nit_ano *ano, uint8_t fixcnt, void **fix,
	    int depth, void *extra)
{
	int i = 0;

	if (!depth) {
		for (; i < fixcnt; ++i)
			if (!nat(FT_MES_DAT, ano, fix[i], extra))
				return 0;

		return 1;
	}

	for (; i < fixcnt; ++i)
		if (!nat(FT_MES_ANO, ano, &((Nit_fbnch *) fix[i])->ano, extra))
			return 0;

	return 1;
}

static int
ftree_set_ano(Nit_fnat nat, Nit_ftree *tree, int depth, void *extra)
{
	reset_ano(nat, &tree->ano, extra);

	if (!mes_fix_ano(nat, &tree->ano, tree->precnt, tree->pre, depth, extra))
		return 0;

	if (LIST_NEXT(tree))
		if (!nat(FT_MES_ANO, &tree->ano,
			 &((Nit_ftree *) LIST_NEXT(tree))->ano, extra))
			return 0;

	if (!mes_fix_ano(nat, &tree->ano, tree->sufcnt, tree->suf, depth, extra))
		return 0;

	return 1;
}

int
tree_ano_set(Nit_fnat nat, Nit_ftree *tree, void *elem, int depth, void *extra)
{
	if (!depth)
		return nat(FT_MES_DAT, &tree->ano, elem, extra);

	return nat(FT_MES_ANO, &tree->ano, &((Nit_fbnch *) elem)->ano, extra);
}

int
ftree_prepend(Nit_fnat nat, Nit_ftree *tree, void *elem, int depth, void *extra)
{

	for (;; ++depth) {
		if (!tree_ano_set(nat, tree, elem, depth, extra))
			return 0;

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

		if (!next_tree_add(nat, &tree, &tree->precnt,
				   tree->pre, &elem, depth, extra))
			return 0;

		/* if (!ftree_set_ano(nat, tree, depth, extra)) */
		/* 	return 0; */
	}

	return 0;
}

int
ftree_append(Nit_fnat nat, Nit_ftree *tree, void *elem, int depth, void *extra)
{
        for (;; ++depth) {
		if (!tree_ano_set(nat, tree, elem, depth, extra))
			return 0;

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

		if (!next_tree_add(nat, &tree, &tree->sufcnt,
				   tree->suf, &elem, depth, extra))
			return 0;

		/* if (!ftree_set_ano(nat, tree, depth, extra)) */
		/* 	return 0; */
	}

	return 0;
}

static inline int
promote_node(Nit_fnat nat, Nit_ftree *tree,
	     uint8_t *fixcnt, void **fix, int depth, void *extra)
{
	Nit_ftree *next = LIST_NEXT(tree);
	Nit_fbnch *bnch;

	if (unlikely(!next))
		return 0;

	next = mut_next(nat, tree, depth, extra);
	pcheck(next, 0);

	if (!(bnch = ftree_pop(nat, next, depth + 1, extra)))
		return 0;

	if (unlikely(ftree_type(next) == EMPTY)) {
		ftree_reduce(nat, next, depth + 1, extra);
		LIST_CONS(tree, NULL);
	}

	*fixcnt = bnch->cnt;
	memcpy(fix, bnch->elems, bnch->cnt * sizeof(void *));

	if (bnch->refs-- == 1) {
		nat(FT_DEC, &bnch->ano, NULL, extra);
		free(bnch);
	} else if (depth > 0) {
		bnch_inc_refs(bnch);
	}

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
ftree_pop(Nit_fnat nat, Nit_ftree *tree, int depth, void *extra)
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
			reset_ano(nat, &tree->ano, extra);
			return val;
	}

	/* [ABCD] -> [ABC_] */
	if (likely(tree->precnt > 1)) {
		val = take_val_off(&tree->precnt, tree->pre);
		goto end;
	}

	val = tree->pre[0];

	/* [A___] + [[BCD] [EFG] [HIJ]] -> [BCD_] */
	if (promote_node(nat, tree, &tree->precnt, tree->pre, depth, extra))
	        goto end;

	/* Suffix to prefix */
	fix_to_fix(tree, &tree->precnt, tree->pre, &tree->sufcnt, tree->suf);

end:
	ftree_set_ano(nat, tree, depth, extra);

	return val;
}

void *
ftree_rpop(Nit_fnat nat, Nit_ftree *tree, int depth, void *extra)
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
			reset_ano(nat, &tree->ano, extra);
			return val;
	}

	/* [ABCD] -> [ABC_] */
	if (likely(tree->sufcnt > 1)) {
		val = take_val_off(&tree->sufcnt, tree->suf);
		goto end;
	}

	val = tree->suf[0];

	/* [A___] + [[BCD] [EFG] [HIJ]] -> [BCD_] */
	if (promote_node(nat, tree, &tree->sufcnt, tree->suf, depth, extra))
	        goto end;

	/* Suffix to prefix */
	fix_to_fix(tree, &tree->sufcnt, tree->suf, &tree->precnt, tree->pre);

end:
	if (!ftree_set_ano(nat, tree, depth, extra))
		return NULL;

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

static void
nodes_set_arr(void **arr, Nit_fbnch **mid, int elems,
	      int lsufcnt, void **lsuf,
	      int rprecnt, void **rpre, int depth)
{
	int i;

	if (depth > 0) {
		for (i = 0; i < rprecnt; ++i)
			++((Nit_fbnch *) rpre[i])->refs;

		for (i = 0; i < lsufcnt; ++i)
			++((Nit_fbnch *) lsuf[i])->refs;
	}

	memcpy(arr, lsuf, lsufcnt * sizeof(*lsuf));
	memcpy(arr + lsufcnt, mid, elems * sizeof(*mid));
	memcpy(arr + lsufcnt + elems, rpre, rprecnt * sizeof(*rpre));

}

static int
nodes(Nit_fnat nat, Nit_fbnch **mid, int *elems,
      Nit_ftree *left, Nit_ftree *right, int depth, void *extra)
{
	int new_elems = 0;
	int cnt = *elems + left->sufcnt + right->precnt;
	void *arr[3 * FTREE_BS];
	void **arr_ptr = arr;

	nodes_set_arr(arr, mid, *elems, left->sufcnt, left->suf, right->precnt,
		      right->pre, depth);

	for (; cnt >= FTREE_BS;
	     cnt -= FTREE_BS, arr_ptr += FTREE_BS, ++new_elems)
	        if (!(mid[new_elems] =
		      fbnch_new(nat, arr_ptr, FTREE_BS, depth, extra)))
			goto error;

	if (cnt &&
	    !(mid[new_elems++] = fbnch_new(nat, arr_ptr, cnt, depth, extra)))
		goto error;

	*elems = new_elems;

	return 1;

error:
	while (new_elems--)
		free(arr[new_elems]);

	return 0;
}

static int
concat_mid_append(Nit_fnat nat, Nit_ftree *layer, Nit_fbnch **mid,
		  int elems, int depth, void *extra)
{
	while (elems--)
		if (!ftree_append(nat, layer, mid[elems], depth, extra))
			return 0;
	return 1;
}

static int
concat_mid_prepend(Nit_fnat nat, Nit_ftree *layer, Nit_fbnch **mid,
		   int elems, int depth, void *extra)
{
	int i = 0;

	while (i < elems)
		if (!ftree_prepend(nat, layer, mid[i++], depth, extra))
			return 0;
	return 1;
}

static inline Nit_ftree *
concat_error(Nit_fnat nat, Nit_ftree *top, int depth, void *extra)
{
	ftree_reduce(nat, top, depth, extra);

	return NULL;
}


static int
concat_finals(Nit_fnat nat, Nit_ftree **layer,
	      Nit_ftree *left, Nit_ftree *right, Nit_fbnch **mid,
	      int elems, int depth, void *extra)
{
	enum { RIGHT, LEFT, NULLED = EMPTY - 1 };

	int side;
        int single = 0;
        int ltype = (left)  ? ftree_type(left)  : NULLED;
        int rtype = (right) ? ftree_type(right) : NULLED;

	if (ltype > SINGLE && rtype > SINGLE)
		return 0;

	if (ltype <= EMPTY && rtype <= EMPTY)
		return 1;

	/* One of them is single or empty */
	if (ltype > SINGLE) {
		side = LEFT;

		if (rtype == SINGLE)
			single = 1;
	} else if (ltype == SINGLE) {
		if (rtype >= SINGLE) {
			side = RIGHT;
			single = 1;
		} else {
			side = LEFT;
		}
	} else {
		side = RIGHT;
	}

	if (side == RIGHT) {
		pcheck((*layer = ftree_copy(nat, right, depth, extra)), -1);

		if (single)
			ftree_prepend(nat, *layer,
				      single_elem_get(left), depth, extra);

		if (!concat_mid_append(nat, *layer, mid, elems, depth, extra))
			return -1;
	} else {
		pcheck((*layer = ftree_copy(nat, left, depth, extra)), -1);

		if (single)
			ftree_append(nat, *layer,
				     single_elem_get(right), depth, extra);

		if (!concat_mid_prepend(nat, *layer, mid, elems, depth, extra))
			return -1;
	}

	return 1;
}

Nit_ftree *
ftree_concat(Nit_fnat nat, Nit_ftree *left, Nit_ftree *right, void *extra)
{
	Nit_ftree *top = NULL;
	Nit_ftree **layer = &top;
	Nit_fbnch *mid[3];
	int depth = 0;
	int elems = 0;

	for (; right || left;
	     (LIST_INC(right), LIST_INC(left),
	      layer = NEXT_REF(*layer), ++depth)) {

		switch (concat_finals(nat, layer, left, right, mid,
				      elems, depth, extra)) {
		case -1:
			return concat_error(nat, top, depth, extra);
		case 0:
			break;
		case 1:
			return top;
		}


		pcheck((*layer = concat_top(left, right, depth)), NULL);


		if (!nodes(nat, mid, &elems, left, right, depth, extra))
		        return concat_error(nat, top, depth, extra);
	}

	if (elems) {
		pcheck(*layer = ftree_new(), NULL);

		if (!concat_mid_prepend(nat, *layer, mid, elems, depth, extra))
			return concat_error(nat, top, depth, extra);
	}

	return top;
}

#define FBNCH(PTR) ((Nit_fbnch *) (PTR))

static void *
fbnch_search(Nit_fsrch srch, Nit_fbnch *bnch, void *acc,
	     int depth, void *extra)
{
	int i;

	for (;; bnch = bnch->elems[i], --depth) {
		if (!depth) {
			for (i = 0; i < bnch->cnt; ++i)
				if (srch(FT_DAT, acc, bnch->elems[i], extra))
					return bnch->elems[i];

			/* fprintf(stderr, "Your ftree search makes no sense!\n"); */

			return NULL;
		}

		for (i = 0; i < bnch->cnt; ++i)
			if (srch(FT_ANO, acc,
				 &FBNCH(bnch->elems[i])->ano, extra))
				break;
	}

	return NULL;
}

void *
ftree_search(Nit_fsrch srch, Nit_ftree *tree, void *acc, void *extra)
{
	int depth = 0;

	if (!srch(FT_ANO, acc, &tree->ano, extra))
		return NULL;

	while (1) {
		int i;

		if (!depth) {
			i = tree->precnt;

			while (i--)
				if (srch(FT_DAT, acc, tree->pre[i], extra))
					return tree->pre[i];

			if (LIST_NEXT(tree))
				if (srch(FT_ANO, acc,
					 &LIST_NEXT(tree)->ano, extra)) {
					++depth;
					continue;
				}

			for (i = 0; i < tree->sufcnt; ++i)
				if (srch(FT_DAT, acc, tree->suf[i], extra))
					return tree->suf[i];
		}

		i = tree->precnt;

		while (i--)
			if (srch(FT_ANO, acc, &FBNCH(tree->pre[i])->ano, extra))
				return fbnch_search(srch, tree->pre[i],
						    acc, depth - 1, extra);

		if (LIST_NEXT(tree))
			if (srch(FT_ANO, acc, &LIST_NEXT(tree)->ano, extra)) {
				++depth;
				continue;
			}

		for (i = 0; i < tree->sufcnt; ++i)
			if (srch(FT_ANO, acc, &FBNCH(tree->suf[i])->ano, extra))
				return fbnch_search(srch, tree->suf[i],
						    acc, depth - 1, extra);
	}
}
