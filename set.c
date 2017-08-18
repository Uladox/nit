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
#include "list.h"
#include "set.h"

#define BIN_MAX_DENSITY 5
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
set_bin_num(Nit_set *set)
{
	return bin_num[set->bin_pos];
}

void
rehash(Nit_set *set);

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
		__attribute__ ((fallthrough));
	case 2:
		k1 ^= tail[1] << 8;
		__attribute__ ((fallthrough));
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

Nit_hentry *
hentry_new(void *dat, uint32_t key_size, Nit_hentry **stack)
{
	Nit_hentry *entry;

	if (!*stack) {
		entry = palloc(entry);
		pcheck(entry, NULL);
	} else {
		entry = *stack;
		*stack = LIST_NEXT(*stack, void);
	}

	entry->dat = dat;
	entry->key_size = key_size;
	entry->hash = murmur3_32(dat, key_size, H_SEED);
        LIST_APP(entry, NULL);
	return entry;
}

int
nit_set_init(Nit_set *set, unsigned int sequence)
{
	set->bin_pos = sequence;
	set->entry_num = 0;
	pcheck(set->bins = calloc(bin_num[sequence], sizeof(*set->bins)), -1);
	return 0;
}

void
nit_set_dispose(Nit_set *set, Nit_set_free dat_free, void *extra)
{
	for (int i = 0; i != bin_num[set->bin_pos]; ++i) {
	        Nit_hentry *entry = set->bins[i];

	        delayed_foreach (entry) {
		        dat_free(entry->dat, extra);
			free(entry);
		}
	}

	free(set->bins);
}

void
nit_set_dispose_recycle(Nit_set *set, Nit_set_free dat_free, void *extra,
			 Nit_hentry **stack)
{
	for (int i = 0; i != bin_num[set->bin_pos]; ++i) {
	        Nit_hentry *entry = set->bins[i];

	        delayed_foreach (entry) {
		        dat_free(entry->dat, extra);
			LIST_APP(entry, *stack);
			*stack = entry;
		}
	}

	free(set->bins);
}

void
nit_set_empty_dispose(Nit_set *set)
{
	free(set->bins);
}

Nit_set *
set_new(unsigned int sequence)
{
	Nit_set *set = palloc(set);

	pcheck(set, NULL);

	if (set_init(set, sequence)) {
		free(set);
		return NULL;
	}

	return set;
}

void
set_free(Nit_set *set, Nit_set_free dat_free, void *extra)
{
        set_dispose(set, dat_free, extra);
	free(set);
}

void
set_free_recycle(Nit_set *set, Nit_set_free dat_free, void *extra,
		  Nit_hentry **stack)
{
        set_dispose_recycle(set, dat_free, extra, stack);
	free(set);
}

void
set_empty_free(Nit_set *set)
{
	free(set->bins);
	free(set);
}

static int
row_num(const void *key, uint32_t key_size, int num)
{
	return murmur3_32(key, key_size, H_SEED) % num;
}

Nit_hentry **
set_entry(Nit_set *set, void *key, uint32_t key_size)
{
	Nit_hentry *entry;
        uint32_t row;

	row = row_num(key, key_size, bin_num[set->bin_pos]);
	entry = set->bins[row];

	if (!entry || compare(entry, key, key_size))
		return &set->bins[row];

        foreach (entry) {
		Nit_hentry *next = LIST_NEXT(entry, void);

		if (!next || compare(next, key, key_size))
			break;
	}

	return NEXT_REF(entry, Nit_hentry);
}

int
set_add_reduce(Nit_set *set)
{
	if (++set->entry_num / bin_num[set->bin_pos] >= BIN_MAX_DENSITY)
		return set_rehash(set);

	return 0;
}

int
set_add_unique(Nit_set *set, void *dat, uint32_t key_size,
		Nit_hentry **stack)
{
	Nit_hentry **entry = set_entry(set, dat, key_size);

	if (*entry)
		return -2;

	pcheck(*entry = hentry_new(dat, key_size, stack), -1);

	if (unlikely(set_add_reduce(set))) {
		free(entry);
		return -1;
	}

	return 0;
}

