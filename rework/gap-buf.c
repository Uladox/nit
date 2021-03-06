#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "gap-buf.h"

static inline int
gap_valid_pos(const Nit_gap *gap, ptrdiff_t pos)
{
	return (pos >= 0) && (pos < (ptrdiff_t) gap->size);
}

static inline size_t
size_past_end(const Nit_gap *gap)
{
	return gap->size - gap->end - 1;
}

static inline char *
bytes_past_end(const Nit_gap *gap)
{
	return gap->bytes + gap->end + 1;
}

static inline void
copy_to_start(void *des, const Nit_gap *src)
{
	memcpy(des, src->bytes, src->start);
}

static inline void
copy_from_end(void *des, const Nit_gap *src)
{
	memcpy(des, bytes_past_end(src), size_past_end(src));
}

int
gap_init(Nit_gap *gap, size_t size)
{
	if (unlikely(size <= 0))
		return 0;

	gap->bytes = palloc_a0(gap->bytes, size);
	pcheck(gap->bytes, 0);
	gap->size = size;
	gap->start = 0;
	gap->end = size - 1;
	return 1;
}

int
nit_gap_clone(Nit_gap *clone, const Nit_gap *src)
{
	clone->start = src->start;
	clone->end = src->end;
	pcheck(clone->bytes = malloc(clone->size = src->size), 0);
	memcpy(clone->bytes, src->bytes, src->size);
	return 1;
}

int
gap_replicate(Nit_gap *rep, const Nit_gap *src)
{
	gap_empty(rep);

	if (!gap_resize(rep, src->size))
		return 0;

	rep->start = src->start;
	rep->end = src->end + (rep->size - src->size);
	copy_to_start(rep->bytes, src);
	copy_from_end(rep->bytes + rep->end + 1, src);
	return 1;
}

int
gap_resize(Nit_gap *gap, size_t size)
{
	size_t added_size;
	size_t new_size;
	size_t new_end;
	char *new_bytes;

	if (gap_hole_len(gap) > size)
		return 1;

	added_size = gap->size * 0.5 + size;
	new_size = gap->size + added_size;
	new_end = gap->end + added_size;
	new_bytes = palloc_a0(new_bytes, new_size);
	pcheck(new_bytes, 0);
	copy_to_start(new_bytes, gap);
	copy_from_end(new_bytes + new_end + 1, gap);
	free(gap->bytes);
	gap->size = new_size;
	gap->end = new_end;
	gap->bytes = new_bytes;
	return 1;
}

void
nit_gap_dispose(Nit_gap *gap)
{
	free(gap->bytes);
}

static void
gap_fprint(const Nit_gap *gap, FILE *file)
{
	fprintf(file, "%.*s%.*s\n",
		(int) gap->start, gap->bytes,
		(int) size_past_end(gap),
		bytes_past_end(gap));
}

void
gap_print(const Nit_gap *gap)
{
	gap_fprint(gap, stdout);
}

size_t
gap_hole_len(const Nit_gap *gap)
{
	return gap->end - gap->start + 1;
}

size_t
gap_len(const Nit_gap *gap)
{
	return gap->size - gap_hole_len(gap);
}

int
gap_move_f(Nit_gap *gap, size_t amount)
{
	ptrdiff_t pos;

	if (unlikely(!gap_valid_pos(gap, pos = gap->end + amount)))
		return 0;

	while (gap->end != pos)
		gap->bytes[gap->start++] = gap->bytes[++gap->end];

	return 1;
}

int
gap_move_b(Nit_gap *gap, size_t amount)
{
	ptrdiff_t pos;

	if (unlikely(!gap_valid_pos(gap, pos = gap->start - amount)))
		return 0;

	while (gap->start != pos)
		gap->bytes[gap->end--] = gap->bytes[--gap->start];

	return 1;
}

int
gap_move(Nit_gap *gap, ptrdiff_t amount)
{

	return (amount > 0) ? gap_move_f(gap, amount)
		: gap_move_b(gap, amount);
}

void
gap_rewind(Nit_gap *gap)
{
	for (; gap->start != 0; --gap->start, --gap->end)
		gap->bytes[gap->end] = gap->bytes[gap->start];

	/* Once more now that we are at 0 */
	gap->bytes[gap->end] = gap->bytes[0];
}

void
gap_to_end(Nit_gap *gap)
{
	const ptrdiff_t max_pos = gap->size - 1;

	for (; gap->end < max_pos; ++gap->start, ++gap->end)
		gap->bytes[gap->start] = gap->bytes[gap->end];

	/* Once more now that we are at the end */
	gap->bytes[gap->start] = gap->bytes[max_pos];
}

