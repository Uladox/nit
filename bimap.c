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

#include <stdint.h>
#include <stdlib.h>

#define NIT_SHORT_NAMES
#include "list.h"
#include "hashmap.h"
#include "bimap.h"

#define BIN_MAX_DENSITY 10

Nit_bimap *
nit_bimap_new(unsigned int lsequence, Nit_map_cmp lcompare,
	      Nit_map_free lfree_contents,
	      unsigned int rsequence, Nit_map_cmp rcompare,
	      Nit_map_free rfree_contents)
{
	Nit_bimap *map = malloc(sizeof(*map));

	map->left = hashmap_new(lsequence, lcompare, lfree_contents);
	map->right = hashmap_new(rsequence, rcompare, rfree_contents);

	return map;
}

void
nit_bimap_free(Nit_bimap *map)
{
	hashmap_free(map->left);
	hashmap_free(map->right);
        free(map);
}

void
nit_bimap_add(Nit_bimap *map,
	      void *lkey, uint32_t lsize, void *rkey, uint32_t rsize)
{
	Nit_hashentry **lentry = hashmap_entry(map->left, lkey, lsize);
	Nit_hashentry **rentry = hashmap_entry(map->right, rkey, rsize);
	Nit_entry_list *lstorage = malloc(sizeof(*lstorage));
	Nit_entry_list *rstorage = malloc(sizeof(*rstorage));

	if (!*lentry)
		*lentry = hashentry_new(lkey, lsize, NULL);

	if (!*rentry)
		*rentry = hashentry_new(rkey, rsize, NULL);

	lstorage->entry = *rentry;
        LIST_CONS(lstorage, (*lentry)->storage);
	(*lentry)->storage = lstorage;

	rstorage->entry = *lentry;
        LIST_CONS(rstorage, (*rentry)->storage);
	(*rentry)->storage = rstorage;

	if (!LIST_NEXT(*lentry))
	        if (++map->left->entry_num / map->left->bin_num
		    >= BIN_MAX_DENSITY)
			hashmap_rehash(map->left);

	if (!LIST_NEXT(*rentry))
		if (++map->right->entry_num / map->right->bin_num
		    >= BIN_MAX_DENSITY)
			hashmap_rehash(map->right);

}

