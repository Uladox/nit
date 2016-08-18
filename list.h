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

#if defined NIT_SHORT_NAMES || defined NIT_LIST_SHORT_NAMES
# define LIST_NEXT(LIST)            NIT_LIST_NEXT(LIST)
# define LIST_CONS(LIST, END)       NIT_LIST_CONS(LIST, END)
# define NEXT_REF(LIST)             NIT_NEXT_REF(LIST)
# define foreach(LIST)              nit_foreach(LIST)
# define delayed_foreach(TMP, LIST) nit_delayed_foreach(TMP, LIST)
#endif