int
set_add(Nit_set *set, void *dat, uint32_t key_size, Nit_hentry **stack)
{
	Nit_hentry *entry = hentry_new(dat, key_size, stack);
	Nit_hentry **bin;

	pcheck(entry, -1);

	if (unlikely(set_add_reduce(set))) {
		free(entry);
		return -1;
	}

	bin = set->bins + (entry->hash % bin_num[set->bin_pos]);
	LIST_APP(entry, *bin);
        *bin = entry;
	return 0;
}

int
nit_set_copy_add(Nit_set *set, void *dat, uint32_t key_size,
		  Nit_hentry **stack)
{
	void *new_dat = malloc(key_size);

	pcheck(new_dat, -1);
	memcpy(new_dat, dat, key_size);

	if (set_add(set, new_dat, key_size, stack)) {
	        free(new_dat);
		return -1;
	}

	return 0;
}

void *
set_remove(Nit_set *set, const void *dat, uint32_t key_size, Nit_hentry **stack)
{
        uint32_t row = row_num(dat, key_size, bin_num[set->bin_pos]);
	Nit_hentry *entry = set->bins[row];
	Nit_hentry *prev;
	void *ret;

	pcheck(entry, NULL);

	if (compare(entry, dat, key_size)) {
		set->bins[row] = LIST_NEXT(entry, void);
	        ret = entry->dat;
		free(entry);
		--set->entry_num;
		return ret;
	}

        prev = entry;
	entry = LIST_NEXT(entry, void);

	foreach (entry) {
		if (compare(entry, dat, key_size)) {
		        LIST_APP(prev, LIST_NEXT(entry, void));
		        ret = entry->dat;
			LIST_APP(entry, *stack);
			*stack = entry;
			--set->entry_num;
			return ret;
		}

		prev = entry;
	}

	return NULL;
}

void *
set_get(const Nit_set *set, const void *dat, uint32_t key_size)
{
        uint32_t row = row_num(dat, key_size, bin_num[set->bin_pos]);
	const Nit_hentry *entry = set->bins[row];

        foreach (entry)
		if (compare(entry, dat, key_size))
			return entry->dat;

	return NULL;
}

int
set_contains(const Nit_set *set, const void *dat, uint32_t key_size)
{
	return !!set_get(set, dat, key_size);
}

int
set_subset(const Nit_set *super, const Nit_set *sub)
{
	int i;

	if (sub->entry_num > super->entry_num)
		return 0;

	for (i = 0; i != bin_num[sub->bin_pos]; ++i) {
		Nit_hentry *entry = sub->bins[i];

	        foreach (entry)
			if (!set_contains(super, entry->dat, entry->key_size))
				return 0;
	}

	return 1;
}

int
set_rehash(Nit_set *set)
{
	int new_bin_num = bin_num[set->bin_pos + 1];
	Nit_hentry **new_bins = palloc_a(new_bins, new_bin_num);

	pcheck(new_bins, -1);

	for (int i = 0; i != new_bin_num; ++i)
		new_bins[i] = NULL;

	for (int i = 0; i != bin_num[set->bin_pos]; ++i) {
		Nit_hentry *entry = set->bins[i];

	        delayed_foreach (entry) {
			uint32_t row = entry->hash % new_bin_num;
			Nit_hentry **bin = new_bins + row;

			LIST_APP(entry, *bin);
			*bin = entry;
		}
	}

	free(set->bins);
	set->bins = new_bins;
	++set->bin_pos;
	return 0;
}

static void
iter_next_nonempty_bin(Nit_set_iter *iter)
{
	for (; iter->bin_num < iter->bin_last - 1; ++iter->bin_num)
		if (iter->bins[iter->bin_num])
			break;

	iter->entry = iter->bins[iter->bin_num];
}

void
nit_set_iter_init(Nit_set_iter *iter, Nit_set *set)
{
	iter->bin_num = 0;
	iter->bin_last = bin_num[set->bin_pos];
	iter->bins = set->bins;
	iter_next_nonempty_bin(iter);
}

void *
nit_set_iter_dat(Nit_set_iter *iter)
{
	pcheck(iter->entry, NULL);
	return iter->entry->dat;
}

int
nit_set_iter_next(Nit_set_iter *iter)
{
	pcheck(iter->entry, 0);

	if (!LIST_INC(iter->entry)) {
		if (++iter->bin_num >= iter->bin_last)
			return 0;

		iter_next_nonempty_bin(iter);
		pcheck(iter->entry, 0);
	}

	return 1;
}
