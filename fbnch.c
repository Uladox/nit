#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "fbnch.h"

#define FBNCH(PTR) ((Nit_fbnch *) (PTR))

static inline enum nit_anop
mes_choice(int depth)
{
	return (depth > 0) ? FT_MES_ANO : FT_MES_DAT;
}

static Nit_fbnch *
fmem_get_bnch(Nit_fmem *mem, int cnt)
{
	Nit_fbnch *bnch = mem->bnchs[cnt - 1];

	if (!bnch) {
		bnch = malloc(sizeof(*bnch) + sizeof(void *) * cnt);
		memset(bnch, 0, sizeof(*bnch) + sizeof(void *) * cnt);
	}

	return bnch;
}

static void
fmem_put_bnch(Nit_fmem *mem, Nit_fbnch *bnch)
{
	bnch->elems[0] = mem->bnchs[bnch->cnt - 1];
	mem->bnchs[bnch->cnt - 1] = bnch;
}

void
fbnch_inc_refs(Nit_fbnch *bnch)
{
	int i = 0;

	for (; i < bnch->cnt; ++i)
		++((Nit_fbnch *) bnch->elems[i])->refs;
}

Nit_fbnch *
fbnch_new(Nit_fdat *dat, void *elem, int depth)
{
	Nit_fbnch *bnch = fmem_get_bnch(dat->mem, 1);
	enum nit_anop op = mes_choice(depth);

	pcheck(bnch, NULL);
	bnch->refs = 1;
	bnch->cnt = 1;
	bnch->elems[0] = elem;
	bnch->ano.ptr = NULL;

	if (!dat->nat(op, &bnch->ano, elem, dat->ext)) {
		free(bnch);
		return NULL;
	}

	return bnch;
}

Nit_fbnch *
fbnch_new_arr(Nit_fdat *dat, void **arr, int cnt, int depth)
{
	size_t elems = cnt * sizeof(void *);
	Nit_fbnch *bnch = fmem_get_bnch(dat->mem, cnt);
	enum nit_anop op = mes_choice(depth);
	int i = 0;

	pcheck(bnch, NULL);
	bnch->ano.ptr = NULL;
	bnch->refs = 1;
	bnch->cnt = cnt;
	memcpy(bnch->elems, arr, elems);

	for (; i < cnt; ++i)
		if (!dat->nat(op, &bnch->ano, arr[i], dat->ext)) {
			free(bnch);
			return NULL;
		}

	if (depth > 0)
		fbnch_inc_refs(bnch);

	return bnch;
}

void
fbnch_reduce(Nit_fdat *dat, Nit_fbnch *bnch, int depth)
{
	int i = 0;

	if (!depth || !bnch || --bnch->refs > 0)
		return;

	if (depth > 1)
		for (; i < bnch->cnt; ++i)
		        fbnch_reduce(dat, bnch->elems[i], depth - 1);

	dat->nat(FT_DEC, &bnch->ano, NULL, dat->ext);
        fmem_put_bnch(dat->mem, bnch);
}

void *
fbnch_first(Nit_fbnch *bnch)
{
	return bnch->elems[0];
}

void *
fbnch_last(Nit_fbnch *bnch)
{
	return bnch->elems[bnch->cnt - 1];
}

int
fbnch_prepend(Nit_fdat *dat, Nit_fbnch **bnch, void *elem, int depth)
{
	Nit_fbnch *old = *bnch;
	Nit_fbnch *new = fmem_get_bnch(dat->mem, old->cnt + 1);
	enum nit_anop op = mes_choice(depth);

	pcheck(new, 0);
	new->refs = 1;
        new->cnt = old->cnt + 1;
	new->elems[0] = elem;
	memcpy(new->elems + 1, old->elems, sizeof(elem) * old->cnt);
	memcpy(&new->ano, &old->ano, sizeof(old->ano));

	if (!dat->nat(FT_COPY, &new->ano, NULL, dat->ext) ||
	    !dat->nat(op, &new->ano, elem, dat->ext)) {
		free(new);
		return 0;
	}

	if (depth > 0)
		++((Nit_fbnch *) elem)->refs;

	if (depth > 0)
		fbnch_inc_refs(old);

	fbnch_reduce(dat, old, depth);
	*bnch = new;

	return 1;
}

int
fbnch_append(Nit_fdat *dat, Nit_fbnch **bnch, void *elem, int depth)
{
	Nit_fbnch *old = *bnch;
	Nit_fbnch *new = fmem_get_bnch(dat->mem, old->cnt + 1);
	enum nit_anop op = mes_choice(depth);

	pcheck(new, 0);
	new->refs = 1;
        new->cnt = old->cnt + 1;
	memcpy(new->elems, old->elems, sizeof(elem) * old->cnt);
	new->elems[old->cnt] = elem;
	memcpy(&new->ano, &old->ano, sizeof(old->ano));

	if (!dat->nat(FT_COPY, &new->ano, NULL, dat->ext) ||
	    !dat->nat(op, &new->ano, elem, dat->ext)) {
		free(new);
		return 0;
	}

	if (depth > 0)
		++((Nit_fbnch *) elem)->refs;

	if (depth > 0)
		fbnch_inc_refs(old);

	fbnch_reduce(dat, old, depth);
	*bnch = new;

	return 1;
}

void *
fbnch_pop(Nit_fdat *dat, Nit_fbnch **bnch, int depth)
{
	Nit_fbnch *old = *bnch;
	void *val = fbnch_first(old);

	if (old->cnt == 1)
		*bnch = NULL;
	else if (!(*bnch = fbnch_new_arr(dat, old->elems + 1,
					 old->cnt - 1, depth)))
		return NULL;

	fbnch_reduce(dat, old, depth);

	return val;
}

void *
fbnch_rpop(Nit_fdat *dat, Nit_fbnch **bnch, int depth)
{
	Nit_fbnch *old = *bnch;
	void *val = fbnch_last(old);

	if (old->cnt == 1)
		*bnch = NULL;
	else if (!(*bnch = fbnch_new_arr(dat, old->elems, old->cnt - 1, depth)))
		return NULL;

	fbnch_reduce(dat, old, depth);

	return val;
}

void *
fbnch_search(Nit_fsrch srch, Nit_fbnch *bnch, void *acc,
	     void *ext, int depth)
{
	int i;

	for (;; bnch = bnch->elems[i], --depth) {
		if (!depth)
			for (i = 0; i < bnch->cnt; ++i)
				if (srch(FT_DAT, acc, bnch->elems[i], ext))
					return bnch->elems[i];

		for (i = 0; i < bnch->cnt; ++i)
			if (srch(FT_ANO, acc, &FBNCH(bnch->elems[i])->ano, ext))
				break;
	}

	return NULL;
}
