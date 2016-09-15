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

/* Include these
 * #include <stdint.h>
 * #include "list.h"
 */

/* typedef struct { */
/* 	Nit_list next; */
/* 	void *key; */
/* 	uint32_t key_size; */
/* 	void *storage; */
/* } Nit_hentry; */

/* typedef struct { */
/* 	Nit_hentry *first; */
/* } Nit_hbin; */

typedef void(*Nit_map_free)(void *key, void *storage);

typedef struct {
	Nit_map_free free_contents;
	Nit_hset *set;
} Nit_hmap;

/* Nit_hentry * */
/* nit_hentry_new(void *key, uint32_t key_size, void *storage); */

Nit_hmap *
nit_hmap_new(unsigned int sequence, Nit_map_free free_contents);

void
nit_hmap_free(Nit_hmap *hmap);

/* Nit_hentry ** */
/* nit_hmap_entry(Nit_hmap *map, void *key, uint32_t key_size); */

int
nit_hmap_add_reduce(Nit_hmap *map);

/* extern const char *nit_hmap_present; */
/* extern const char *nit_hmap_no_mem; */

const char *
nit_hmap_add(Nit_hmap *hmap, void *key, uint32_t key_size,
	     void *storage, uint32_t *storage_size);

void *
nit_hmap_remove(Nit_hmap *map, void *key, uint32_t key_size);

void *
nit_hmap_get(const Nit_hmap *map, const void *key, uint32_t key_size);

int
nit_hmap_rehash(Nit_hmap *map);

#if defined NIT_SHORT_NAMES || defined NIT_HMAP_SHORT_NAMES
# define hentry_new(...)      nit_hentry_new(__VA_ARGS__)
# define hmap_new(...)        nit_hmap_new(__VA_ARGS__)
# define hmap_free(...)       nit_hmap_free(__VA_ARGS__)
# define hmap_entry(...)      nit_hmap_entry(__VA_ARGS__)
# define hmap_add_reduce(...) nit_hmap_add_reduce(__VA_ARGS__)
# define hmap_add(...)        nit_hmap_add(__VA_ARGS__)
# define hmap_remove(...)     nit_hmap_remove(__VA_ARGS__)
# define hmap_get(...)        nit_hmap_get(__VA_ARGS__)
# define hmap_rehash(...)     nit_hmap_rehash(__VA_ARGS__)
#endif
