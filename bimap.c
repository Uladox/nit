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
#include "hset.h"
#include "hmap.h"
#include "bimap.h"

#define BIN_MAX_DENSITY 10

Nit_bimap *
nit_bimap_new(unsigned int lsequence, unsigned int rsequence)
{
	Nit_bimap *map = palloc(map);

	pcheck(map, NULL);
	map->left = hmap_new(lsequence);
	map->right = hmap_new(rsequence);

	return map;
}

void
nit_bimap_free(Nit_bimap *map, Nit_map_free lfree_contents,
	       Nit_map_free rfree_contents)
{
        hmap_free(map->left, lfree_contents);
        hmap_free(map->right, rfree_contents);
        free(map);
}

enum nit_hset_error
nit_bimap_add(Nit_bimap *map,
	      void *lkey, uint32_t lsize, void *rkey, uint32_t rsize)
{
	Nit_hentry **lentry = hset_entry(map->left, lkey, lsize);
	Nit_hentry **rentry = hset_entry(map->right, rkey, rsize);
	Nit_entry_list *lstorage = palloc(lstorage);
	Nit_entry_list *rstorage = palloc(rstorage);

	pcheck_c(lstorage, NIT_HSET_NO_MEM, free(rstorage));
	pcheck_c(rstorage, NIT_HSET_NO_MEM, free(lstorage));

	if (!*lentry) {
		void *dat = hmap_dat_new(lkey, lsize, NULL);

		*lentry = hentry_new(dat, lsize);
	}

	if (!*rentry) {
		void *dat = hmap_dat_new(rkey, rsize, NULL);

		*rentry = hentry_new(dat, rsize);
	}

	lstorage->entry = *rentry;
        LIST_CONS(lstorage, hmap_storage((*lentry)->dat, lsize));
	*hmap_storage_ref((*lentry)->dat, lsize) = lstorage;

	rstorage->entry = *lentry;
        LIST_CONS(rstorage, hmap_storage((*rentry)->dat, rsize));
	*hmap_storage_ref((*rentry)->dat, rsize) = rstorage;

	if (!LIST_NEXT(*lentry))
		if (unlikely(hset_add_reduce(map->left)))
			return NIT_HSET_NO_MEM;

	if (!LIST_NEXT(*rentry))
		if (unlikely(hset_add_reduce(map->right)))
			return NIT_HSET_NO_MEM;

	return NIT_HSET_OK;
}
