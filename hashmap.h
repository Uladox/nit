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

typedef struct {
	Nit_list next;
	void *key;
	uint32_t key_size;
	void *storage;
} Nit_hashentry;

typedef struct {
	Nit_hashentry *first;
} Nit_hashbin;

typedef int (*Nit_map_cmp)(const void *entry_key, uint32_t entry_key_size,
			   const void *key, uint32_t key_size);

typedef void(*Nit_map_free)(void *key, void *storage);

typedef struct {
	Nit_map_cmp compare;
	Nit_map_free free_contents;
	unsigned int bin_num;
	int entry_num;
	const int *primes_pointer;
	Nit_hashbin *bins;
} Nit_hashmap;

Nit_hashentry *
nit_hashentry_new(void *key, uint32_t key_size, void *storage);

Nit_hashmap *
nit_hashmap_new(unsigned int sequence, Nit_map_cmp compare,
		Nit_map_free free_contents);

void
nit_hashmap_free(Nit_hashmap *hashmap);

Nit_hashentry **
nit_hashmap_entry(Nit_hashmap *map, void *key, uint32_t key_size);

int
nit_hashmap_add(Nit_hashmap *hashmap, void *key,
		uint32_t key_size, void *storage);

void
nit_hashmap_remove(Nit_hashmap *map, void *key, uint32_t key_size);

void *
nit_hashmap_get(const Nit_hashmap *map, const void *key, uint32_t key_size);

void
nit_hashmap_rehash(Nit_hashmap *map);

#if defined NIT_SHORT_NAMES || defined NIT_HASHMAP_SHORT_NAMES
#define hashentry_new(...) nit_hashentry_new(__VA_ARGS__)
#define hashmap_new(...) nit_hashmap_new(__VA_ARGS__)
#define hashmap_free(...) nit_hashmap_free(__VA_ARGS__)
#define hashmap_entry(...) nit_hashmap_entry(__VA_ARGS__)
#define hashmap_add(...) nit_hashmap_add(__VA_ARGS__)
#define hashmap_remove(...) nit_hashmap_remove(__VA_ARGS__)
#define hashmap_get(...) nit_hashmap_get(__VA_ARGS__)
#define hashmap_rehash(...) nit_hashmap_rehash(__VA_ARGS__)
#endif
