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

struct nit_list {
	void *next;
};

#define NIT_LIST(LIST)				\
	((struct nit_list *) (LIST))
#define NIT_LIST_NEXT(LIST)			\
	((typeof(LIST)) (NIT_LIST(LIST)->next))
#define NIT_LIST_CONS(LIST, END)		\
	(NIT_LIST(LIST)->next = (END))
#define NIT_NEXT_REF(LIST)			\
	((typeof(LIST) *) &NIT_LIST(LIST)->next)
#define nit_foreach(LIST)			\
	for (; LIST; LIST = NIT_LIST_NEXT(LIST))
