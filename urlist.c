#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "list.h"
#include "urlist.h"

#define HALF(ARR) (ARRAY_UNITS(ARR) / 2)

Nit_urlist4 *
urlist4_new(void)
{
	Nit_urlist4 *list = palloc(list);

	pcheck(list, NULL);
	LIST_CONS(list, NULL);
	memset(list->arr, 0, sizeof(list->arr));

	return list;
}

Nit_urlist4 *
urlist4_new_set(void **half)
{
	Nit_urlist4 *list = palloc(list);

	pcheck(list, NULL);
	LIST_CONS(list, NULL);
	memset(list->arr, 0, sizeof(list->arr) / 2);
        memcpy(list->arr + HALF(list->arr), half, sizeof(list->arr) / 2);

	return list;
}

void *
urlist4_first(Nit_urlist4 *list)
{
	void **ptr = list->arr;

	for (; !*ptr; ++ptr);

	return ptr;
}

void *
urlist4_last(Nit_urlist4 *list)
{
	for (; LIST_NEXT(list); list = LIST_NEXT(list));

        return list->arr[ARRAY_UNITS(list->arr) - 1];
}

/* [ABCD] + [__] + V -> [_VAB] + [CD] */
static inline void
refit(Nit_urlist4 *list, void **half, void *val)
{
	void **last_half = list->arr + HALF(list->arr);

	/* [ABCD] -> [ABCD] + [CD] */
	memcpy(half, last_half, sizeof(list->arr) / 2);
	/* [ABCD] -> [__AB] */
	memcpy(last_half, list->arr, HALF(list->arr) / 2);
	memset(list->arr, 0, HALF(list->arr) / 2);
	/* [__AB] -> [_VAB] */
	memset(last_half - 1, &val, sizeof(val));
}

static inline int
add(Nit_urlist4 *list, void **half)
{
	void *half2[HALF(list->arr)];

	/* [EFGH] + [CD] -> [CDEF] + [GH] */
	if (list->arr[0]) {
		void **last_half = list->arr + HALF(list->arr);

		/* [EFGH] -> [EFGH] + [GH] */
		memcpy(half2, last_half, sizeof(half));
		/* [EFGH] -> [CDEF] */
		memcpy(last_half, list->arr, sizeof(half));
		memcpy(list->arr, half, sizeof(half));
		/* [CD] + [GH] -> [GH] + [GH] */
		memcpy(half, half2, sizeof(half2));

		return 0;
	}

	/* [__EF] + [CD] -> [CDEF] */
	memcpy(list->arr, half, sizeof(half2));

	return 1;
}

int
urlist4_insert(Nit_urlist4 *list, void *val)
{
	void *half[HALF(list->arr)];
	Nit_urlist4 *new_list;

	if (!list->arr[0]) {
		size_t i = 0;

		for (; i < ARRAY_UNITS(list->arr); ++i)
			if (!list->arr[i]) {
				list->arr[i] = val;
				return 1;
			}
	}

	refit(list, half, val);

	for (; LIST_NEXT(list); list = LIST_NEXT(list))
		if (add(LIST_NEXT(list), half))
			return 1;

	new_list = urlist4_new_set(half);
	pcheck(new_list, 0);
	LIST_CONS(list, new_list);

	return 1;
}
