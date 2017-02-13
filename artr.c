#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "artr.h"

#define NODE8(VAL)   ((Nit_artr_node8 *) (VAL))
#define NODE16(VAL)  ((Nit_artr_node16 *) (VAL))
#define NODE48(VAL)  ((Nit_artr_node48 *) (VAL))
#define NODE256(VAL) ((Nit_artr_node256 *) (VAL))
#define EDGE(VAL)    ((Nit_artr_edge *) (VAL))

#define INVALID_48 48

void
artr_reuse_init(Nit_artr_reuse *reuse)
{
	memset(reuse, 0, sizeof(*reuse));
}

static Nit_artr_node8 *
get_8(Nit_artr_reuse *reuse)
{
	Nit_artr_node8 *node = reuse->node8s;

        if (!node) {
		node = palloc(node);
		pcheck(node, NULL);
		node->artr.type = ARTR8;
	} else {
		reuse->node8s = node->artr.val;
	}

	node->artr.count = 0;
	node->artr.val = NULL;
	return node;
}

static Nit_artr_node8 *
get_16(Nit_artr_reuse *reuse)
{
	Nit_artr_node16 *node = reuse->node16s;

        if (!node) {
		node = palloc(node);
		pcheck(node, NULL);
		node->artr.type = ARTR16;
	} else {
		reuse->node16s = node->artr.val;
	}

	node->artr.count = 0;
	node->artr.val = NULL;
	return node;
}

static Nit_artr_node48 *
get_48(Nit_artr_reuse *reuse)
{
	Nit_artr_node48 *node = reuse->node48s;

        if (!node) {
		node = palloc(node);
		pcheck(node, NULL);
		node->artr.type = ARTR48;
	} else {
		reuse->node48s = node->artr.val;
	}

	node->artr.count = 0;
	node->artr.val = NULL;
	return node;
}

static Nit_artr_node256 *
get_256(Nit_artr_reuse *reuse)
{
	Nit_artr_node256 *node = reuse->node256s;

        if (!node) {
		node = palloc(node);
		pcheck(node, NULL);
		node->artr.type = ARTR256;
	} else {
		reuse->node256s = node->artr.val;
	}

	node->artr.count = 0;
	node->artr.val = NULL;
	return node;
}

