#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "artr.h"

#define ARTR(VAL)    ((Nit_artr *) (VAL))
#define NODE8(VAL)   ((Nit_artr_node8 *) (VAL))
#define NODE16(VAL)  ((Nit_artr_node16 *) (VAL))
#define NODE48(VAL)  ((Nit_artr_node48 *) (VAL))
#define NODE256(VAL) ((Nit_artr_node256 *) (VAL))
#define EDGE(VAL)    ((Nit_artr_edge *) (VAL))

#define INVALID_48 48

#include "artr_repeating.c"

void
artr_reuse_init(Nit_artr_reuse *reuse)
{
	memset(reuse, 0, sizeof(*reuse));
}

void
artr_reuse_dispose(Nit_artr_reuse *reuse)
{
	Nit_artr **array = (void *) reuse;
	int i = 0;
	Nit_artr *artr;
	Nit_artr *prev;

	for (i = 0; i < 5; ++i)
		for (artr = array[i]; artr;
		     prev = artr, artr = artr->val, free(prev));
}

int
artr_init(Nit_artr **artr, Nit_artr_reuse *reuse)
{
	Nit_artr *tmp = (Nit_artr *) get_8(reuse);

	pcheck(tmp, 0);
	*artr = tmp;
	return 1;
}

void
artr_iter_set(Nit_artr_iter *iter, Nit_artr **artr)
{
	iter->artr = artr;
	iter->offset = 0;
	iter->passed = 1;
}

void
artr_iter_copy(Nit_artr_iter *des, Nit_artr_iter *src)
{
	memcpy(des, src, sizeof(*des));
}

static void
node_next_update(const uint8_t **str_ref, size_t *len)
{
	++*str_ref;
	*len -= 1;
}


static void
edge_passed(const uint8_t **str_ref, size_t *size, size_t i)
{
	*str_ref += i;
	*size -= i;
}

static void
edge_not_passed(Nit_artr_iter *iter, const uint8_t **str_ref,
		size_t *size, size_t i)
{
	*str_ref += i;
	*size -= i;
	iter->offset = i;
	iter->passed = 0;
}

#include <stdio.h>

size_t
artr_iter_move(Nit_artr_iter *iter, const void *dat, size_t len)
{
	const uint8_t *str = dat;
	const uint8_t *str2;
	Nit_artr *artr = *iter->artr;
	Nit_artr **tmp;
	size_t i = 0;
	size_t shorter;

	iter->offset = 0;
	iter->passed = 1;

next_iteration:
	while (len) {
		/* printf("%u\n", *str); */
		switch (artr->type) {
		case ARTR8:
			for (i = 0; i < artr->count; ++i)
				if (NODE8(artr)->keys[i] == *str) {
					node_next_update(&str, &len);
					artr = *(iter->artr = &NODE8(artr)->sub[i]);
					goto next_iteration;
				}

		return len;
		case ARTR16:
			for (i = 0; i < artr->count; ++i)
				if (NODE16(artr)->keys[i] == *str) {
					node_next_update(&str, &len);
					artr = *(iter->artr = &NODE16(artr)->sub[i]);
					goto next_iteration;
			}

		return len;
		case ARTR48:
			if ((i = NODE48(artr)->keys[*str]) == INVALID_48)
				return len;

		node_next_update(&str, &len);
		artr = *(iter->artr = &NODE48(artr)->sub[i]);
		break;
		case ARTR256:
			if (!*(tmp = &NODE256(artr)->sub[*str]))
				return len;

			node_next_update(&str, &len);
			artr = *(iter->artr = tmp);
			break;
		case ARTR_EDGE:
			str2 = EDGE(artr)->str;
			shorter = artr->count < len ? artr->count : len;

			for (; i < shorter; ++str, ++str2, ++i)
				if (*str != *str2) {
					edge_not_passed(iter, &str, &len, i);
					return len;
				}

			edge_passed(&str, &len, artr->count);
			artr = *(iter->artr = (Nit_artr **) &artr->val);
			break;
		case ARTR_EDGE_WITH_VAL:
			str2 = EDGE(artr)->str;

			if (!str2)
				return len;

			shorter = artr->count < len ? artr->count : len;

			for (; i < shorter; ++str, ++str2, ++i)
				if (*str != *str2) {
					edge_not_passed(iter, &str, &len, i);
					return len;
				}

			edge_passed(&str, &len, artr->count);
			return len;
		}
	}

	return 0;
}

void *
artr_iter_get(Nit_artr_iter *iter)
{
	Nit_artr *artr = *iter->artr;

	switch (artr->type) {
		case ARTR8:
		case ARTR16:
		case ARTR48:
		case ARTR256:
			return artr->val;
		case ARTR_EDGE_WITH_VAL:
			if (iter->passed)
				return artr->val;
		case ARTR_EDGE:
		        return NULL;
	}

	return NULL;
}

void *
artr_iter_lookup(Nit_artr_iter *iter, const void *dat, size_t len)
{
	Nit_artr_iter tmp;

	artr_iter_copy(&tmp, iter);
	artr_iter_move(&tmp, dat, len);
	return artr_iter_get(&tmp);
}

static Nit_artr_edge *
remainder_edge(Nit_artr_reuse *reuse, const uint8_t *rest,
	       size_t left, void *val)
{
	return get_edge(reuse, NIT_ARTR_EDGE_WITH_VAL,
			rest + 1, left - 1, val);
}

int
artr_iter_insert(Nit_artr_iter *iter, const void *dat, size_t len, void *val,
		 Nit_artr_reuse *reuse)
{
	Nit_artr_edge *edge;
	Nit_artr_iter tmp;
	size_t left;
	size_t moved;
	const uint8_t *rest;
	Nit_artr *artr;

	artr_iter_copy(&tmp, iter);
        left = artr_iter_move(&tmp, dat, len);
        moved = len - left;
	rest = ((const uint8_t *) dat) + moved;
        artr = *tmp.artr;

	if (!left && tmp.passed) {
		artr->val = val;
		return len;
	}

	if (artr->type == ARTR_EDGE_WITH_VAL) {
		if (!EDGE(*tmp.artr)->str)
			return insert_edge_nulled(tmp.artr, rest,
						  left, val, reuse);
	}

	/* if (artr->type == ARTR_EDGE_WITH_VAL || */
	/*     artr->type == ARTR_EDGE) { */
		/* if (iter->passed) */
		/* 	return insert_after_edge(); */

		/* if (!left && !iter->offset) */
		/* 	return insert_before_edge(); */

		/* return insert_middle_edge(); */
	/* } */

	edge = remainder_edge(reuse, rest, left, val);
	pcheck(edge, 0);

	switch (artr->type) {
	case ARTR8:
		return insert_8(tmp.artr, reuse, *rest, edge);
	case ARTR16:
		return insert_16(tmp.artr, reuse, *rest, edge);
	case ARTR48:
		return insert_48(tmp.artr, reuse, *rest, edge);
	case ARTR256:
		return insert_256(tmp.artr, *rest, edge);
	default:
		break;
	}

	return 0;
}



