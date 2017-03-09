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

static Nit_artr_node16 *
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
get_edge(Nit_artr_reuse *reuse, Nit_artr_type type, const uint8_t *str,
	 size_t count, void *val)
{
	Nit_artr_edge *edge = reuse->edges;
	uint8_t *str_cpy = NULL;

	if (count) {
		str_cpy = malloc(count);
		pcheck(str_cpy, NULL);
		memcpy(str_cpy, str, count);
	}

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
	return edge;
}

static void
recycle_8(Nit_artr *artr, Nit_artr_reuse *reuse)
{
	artr->val = reuse->node8s;
	reuse->node8s = NODE8(artr);
}

static void
recycle_16(Nit_artr *artr, Nit_artr_reuse *reuse)
{
	artr->val = reuse->node16s;
	reuse->node16s = NODE16(artr);
}

static void
recycle_48(Nit_artr *artr, Nit_artr_reuse *reuse)
{
	artr->val = reuse->node48s;
	reuse->node48s = NODE48(artr);
}

static void
recycle_256(Nit_artr *artr, Nit_artr_reuse *reuse)
{
	artr->val = reuse->node256s;
	reuse->node256s = NODE256(artr);
}

static void
recycle_edge(Nit_artr *artr, Nit_artr_reuse *reuse)
{
	free(EDGE(artr)->str);
	artr->val = reuse->edges;
	reuse->edges = EDGE(artr);
}

static int
artr_8_to_16(Nit_artr **artr, Nit_artr_reuse *reuse)
{
	Nit_artr_node16 *new = get_16(reuse);

	pcheck(new, 0);
	memcpy(new, *artr, sizeof(**artr));
	ARTR(new)->type = ARTR16;
	memcpy(new->keys, NODE8(*artr)->keys, sizeof(NODE8(*artr)->keys));
	memcpy(new->sub, NODE8(*artr)->sub, sizeof(NODE8(*artr)->sub));
	recycle_8(*artr, reuse);
	*artr = ARTR(new);
	return 1;
}

static int
insert_8(Nit_artr **artr, Nit_artr_reuse *reuse,
	 uint8_t key, void *val)
{
	if ((*artr)->count == 8) {
		if (!artr_8_to_16(artr, reuse))
			return 0;

		NODE16(*artr)->keys[8] = key;
		NODE16(*artr)->sub[8] = val;
		++(*artr)->count;
		return 1;
	}

	NODE8(*artr)->keys[(*artr)->count] = key;
	NODE8(*artr)->sub[(*artr)->count++] = val;
	return 1;
}

static int
artr_16_to_48(Nit_artr **artr, Nit_artr_reuse *reuse)
{
	Nit_artr_node48 *new = get_48(reuse);
	int i = 0;

	pcheck(new, 0);
	memcpy(new, *artr, sizeof(**artr));
	ARTR(new)->type = ARTR48;

	for (; i < 256; ++i)
		new->keys[i] = INVALID_48;

	for (i = 0; i < 16; ++i) {
		new->keys[NODE16(*artr)->keys[i]] = i;
		new->sub[i] = NODE16(*artr)->sub[i];
	}

	recycle_16(*artr, reuse);
	*artr = ARTR(new);
	return 1;
}

static int
insert_16(Nit_artr **artr, Nit_artr_reuse *reuse,
	  uint8_t key, void *val)
{
	if ((*artr)->count == 16) {
		if (!artr_16_to_48(artr, reuse))
			return 0;

		NODE48(*artr)->keys[key] = 16;
		NODE48(*artr)->sub[16] = val;
		++(*artr)->count;
		return 1;
	}

	NODE16(*artr)->keys[(*artr)->count] = key;
	NODE16(*artr)->sub[(*artr)->count++] = val;
	return 1;
}

