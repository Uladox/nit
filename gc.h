/* Include these
   #include <stddef.h> or <stdio.h> or <stdlib.h> something defining size_t
   #include "list.h"
 */

typedef struct {
	Nit_dlist list;
	int mark;
	void *data;
} Nit_gclist;

typedef struct {
        int mark;
	Nit_gclist *scan;
	Nit_gclist *scan_end;
	Nit_gclist *check;
	Nit_gclist *uncheck;
	Nit_gclist *free;

	/* Iterator for real-time gc. */
	void *iter;
	/* Marks returned pointer. */
	void *(*next)(void *scan, void *iter);
} Nit_gc;

/* setup */

Nit_gc *
nit_gc_new(void *iter,
	   void *(*next)(void *scan, void *iter));

int
nit_gc_free(Nit_gc *gc);

/* alloc */

void *
nit_gc_malloc(Nit_gc *gc, size_t size);

void *
nit_gc_calloc(Nit_gc *gc, size_t size);

/* freeing */

void *
nit_gc_collect_1(Nit_gc *gc);

void
nit_gc_reclaim(Nit_gc *gc, void *data);

/* scanning */

void
nit_gc_mark(Nit_gc *gc, void *data);

int
nit_gc_scan_1(Nit_gc *gc);

int
nit_gc_restart(Nit_gc *gc, void *data);

#if defined NIT_SHORT_NAMES || defined NIT_GC_SHORT_NAMES
# define gc_new(...)       nit_gc_new(__VA_ARGS__)
# define gc_free(...)      nit_gc_free(__VA_ARGS__)
# define gc_malloc(...)    nit_gc_malloc(__VA_ARGS__)
# define gc_calloc(...)    nit_gc_calloc(__VA_ARGS__)
# define gc_collect_1(...) nit_gc_collect_1(__VA_ARGS__)
# define gc_reclaim(...)   nit_gc_reclaim(__VA_ARGS__)
# define gc_mark(...)      nit_gc_mark(__VA_ARGS__)
# define gc_scan_1(...)    nit_gc_scan_1(__VA_ARGS__)
# define gc_restart(...)   nit_gc_restart(__VA_ARGS__)
#endif
