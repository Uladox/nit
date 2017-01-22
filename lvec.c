#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "lvec.h"

int
lvec_init(Nit_lvec *lvec, size_t max)
{
	lvec->size = 0;
	return !!(lvec->dat = calloc(lvec->max = max, 1));
}

void
lvec_dispose(Nit_lvec *lvec)
{
	free(lvec->dat);
}

Nit_lvec *
lvec_new(size_t max)
{
	Nit_lvec *lvec = palloc(lvec);

	pcheck(lvec, NULL);

	if (!lvec_init(lvec, max)) {
		free(lvec);
		return NULL;
	}

	return lvec;
}

void
lvec_free(Nit_lvec *lvec)
{
	lvec_dispose(lvec);
	free(lvec);
}

int
lvec_insert(Nit_lvec *lvec, void *dat, size_t num, size_t size)
{
	char *new_dat;
	size_t new_max;
	size_t pos = num * size;

	if (lvec->max < lvec->size + size) {
		new_max = size > lvec->max ? lvec->max + size : 2 * lvec->max;
		new_dat = malloc(new_max);
		pcheck(new_dat, 0);
		memcpy(new_dat, lvec->dat, pos);
		memcpy(new_dat + pos, dat, size);
		memcpy(new_dat + pos + size, lvec->dat + pos, lvec->size - pos);
		memset(new_dat + lvec->max + size, 0,
		       new_max - (lvec->max + size));
		lvec->size += size;
		lvec->max = new_max;
		free(lvec->dat);
		lvec->dat = new_dat;
	} else {
		memmove(lvec->dat + pos + size, lvec->dat + pos, lvec->size - pos);
		memcpy(lvec->dat + pos, dat, size);
		lvec->size += size;
	}

	return 1;

}

int
lvec_insert_ptr(Nit_lvec *lvec, void *ptr, size_t num)
{
	return lvec_insert(lvec, &ptr, num, sizeof(ptr));
}

int
lvec_push(Nit_lvec *lvec, void *dat, size_t size)
{
	char *new_dat;
	size_t new_max;

	if (lvec->max < lvec->size + size) {
		new_max = size > lvec->max ? lvec->max + size : 2 * lvec->max;
		new_dat = realloc(lvec->dat, new_max);
		pcheck(new_dat, 0);
		memcpy(new_dat + lvec->size, dat, size);
		lvec->size += size;
		lvec->max = new_max;
		lvec->dat = new_dat;
		memset(new_dat + lvec->size, 0, new_max - lvec->size);
	} else {
		memcpy(lvec->dat + lvec->size, dat, size);
		lvec->size += size;
	}

	return 1;
}

int
lvec_push_ptr(Nit_lvec *lvec, void *ptr)
{
	return lvec_push(lvec, &ptr, sizeof(ptr));
}

void *
lvec_get(Nit_lvec *lvec, size_t num, size_t size)
{
	size_t pos = num * size;

	if (pos >= lvec->size)
		return NULL;

	return lvec->dat + pos;
}

void *
lvec_get_last(Nit_lvec *lvec, size_t size)
{
	return lvec_get(lvec, (lvec->size - size) / size, size);
}

void *
lvec_get_ptr(Nit_lvec *lvec, size_t num)
{
	size_t pos = num * sizeof(void *);

	if (pos >= lvec->size)
		return NULL;

	return *(void **) (lvec->dat + pos);
}

static size_t
last_ptr_index(Nit_lvec *lvec)
{
	return (lvec->size - sizeof(void *)) / sizeof(void *);
}

void *
lvec_get_last_ptr(Nit_lvec *lvec)
{
	return lvec_get_ptr(lvec, last_ptr_index(lvec));
}

int
lvec_remove(Nit_lvec *lvec, size_t num, size_t size)
{
	char *end = lvec_get(lvec, num, size);

	pcheck(end, 0);
	memmove(end, end + size, (lvec->dat + lvec->size) - (end + size));
	memset(lvec->dat + (lvec->size -= size), 0, size);
	return 1;
}

int
lvec_remove_ptr(Nit_lvec *lvec, size_t num)
{
	return lvec_remove(lvec, num, sizeof(void *));
}

void *
lvec_pop_ptr(Nit_lvec *lvec)
{
	void *ptr = lvec_get_last_ptr(lvec);

	pcheck(ptr, NULL);
	lvec_remove_ptr(lvec, last_ptr_index(lvec));
	return ptr;
}
