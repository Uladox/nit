#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <stdint.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "list.h"
#include "hset.h"
#include "hmap.h"
#include "radix.h"

static int
radix_emp(Nit_radix *radix)
{
	return radix->map.entry_num < 0;
}

Nit_redge **
radix_get_ref(Nit_radix *radix, char c)
{
	return radix_emp(radix) ? NULL :
		hmap_get_ref(&radix->map, &c, 1);
}

Nit_redge *
radix_get(Nit_radix *radix, char c)
{
	return radix_emp(radix) ? NULL :
		hmap_get(&radix->map, &c, 1);
}

int
radix_add(Nit_radix *radix, char c, Nit_redge *edge)
{
	if (radix_emp(radix))
		hmap_init(&radix->map, 0);

	return hmap_add(&radix->map, &c, 1, edge);
}

void *
radix_lookup(Nit_radix *radix, const void *key, size_t len)
{
	Nit_redge *e;
	const char *str = key;

	for (; len > 0; str += e->len, len -= e->len, radix = e->radix) {
		e = radix_get(radix, *str++);
		--len;

		if (!e || len < e->len || memcmp(str, e->str, e->len) != 0)
			return NULL;
	}

	return radix->dat;
}

void
radix_init(Nit_radix *radix, void *dat)
{
	radix->dat = dat;
	radix->map.entry_num = -1;
}

/* void */
/* nit_radix_release(Nit_radix *radix) */
/* { */
	
/* } */

Nit_radix *
radix_new(void *dat)
{
	Nit_radix *radix = palloc(radix);

	pcheck(radix, NULL);
	radix_init(radix, dat);
	return radix;
}

Nit_redge *
redge_new(Nit_radix *radix, const void *pre, size_t len)
{
	Nit_redge *e = malloc(sizeof(*e) + len);

	pcheck(e, NULL);
	e->radix = radix;
	e->len = len;
	memcpy(e->str, pre, len);

	return e;
}

int
redge_split(Nit_redge **old_ref, const void *key, size_t len, void *dat)
{
	const char *str = key;
	Nit_radix *split;
	Nit_redge *common;
	Nit_redge *tmp;
	Nit_redge *e = *old_ref;
	char *e_str = e->str;
	size_t i = 0;

	for (; len && i < e->len; ++i, ++e_str, ++str, --len)
		if (*e_str != *str)
			break;

	if (i == e->len)
		return 0;

	if (!len) {
		pcheck(split = radix_new(dat), -1);
	} else {
		char c = *str++;
		Nit_radix *end = radix_new(dat);

		--len;
		pcheck(end, -1);
		pcheck(split = radix_new(NULL), -1);
		tmp = redge_new(end, str, len);
		pcheck(tmp, -1);

		if (radix_add(split, c, tmp) <= 0)
			return -1;
	}

        pcheck(common = redge_new(split, e->str, i), -1);
	pcheck(tmp = redge_new(e->radix, e_str + 1, e->len - i - 1), -1);

	if (radix_add(split, *e_str, tmp) <= 0)
		return -1;

	free(*old_ref);
	*old_ref = common;
	return 1;
}

int
radix_insert(Nit_radix *radix, const void *key, size_t len, void *dat)
{
	Nit_redge *e;
	Nit_redge **er;
	const char *str = key;
	Nit_radix *new_radix = radix_new(dat);

	for (; *str;
	     e = *er, str += e->len, len -= e->len, radix = e->radix) {
		if (!(er = radix_get_ref(radix, *str))) {
			char c = *str++;

			--len;
			pcheck(e = redge_new(new_radix, str, len), 0);
			return radix_add(radix, c, e);
		}

		switch (redge_split(er, ++str, --len, dat)) {
		case -1:
			return 0;
		case 0:
			break;
		case 1:
			return 1;
		}
	}

	return 1;
}

void
radix_iter_init(Nit_radix_iter *iter, Nit_radix *radix)
{
	iter->type = NIT_T_RADIX;
	iter->pos = 0;
	iter->root = radix;
	iter->d.radix = radix;
}

int
radix_iter_move(Nit_radix_iter *iter, const void *key, size_t len)
{
	const char *str = key;
	Nit_redge *e;
	int e_len;
	const char *e_str;

	if (!len)
		return 0;

	do {
		switch (iter->type) {
		case NIT_T_RADIX:
			if (!(e = radix_get(iter->d.radix, *str++)))
				return len;

			--len;
			iter->type = NIT_T_REDGE;
			iter->pos = 0;
			iter->d.redge = e;
			break;
		case NIT_T_REDGE:
			e = iter->d.redge;
			e_len = e->len - iter->pos;
			e_str = e->str + iter->pos;

			for (; e_len && len;
			     --len, --e_len, ++str, ++e_str, ++iter->pos)
				if (*str != *e_str)
					return len;

			if (!e_len) {
				iter->type = NIT_T_RADIX;
				iter->pos = 0;
				iter->d.radix = e->radix;
			}
		}
	} while (len);

	return 0;
}

void *
radix_iter_get(Nit_radix_iter *iter)
{
	if (iter->type == NIT_T_RADIX)
		return iter->d.radix->dat;

	return NULL;
}
