#include <stdlib.h>
#include <string.h>

static inline enum nit_anop
mes_choice(int depth)
{
	return (depth > 0) ? FT_MES_ANO : FT_MES_DAT;
}

static Nit_fbnch *
fmem_get_bnch(Nit_fmem *mem, int cnt)
{
	Nit_fbnch *bnch = mem->bnchs[cnt - 1];

	if (!bnch)
		bnch = malloc(sizeof(*bnch) + sizeof(void *) * cnt);

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
		        fbnch_reduce(s, bnch->elems[i], depth - 1);

	dat->nat(FT_DEC, &bnch->ano, NULL, dat->extra);
        fmem_put_bnch(dat->mem, bnch);
}

Nit_fbnch *
fbnch_prepend(Nit_fdat *dat, Nit_fbnch *old, void *elem, int depth)
{
	Nit_fbnch *new = fmem_get_bnch(dat->mem, old->cnt + 1);
	enum nit_anop op = mes_choice(depth);

	pcheck(new, NULL);
	new->refs = 1;
        new->cnt = old->cnt + 1;
	new->elems[0] = elem;
	memcpy(new->elem + 1, old->elem, sizeof(elem) * old->cnt);
	memcpy(new->ano, old->ano, sizeof(old->ano));

	if (!dat->nat(FT_COPY, &new->ano, NULL, extra) ||
	    !dat->nat(op, &new->ano, elem, dat->ext)) {
		free(new);
		return NULL;
	}

	if (depth > 0)
		++((Nit_fbnch *) elem)->refs;

	if (depth > 0)
		fbnch_inc_refs(old);

	fbnch_reduce(dat, old, depth);

	return new;
}

Nit_fbnch *
fbnch_append(Nit_fdat *dat, Nit_fbnch *old, void *elem, int depth)
{
	Nit_fbnch *new = fmem_get_bnch(dat->mem, old->cnt + 1);
	enum nit_anop op = mes_choice(depth);

	pcheck(new, NULL);
	new->refs = 1;
        new->cnt = old->cnt + 1;
	memcpy(new->elem, old->elem, sizeof(elem) * old->cnt);
	new->elem[old->cnt] = elem;
	memcpy(new->ano, old->ano, sizeof(old->ano));

	if (!dat->nat(FT_COPY, &new->ano, NULL, extra) ||
	    !dat->nat(op, &new->ano, elem, dat->ext)) {
		free(new);
		return NULL;
	}

	if (depth > 0)
		++((Nit_fbnch *) elem)->refs;

	if (depth > 0)
		fbnch_inc_refs(old);

	fbnch_reduce(dat, old, depth);

	return new;
}

void *
fbnch_pop(Nit_fdat *dat, Nit_fbnch **bnch, void *elem, int depth)
{
	Nit_fbnch *old = *bnch;
	void *val = old->elems[0];

	if (!(*bnch = fbnch_new_arr(dat, old->elems + 1, old->cnt - 1, depth)))
		return NULL;

	fbnch_reduce(dat, old, depth);

	return val;
}

void *
fbnch_rpop(Nit_fdat *dat, Nit_fbnch **bnch, void *elem, int depth)
{
	Nit_fbnch *old = *bnch;
	void *val = old->elems[old->cnt - 1];

	if (!(*bnch = fbnch_new_arr(dat, old->elems, old->cnt - 1, depth)))
		return NULL;

	fbnch_reduce(dat, old, depth);

	return val;
}
