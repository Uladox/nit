#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "vec.h"

int
vec_init(Nit_vec *vec, size_t max)
{
	vec->size = 0;
	return !!(vec->dat = calloc(vec->max = max, 1));
}

void
vec_dispose(Nit_vec *vec)
{
	free(vec->dat);
}

Nit_vec *
vec_new(size_t max)
{
	Nit_vec *vec = palloc(vec);

	pcheck(vec, NULL);

	if (!vec_init(vec, max)) {
		free(vec);
		return NULL;
	}

	return vec;
}

void
vec_free(Nit_vec *vec)
{
	vec_dispose(vec);
	free(vec);
}

int
vec_push(Nit_vec *vec, void *dat, size_t size)
{
	char *new_dat;
	size_t new_max;

	if (vec->max < vec->size + size) {
		new_max = size > vec->max ? vec->max + size : 2 * vec->max;
		new_dat = realloc(vec->dat, new_max);
		pcheck(new_dat, 0);
		memcpy(new_dat, dat, size);
		vec->size += size;
		vec->max = new_max;
		vec->dat = new_dat;
		memset(new_dat + vec->size, 0, new_max - vec->size);
	} else {
		memcpy(vec->dat, dat, size);
		vec->size += size;
	}

	return 1;
}

int
vec_push_ptr(Nit_vec *vec, void *ptr)
{
	return vec_push(vec, &ptr, sizeof(void *));
}

void *
vec_get(Nit_vec *vec, size_t num, size_t size)
{
	size_t pos = num * size;

	if (pos >= vec->size)
		return NULL;

	return vec->dat + pos;
}

void *
vec_get_last(Nit_vec *vec, size_t size)
{
	return vec_get(vec, (vec->size - size) / size, size);
}

void *
vec_get_ptr(Nit_vec *vec, size_t num)
{
	size_t pos = num * sizeof(void *);

	if (pos >= vec->size)
		return NULL;

	return *(void **) (vec->dat + pos);
}

void *
vec_get_last_ptr(Nit_vec *vec)
{
	return vec_get_ptr(vec, (vec->size - sizeof(void *)) / sizeof(void *));
}

int
vec_remove(Nit_vec *vec, size_t num, size_t size)
{
	char *end = vec_get(vec, num, size);

	pcheck(end, 0);
	memmove(end, end + size, (vec->dat + vec->size) - (end + size));
	memset(vec->dat + (vec->size -= size), 0, size);
	return 1;
}
