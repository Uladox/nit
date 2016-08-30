
enum nit_gc_ercu {
	NIT_GC_COLOR1,
	NIT_GC_COLOR2
};

typedef struct {
	Nit_dlist list;
	enum nit_gc_ercu color;
	void *data;
} Nit_gclist;

/* Nit garbage collector cycle
 *                                  <--
 *           Scan               Scan   Checked
 *          /    \                  \ /
 * Unchecked      Checked            X
 *          \    /                  / \
 *           Free          Unchecked   Free
 *                                  <--
 */

typedef struct {
	enum nit_gc_ercu black;
	Nit_gclist *scan;    /* grey */
	Nit_gclist *check;   /* black */
	Nit_gclist *free;    /* white */
	Nit_gclist *uncheck; /* ercu */
	int (*dat_free)(void *data);
} Nit_gc;

void *
nit_gc_alloc(Nit_gc *gc, size_t size)
{
	Nit_gclist *list;
	void *data = malloc(size + sizeof(list));

	pcheck(data, NULL);

	if (gc->free != gc->uncheck) {
		list = gc->free;
		gc->free = DLIST_PREV(gc->free);
		goto end;
	}

	pcheck_c((list = palloc(list)), NULL, free(data));
	dlist_put_after(gc->free, list);

end:
	list->color = gc->black;
	*((Nit_gclist **) data) = list;
	list->data = ((Nit_gclist **) data) + 1;
	return list->data;
}
