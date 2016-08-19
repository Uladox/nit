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
 * #include "hmap.h"
 */


typedef struct {
        Nit_hmap *left;
        Nit_hmap *right;
} Nit_bimap;

typedef struct {
        Nit_list next;
	Nit_hentry *entry;
} Nit_entry_list;

Nit_bimap *
nit_bimap_new(unsigned int lsequence, Nit_map_cmp lcompare,
	      Nit_map_free lfree_contents,
	      unsigned int rsequence, Nit_map_cmp rcompare,
	      Nit_map_free rfree_contents);

void
nit_bimap_free(Nit_bimap *map);

const char *
nit_bimap_add(Nit_bimap *map,
	      void *lkey, uint32_t lsize, void *rkey, uint32_t rsize);

static inline Nit_entry_list *
nit_bimap_lget(Nit_bimap *map, const void *key, uint32_t size)
{
	return nit_hmap_get(map->left, key, size);
}

static inline Nit_entry_list *
nit_bimap_rget(Nit_bimap *map, const void *key, uint32_t size)
{
	return nit_hmap_get(map->right, key, size);
}

#if defined NIT_SHORT_NAMES || defined NIT_BIMAP_SHORT_NAMES
# define bimap_new(...)  nit_bimap_new(__VA_ARGS__)
# define bimap_free(...) nit_bimap_free(__VA_ARGS__)
# define bimap_add(...)  nit_bimap_add(__VA_ARGS__)
# define bimap_lget(...) nit_bimap_lget(__VA_ARGS__)
# define bimap_rget(...) nit_bimap_rget(__VA_ARGS__)
#endif
