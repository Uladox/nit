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
 */

typedef struct nit_hentry {
	Nit_list next;
	void *dat;
	uint32_t key_size;
	uint32_t hash;
} Nit_hentry;

typedef void(*Nit_set_free)(void *dat, void *extra);
typedef struct nit_set Nit_set;

struct nit_set {
	int entry_num;
	int bin_pos;
	Nit_hentry **bins;
};

typedef struct nit_set_iter Nit_set_iter;

struct nit_set_iter {
	int bin_num;
	int bin_last;
	Nit_hentry **bins;
	Nit_hentry *entry;
};

int
nit_set_bin_num(Nit_set *set);

Nit_hentry *
nit_hentry_new(void *dat, uint32_t key_size, Nit_hentry **stack);

int
nit_set_init(Nit_set *set, unsigned int sequence);

void
nit_set_dispose(Nit_set *set, Nit_set_free dat_free, void *extra);

void
nit_set_dispose_recycle(Nit_set *set, Nit_set_free dat_free, void *extra,
			 Nit_hentry **stack);

void
nit_set_empty_dispose(Nit_set *set);

Nit_set *
nit_set_new(unsigned int sequence);

void
nit_set_free(Nit_set *set, Nit_set_free dat_free, void *extra);

void
nit_set_free_recycle(Nit_set *set, Nit_set_free dat_free, void *extra,
		      Nit_hentry **stack);

void
nit_set_empty_free(Nit_set *set);

Nit_hentry **
nit_set_entry(Nit_set *set, void *dat, uint32_t key_size);

int
nit_set_add_reduce(Nit_set *set);

int
set_add_unique(Nit_set *set, void *dat, uint32_t key_size);

int
nit_set_add(Nit_set *set, void *dat, uint32_t key_size, Nit_hentry **stack);

int
nit_set_copy_add(Nit_set *set, void *dat, uint32_t key_size,
		  Nit_hentry **stack);

void *
nit_set_remove(Nit_set *set, const void *dat, uint32_t key_size,
		Nit_hentry **stack);

void *
nit_set_get(const Nit_set *set, const void *dat, uint32_t key_size);

int
nit_set_contains(const Nit_set *set, const void *dat, uint32_t key_size);

int
nit_set_subset(const Nit_set *super, const Nit_set *sub);

int
nit_set_rehash(Nit_set *set);

void
nit_set_iter_init(Nit_set_iter *iter, Nit_set *set);

void *
nit_set_iter_dat(Nit_set_iter *iter);

int
nit_set_iter_next(Nit_set_iter *iter);


#if defined NIT_SHORT_NAMES || defined NIT_SET_SHORT_NAMES
# define set_bin_num(...)         nit_set_bin_num(__VA_ARGS__)
# define hentry_new(...)           nit_hentry_new(__VA_ARGS__)
# define set_init(...)            nit_set_init(__VA_ARGS__)
# define set_dispose(...)         nit_set_dispose(__VA_ARGS__)
# define set_dispose_recycle(...) nit_set_dispose_recycle(__VA_ARGS__)
# define set_empty_dispose(...)   nit_set_empty_dispose(__VA_ARGS__)
# define set_new(...)             nit_set_new(__VA_ARGS__)
# define set_free(...)            nit_set_free(__VA_ARGS__)
# define set_free_recycle(...)    nit_set_free_recycle(__VA_ARGS__)
# define set_empty_free(...)      nit_set_empty_free(__VA_ARGS__)
# define set_entry(...)           nit_set_entry(__VA_ARGS__)
# define set_add_reduce(...)      nit_set_add_reduce(__VA_ARGS__)
# define set_add_unique(...)      nit_set_add_unique(__VA_ARGS__)
# define set_add(...)             nit_set_add(__VA_ARGS__)
# define set_copy_add(...)        nit_set_copy_add(__VA_ARGS__)
# define set_remove(...)          nit_set_remove(__VA_ARGS__)
# define set_get(...)             nit_set_get(__VA_ARGS__)
# define set_contains(...)        nit_set_contains(__VA_ARGS__)
# define set_subset(...)          nit_set_subset(__VA_ARGS__)
# define set_rehash(...)          nit_set_rehash(__VA_ARGS__)
# define set_iter_init(...)       nit_set_iter_init(__VA_ARGS__)
# define set_iter_dat(...)        nit_set_iter_dat(__VA_ARGS__)
# define set_iter_next(...)       nit_set_iter_next(__VA_ARGS__)
#endif
