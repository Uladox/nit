#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "list.h"
#include "urlist.h"

#define HALF(ARR) (ARRAY_UNITS(ARR) / 2)
#define END(ARR) (ARR + ARRAY_UNITS(ARR) - 1)

Nit_urlist4 *
urlist4_new(void)
{
	Nit_urlist4 *list = palloc(list);

	pcheck(list, NULL);
	LIST_CONS(list, NULL);
	memset(list->arr, 0, sizeof(list->arr));
	list->count = 0;

	return list;
}

static Nit_urlist4 *
urlist4_new_set(void **half)
{
	Nit_urlist4 *list = palloc(list);

	pcheck(list, NULL);
	LIST_CONS(list, NULL);
	list->count = HALF(list->arr);
	memset(list->arr, 0, sizeof(list->arr) / 2);
        memcpy(list->arr + HALF(list->arr), half, sizeof(list->arr) / 2);

	return list;
}

void *
urlist4_first(Nit_urlist4 *list)
{
	void **ptr;

	for (; list; list = LIST_NEXT(list))
		if (list->count)
			goto ok;

	return NULL;

ok:
	ptr = list->arr;

	for (; !*ptr; ++ptr);

	return *ptr;
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
	memcpy(last_half - 1, &val, sizeof(val));
}

/*                                e  p   d  -d */
/* [EFGH] + [CD] -> [CDEF] + [GH] 0, 2,  2, -2 */
/* [EFGH] + [C_] -> [CEFG] + [H_] 0, 1,  1, -1 */
/* [_EFG] + [CD] -> [CDEF] + [G_] 1, 2,  1, -1 */
/* [__EF] + [CD] -> [CDEF] + [__] 2, 2,  0,  0 */
/* [___F] + [CD] -> [_CDF] + [__] 3, 2, -1,  1 */
/* [____] + [CD] -> [__CD] + [__] 4, 2, -2,  2 */
/* [____] + [C_] -> [___C] + [__] 4, 1, -3,  3 */
static void
copy_half(Nit_urlist4 *list, void **half, int empty, int passed)
{
	void *half2[HALF(list->arr)];
	int diff = passed - empty;

	if (diff > 0) {
		/* If there is stuff to store for latter, store it
		 * [CD] + [EFGH] -> [GH]
		 * [C_] + [EFGH] -> [H_]
		 */
		memcpy(half2, END(list->arr) - diff, diff * sizeof(*half2));

		/* Move to make room for new stuff
		 * [EFGH] + [CD] -> [EFEF]
		 * [_EFG] + [CD] -> [_EEF]
		 * X [_EFG] + [C_] X [___E] + [CD]
		 */
		memcpy(list->arr + passed, list->arr + empty,
		       passed * sizeof(*list->arr));

		memcpy(half, half2, diff * sizeof(*half2));

	}

	/* Copies half into list->arr at the right place */
	/* [CD] + [EFGH] -> [CDGH] */
	/* [CD] + [_EFG] -> [CDFG] */
	memcpy(list->arr + ((diff >= 0) ? 0 : -diff),
	       half, passed * sizeof(*half));

}

static inline int
add(Nit_urlist4 *list, void **half, int *left)
{
	int empty = 0; //ARRAY_UNITS(list->arr) - list->count;
	void **ptr = list->arr;

	for (; !*ptr; ++ptr, ++empty);

	copy_half(list, half, empty, *left);
	*left -= empty;

	return !*left;
}

#define INSERTED(LIST) (++(LIST)->count, 1)

int
urlist4_insert(Nit_urlist4 *list, void *val)
{
	void *half[HALF(list->arr)];
	Nit_urlist4 *new_list;
	int left;

	if (!list->arr[0]) {
		size_t i = 0;

		for (; i < ARRAY_UNITS(list->arr); ++i)
			if (!list->arr[i]) {
				list->arr[i] = val;
			        return INSERTED(list);
			}
	}

	refit(list, half, val);
	left = HALF(list->arr);

	for (; LIST_NEXT(list); list = LIST_NEXT(list))
		if (add(LIST_NEXT(list), half, &left))
			return INSERTED(list);

	new_list = urlist4_new_set(half);
	pcheck(new_list, 0);
	LIST_CONS(list, new_list);

	return INSERTED(list);
}
