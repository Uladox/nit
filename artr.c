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

/* Returns bytes to move past, if it's zero, we can't go further.
 * The offset is used when the artr is an edge with a number of bytes such that
 * str could go on to the next part of the trie, but does not since the bytes
 * are not equal to those in str. This sets offset to the number of bytes that
 * are equal. Next needs to be tree stars (despite that typically meaning you
 * are doing something horribly wrong) since we need to get a refernce to the
 * array index so that we can replace the pointer in it with another one when
 *  resizeing. It's not as bad as it seems.
 */
#include <stdio.h>
/* static inline size_t */
/* artr_move_one_level(Nit_artr *artr, Nit_artr ***next, */
/* 		    const uint8_t *str, size_t size, size_t *offset) */
/* { */
/* 	unsigned int i = 0; */
/* 	const uint8_t *str2; */
/* 	Nit_artr **tmp; */

/* 	switch (artr->type) { */
/* 	case ARTR8: */
/* 		for (; i < artr->count; ++i) */
/* 			if (NODE8(artr)->keys[i] == *str) { */
/* 				*next = &NODE8(artr)->sub[i]; */
/* 				return 1; */
/* 			} */

/* 		return 0; */
/* 	case ARTR16: */
/* 		for (; i < artr->count; ++i) */
/* 			if (NODE16(artr)->keys[i] == *str) { */
/* 				*next = &NODE16(artr)->sub[i]; */
/* 				return 1; */
/* 			} */

/* 		return 0; */
/* 	case ARTR48: */
/* 		if ((i = NODE48(artr)->keys[*str]) == INVALID_48) */
/* 			return 0; */

/* 		*next = &NODE48(artr)->sub[i]; */
/* 		return 1; */
/* 	case ARTR256: */
/* 		if (!*(tmp = &NODE256(artr)->sub[*str])) */
/* 			return 0; */

/* 		*next = tmp; */
/* 		return 1; */
/* 	case ARTR_EDGE: */
/* 		if (artr->count > size) */
/* 			return 0; */

/* 		str2 = EDGE(artr)->str; */

/* 		for (; i < artr->count; ++str, ++str2, ++i) */
/* 			if (*str != *str2) { */
/* 				*offset = i; */
/* 				return 0; */
/* 			} */

/* 		*next = (Nit_artr **) &artr->val; */

/* 		return artr->count; */
/* 	case ARTR_EDGE_WITH_VAL: */
/* 		printf("%.*s\n", artr->count, EDGE(artr)->str); */
/* 		if (artr->count != size) */
/* 			return 0; */

/* 		str2 = EDGE(artr)->str; */

/* 		for (; i < size; ++str, ++str2, ++i) */
/* 			    if (*str != *str2) { */
/* 				    *offset = i; */
/* 				    return 0; */
/* 			    } */

/* 		return size; */
/* 	} */

/* 	return -1; /\* can never happen, ever *\/ */
/* } */

static void
node_next_update(const uint8_t **str_ref, size_t *size)
{
	++*str_ref;
	--*size;
}

static Nit_artr **
artr_next_level(Nit_artr *artr, const uint8_t **str_ref,
		size_t *size, size_t *offset)
{
	unsigned int i = 0;
	const uint8_t *str = *str_ref;
	const uint8_t *str2;
	Nit_artr **tmp;

	switch (artr->type) {
	case ARTR8:
		for (; i < artr->count; ++i)
			if (NODE8(artr)->keys[i] == *str) {
				node_next_update(str_ref, size);
				return &NODE8(artr)->sub[i];
			}

		return NULL;
	case ARTR16:
		for (; i < artr->count; ++i)
			if (NODE16(artr)->keys[i] == *str) {
				node_next_update(str_ref, size);
				return &NODE16(artr)->sub[i];
			}

		return NULL;
	case ARTR48:
		if ((i = NODE48(artr)->keys[*str]) == INVALID_48)
			return NULL;

		node_next_update(str_ref, size);
		return &NODE48(artr)->sub[i];
	case ARTR256:
		if (!*(tmp = &NODE256(artr)->sub[*str]))
			return NULL;

		node_next_update(str_ref, size);
		return tmp;
	case ARTR_EDGE:
		if (artr->count > *size)
			return NULL;

		str2 = EDGE(artr)->str;

		for (; i < artr->count; ++str, ++str2, ++i)
			if (*str != *str2) {
				*offset = i;
				return NULL;
			}

		*str_ref += artr->count;
		*size -= artr->count;

		return (Nit_artr **) &artr->val;
	case ARTR_EDGE_WITH_VAL:
		if (artr->count != *size)
			return NULL;

		str2 = EDGE(artr)->str;

		for (; i < *size; ++str, ++str2, ++i)
			    if (*str != *str2) {
				    *offset = i;
				    return NULL;
			    }

		*str_ref += artr->count;
		*size -= artr->count;
		/* printf("%.*s\n", artr->count, EDGE(artr)->str); */
		/* printf("%zu\n", *size); */
		return NULL;
	}

	return NULL; /* can never happen, ever */
}


