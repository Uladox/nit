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
insert_8(Nit_artr **artr, Nit_artr_reuse *reuse,
	 uint8_t key, void *val)
{
	/* if (artr->count == 8) { */
	/* 	if (!artr_8_to_16(artr, reuse)) */
	/* 		return 0; */

	/* 	NODE16(*artr)->keys[8] = key; */
	/* 	NODE16(*artr)->sub[8] = val; */
	/* 	++(*artr)->count; */
	/* 	return 1; */
	/* } */

	NODE8(*artr)->keys[(*artr)->count] = key;
	NODE8(*artr)->sub[(*artr)->count++] = val;
	return 1;
}

static int
insert_16(Nit_artr **artr, Nit_artr_reuse *reuse,
	  uint8_t key, void *val)
{
	/* if (artr->count == 16) { */
	/* 	if (!artr_16_to_48(artr, reuse)) */
	/* 		return 0; */

	/* 	NODE48(*artr)->keys[key] = 16; */
	/* 	NODE48(*artr)->sub[16] = val; */
	/* 	++(*artr)->count; */
	/* 	return 1; */
	/* } */

	NODE16(*artr)->keys[(*artr)->count] = key;
	NODE16(*artr)->sub[(*artr)->count++] = val;
	return 1;
}

static int
insert_48(Nit_artr **artr, Nit_artr_reuse *reuse,
	  uint8_t key, void *val)
{
	/* if (artr->count == 48) { */
	/* 	if (!artr_48_to_256(artr, reuse)) */
	/* 		return 0; */

	/* 	NODE256(*artr)->sub[key] = val; */
	/* 	++(*artr)->count; */
	/* 	return 1; */
	/* } */

	NODE48(*artr)->keys[key] = (*artr)->count;
	NODE48(*artr)->sub[(*artr)->count++] = val;
	return 1;
}

static int
insert_256(Nit_artr **artr, Nit_artr_reuse *reuse,
	   uint8_t key, void *val)
{
	NODE256(*artr)->sub[key] = val;
	++(*artr)->count;
	return 1;
}

