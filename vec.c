#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "vec.h"

int
vec_init(Nit_vec *vec, uint32_t max)
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
vec_new(uint32_t max)
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
vec_insert(Nit_vec *vec, void *dat, uint32_t num, uint32_t size)
{
	char *new_dat;
	uint32_t new_max;
	uint32_t pos = num * size;

	if (vec->max < vec->size + size) {
		new_max = size > vec->max ? vec->max + size : 2 * vec->max;
		new_dat = malloc(new_max);
		pcheck(new_dat, 0);
		memcpy(new_dat, vec->dat, pos);
		memcpy(new_dat + pos, dat, size);
		memcpy(new_dat + pos + size, vec->dat + pos, vec->size - pos);
		memset(new_dat + vec->max + size, 0,
		       new_max - (vec->max + size));
		vec->size += size;
		vec->max = new_max;
		free(vec->dat);
		vec->dat = new_dat;
	} else {
		memmove(vec->dat + pos + size, vec->dat + pos, vec->size - pos);
		memcpy(vec->dat + pos, dat, size);
		vec->size += size;
	}

	return 1;

}

int
vec_insert_ptr(Nit_vec *vec, void *ptr, uint32_t num)
{
	return vec_insert(vec, &ptr, num, sizeof(ptr));
}

int
vec_push(Nit_vec *vec, void *dat, uint32_t size)
{
	char *new_dat;
	uint32_t new_max;

	if (vec->max < vec->size + size) {
		new_max = size > vec->max ? vec->max + size : 2 * vec->max;
		new_dat = realloc(vec->dat, new_max);
		pcheck(new_dat, 0);
		memcpy(new_dat + vec->size, dat, size);
		vec->size += size;
		vec->max = new_max;
		vec->dat = new_dat;
		memset(new_dat + vec->size, 0, new_max - vec->size);
	} else {
		memcpy(vec->dat + vec->size, dat, size);
		vec->size += size;
	}

	return 1;
}

int
vec_push_ptr(Nit_vec *vec, void *ptr)
{
	return vec_push(vec, &ptr, sizeof(ptr));
}

void *
vec_get(Nit_vec *vec, uint32_t num, uint32_t size)
{
	uint32_t pos = num * size;

	if (pos >= vec->size)
		return NULL;

	return vec->dat + pos;
}

void *
vec_get_last(Nit_vec *vec, uint32_t size)
{
	return vec_get(vec, (vec->size - size) / size, size);
}

void *
vec_get_ptr(Nit_vec *vec, uint32_t num)
{
	uint32_t pos = num * sizeof(void *);

	if (pos >= vec->size)
		return NULL;

	return *(void **) (vec->dat + pos);
}

static uint32_t
last_ptr_index(Nit_vec *vec)
{
	return (vec->size - sizeof(void *)) / sizeof(void *);
}

void *
vec_get_last_ptr(Nit_vec *vec)
{
	return vec_get_ptr(vec, last_ptr_index(vec));
}

int
vec_remove(Nit_vec *vec, uint32_t num, uint32_t size)
{
	char *end = vec_get(vec, num, size);

	pcheck(end, 0);
	memmove(end, end + size, (vec->dat + vec->size) - (end + size));
	memset(vec->dat + (vec->size -= size), 0, size);
	return 1;
}

int
vec_remove_ptr(Nit_vec *vec, uint32_t num)
{
	return vec_remove(vec, num, sizeof(void *));
}

void *
vec_pop_ptr(Nit_vec *vec)
{
	void *ptr = vec_get_last_ptr(vec);

	pcheck(ptr, NULL);
	vec_remove_ptr(vec, last_ptr_index(vec));
	return ptr;
}