void *
artr_lookup(Nit_artr *artr, const void *dat, size_t size)
{
	size_t offset;
	Nit_artr **next = &artr;
	const uint8_t *str = dat;

	for (; size; artr = *next)
		if (!(next = artr_next_level(artr, &str,
					     &size, &offset))) {
			if (!size) {
				return artr->val;
			}

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

static int
insert_edge_nulled(Nit_artr **artr, const uint8_t *str, size_t size, void *val,
		   Nit_artr_reuse *reuse)
{
	Nit_artr_node8 *replace = get_8(reuse);
	Nit_artr_edge *new_edge;
	uint8_t key = *str;

	ARTR(replace)->val = (*artr)->val;
	recycle_edge(*artr, reuse);
	new_edge = get_edge(reuse, ARTR_EDGE_WITH_VAL, str + 1, size - 1, val);
	insert_8((Nit_artr **) &replace, reuse, key, new_edge);
	*artr = ARTR(replace);
	return 1;
}

static int
insert_edge_no_common(Nit_artr **artr, const uint8_t *str, size_t size, void *val,
		      Nit_artr_reuse *reuse)
{
	Nit_artr_node8 *replace = get_8(reuse);
	Nit_artr_edge *new_edge;
	uint8_t key1 = *str;
	uint8_t key2 = *EDGE(*artr)->str;

	pcheck(replace, 0);
	new_edge = get_edge(reuse, ARTR_EDGE_WITH_VAL,
			    str + 1, size - 1, val);

	if ((*artr)->count == 1 && (*artr)->type != ARTR_EDGE_WITH_VAL) {
		insert_8((Nit_artr **) &replace, reuse, key1, (*artr)->val);
		recycle_8(*artr, reuse);
	} else {
		edge_cut_off_start(EDGE(*artr), 1);
		insert_8((Nit_artr **) &replace, reuse, key2, *artr);
	}

	insert_8((Nit_artr **) &replace, reuse, key1, new_edge);
	*artr = ARTR(replace);
	return 1;

}

static int
edge_insert_common(Nit_artr *artr, const uint8_t *str, size_t size, void *val,
		   size_t offset, Nit_artr_reuse *reuse)
{
	uint8_t key1 = EDGE(artr)->str[offset];
	uint8_t key2 = str[offset];
	Nit_artr_node8 *split = get_8(reuse);
	Nit_artr_edge *old_rest;
	Nit_artr_edge *new_rest;

	pcheck(split, 0);
	old_rest = get_edge(reuse, artr->type, EDGE(artr)->str + 1,
			    artr->count - offset - 1, artr->val);
	new_rest = get_edge(reuse, ARTR_EDGE_WITH_VAL, str + 1,
			    size - offset - 1, val);
	insert_8((Nit_artr **) &split, reuse, key1, old_rest);
	insert_8((Nit_artr **) &split, reuse, key2, new_rest);
	return edge_cut_off_end(EDGE(artr), offset);
}

static int
insert_edge(Nit_artr **artr, const uint8_t *str, size_t size, void *val,
	    size_t offset, Nit_artr_reuse *reuse)
{
	if (!EDGE(*artr)->str)
		return insert_edge_nulled(artr, str, size, val, reuse);

	if (!offset)
		offset = get_offset((*artr)->val, (*artr)->count, str, size);

	if (!offset)
		return insert_edge_no_common(artr, str, size, val, reuse);

	return edge_insert_common(*artr, str, size, val, offset, reuse);
}

static Nit_artr_edge *
remainder_edge(Nit_artr_reuse *reuse, const uint8_t *str,
	       size_t size, void *val)
{
	return get_edge(reuse, NIT_ARTR_EDGE_WITH_VAL,
			str + 1, size - 1, val);
}

static int
insert_here(Nit_artr **artr, const uint8_t *str, size_t size, void *val,
	    size_t offset, Nit_artr_reuse *reuse)
{
	Nit_artr_edge *edge;

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

	(*artr)->val = val;
	return 1;
}