static Nit_artr_edge *
get_edge(Nit_artr_reuse *reuse, Nit_artr_type type, uint8_t *str, size_t count,
	void *val)
{
	Nit_artr_edge *edge = reuse->edges;
	uint8_t *str_cpy = malloc(count);

	pcheck(str_cpy, NULL);
	memcpy(str_cpy, str, count);

	if (!edge) {
		edge = palloc(edge);
		pcheck_c(edge, NULL, free(str_cpy));
		edge->artr.type = type;
	} else {
		reuse->edges = edge->artr.val;
	}

	edge->str = str_cpy;
	edge->artr.count = count;
	edge->artr.val = val;
	return edge
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

/* Returns bytes to move past, if it's zero, we can't go further.
 * The offset is used when the artr is an edge with a number of bytes such that
 * str could go on to the next part of the trie, but does not since the bytes
 * are not equal to those in str. This sets offset to the number of bytes that
 * are equal. Next needs to be tree stars (despite that typically meaning you
 * are doing something horribly wrong) since we need to get a refernce to the
 * array index so that we can replace the pointer in it with another one when
 *  resizeing. It's not as bad as it seems.
 */
static inline size_t
artr_move_one_level(Nit_artr *artr, Nit_artr ***next,
		    const uint8_t *str, size_t size, size_t *offset)
{
	unsigned int i = 0;
	const uint8_t *str2;
	Nit_artr **tmp;

	switch (artr->type) {
	case ARTR8:
		for (; i < artr->count; ++i)
			if (NODE8(artr)->keys[i] == *str) {
				*next = &NODE8(artr)->sub[i];
				return 1;
			}

		return 0;
	case ARTR16:
		for (; i < artr->count; ++i)
			if (NODE16(artr)->keys[i] == *str) {
				*next = &NODE16(artr)->sub[i];
				return 1;
			}

		return 0;
	case ARTR48:
		if ((i = NODE48(artr)->keys[*str]) == INVALID_48)
			return 0;

		*next = &NODE48(artr)->sub[i];
		return 1;
	case ARTR256:
		if (!*(tmp = &NODE256(artr)->sub[*str]))
			return 0;

		*next = tmp;
		return 1;
	case ARTR_EDGE:
		if (artr->count > size)
			return 0;

		str2 = EDGE(artr)->str;

		for (; i < artr->count; ++str, ++str2, ++i)
			if (*str != *str2) {
				*offset = i;
				return 0;
			}

		*next = (Nit_artr **) &artr->val;

		return artr->count;
	case ARTR_EDGE_WITH_VAL:
		if (artr->count != size)
			return 0;

		str2 = EDGE(artr)->str;

		for (; i < size; ++str, ++str2, ++i)
			    if (*str != *str2) {
				    *offset = i;
				    return 0;
			    }

		return size;
	}

	return -1; /* can never happen, ever */
}

void *
artr_lookup(Nit_artr *artr, const void *dat, size_t size)
{
	size_t tmp;
	size_t offset;
	Nit_artr **next = NULL;
	const uint8_t *str = dat;

	for (; size; artr = *next) {
		if (!(tmp = artr_move_one_level(artr, &next, str,
						size, &offset)))
			return NULL;

		str += tmp;
		size -= tmp;
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
insert_8(Nit_artr **artr, Nit_artr_reuse *reuse,
	 uint8_t *str, size_t size, void *val)
{
	uint8_t key = *str;
	Nit_edge *edge = get_edge(reuse, NIT_ARTR_EDGE_WITH_VAL,
				  str + 1, size - 1, val);

	pcheck(edge, 0);

	/* if (artr->count == 8) { */
	/* 	if (!artr_8_to_16(artr, reuse)) */
	/* 		return 0; */

	/* 	NODE16(*artr)->keys[8] = key; */
	/* 	NODE16(*artr)->sub[8] = val; */
	/* 	++(*artr)->count; */
	/* 	return 1; */
	/* } */

	NODE8(*artr)->keys[(*artr)->count] = key;
	NODE8(*artr)->sub[(*artr)->count++] = edge;
	return 1;
}

static int
insert_16(Nit_artr **artr, Nit_artr_reuse *reuse,
	  uint8_t *str, size_t size, void *val)
{
	uint8_t key = *str;
	Nit_edge *edge = get_edge(reuse, NIT_ARTR_EDGE_WITH_VAL,
				  str + 1, size - 1, val);

	pcheck(edge, 0);

	/* if (artr->count == 16) { */
	/* 	if (!artr_16_to_48(artr, reuse)) */
	/* 		return 0; */

	/* 	NODE48(*artr)->keys[key] = 16; */
	/* 	NODE48(*artr)->sub[16] = val; */
	/* 	++(*artr)->count; */
	/* 	return 1; */
	/* } */

	NODE16(*artr)->keys[(*artr)->count] = key;
	NODE16(*artr)->sub[(*artr)->count++] = edge;
	return 1;
}

static int
insert_48(Nit_artr **artr, Nit_artr_reuse *reuse,
	  uint8_t *str, size_t size, void *val)
{
	uint8_t key = *str;
	Nit_edge *edge = get_edge(reuse, NIT_ARTR_EDGE_WITH_VAL,
				  str + 1, size - 1, val);

	pcheck(edge, 0);

	/* if (artr->count == 48) { */
	/* 	if (!artr_48_to_256(artr, reuse)) */
	/* 		return 0; */

	/* 	NODE256(*artr)->sub[key] = val; */
	/* 	++(*artr)->count; */
	/* 	return 1; */
	/* } */

	NODE48(*artr)->keys[key] = (*artr)->count;
	NODE48(*artr)->sub[(*artr)->count++] = edge;
	return 1;
}

static int
insert_256(Nit_artr **artr, Nit_artr_reuse *reuse,
	   uint8_t *str, size_t size, void *val)
{
	uint8_t key = *str;
	Nit_edge *edge = get_edge(reuse, NIT_ARTR_EDGE_WITH_VAL,
				  str + 1, size - 1, val);

	pcheck(edge, 0);

	NODE256(*artr)->sub[key] = val;
	++(*artr)->count;
	return 1;
}

static int
insert_edge(Nit_artr **artr, const uint8_t *str, size_t size, void *val,
	    size_t *offset, Nit_artr_reuse *reuse)
{
	Nit_artr_node8 *replace = get_8(reuse);

	pcheck(replace, 0);

	if (!*offset)
		*offset = get_offset((*artr)->val, (*artr)->count, str, size);

	if (!*offset) {
		Nit_artr_edge *new;
		uint8_t *key1 = *EDGE(*artr)->str;
		uint8_t *key2 = *str;

		pcheck(replace, 0);

		if ((*artr)->count - 1 == 0) {
			Nit_artr *lower = (*artr)->val;

			new = get_edge(reuse, ARTR_EDGE_WITH_VAL);
			new->artr.count = size - 1;

			if (size - 1 > 0) {
				new->str = malloc(size - 1);
				memcpy(new->str, str + 1, size - 1);
			} else {
				new->str = NULL;
			}

			insert_8(replace, lower);
			insert_8();
		}

		char *lesser = malloc();
		Nit_artr *new = get_edge(reuse, ARTR_EDGE_WITH_VAL);

		new->artr.val = val;
		

		pcheck(tmp, 0);
	}

	uint8_t *common = malloc(*offset);
	uint8_t *diff1 = malloc((*artr)->count - *offset);
	uint8_t *diff2 = malloc(size - *offset);

	memcpy(common, str, *offset);
	memcpy(diff1, str, (*artr)->count - *offset);
	memcpy(diff2, str, size - *offset);
}

static int
insert_here(Nit_artr **artr, const uint8_t *str, size_t size, void *val,
	    size_t *offset, Nit_artr_reuse *reuse)
{
	switch ((*artr)->type) {
	case ARTR8:
		return insert_8(artr, reuse, str, size, val);
	case ARTR16:
		return insert_16(artr, reuse, str, size, val);
	case ARTR48:
		return insert_48(artr, reuse, str, size, val);
	case ARTR256:
		return insert_256(artr, reuse, str, size, val);
	case ARTR_EDGE:
		return insert_edge();
	case ARTR_EDGE_WITH_VAL:
		break;
	}

	return 0;
}

int
artr_insert(Nit_artr **artr, const void *dat, size_t size, void *val,
	    Nit_artr_reuse *reuse)
{
	Nit_artr **next = NULL;
	size_t tmp;
	size_t offset = 0;
	const uint8_t *str = dat;

	for (; size; artr = next) {
		if (!(tmp = artr_move_one_level(*artr, &next,
						str, size, &offset)))
			return insert_here(artr, str, size, val,
					   &offset, reuse);

		str += tmp;
		size -= tmp;
	}

	(*artr)->val = val;
	return 1;
}
