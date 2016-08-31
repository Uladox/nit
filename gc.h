/* Include these
 * #include <stddef.h> or <stdio.h> or <stdlib.h>
 * #include "list.h"
 */

/*               Nit garbage collector
 *
 *        -|Regions|-           -|Pointers|-
 *
 *                                  <-- [puts scanned into checked]
 *         < scan               scan   check
 *          /    \                  \ /
 * v uncheck      check ^            X
 *          \    /                  / \
 *           free >          uncheck   free
 *                                  <-- [puts free into checked]
 *
 *          < scan
 *              |
 *     [uncheck]|[scan]
 * v uncheck----+---- check ^
 *        [free]|[check]
 *              |
 *            free >
 *
 *       < scan/check          < uncheck/scan/check
 *             |                          |
 *            .^.         ===>            ^.
 *           /	 \                          \
 *  v uncheck     free ^                     free ^
 */

typedef struct {
	Nit_dlist list;
	int color;
	void *data;
} Nit_gclist;

typedef struct {
        int white;
	Nit_gclist *scan;    /* grey */
	Nit_gclist *check;   /* black */
	Nit_gclist *free;    /* white */
	Nit_gclist *uncheck; /* ercu */

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
nit_gc_free(Nit_gc *gc, void *data);

void
nit_gc_collect_1(Nit_gc *gc);