static int
artr_48_to_256(Nit_artr **artr, Nit_artr_reuse *reuse)
{
	Nit_artr_node256 *new = get_256(reuse);
	uint8_t index;
	int i = 0;

	pcheck(new, 0);
	memcpy(new, *artr, sizeof(**artr));
	ARTR(new)->type = ARTR256;
	memset(new->sub, 0, sizeof(new->sub));

	for (; i < 256; ++i)
		if ((index = NODE48(*artr)->keys[i]) != INVALID_48) {
			new->sub[i] = NODE48(*artr)->sub[index];
		}

	recycle_48(*artr, reuse);
	*artr = ARTR(new);
	return 1;
}


static int
insert_48(Nit_artr **artr, Nit_artr_reuse *reuse,
	  uint8_t key, void *val)
{
	if ((*artr)->count == 48) {
		if (!artr_48_to_256(artr, reuse))
			return 0;

		NODE256(*artr)->sub[key] = val;
		++(*artr)->count;
		return 1;
	}

	NODE48(*artr)->keys[key] = (*artr)->count;
	NODE48(*artr)->sub[(*artr)->count++] = val;
	return 1;
}

static int
insert_256(Nit_artr **artr, uint8_t key, void *val)
{
	NODE256(*artr)->sub[key] = val;
	++(*artr)->count;
	return 1;
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
insert_edge_before(Nit_artr **artr, const uint8_t *str, size_t size, void *val,
		   Nit_artr_reuse *reuse)
{
	Nit_artr_node8 *replace = get_8(reuse);
	Nit_artr_edge *new_edge;
	uint8_t key1 = *str;
	uint8_t key2 = *EDGE(*artr)->str;

	pcheck(replace, 0);
	new_edge = get_edge(reuse, ARTR_EDGE_WITH_VAL, str, size, val);

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
edge_insert_middle(Nit_artr *artr, const uint8_t *str, size_t size, void *val,
		   size_t offset, Nit_artr_reuse *reuse)
{
	uint8_t key1 = EDGE(artr)->str[offset];
	uint8_t key2 = str[offset];
	Nit_artr_node8 *split = get_8(reuse);
	Nit_artr_edge *old_rest;
	Nit_artr_edge *new_rest;

	pcheck(split, 0);
	old_rest = get_edge(reuse, artr->type, EDGE(artr)->str,
			    artr->count - offset, artr->val);
	new_rest = get_edge(reuse, ARTR_EDGE_WITH_VAL, str,
			    size - offset, val);
	insert_8((Nit_artr **) &split, reuse, key1, old_rest);
	insert_8((Nit_artr **) &split, reuse, key2, new_rest);
	return edge_cut_off_end(EDGE(artr), offset);
}

/* static int */
/* insert_after_edge() */
/* { */
/* 	if (!artr->str) */
		
/* } */

/* static int */
/* edge_insert_before(Nit_artr *artr, void *val, */
/* 		   size_t offset, Nit_artr_reuse *reuse) */
/* { */
/* 	Nit_artr_node8 *replace = get_8(reuse); */
/* 	uint8_t key = *str; */

/* 	ARTR(replace)->val = (*artr)->val; */
/* 	recycle_edge(*artr, reuse); */
/* 	new_edge = get_edge(reuse, ARTR_EDGE_WITH_VAL, str + 1, size - 1, val); */
/* 	insert_8((Nit_artr **) &replace, reuse, key, new_edge); */
/* 	*artr = ARTR(replace); */
/* 	return 1; */
/* } */

/* static int */
/* insert_edge(Nit_artr **artr, const uint8_t *str, size_t size, void *val, */
/* 	    size_t offset, Nit_artr_reuse *reuse) */
/* { */
/* 	if (!offset) */
/* 		offset = get_offset(EDGE(*artr)->str, (*artr)->count, str, size); */

/* 	if (size < offset) */
/* 		edge_insert_before(artr, val, ) */

/* 	if (!EDGE(*artr)->str) */
/* 		return insert_edge_nulled(artr, str, size, val, reuse); */

/* 	if (!offset) */
/* 		return insert_edge_no_common(artr, str, size, val, reuse); */

/* 	return edge_insert_common(*artr, str, size, val, offset, reuse); */
/* } */