int
gap_gap_put(Nit_gap *des, const Nit_gap *src)
{
	return gap_write(des, src->bytes, src->start) &&
		gap_write(des, bytes_past_end(src), size_past_end(src));
}


int
gap_write(Nit_gap *gap, const void *data, size_t size)
{
	size_t count = 0;

	if (!gap_resize(gap, size))
		return 0;

	for (; count < size; ++count)
		gap->bytes[gap->start++] = ((char *) data)[count];

	return 1;
}

void
gap_read(const Nit_gap *gap, void *data)
{
	copy_to_start(data, gap);
	copy_from_end(data, gap);
}

void
gap_read_str(const Nit_gap *gap, void *data)
{
	gap_read(gap, data);
	((char *) data)[gap_len(gap)] = '\0';
}

char *
gap_str(const Nit_gap *gap)
{
	size_t size = gap_len(gap);
	char *str = malloc(size + 1);

	pcheck(str, NULL);
	gap_read(gap, str);
	str[size] = '\0';
	return str;
}

int
gap_copy_f(const Nit_gap *gap, void *data, size_t size)
{
	if (unlikely(!gap_valid_pos(gap, gap->end + size)))
		return 0;

	memcpy(data, bytes_past_end(gap), size);
	return 1;
}

int
gap_copy_b(const Nit_gap *gap, void *data, size_t size)
{
	if (unlikely(!gap_valid_pos(gap, gap->start - size)))
		return 0;

	memcpy(data, gap->bytes + gap->start - size, size);
	return 1;
}

int
gap_cut_f(Nit_gap *gap, void *data, size_t size)
{
	if (unlikely(!gap_copy_f(gap, data, size)))
		return 0;

	gap->end += size;
	return 1;
}

int
gap_cut_b(Nit_gap *gap, void *data, size_t size)
{
	if (unlikely(!gap_copy_b(gap, data, size)))
		return 0;

	gap->start -= size;
	return 1;
}

int
gap_next(Nit_gap *gap, void *data, size_t size)
{
	return gap_copy_f(gap, data, size) && gap_move_f(gap, size);
}

int
gap_prev(Nit_gap *gap, void *data, size_t size)
{
	return gap_copy_b(gap, data, size) && gap_move_b(gap, size);
}

int
gap_erase_f(Nit_gap *gap, size_t amount)
{
	size_t pos = gap->end + amount;

	if (unlikely(!gap_valid_pos(gap, pos)))
		return 0;

	gap->end = pos;
	return 1;
}

int
gap_erase_b(Nit_gap *gap, size_t amount)
{
	size_t pos = gap->start - amount;

	if (unlikely(!gap_valid_pos(gap, pos)))
		return 0;

	gap->start = pos;
	return 1;
}

int
gap_erase(Nit_gap *gap, ptrdiff_t amount)
{
	return amount > 0 ? gap_erase_f(gap, amount)
		: gap_erase_b(gap, -1 * amount);
}

void
gap_empty(Nit_gap *gap)
{
	gap->start = 0;
	gap->end = gap->size - 1;
}

int
nit_gap_compare(const Nit_gap *gap1, const Nit_gap *gap2)
{
	int shorter;
	const Nit_gap *gaps[2] = { gap1, gap2 };
	const char *str1 = gap1->bytes;
	const char *str2 = gap2->bytes;
        size_t num1;
	size_t num2;

	if (gap_len(gap1) != gap_len(gap2))
		return 0;

	if (gap1->start < gap2->start) {
		shorter = 0;
		num1 = gap1->start;
	} else {
		shorter = 1;
		num1 = gap2->start;
	}

	if (memcmp(str1, str2, num1))
		return 0;

	if (!(num2 = gaps[!shorter]->start - num1)) {
		str1 = bytes_past_end(gap1);
		str2 = bytes_past_end(gap2);
		return !memcmp(str1, str2, size_past_end(gap1));
	}

	str1 = bytes_past_end(gaps[shorter]);
	str2 = gaps[!shorter]->bytes + num1;

	if (memcmp(str1, str2, num2))
		return 0;

	str1 += num2;
	str2 = bytes_past_end(gaps[!shorter]);
	num2 = size_past_end(gaps[!shorter]);
	return !memcmp(str1, str2, num2);
}

void *
nit_gap_ptr(Nit_gap *gap, size_t pos)
{
	if (pos > gap_len(gap) - 1)
		return NULL;

	if ((ptrdiff_t) pos < gap->start)
		    return gap->bytes + pos;

	return bytes_past_end(gap) + pos - gap->start;
}
