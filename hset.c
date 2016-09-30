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
#include "list.h"
#include "hset.h"

#define BIN_MAX_DENSITY 5
#define H_SEED 37

static const int primes[] = {
	8 + 3, 16 + 3, 32 + 5, 64 + 3, 128 + 3, 256 + 27, 512 + 9,
	1024 + 9, 2048 + 5, 4096 + 3, 8192 + 27, 16384 + 43, 32768 + 3,
	65536 + 45, 131072 + 29, 262144 + 3, 524288 + 21, 1048576 + 7,
	2097152 + 17, 4194304 + 15, 8388608 + 9, 16777216 + 43, 33554432 + 35,
	67108864 + 15, 134217728 + 29, 268435456 + 3, 536870912 + 11,
	1073741824 + 85, 0
};

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

Nit_hentry *
hentry_new(void *dat, uint32_t key_size)
{
	Nit_hentry *entry = palloc(entry);

	pcheck(entry, NULL);
	entry->dat = dat;
	entry->key_size = key_size;
        LIST_CONS(entry, NULL);

	return entry;
}

Nit_hset *
hset_new(unsigned int sequence)
{
	int i = 0;
	Nit_hbin *bin;
	Nit_hset *set = palloc(set);

	pcheck(set, NULL);
	set->bin_num = primes[sequence];
	set->bins = malloc(sizeof(*set->bins) * primes[sequence]);
	set->entry_num = 0;
	set->primes_pointer = primes;
	bin = set->bins;


	for (; i != primes[sequence]; ++i, ++bin)
		bin->first = NULL;

	return set;
}

void
hset_free(Nit_hset *set, Nit_set_free dat_free)
{
	Nit_hbin *bin = set->bins;
	int i;

	for (i = 0; i != set->bin_num; ++i, ++bin) {
		Nit_hentry *entry = bin->first;
		Nit_hentry *tmp;

		delayed_foreach (tmp, entry) {
		        dat_free(tmp->dat);
			free(tmp);
		}
	}

	free(set->bins);
	free(set);
}

Nit_hentry **
hset_entry(Nit_hset *set, void *key, uint32_t key_size)
{
	Nit_hentry *entry;
	unsigned int row;

	row = murmur3_32(key, key_size, H_SEED) % set->bin_num;
	entry = set->bins[row].first;

	if (!entry || compare(entry, key, key_size))
		return &set->bins[row].first;

        foreach (entry) {
		Nit_hentry *next = LIST_NEXT(entry);

		if (!next || compare(next, key, key_size))
			break;
	}

	return NEXT_REF(entry);
}

int
hset_add_reduce(Nit_hset *set)
{
	if (++set->entry_num / set->bin_num >= BIN_MAX_DENSITY)
		return hset_rehash(set);

	return 0;
}

enum nit_hset_error
hset_add(Nit_hset *set, void *dat, uint32_t key_size)
{
	Nit_hentry **entry = hset_entry(set, dat, key_size);

	if (*entry)
		return NIT_HSET_OCCUPIED;

	pcheck((*entry = hentry_new(dat, key_size)),
	       NIT_HSET_NO_MEM);

	if (unlikely(hset_add_reduce(set)))
		return NIT_HSET_NO_MEM;

	return NIT_HSET_OK;
}

enum nit_hset_error
nit_hset_copy_add(Nit_hset *set, void *dat, uint32_t key_size)
{
	void *new_dat = malloc(key_size);

	if (!dat)
		return NIT_HSET_NO_MEM;

	memcpy(new_dat, dat, key_size);

	return hset_add(set, new_dat, key_size);
}

void *
hset_remove(Nit_hset *set, const void *dat, uint32_t key_size)
{
	unsigned int row = murmur3_32(dat, key_size, H_SEED) % set->bin_num;
	Nit_hentry *entry = set->bins[row].first;
	Nit_hentry *prev;
	void *ret;

	if (!entry)
		return NULL;

	if (compare(entry, dat, key_size)) {
		set->bins[row].first = LIST_NEXT(entry);
	        ret = entry->dat;
		free(entry);
		--set->entry_num;
		return ret;
	}

        prev = entry;
	entry = LIST_NEXT(entry);

	foreach (entry) {
		if (compare(entry, dat, key_size)) {
		        LIST_CONS(prev, LIST_NEXT(entry));
		        ret = entry->dat;
			free(entry);
			--set->entry_num;
			return ret;
		}
		prev = entry;
	}

	return NULL;
}

void *
hset_get(const Nit_hset *set, const void *dat, uint32_t key_size)
{
	unsigned int row = murmur3_32(dat, key_size, H_SEED) % set->bin_num;
	const Nit_hentry *entry = set->bins[row].first;

        foreach (entry)
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
	Nit_hbin *bin = sub->bins;
	int i;

	if (sub->entry_num > super->entry_num)
		return 0;

	for (i = 0; i != sub->bin_num; ++i, ++bin) {
		Nit_hentry *entry = bin->first;

	        foreach (entry)
			if (!hset_contains(super, entry->dat, entry->key_size))
				return 0;
	}

	return 1;
}

/* Adds to a bin something already in the hset during a reh */
static void
rehash_add(Nit_hbin *bin, Nit_hentry *entry)
{
	Nit_hentry *tmp = bin->first;

        LIST_CONS(entry, NULL);

	if (!tmp) {
		bin->first = entry;
		return;
	}

	/* Finds end of list */
	while (LIST_NEXT(tmp))
		tmp = LIST_NEXT(tmp);

        LIST_CONS(tmp, entry);
}

int
hset_rehash(Nit_hset *set)
{
	int i;
	int new_bin_num = set->primes_pointer[1];
	Nit_hbin *bin = set->bins;
	Nit_hbin *new_bins = palloc_a(new_bins, new_bin_num);

	pcheck(new_bins, 1);

	for (i = 0; i != new_bin_num; ++i)
		new_bins[i].first = NULL;

	for (i = 0; i != set->bin_num; ++i, ++bin) {
		Nit_hentry *entry = bin->first;
		Nit_hentry *tmp;

		delayed_foreach (tmp, entry) {
			uint32_t row = murmur3_32(tmp->dat, tmp->key_size,
						  H_SEED) % new_bin_num;

			rehash_add(new_bins + row, tmp);
		}
	}

	free(set->bins);
	set->bins = new_bins;
	++set->primes_pointer;
	set->bin_num = new_bin_num;

	return 0;
}
