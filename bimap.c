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
#include "macros.h"
#include "palloc.h"
#include "list.h"
#include "hmap.h"
#include "bimap.h"

#define BIN_MAX_DENSITY 10

Nit_bimap *
nit_bimap_new(unsigned int lsequence, Nit_map_cmp lcompare,
	      Nit_map_free lfree_contents,
	      unsigned int rsequence, Nit_map_cmp rcompare,
	      Nit_map_free rfree_contents)
{
	Nit_bimap *map = palloc(map);

	pcheck(map, NULL);
	map->left = hmap_new(lsequence, lcompare, lfree_contents);
	map->right = hmap_new(rsequence, rcompare, rfree_contents);

	return map;
}

void
nit_bimap_free(Nit_bimap *map)
{
	hmap_free(map->left);
	hmap_free(map->right);
        free(map);
}

const char *
nit_bimap_add(Nit_bimap *map,
	      void *lkey, uint32_t lsize, void *rkey, uint32_t rsize)
{
	Nit_hentry **lentry = hmap_entry(map->left, lkey, lsize);
	Nit_hentry **rentry = hmap_entry(map->right, rkey, rsize);
	Nit_entry_list *lstorage = palloc(lstorage);
	Nit_entry_list *rstorage = palloc(rstorage);

	pcheck_c(lstorage, nit_hmap_no_mem, free(rstorage));
	pcheck_c(rstorage, nit_hmap_no_mem, free(lstorage));

	if (!*lentry)
		*lentry = hentry_new(lkey, lsize, NULL);

	if (!*rentry)
		*rentry = hentry_new(rkey, rsize, NULL);

	lstorage->entry = *rentry;
        LIST_CONS(lstorage, (*lentry)->storage);
	(*lentry)->storage = lstorage;

	rstorage->entry = *lentry;
        LIST_CONS(rstorage, (*rentry)->storage);
	(*rentry)->storage = rstorage;

	if (!LIST_NEXT(*lentry))
		if (unlikely(hmap_add_reduce(map->left)))
			return nit_hmap_no_mem;

	if (!LIST_NEXT(*rentry))
		if (unlikely(hmap_add_reduce(map->right)))
			return nit_hmap_no_mem;

	return NULL;
}
