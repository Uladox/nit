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
radix_lookup(Nit_radix *radix, char *str)
{
	Nit_redge *e;

	for (; *str; str += e->len, radix = e->radix) {
		e = radix_get(radix, *str++);

		if (!e || strncmp(str, e->str, e->len) != 0)
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

Nit_radix *
radix_new(void *dat)
{
	Nit_radix *radix = palloc(radix);

	pcheck(radix, NULL);
	radix_init(radix, dat);
	return radix;
}

Nit_redge *
redge_new(Nit_radix *radix, char *pre, size_t len)
{
	Nit_redge *e = malloc(sizeof(*e) + len);

	pcheck(e, NULL);
	e->radix = radix;
	e->len = len;
	strncpy(e->str, pre, len);

	return e;
}

int
redge_split(Nit_redge **old_ref, char *str, void *dat)
{
	Nit_radix *split;
	Nit_redge *common;
	Nit_redge *e = *old_ref;
	char *e_str = e->str;
	size_t i = 0;

	for (; i < e->len; ++i, ++e_str, ++str)
		if (!*str || *e_str != *str)
			break;

	if (i == e->len)
		return 0;

	if (!*str) {
		pcheck(split = radix_new(dat), -1);
	} else {
		Nit_redge *tmp;
		char c = *str++;

		pcheck(split = radix_new(NULL), -1);
		tmp = redge_new(radix_new(dat), str, strlen(str));
		pcheck(tmp, -1);
		radix_add(split, c, tmp);
	}

        common = redge_new(split, e->str, i);
	radix_add(split, *e_str, redge_new(e->radix, e_str + 1, e->len - i - 1));
	free(*old_ref);
	*old_ref = common;
	return 1;
}

int
radix_insert(Nit_radix *radix, char *str, void *dat)
{
	Nit_redge *e;
	Nit_redge **er;
	Nit_radix *new_radix = radix_new(dat);

	for (; *str; e = *er, str += e->len, radix = e->radix) {
		if (!(er = radix_get_ref(radix, *str))) {
			char c = *str++;

			e = redge_new(new_radix, str, strlen(str));
			return radix_add(radix, c, e);
		}

		if (redge_split(er, ++str, dat))
			return 1;
	}

	return 1;
}
