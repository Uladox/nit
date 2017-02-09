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
   #include <stdint.h>
   #include "list.h"
   #include "hset.h"
 */

typedef Nit_hset Nit_hmap;
typedef Nit_hset_iter Nit_hmap_iter;
typedef void(*Nit_map_free)(void *key, void *storage, void *extra);

static inline void **
nit_hmap_storage_ref(void *dat, uint32_t key_size)
{
	return (void **) (((char *) dat) + key_size);
}

static inline void *
nit_hmap_storage(void *dat, uint32_t key_size)
{
	return *nit_hmap_storage_ref(dat, key_size);
}

static inline int
nit_hmap_init(Nit_hmap *map, unsigned int sequence)
{
	return nit_hset_init(map, sequence);
}

void
nit_hmap_dispose(Nit_hmap *map, Nit_map_free dat_free, void *extra);

void
nit_hmap_dispose_recycle(Nit_hmap *map, Nit_map_free dat_free, void *extra,
			 Nit_hentry **stack);

static inline Nit_hmap *
nit_hmap_new(unsigned int sequence)
{
	return nit_hset_new(sequence);
}

void
nit_hmap_free(Nit_hmap *hmap, Nit_map_free dat_free, void *extra);

void
nit_hmap_free_recycle(Nit_hmap *map, Nit_map_free dat_free, void *extra,
		      Nit_hentry **stack);

void *
nit_hmap_dat_new(void *key, uint32_t key_size, void *storage);

static inline int
nit_hmap_add_reduce(Nit_hmap *map)
{
	return nit_hset_add_reduce(map);
}

int
nit_hmap_add(Nit_hmap *hmap, void *key, uint32_t key_size, void *storage,
	     Nit_hentry **stack);

void *
nit_hmap_remove(Nit_hmap *map, void *key, uint32_t key_size,
		Nit_hentry **stack);

static inline void *
nit_hmap_get_ref(const Nit_hmap *map, const void *key, uint32_t key_size)
{
	void *dat = nit_hset_get(map, key, key_size);

	if (!dat)
		return NULL;

	return (void *) nit_hmap_storage_ref(dat, key_size);
}

static inline void *
nit_hmap_get(const Nit_hmap *map, const void *key, uint32_t key_size)
{
	void *dat = nit_hset_get(map, key, key_size);

	if (!dat)
		return NULL;

	return nit_hmap_storage(dat, key_size);
}

static inline int
nit_hmap_rehash(Nit_hmap *map)
{
	return nit_hset_rehash(map);
}

static inline void
nit_hmap_iter_init(Nit_hmap_iter *iter, Nit_hmap *map)
{
	nit_hset_iter_init(iter, map);
}

static inline void *
nit_hmap_iter_key(Nit_hmap_iter *iter)
{
	return nit_hset_iter_dat(iter);
}

static inline void *
nit_hmap_iter_val(Nit_hmap_iter *iter)
{
	void *dat = nit_hset_iter_dat(iter);

	if (!dat)
		return NULL;

	return nit_hmap_storage(dat, iter->entry->key_size);
}

static inline int
nit_hmap_iter_next(Nit_hmap_iter *iter)
{
	return nit_hset_iter_next(iter);
}

#if defined NIT_SHORT_NAMES || defined NIT_HMAP_SHORT_NAMES
# define hmap_storage_ref(...)     nit_hmap_storage_ref(__VA_ARGS__)
# define hmap_storage(...)         nit_hmap_storage(__VA_ARGS__)
# define hmap_init(...)            nit_hmap_init(__VA_ARGS__)
# define hmap_dispose(...)         nit_hmap_dispose(__VA_ARGS__)
# define hmap_new(...)             nit_hmap_new(__VA_ARGS__)
# define hmap_dispose_recycle(...) nit_hmap_dispose_recycle(__VA_ARGS__)
# define hmap_free(...)            nit_hmap_free(__VA_ARGS__)
# define hmap_free_recycle(...)    nit_hmap_free_recycle(__VA_ARGS__)
# define hmap_dat_new(...)         nit_hmap_dat_new(__VA_ARGS__)
# define hmap_entry(...)           nit_hmap_entry(__VA_ARGS__)
# define hmap_add_reduce(...)      nit_hmap_add_reduce(__VA_ARGS__)
# define hmap_add(...)             nit_hmap_add(__VA_ARGS__)
# define hmap_remove(...)          nit_hmap_remove(__VA_ARGS__)
# define hmap_get_ref(...)         nit_hmap_get_ref(__VA_ARGS__)
# define hmap_get(...)             nit_hmap_get(__VA_ARGS__)
# define hmap_rehash(...)          nit_hmap_rehash(__VA_ARGS__)
# define hmap_iter_init(...)       nit_hmap_iter_init(__VA_ARGS__)
# define hmap_iter_key(...)        nit_hmap_iter_key(__VA_ARGS__)
# define hmap_iter_val(...)        nit_hmap_iter_val(__VA_ARGS__)
# define hmap_iter_next(...)       nit_hmap_iter_next(__VA_ARGS__)
#endif
