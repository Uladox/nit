/*    This file is part of nit.
 *
 *    Nit is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Nit is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with nit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "err.h"
#include "list.h"
#include "set.h"
#include "map.h"

void
map_dispose(Nit_map *map, Nit_map_free dat_free, void *extra)
{
	int bin_num = set_bin_num(map);

	for (int i = 0; i != bin_num; ++i) {
		Nit_hentry *entry = map->bins[i];

		delayed_foreach (entry) {
		        dat_free(entry->dat,
				 map_storage(entry->dat, entry->key_size),
				 extra);
			free(entry->dat);
			free(entry);
		}
	}

	free(map->bins);
}

void
map_dispose_recycle(Nit_map *map, Nit_map_free dat_free, void *extra,
		     Nit_hentry **stack)
{
	int bin_num = set_bin_num(map);

	for (int i = 0; i != bin_num; ++i) {
		Nit_hentry *entry = map->bins[i];

		delayed_foreach (entry) {
		        dat_free(entry->dat,
				 map_storage(entry->dat, entry->key_size),
				 extra);
			free(entry->dat);
		        LIST_APP(entry, *stack);
			*stack = entry;
		}
	}

	free(map->bins);
}

void
map_free(Nit_map *map, Nit_map_free dat_free, void *extra)
{
	map_dispose(map, dat_free, extra);
	free(map);
}

void
map_free_recycle(Nit_map *map, Nit_map_free dat_free, void *extra,
		  Nit_hentry **stack)
{
	map_dispose_recycle(map, dat_free, extra, stack);
	free(map);
}

void *
map_dat_new(void *key, uint32_t key_size, void *storage)
{
	char *dat = malloc(key_size + sizeof(void *));

	pcheck(dat, NULL);
	memcpy(dat, key, key_size);
	memcpy(dat + key_size, &storage, sizeof(void *));
	return dat;
}

int
nit_map_add(Nit_map *map, void *key, uint32_t key_size, void *storage,
	     Nit_hentry **stack)
{
	void *dat = map_dat_new(key, key_size, storage);

	pcheck(dat, -1);

        if (nit_set_add(map, dat, key_size, stack)) {
		free(dat);
		return -1;
	}

	return 0;
}

void *
nit_map_remove(Nit_map *map, void *key, uint32_t key_size,
		Nit_hentry **stack)
{
	void *dat = set_remove(map, key, key_size, stack);
	void *storage;

	pcheck(dat, NULL);
	storage = map_storage(dat, key_size);
	free(dat);
	return storage;
}

