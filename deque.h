/* Include these
 * #include "list.h"
 */

typedef struct {
	Nit_dlist *front;
	Nit_dlist *end;
} Nit_deque;

typedef void(*Nit_deque_free)(Nit_dlist *list);

Nit_deque *
nit_deque_new(void);

void
nit_deque_free(Nit_deque *deq, Nit_deque_free list_free);

void
nit_deque_push(Nit_deque *deq, Nit_dlist *list);

void
nit_deque_append(Nit_deque *deq, Nit_dlist *list);

Nit_dlist *
nit_deque_pop(Nit_deque *deq);

Nit_dlist *
nit_deque_rpop(Nit_deque *deq);

#if defined NIT_SHORT_NAMES || defined NIT_DEQUE_SHORT_NAMES
# define deque_new(...)    nit_deque_new(__VA_ARGS__)
# define deque_free(...)   nit_deque_free(__VA_ARGS__)
# define deque_push(...)   nit_deque_push(__VA_ARGS__)
# define deque_append(...) nit_deque_append(__VA_ARGS__)
# define deque_pop(...)    nit_deque_pop(__VA_ARGS__)
# define deque_rpop(...)   nit_deque_rpop(__VA_ARGS__)
#endif
