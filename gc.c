#include <stdlib.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "list.h"
#include "gc.h"


static void *
nit_gc_place(Nit_gc *gc, void *data)
{
	Nit_gclist *list;

	if (gc->free) {
		list = gc->free;
		gc->free = DLIST_PREV(gc->free);
		goto end;
	}

	pcheck_c((list = palloc(list)), NULL, free(data));

	if (gc->check_end) {
		dlist_put_after(list, gc->check_end);
	} else {
		gc->check = gc->check_end = list;
		LIST_CONS(list, NULL);
		DLIST_RCONS(list, NULL);
	}

end:
	list->color = !gc->white;
	*((Nit_gclist **) data) = list;
	list->data = ((Nit_gclist **) data) + 1;
	return list->data;
}

void *
nit_gc_malloc(Nit_gc *gc, size_t size)
{
	void *data = malloc(size + sizeof(Nit_gclist *));

	pcheck(data, NULL);

	return nit_gc_place(gc, data);
}

void *
nit_gc_calloc(Nit_gc *gc, size_t size)
{
        void *data = calloc(size + sizeof(Nit_gclist *), 1);

	pcheck(data, NULL);

	return nit_gc_place(gc, data);
}

void
nit_gc_free(Nit_gc *gc, void *data)
{
	Nit_gclist **place = ((Nit_gclist **) data) - 1;
	Nit_gclist *list = *place;

	free(place);

	if (gc->free) {
		dlist_move_b(list, gc->free);
		return;
	}

	gc->free = list;
	DLIST_RCONS(list, NULL);
	LIST_CONS(list, NULL);
}

static inline Nit_gclist *
get_list(void *data)
{
	return *((Nit_gclist **) data) - 1;
}

void
nit_gc_mark(Nit_gc *gc, void *data)
{
	Nit_gclist *list = get_list(data);

	if (list->color == gc->white) {
		list->color = !gc->white;
	        place_check(gc, list);
	}
}

void
nit_gc_collect_1(Nit_gc *gc)
{
	if (gc->check == gc->scan) {
		if (gc->uncheck == gc->scan) {
			gc->white = !gc->white;
			return;
		}
	}

	void *data = gc->next(DLIST_PREV(gc->check), gc->iter);

	if (!data) {
		gc->check = DLIST_PREV(gc->check);
	}
}
