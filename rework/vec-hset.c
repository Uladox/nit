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
#include "vec.h"
#include "hset.h"

#define BIN_MAX_DENSITY 4
#define H_SEED 37

static const int bin_num[] = {
	1, 3,
	8 + 3, 16 + 3, 32 + 5, 64 + 3, 128 + 3, 256 + 27, 512 + 9,
	1024 + 9, 2048 + 5, 4096 + 3, 8192 + 27, 16384 + 43, 32768 + 3,
	65536 + 45, 131072 + 29, 262144 + 3, 524288 + 21, 1048576 + 7,
	2097152 + 17, 4194304 + 15, 8388608 + 9, 16777216 + 43, 33554432 + 35,
	67108864 + 15, 134217728 + 29, 268435456 + 3, 536870912 + 11,
	1073741824 + 85, 0
};

int
hset_bin_num(Nit_hset *set)
{
	return bin_num[set->bin_pos];
}

void
rehash(Nit_hset *set);

#define ROT32(x, y) ((x << y) | (x >> (32 - y)))
static uint32_t
murmur3_32(const char *key, uint32_t len, uint32_t seed)
{
	static const uint32_t c1 = 0xcc9e2d51;
	static const uint32_t c2 = 0x1b873593;
	static const uint32_t r1 = 15;
	static const uint32_t r2 = 13;
	static const uint32_t m = 5;
	static const uint32_t n = 0xe6546b64;

	uint32_t h = seed;

	const int nblocks = len / 4;
	const uint32_t *blocks = (const uint32_t *) key;
	int i;
	uint32_t k;

	for (i = 0; i < nblocks; i++) {
		k = blocks[i];
		k *= c1;
		k = ROT32(k, r1);
		k *= c2;

		h ^= k;
		h = ROT32(h, r2) * m + n;
	}

	const uint8_t *tail = (const uint8_t *) (key + nblocks * 4);
	uint32_t k1 = 0;

	switch (len & 3) {
	case 3:
		k1 ^= tail[2] << 16;
	case 2:
		k1 ^= tail[1] << 8;
	case 1:
		k1 ^= tail[0];

		k1 *= c1;
		k1 = ROT32(k1, r1);
		k1 *= c2;
		h ^= k1;
	}

	h ^= len;
	h ^= (h >> 16);
	h *= 0x85ebca6b;
	h ^= (h >> 13);
	h *= 0xc2b2ae35;
	h ^= (h >> 16);

	return h;
}

static int
compare(const Nit_hentry *entry , const void *dat, uint32_t key_size)
{
	if (key_size != entry->key_size)
		return 0;

	return !memcmp(entry->dat, dat, key_size);
}

void
hentry_init(Nit_hentry *entry, void *dat, uint32_t key_size)
{
	entry->key_size = key_size;
	entry->hash = murmur3_32(dat, key_size, H_SEED);
	entry->dat = dat;
}

int
nit_hset_init(Nit_hset *set, unsigned int sequence)
{
	int i = 0;
	Nit_vec *bin;
	size_t bin_start =  !sequence ? 0 :
		bin_num[sequence - 1] * 5 / bin_num[sequence];

	set->bin_pos = sequence;
	set->entry_num = 0;

	if (!(set->bins = malloc(sizeof(*set->bins) * bin_num[sequence])))
		return 0;

	bin = set->bins;

	for (; i != bin_num[sequence]; ++i, ++bin)
		vec_init(bin, bin_start * sizeof(Nit_hentry));

	return 1;
}

void
nit_hset_release(Nit_hset *set, Nit_set_free dat_free)
{
	Nit_vec *bin = set->bins;
	int i;
	Nit_hentry *entry;

	for (i = 0; i != bin_num[set->bin_pos]; ++i, ++bin) {
		vec_foreach(bin, entry, Nit_hentry)
			dat_free(entry->dat);

		vec_dispose(bin);
	}

	free(set->bins);
}

Nit_hset *
hset_new(unsigned int sequence)
{
	Nit_hset *set = palloc(set);

	pcheck(set, NULL);

	if (!nit_hset_init(set, sequence)) {
		free(set);
		return NULL;
	}

	return set;
}

void
hset_free(Nit_hset *set, Nit_set_free dat_free)
{
	nit_hset_release(set, dat_free);
	free(set);
}

static int
row_num(const Nit_hentry *entry, int num)
{
	return entry->hash % num;
}

int
hset_rehash_maybe(Nit_hset *set)
{
	if (++set->entry_num / bin_num[set->bin_pos] >= BIN_MAX_DENSITY)
		return hset_rehash(set);

	return 1;
}

static int
vec_push_entry(Nit_vec *vec, Nit_hentry *entry)
{
	return vec_push(vec, entry, sizeof(*entry));
}

int
hset_add(Nit_hset *set, void *dat, uint32_t key_size)
{
	Nit_hentry entry;
	Nit_vec *vec;

	hentry_init(&entry, dat, key_size);
	vec = set->bins + row_num(&entry, bin_num[set->bin_pos]);

	if (!vec_push_entry(vec, &entry))
		return 0;

        return hset_rehash_maybe(set);
}

