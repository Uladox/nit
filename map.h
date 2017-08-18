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

/* Include these
   #include <stdint.h>
   #include "list.h"
   #include "set.h"
 */

typedef Nit_set Nit_map;
typedef Nit_set_iter Nit_map_iter;
typedef void(*Nit_map_free)(void *key, void *storage, void *extra);

static inline void **
nit_map_storage_ref(void *dat, uint32_t key_size)
{
	return (void **) (((char *) dat) + key_size);
}

static inline void *
nit_map_storage(void *dat, uint32_t key_size)
{
	return *nit_map_storage_ref(dat, key_size);
}

static inline int
nit_map_init(Nit_map *map, unsigned int sequence)
{
	return nit_set_init(map, sequence);
}

void
nit_map_dispose(Nit_map *map, Nit_map_free dat_free, void *extra);

void
nit_map_dispose_recycle(Nit_map *map, Nit_map_free dat_free, void *extra,
			 Nit_hentry **stack);

static inline void
nit_map_empty_dispose(Nit_map *map)
{
	nit_set_empty_dispose(map);
}

static inline Nit_map *
nit_map_new(unsigned int sequence)
{
	return nit_set_new(sequence);
}

void
nit_map_free(Nit_map *map, Nit_map_free dat_free, void *extra);

void
nit_map_free_recycle(Nit_map *map, Nit_map_free dat_free, void *extra,
		      Nit_hentry **stack);

static inline void
nit_map_empty_free(Nit_map *map)
{
        nit_set_empty_free(map);
}

void *
nit_map_dat_new(void *key, uint32_t key_size, void *storage);

static inline int
nit_map_add_reduce(Nit_map *map)
{
	return nit_set_add_reduce(map);
}

int
nit_map_add(Nit_map *map, void *key, uint32_t key_size, void *storage,
	     Nit_hentry **stack);

void *
nit_map_remove(Nit_map *map, void *key, uint32_t key_size,
		Nit_hentry **stack);

static inline void *
nit_map_get_ref(const Nit_map *map, const void *key, uint32_t key_size)
{
	void *dat = nit_set_get(map, key, key_size);

	if (!dat)
		return NULL;

	return (void *) nit_map_storage_ref(dat, key_size);
}

static inline void *
nit_map_get(const Nit_map *map, const void *key, uint32_t key_size)
{
	void *dat = nit_set_get(map, key, key_size);

	if (!dat)
		return NULL;

	return nit_map_storage(dat, key_size);
}

static inline int
nit_map_rehash(Nit_map *map)
{
	return nit_set_rehash(map);
}

static inline void
nit_map_iter_init(Nit_map_iter *iter, Nit_map *map)
{
	nit_set_iter_init(iter, map);
}

static inline void *
nit_map_iter_key(Nit_map_iter *iter)
{
	return nit_set_iter_dat(iter);
}

static inline void *
nit_map_iter_val(Nit_map_iter *iter)
{
	void *dat = nit_set_iter_dat(iter);

	if (!dat)
		return NULL;

	return nit_map_storage(dat, iter->entry->key_size);
}

static inline int
nit_map_iter_next(Nit_map_iter *iter)
{
	return nit_set_iter_next(iter);
}

#if defined NIT_SHORT_NAMES || defined NIT_MAP_SHORT_NAMES
# define map_storage_ref(...)     nit_map_storage_ref(__VA_ARGS__)
# define map_storage(...)         nit_map_storage(__VA_ARGS__)
# define map_init(...)            nit_map_init(__VA_ARGS__)
# define map_dispose(...)         nit_map_dispose(__VA_ARGS__)
# define map_new(...)             nit_map_new(__VA_ARGS__)
# define map_dispose_recycle(...) nit_map_dispose_recycle(__VA_ARGS__)
# define map_empty_dispose(...)   nit_map_empty_dispose(__VA_ARGS__)
# define map_free(...)            nit_map_free(__VA_ARGS__)
# define map_free_recycle(...)    nit_map_free_recycle(__VA_ARGS__)
# define map_empty_free(...)      nit_map_empty_free(__VA_ARGS__)
# define map_dat_new(...)         nit_map_dat_new(__VA_ARGS__)
# define map_entry(...)           nit_map_entry(__VA_ARGS__)
# define map_add_reduce(...)      nit_map_add_reduce(__VA_ARGS__)
# define map_add(...)             nit_map_add(__VA_ARGS__)
# define map_remove(...)          nit_map_remove(__VA_ARGS__)
# define map_get_ref(...)         nit_map_get_ref(__VA_ARGS__)
# define map_get(...)             nit_map_get(__VA_ARGS__)
# define map_rehash(...)          nit_map_rehash(__VA_ARGS__)
# define map_iter_init(...)       nit_map_iter_init(__VA_ARGS__)
# define map_iter_key(...)        nit_map_iter_key(__VA_ARGS__)
# define map_iter_val(...)        nit_map_iter_val(__VA_ARGS__)
# define map_iter_next(...)       nit_map_iter_next(__VA_ARGS__)
#endif
