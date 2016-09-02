#include <stdlib.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "list.h"
#include "gc.h"

/* setup */

Nit_gc *
gc_new(void *iter,
       void *(*next)(void *scan, void *iter))
{
	Nit_gc *gc = palloc(gc);

	gc->mark = 0;
	gc->scan = NULL;
	gc->scan_end = NULL;
	gc->check = NULL;
	gc->uncheck = NULL;
	gc->free = NULL;

	gc->iter = iter;
	gc->next = next;

	return gc;
}

int
gc_free(Nit_gc *gc)
{
	Nit_gclist *list = gc->free;
	Nit_gclist *tmp;

	if (gc->scan || gc->scan_end || gc->check || gc->uncheck)
		return 1;

	delayed_foreach (tmp, list)
		free(tmp);

	free(gc);

	return 0;
}

/* alloc */

static void *
gc_add(Nit_gc *gc, void *data)
{
	Nit_gclist *list;

	if (gc->free) {
		list = gc->free;
		gc->free = DLIST_PREV(gc->free);
		goto end;
	}

	pcheck_c((list = palloc(list)), NULL, free(data));

	if (gc->scan_end) {
		dlist_put_after(list, gc->scan_end);
	} else {
		gc->scan = gc->scan_end = list;
		LIST_CONS(list, NULL);
		DLIST_RCONS(list, NULL);
	}

end:
	list->mark = !gc->mark;
	((Nit_gclist **) data)[0] = list;
	list->data = ((Nit_gclist **) data) + 1;

	return list->data;
}

void *
gc_malloc(Nit_gc *gc, size_t size)
{
	void *data = malloc(size + sizeof(Nit_gclist *));

	pcheck(data, NULL);

	return gc_add(gc, data);
}

void *
gc_calloc(Nit_gc *gc, size_t size)
{
        void *data = calloc(size + sizeof(Nit_gclist *), 1);

	pcheck(data, NULL);

	return gc_add(gc, data);
}

/* freeing */

static void
gc_place(Nit_gclist *list, Nit_gclist **place)
{
	if (*place) {
		dlist_move_b(list, *place);
		*place = list;
		return;
	}

	dlist_remove(list);
	*place = list;
	DLIST_RCONS(list, NULL);
	LIST_CONS(list, NULL);
}

static void
uncheck_place(Nit_gc *gc, Nit_gclist *list, Nit_gclist **place)
{
	if (list == gc->uncheck)
		gc->uncheck = LIST_NEXT(list);

	gc_place(list, place);
}

static void
unchech_place_scan(Nit_gc *gc, Nit_gclist *list)
{
	uncheck_place(gc, list, &gc->scan_end);

	if (!gc->scan)
		gc->scan = gc->scan_end;
}

void *
gc_collect_1(Nit_gc *gc)
{
	if (gc->scan || !gc->uncheck)
		return NULL;

	return gc->uncheck->data;
}

void
gc_reclaim(Nit_gc *gc, void *data)
{
	Nit_gclist **start = ((Nit_gclist **) data) - 1;
	Nit_gclist *list = *start;

	free(start);
	uncheck_place(gc, list, &gc->free);
}

/* scanning */

static inline Nit_gclist *
get_list(void *data)
{
	return ((Nit_gclist **) data)[-1];
}

static void
scan(Nit_gc *gc, Nit_gclist *list)
{
	if (list->mark == gc->mark)
		return;

	list->mark = gc->mark;
	unchech_place_scan(gc, list);
}

void
gc_mark(Nit_gc *gc, void *data)
{
        scan(gc, get_list(data));
}

int
gc_scan_1(Nit_gc *gc)
{
	Nit_gclist *list;
	void *data;

	if (!gc->scan)
		return 1;

	if ((data = gc->next(gc->scan->data, gc->iter)))
		scan(gc, get_list(data));

	list = LIST_NEXT(gc->scan);
	gc_place(gc->scan, &gc->check);
	gc->scan = list;

	if (list)
		return 0;

	gc->scan_end = NULL;

	return 1;
}

int
gc_restart(Nit_gc *gc, void *data)
{
	if (gc->uncheck)
		return 1;

	gc->mark = !gc->mark;
	gc->uncheck = gc->check;
	gc->check = NULL;

	if (data)
		gc_mark(gc, data);

	return 0;
}
