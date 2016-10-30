/* Include these
 * #include "list.h"
 * #include "urlist_gen.h"
 */

/* #define NIT_ARR_TYPE void * */

/* #undef NIT_ARR_TYPE */

typedef struct {
	Nit_list list;
	int count;
	void *arr[4];
} Nit_urlist4;

Nit_urlist4 *
urlist4_new(void);

void *
urlist4_first(Nit_urlist4 *list);

void *
urlist4_last(Nit_urlist4 *list);

int
urlist4_insert(Nit_urlist4 *list, void *val);
