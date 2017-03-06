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

int
artr_iter_init(Nit_artr_iter *iter, Nit_artr **artr)
{
	iter->artr = artr;
	iter->offset = 0;
	iter->passed = 1;
}

static void
node_next_update(const uint8_t **str_ref, size_t *size)
{
	++*str_ref;
	--*size;
}


static void
edge_passed(Nit_artr_iter *iter, const uint8_t **str_ref,
	    size_t *size, size_t i)
{
	*str_ref += i;
	*size -= i;
	iter->offset = 0;
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

size_t
artr_iter_move(Nit_artr_iter *iter, const void *dat, size_t len)
{
	const uint8_t *str = dat;
	const uint8_t *str2;
	Nit_artr *artr = *iter->artr;
	Nit_artr **tmp;
	int i;

	iter->passed = 1;

	while (len)
		switch (artr->type) {
		case ARTR8:
			for (i = 0; i < artr->count; ++i)
				if (NODE8(artr)->keys[i] == *str) {
					node_next_update(&str, &len);
					artr = *(iter->artr = &NODE8(artr)->sub[i]);
					break;
				}

		return len;
		case ARTR16:
			for (i = 0; i < artr->count; ++i)
				if (NODE16(artr)->keys[i] == *str) {
					node_next_update(&str, &len);
					artr = *(iter->artr = &NODE16(artr)->sub[i]);
					break;
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

			for (; i < artr->count; ++str, ++str2, ++i)
				if (*str != *str2) {
					edge_not_passed(&str, &len, offset, i);
					return len;
				}

			edge_passed(&str, &len, &iter->offset, artr->count);
			artr = *(iter->artr = (Nit_artr **) &artr->val);
			break;
		case ARTR_EDGE_WITH_VAL:
			str2 = EDGE(artr)->str;

			for (; i < *size; ++str, ++str2, ++i)
				if (*str != *str2) {
					edge_not_passed(str_ref, size, offset, i);
					return len;
				}

			edge_passed(&str, &len, &iter->offset, artr->count);
			return len;
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

size_t
artr_iter_insert(Nit_artr_iter *iter, const void *dat, size_t len, void *val)
{
	Nit_artr_edge *edge;
	size_t left = artr_iter_move(iter, dat, len);
	size_t size = len - left;
	const uint8_t *str = ((const uint8_t *) dat) + size;
	Nit_artr *artr = *iter->artr;

	if (!left && iter->passed) {
		artr->val = val;
		return len;
	}

	if (iter->type == ARTR_EDGE_WITH_VAL ||
	    iter->type == ARTR_EDGE) {
		if (iter->passed)
			insert_after_edge();

		if (!left && !iter->offset)
			insert_before_edge();

		return insert_middle_edge();
	}

	edge = remainder_edge(reuse, str, size, val);
	pcheck(edge, 0);

	switch (artr->type) {
	case ARTR8:
		return insert_8(artr, reuse, *str, edge);
	case ARTR16:
		return insert_16(artr, reuse, *str, edge);
	case ARTR48:
		return insert_48(artr, reuse, *str, edge);
	case ARTR256:
		return insert_256(artr, *str, edge);
	}

	return 0;
}

/* static Nit_artr ** */
/* artr_next_level(Nit_artr *artr, const uint8_t **str_ref, */
/* 		size_t *size, size_t *offset) */
/* { */
/* 	unsigned int i = 0; */
/* 	const uint8_t *str = *str_ref; */
/* 	const uint8_t *str2; */
/* 	Nit_artr **tmp; */

/* 	switch (artr->type) { */
/* 	case ARTR8: */
/* 		for (; i < artr->count; ++i) */
/* 			if (NODE8(artr)->keys[i] == *str) { */
/* 				node_next_update(str_ref, size); */
/* 				return &NODE8(artr)->sub[i]; */
/* 			} */

/* 		return NULL; */
/* 	case ARTR16: */
/* 		for (; i < artr->count; ++i) */
/* 			if (NODE16(artr)->keys[i] == *str) { */
/* 				node_next_update(str_ref, size); */
/* 				return &NODE16(artr)->sub[i]; */
/* 			} */

/* 		return NULL; */
/* 	case ARTR48: */
/* 		if ((i = NODE48(artr)->keys[*str]) == INVALID_48) */
/* 			return NULL; */

/* 		node_next_update(str_ref, size); */
/* 		return &NODE48(artr)->sub[i]; */
/* 	case ARTR256: */
/* 		if (!*(tmp = &NODE256(artr)->sub[*str])) */
/* 			return NULL; */

/* 		node_next_update(str_ref, size); */
/* 		return tmp; */
/* 	case ARTR_EDGE: */
/* 		if (artr->count > *size) */
/* 			return NULL; */

/* 		str2 = EDGE(artr)->str; */

/* 		for (; i < artr->count; ++str, ++str2, ++i) */
/* 			if (*str != *str2) { */
/* 				edge_not_passed(str_ref, size, offset, i); */
/* 				return NULL; */
/* 			} */

/* 		edge_passed(str_ref, size, offset, artr->count); */
/* 		return (Nit_artr **) &artr->val; */
/* 	case ARTR_EDGE_WITH_VAL: */
/* 		if (artr->count != *size) */
/* 			return NULL; */

/* 		str2 = EDGE(artr)->str; */

/* 		for (; i < *size; ++str, ++str2, ++i) */
/* 			    if (*str != *str2) { */
/* 				    edge_not_passed(str_ref, size, offset, i); */
/* 				    return NULL; */
/* 			    } */

/* 		edge_passed(str_ref, size, offset, artr->count); */
/* 		return NULL; */
/* 	} */

/* 	return NULL; /\* can never happen, ever *\/ */
/* } */


void *
artr_lookup(Nit_artr *artr, const void *dat, size_t size)
{
	size_t offset;
	Nit_artr **next = &artr;
	const uint8_t *str = dat;

	for (; size; artr = *next)
		if (!(next = artr_next_level(artr, &str,
					     &size, &offset))) {
			if (!size)
				return artr->val;

			return NULL;
		}

	if (artr->type == ARTR_EDGE)
		return NULL;

	return artr->val;
}

static size_t
get_offset(const uint8_t *str1, size_t size1,
	   const uint8_t *str2, size_t size2)
{
	size_t i = 0;
	size_t smaller = size1 > size2 ? size2 : size1;

	for (; i < smaller; ++str1, ++str2, ++i)
		if (*str1 != *str2)
				return i;

	return -1; /* should never happen */
}

static int
edge_cut_off_start(Nit_artr_edge *edge, size_t amount)
{
        size_t size = edge->artr.count - amount;
	uint8_t *str = NULL;

	if (size) {
		str = malloc(size);
		pcheck(str, 0);
		memcpy(str, edge->str + amount, size);
	}

	edge->artr.count = size;
	free(edge->str);
	edge->str = str;
	return 1;
}

static int
edge_cut_off_end(Nit_artr_edge *edge, size_t amount)
{
        size_t size = edge->artr.count - amount;
	uint8_t *str = NULL;

	if (size) {
		str = malloc(size);
		pcheck(str, 0);
		memcpy(str, edge->str, size);
	}

	edge->artr.count = size;
	free(edge->str);
	edge->str = str;
	return 1;
}

static Nit_artr_edge *
remainder_edge(Nit_artr_reuse *reuse, const uint8_t *str,
	       size_t size, void *val)
{
	return get_edge(reuse, NIT_ARTR_EDGE_WITH_VAL,
			str, size, val);
}

static int
insert_here(Nit_artr **artr, const uint8_t *str, size_t size, void *val,
	    size_t offset, Nit_artr_reuse *reuse)
{
	Nit_artr_edge *edge;

	printf("edgy!\n");

	switch ((*artr)->type) {
	case ARTR8:
		edge = remainder_edge(reuse, str, size, val);
		pcheck(edge, 0);
		return insert_8(artr, reuse, *str, edge);
	case ARTR16:
		edge = remainder_edge(reuse, str, size, val);
		pcheck(edge, 0);
		return insert_16(artr, reuse, *str, edge);
	case ARTR48:
		edge = remainder_edge(reuse, str, size, val);
		pcheck(edge, 0);
		return insert_48(artr, reuse, *str, edge);
	case ARTR256:
		edge = remainder_edge(reuse, str, size, val);
		pcheck(edge, 0);
		return insert_256(artr, *str, edge);
	case ARTR_EDGE:
	case ARTR_EDGE_WITH_VAL:
		return insert_edge(artr, str, size, val, offset, reuse);
	}

	return 0;
}

int
artr_insert(Nit_artr **artr, const void *dat, size_t size, void *val,
	    Nit_artr_reuse *reuse)
{
	Nit_artr **next = artr;
	size_t offset = 0;
	const uint8_t *str = dat;

	for (; size; artr = next)
		if (!(next = artr_next_level(*artr,
					     &str, &size, &offset)))
			return insert_here(artr, str, size, val,
					   offset, reuse);

	if ((*artr)->type == ARTR_EDGE ||
	    (*artr)->type == ARTR_EDGE_WITH_VAL)
		return insert_edge(artr, str, size, val, offset, reuse);

	(*artr)->val = val;
	return 1;
}
