/*    This file is part of nitlib.
 *
 *    Nitlib is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Foobar is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

typedef struct {
	void *next;
} Nit_list;

typedef struct {
	Nit_list list;
	void *prev;
} Nit_dlist;

#define NIT_LIST(LIST)				\
	((Nit_list *) (LIST))
#define NIT_LIST_NEXT(LIST)			\
	((typeof(LIST)) (NIT_LIST(LIST)->next))
#define NIT_LIST_CONS(LIST, END)		\
	(NIT_LIST(LIST)->next = (END))
#define NIT_NEXT_REF(LIST)			\
	((typeof(LIST) *) &NIT_LIST(LIST)->next)
#define nit_foreach(LIST)			\
	for (; LIST; LIST = NIT_LIST_NEXT(LIST))
#define nit_delayed_foreach(TMP, LIST)					\
	for (TMP = LIST, LIST ? (LIST = NIT_LIST_NEXT(LIST)) : NULL;	\
	     TMP;							\
	     TMP = LIST, LIST ? (LIST = NIT_LIST_NEXT(LIST)) : NULL)

#define NIT_DLIST(LIST)				\
	((Nit_dlist *) LIST)
#define NIT_DLIST_PREV(LIST)			\
	((typeof(LIST)) (NIT_DLIST(LIST)->prev))
#define NIT_DLIST_RCONS(LIST, BEGIN)		\
	(NIT_DLIST(LIST)->prev = (BEGIN))
#define NIT_PREV_REF(LIST)				\
	((typeof(LIST) *) &NIT_DLIST(LIST)->prev)
#define nit_preveach(LIST)				\
	for (; LIST; LIST = NIT_DLIST_PREV(LIST))
#define nit_delayed_preveach(TMP, LIST)					\
	for (TMP = LIST, LIST ? (LIST = NIT_DLIST_PREV(LIST)) : NULL;	\
	     TMP;							\
	     TMP = LIST, LIST ? (LIST = NIT_DLIST_PREV(LIST)) : NULL)

static inline void
nit_dlist_remove(void *list)
{
	/*             +----+
	 *             |list|
	 *             +----+
	 *
	 * +----------+ ---> +----------+
	 * |list->prev|      |list->next|
	 * +----------+ <--- +----------+
	 */
	NIT_LIST_CONS(NIT_DLIST_PREV(list), NIT_LIST_NEXT(list));
	NIT_DLIST_RCONS(NIT_LIST_NEXT(list), NIT_DLIST_PREV(list));
}

static inline void
nit_dlist_put_after(void *next, void *first)
{
	/*  +-----+ ---> +----+
	 *  |first|      |next|
	 *  +-----+ <--- +----+
	 */

	NIT_DLIST_RCONS(next, first);
	NIT_LIST_CONS(next, NIT_LIST_NEXT(first));
	NIT_LIST_CONS(first, next);

	/* +----+      +----------+
	 * |next| <--- |next->next|
	 * +----+      +----------+
	 */

	if (NIT_LIST_NEXT(next))
		NIT_DLIST_RCONS(NIT_LIST_NEXT(next), next);
}

static inline void
nit_dlist_put_before(void *first, void *next)
{
	/* +----------+      +-----+
	 * |next->prev| ---> |first|
	 * +----------+      +-----+
	 */

	if (NIT_DLIST_PREV(next))
		NIT_LIST_CONS(NIT_DLIST_PREV(next), next);

	/*  +-----+ ---> +----+
	 *  |first|      |next|
	 *  +-----+ <--- +----+
	 */

	NIT_LIST_CONS(first, next);
	NIT_DLIST_RCONS(first, NIT_DLIST_PREV(next));
	NIT_DLIST_RCONS(next, first);
}

static inline void
nit_dlist_move_a(void *next, void *first)
{
	nit_dlist_remove(next);
	nit_dlist_put_after(next, first);
}

static inline void
nit_dlist_move_b(void *first, void *next)
{
	nit_dlist_remove(first);
	nit_dlist_put_before(first, next);
}

#if defined NIT_SHORT_NAMES || defined NIT_LIST_SHORT_NAMES
# define LIST_NEXT(...)        NIT_LIST_NEXT(__VA_ARGS__)
# define LIST_CONS(...)        NIT_LIST_CONS(__VA_ARGS__)
# define NEXT_REF(...)         NIT_NEXT_REF(__VA_ARGS__)
# define foreach(...)          nit_foreach(__VA_ARGS__)
# define delayed_foreach(...)  nit_delayed_foreach(__VA_ARGS__)
# define DLIST_PREV(...)       NIT_DLIST_PREV(__VA_ARGS__)
# define DLIST_RCONS(...)      NIT_DLIST_RCONS(__VA_ARGS__)
# define PREV_REF(...)         NIT_PREV_REF(__VA_ARGS__)
# define preveach(...)         nit_preveach(__VA_ARGS__)
# define delayed_preveach(...) nit_delayed_preveach(__VA_ARGS__)
# define dlist_put_after(...)  nit_dlist_put_after(__VA_ARGS__)
# define dlist_remove(...)     nit_dlist_remove(__VA_ARGS__)
# define dlist_move_a(...)     nit_dlist_move_a(__VA_ARGS__)
# define dlist_move_b(...)     nit_dlist_move_b(__VA_ARGS__)
#endif
