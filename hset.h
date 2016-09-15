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
	void *dat;
	uint32_t key_size;
} Nit_hentry;

typedef struct {
	Nit_hentry *first;
} Nit_hbin;

typedef void(*Nit_set_free)(void *dat);

typedef struct {
        int bin_num;
	int entry_num;
	const int *primes_pointer;
	Nit_hbin *bins;
} Nit_hset;

Nit_hentry *
nit_hentry_new(void *dat, uint32_t key_size);

Nit_hset *
nit_hset_new(unsigned int sequence);

void
nit_hset_free(Nit_hset *hset, Nit_set_free dat_free);

Nit_hentry **
nit_hset_entry(Nit_hset *set, void *dat, uint32_t key_size);

int
nit_hset_add_reduce(Nit_hset *set);

extern const char *nit_hset_present;
extern const char *nit_hset_no_mem;

const char *
nit_hset_add(Nit_hset *hset, void *dat,
	     uint32_t key_size);

void *
nit_hset_remove(Nit_hset *set, const void *dat, uint32_t key_size);

void *
nit_hset_get(const Nit_hset *set, const void *dat, uint32_t key_size);

int
nit_hset_contains(const Nit_hset *set, const void *dat, uint32_t key_size);

int
nit_hset_subset(const Nit_hset *super, const Nit_hset *sub);

int
nit_hset_rehash(Nit_hset *set);

#if defined NIT_SHORT_NAMES || defined NIT_HSET_SHORT_NAMES
# define hentry_new(...)      nit_hentry_new(__VA_ARGS__)
# define hset_new(...)        nit_hset_new(__VA_ARGS__)
# define hset_free(...)       nit_hset_free(__VA_ARGS__)
# define hset_entry(...)      nit_hset_entry(__VA_ARGS__)
# define hset_add_reduce(...) nit_hset_add_reduce(__VA_ARGS__)
# define hset_add(...)        nit_hset_add(__VA_ARGS__)
# define hset_remove(...)     nit_hset_remove(__VA_ARGS__)
# define hset_get(...)        nit_hset_get(__VA_ARGS__)
# define hset_contains(...)   nit_hset_contains(__VA_ARGS__)
# define hset_subset(...)     nit_hset_subset(__VA_ARGS__)
# define hset_rehash(...)     nit_hset_rehash(__VA_ARGS__)
#endif