int
nit_hset_copy_add(Nit_hset *set, void *dat, uint32_t key_size)
{
	void *new_dat = malloc(key_size);

	if (!new_dat)
		return 0;

	memcpy(new_dat, dat, key_size);

	return hset_add(set, new_dat, key_size);
}

/* void * */
/* hset_remove(Nit_hset *set, const void *dat, uint32_t key_size) */
/* { */
/* 	unsigned int row = row_num(dat, key_size, bin_num[set->bin_pos]); */
/* 	Nit_hentry *entry = set->bins[row].first; */
/* 	Nit_hentry *prev; */
/* 	void *ret; */

/* 	if (!entry) */
/* 		return NULL; */

/* 	if (compare(entry, dat, key_size)) { */
/* 		set->bins[row].first = LIST_NEXT(entry); */
/* 	        ret = entry->dat; */
/* 		free(entry); */
/* 		--set->entry_num; */
/* 		return ret; */
/* 	} */

/*         prev = entry; */
/* 	entry = LIST_NEXT(entry); */

/* 	foreach (entry) { */
/* 		if (compare(entry, dat, key_size)) { */
/* 		        LIST_CONS(prev, LIST_NEXT(entry)); */
/* 		        ret = entry->dat; */
/* 			free(entry); */
/* 			--set->entry_num; */
/* 			return ret; */
/* 		} */
/* 		prev = entry; */
/* 	} */

/* 	return NULL; */
/* } */

void *
hset_get(const Nit_hset *set, const void *dat, uint32_t key_size)
{
	uint32_t row = murmur3_32(dat, key_size, H_SEED) %
		bin_num[set->bin_pos];
        Nit_vec *vec = set->bins + row;
	Nit_hentry *entry;

        vec_foreach (vec, entry, Nit_hentry)
		if (compare(entry, dat, key_size))
			return entry->dat;

	return NULL;
}

int
hset_contains(const Nit_hset *set, const void *dat, uint32_t key_size)
{
	return !!hset_get(set, dat, key_size);
}

int
hset_subset(const Nit_hset *super, const Nit_hset *sub)
{
	Nit_vec *bin = sub->bins;
	int i;
	Nit_hentry *entry;


	if (sub->entry_num > super->entry_num)
		return 0;

	for (i = 0; i != bin_num[sub->bin_pos]; ++i, ++bin)
	        vec_foreach (bin, entry, Nit_hentry)
			if (!hset_contains(super, entry->dat, entry->key_size))
				return 0;

	return 1;
}

int
hset_rehash(Nit_hset *set)
{
	int i;
	Nit_vec *bin = set->bins;
	int new_bin_num = bin_num[set->bin_pos + 1];
        Nit_vec *new_bins = palloc_a(new_bins, new_bin_num);
	size_t bin_start = bin_num[set->bin_pos] * 4 / new_bin_num;
	Nit_hentry *entry;

	pcheck(new_bins, 0);

	for (i = 0; i != new_bin_num; ++i)
	        vec_init(new_bins + i, bin_start * sizeof(Nit_hentry));

	for (i = 0; i != bin_num[set->bin_pos]; ++i, ++bin) {
		vec_foreach (bin, entry, Nit_hentry) {
			uint32_t row = row_num(entry, new_bin_num);

			if (!vec_push_entry(new_bins + row, entry)) {
				for (i = 0; i != new_bin_num; ++i)
					vec_dispose(new_bins + i);

				return 0;
			}
		}
	}

	for (i = 0; i != bin_num[set->bin_pos]; ++i)
		vec_dispose(set->bins + i);

	free(set->bins);
	set->bins = new_bins;
	++set->bin_pos;

	return 1;
}

/* static void */
/* iter_next_nonempty_bin(Nit_hset_iter *iter) */
/* { */
/* 	for (; iter->bin_num < iter->bin_last - 1; ++iter->bin_num) */
/* 		if (iter->bins[iter->bin_num].size) */
/* 			break; */

/* 	iter->entry = (Nit_hentry *) iter->bins[iter->bin_num].dat; */
/* } */

/* void */
/* nit_hset_iter_init(Nit_hset_iter *iter, Nit_hset *set) */
/* { */
/* 	iter->bin_num = 0; */
/* 	iter->bin_last = bin_num[set->bin_pos]; */
/* 	iter->bins = set->bins; */
/* 	iter_next_nonempty_bin(iter); */
/* } */

/* void * */
/* nit_hset_iter_dat(Nit_hset_iter *iter) */
/* { */
/* 	if (!iter->entry) */
/* 		return NULL; */

/* 	return iter->entry->dat; */
/* } */

/* int */
/* nit_hset_iter_next(Nit_hset_iter *iter) */
/* { */
/* 	if (!iter->entry) */
/* 		return 0; */

/* 	if (!LIST_INC(iter->entry)) { */
/* 		if (++iter->bin_num >= iter->bin_last) */
/* 			return 0; */

/* 		iter_next_nonempty_bin(iter); */

/* 		if (!iter->entry) */
/* 			return 0; */
/* 	} */

/* 	return 1; */
/* } */
