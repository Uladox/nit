/* Include these
 * #include <stddef.h> or <stdio.h> or <stdlib.h>
 * #include "list.h"
 */

typedef struct {
	Nit_dlist list;
	int mark;
	void *data;
} Nit_gclist;

typedef struct {
        int mark;
	Nit_gclist *scan;
	Nit_gclist *check;
	Nit_gclist *check_end;
	Nit_gclist *free;
	Nit_gclist *uncheck;

	int (*dat_free)(void *data); /* Frees set data, not pointer. */

	void *iter; /* Iterator for real-time gc. */
	/* Marks returned pointer. */
	void *(*next)(Nit_gclist *scan, void *iter);
} Nit_gc;

void *
nit_gc_malloc(Nit_gc *gc, size_t size);

void *
nit_gc_calloc(Nit_gc *gc, size_t size);

void
nit_gc_reclaim(Nit_gc *gc, void *data);

void
nit_gc_collect_1(Nit_gc *gc);
