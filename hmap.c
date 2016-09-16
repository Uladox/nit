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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "list.h"
#include "hset.h"
#include "hmap.h"

void
nit_hmap_free(Nit_hmap *map, Nit_map_free dat_free)
{
	Nit_hbin *bin = map->bins;
	int i;

	for (i = 0; i != map->bin_num; ++i, ++bin) {
		Nit_hentry *entry = bin->first;
		Nit_hentry *tmp;

		delayed_foreach (tmp, entry) {
		        dat_free(tmp->dat,
				 hmap_storage(tmp->dat, tmp->key_size));
			free(tmp->dat);
			free(tmp);
		}
	}

	free(map->bins);
	free(map);
}

void *
hmap_dat_new(void *key, uint32_t key_size, void *storage)
{
	char *dat = malloc(key_size + sizeof(void *));

	if (!dat)
		return NULL;

	memcpy(dat, key, key_size);
	memcpy(dat + key_size, &storage, sizeof(void *));

	return dat;
}

const char *
nit_hmap_add(Nit_hmap *map, void *key, uint32_t key_size, void *storage)
{
	void *dat = hmap_dat_new(key, key_size, storage);

	if (!dat)
		return nit_hset_no_mem;

	return nit_hset_add(map, dat, key_size);
}

void *
nit_hmap_remove(Nit_hmap *map, void *key, uint32_t key_size)
{
	void *dat = nit_hset_remove(map, key, key_size);
	void *storage;

	if (!dat)
		return NULL;

	storage = hmap_storage(dat, key_size);
	free(dat);

	return storage;
}

