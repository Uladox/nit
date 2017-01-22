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

#define NIT_LIST_INC(LIST)			\
	(LIST = NIT_LIST_NEXT(LIST))

#define NIT_LIST_CONS(LIST, END)		\
	(NIT_LIST(LIST)->next = (END))

#define NIT_NEXT_REF(LIST)			\
	((typeof(LIST) *) &NIT_LIST(LIST)->next)

#define nit_foreach(LIST)			\
	for (; LIST; LIST = NIT_LIST_NEXT(LIST))

#define nit_delayed_foreach(LIST)					\
	typeof(LIST) TMP = LIST ? LIST_NEXT(LIST) : NULL;		\
	for (; LIST;							\
	     LIST = TMP, TMP = TMP ? LIST_NEXT(TMP) : NULL)

#define NIT_DLIST(LIST)				\
	((Nit_dlist *) LIST)

#define NIT_DLIST_PREV(LIST)			\
	((typeof(LIST)) (NIT_DLIST(LIST)->prev))

#define NIT_DLIST_DEC(LIST)			\
	(LIST = NIT_DLIST_PREV(LIST))

#define NIT_DLIST_RCONS(LIST, BEGIN)		\
	(NIT_DLIST(LIST)->prev = (BEGIN))

#define NIT_PREV_REF(LIST)				\
	((typeof(LIST) *) &NIT_DLIST(LIST)->prev)

#define nit_preveach(LIST)				\
	for (; LIST; LIST = NIT_DLIST_PREV(LIST))

#define nit_delayed_preveach(LIST)					\
	typeof(LIST) TMP = LIST ? LIST_PREV(LIST) : NULL;		\
	for (; LIST;							\
	     LIST = TMP, TMP = TMP ? LIST_PREV(TMP) : NULL)

#if defined NIT_SHORT_NAMES || defined NIT_LIST_SHORT_NAMES
# define LIST_NEXT(...)        NIT_LIST_NEXT(__VA_ARGS__)
# define LIST_INC(...)         NIT_LIST_INC(__VA_ARGS__)
# define LIST_CONS(...)        NIT_LIST_CONS(__VA_ARGS__)
# define NEXT_REF(...)         NIT_NEXT_REF(__VA_ARGS__)
# define foreach(...)          nit_foreach(__VA_ARGS__)
# define delayed_foreach(...)  nit_delayed_foreach(__VA_ARGS__)
# define DLIST_PREV(...)       NIT_DLIST_PREV(__VA_ARGS__)
# define LIST_DEC(...)         NIT_LIST_DEC(__VA_ARGS__)
# define DLIST_RCONS(...)      NIT_DLIST_RCONS(__VA_ARGS__)
# define PREV_REF(...)         NIT_PREV_REF(__VA_ARGS__)
# define preveach(...)         nit_preveach(__VA_ARGS__)
# define delayed_preveach(...) nit_delayed_preveach(__VA_ARGS__)
#endif
