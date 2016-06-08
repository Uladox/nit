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

enum nit_map_occured{
	NIT_HASHMAP_ALREADY_PRESENT,
	NIT_HASHMAP_ADDED
};

struct nit_hashentry {
	struct nit_list next;
	void *key;
	uint32_t key_size;
	void *storage;
};

struct nit_hashbin {
	struct nit_hashentry *first;
};

struct nit_hashmap {
	int (*compare)(const void *entry_key, uint32_t entry_key_size,
		       const void *key, uint32_t key_size);
	void (*free_contents)(void *key, void *storage);
	unsigned int bin_num;
	int entry_num;
	const int *primes_pointer;
	struct nit_hashbin *bins;
};

struct nit_hashentry *
nit_hashentry_new(void *key, uint32_t key_size, void *storage);

struct nit_hashmap *
nit_hashmap_new(unsigned int sequence,
		int (*compare)(const void *entry_key, uint32_t entry_key_size,
			       const void *key, uint32_t key_size),
		void (*free_contents)(void *key, void *storage));

void
nit_hashmap_free(struct nit_hashmap *hashmap);

struct nit_hashentry **
nit_hashmap_entry(struct nit_hashmap *map,
		  void *key, uint32_t key_size);

enum nit_map_occured
nit_hashmap_add(struct nit_hashmap *hashmap,
		void *key, uint32_t key_size,
		void *storage);

void
nit_hashmap_remove(struct nit_hashmap *map,
		   void *key, uint32_t key_size);

void *
nit_hashmap_get(const struct nit_hashmap *map,
		const void *key, uint32_t key_size);

void
nit_hashmap_rehash(struct nit_hashmap *map);

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
