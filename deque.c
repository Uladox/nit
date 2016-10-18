#include <stdlib.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "list.h"
#include "deque.h"

Nit_deque *
deque_new(void)
{
	Nit_deque *deq = palloc(deq);

	pcheck(deq, NULL);
	deq->front = NULL;
	deq->end = NULL;

	return deq;
}

void
deque_free(Nit_deque *deq, Nit_deque_free list_free)
{
	Nit_dlist *tmp;
	Nit_dlist *list = deq->front;

	delayed_foreach(tmp, list)
		list_free(tmp);

	free(deq);
}

void
deque_push(Nit_deque *deq, Nit_dlist *list)
{
	DLIST_RCONS(list, NULL);
	dlist_connect(list, deq->front);
}

void
deque_append(Nit_deque *deq, Nit_dlist *list)
{
	LIST_CONS(list, NULL);
	dlist_rconnect(list, deq->end);
}

Nit_dlist *
deque_pop(Nit_deque *deq)
{
	Nit_dlist *list = deq->front;

	if (!list)
		return NULL;

	deq->front = LIST_NEXT(list);
	DLIST_RCONS(deq->front, NULL);

	if (!deq->end)
		deq->end = deq->front;

	return list;
}

Nit_dlist *
deque_rpop(Nit_deque *deq)
{
	Nit_dlist *list = deq->end;

	if (!list)
		return NULL;

	deq->end = DLIST_PREV(list);
	DLIST_RCONS(deq->end, NULL);

	if (!deq->front)
		deq->front = deq->end;

	return list;
}
