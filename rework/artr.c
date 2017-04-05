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

size_t
artr_iter_move(Nit_artr_iter *iter, const void *dat, size_t len)
{
	const uint8_t *str = dat;
	Nit_artr *artr = *iter->artr;

	iter->offset = 0;
	iter->passed = 1;

	for (; len; artr = *iter->artr) {
		switch (artr->type) {
		case ARTR8:
			if (artr_iter_move8(iter, artr, &str, &len))
				break;

			return len;
		case ARTR16:
		        if (artr_iter_move16(iter, artr, &str, &len))
				break;

			return len;
		case ARTR48:
			if (artr_iter_move48(iter, artr, &str, &len))
				break;

			return len;
		case ARTR256:
			if (artr_iter_move256(iter, artr, &str, &len))
				break;

			return len;
		case ARTR_EDGE:
			if (artr_iter_move256(iter, artr, &str, &len))
				break;

			return len;
		case ARTR_EDGE_WITH_VAL:
			artr_iter_move_edge_value(iter, artr, &str, &len);
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



