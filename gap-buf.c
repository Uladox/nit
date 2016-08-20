#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "gap-buf.h"

static int
buf_valid_pos(const Nit_buf *buf, ptrdiff_t pos)
{
	return (pos >= 0) && (pos < (ptrdiff_t) buf->size);
}

int
buf_init(Nit_buf *buf, size_t size)
{
	if (unlikely(size <= 0))
		return 1;

	buf->bytes = palloc_a0(buf->bytes, size);
	pcheck(buf->bytes, 1);
	buf->size = size;

	return 0;
}

int
gap_init(Nit_gap *gap, size_t size)
{
	gap->start = 0;
	gap->end = size - 1;

	return buf_init(&gap->buf, size);
}

static void
gap_fprint(const Nit_gap *gap, FILE *file)
{
	fprintf(file, "%.*s%.*s\n",
		(int) gap->start, gap->buf.bytes,
		(int) (gap->buf.size - gap->end - 1),
		gap->buf.bytes + gap->end + 1);
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
	return gap->buf.size - gap_hole_len(gap);
}

int
gap_move(Nit_gap *gap, ptrdiff_t amount)
{
	ptrdiff_t pos;

	if (unlikely(!buf_valid_pos(&gap->buf, pos = gap->end + amount)))
			return 1;

	if (amount > 0) {
		for (; gap->end != pos; ++gap->start, ++gap->end)
			gap->buf.bytes[gap->start] = gap->buf.bytes[gap->end];
	} else {
		for (; gap->start != pos; --gap->start, --gap->end)
			gap->buf.bytes[gap->end] = gap->buf.bytes[gap->start];
	}

	return 1;
}

void
gap_rewind(Nit_gap *gap)
{
	for(; gap->start != 0; --gap->start, --gap->end)
		gap->buf.bytes[gap->end] = gap->buf.bytes[gap->start];

	/* Once more now that we are at 0 */
	gap->buf.bytes[gap->end] = gap->buf.bytes[0];
}

void
gap_to_end(Nit_gap *gap)
{
	const ptrdiff_t max_pos = gap->buf.size - 1;

	for(; gap->end < max_pos; ++gap->start, ++gap->end)
		gap->buf.bytes[gap->start] = gap->buf.bytes[gap->end];

	/* Once more now that we are at the end */
	gap->buf.bytes[gap->start] = gap->buf.bytes[max_pos];
}

int
gap_resize(Nit_gap *gap, size_t size)
{
	size_t added_size = gap->buf.size * 0.5 + size;
	size_t new_size = gap->buf.size + added_size;
	char *new_bytes = palloc_a0(new_bytes, new_size);

	pcheck(new_bytes, 1);
	memcpy(new_bytes, gap->buf.bytes, gap->start);
	memcpy(new_bytes + added_size + gap->end + 1,
	       gap->buf.bytes + gap->end + 1,
	       gap->buf.size - gap->end - 1);
	gap->end += added_size;
	free(gap->buf.bytes);
	gap->buf.bytes = new_bytes;
	gap->buf.size = new_size;

	return 0;
}

int
gap_write(Nit_gap *gap, const void *data, size_t size)
{
	size_t count = 0;

	if (gap_hole_len(gap) < size && gap_resize(gap, size))
		return 1;

	for (; count < size; ++count)
		gap->buf.bytes[gap->start++] = ((char *) data)[count];

	return 0;
}

void
gap_read(const Nit_gap *gap, void *data)
{
	memcpy(data, gap->buf.bytes, gap->start);
	memcpy(data, gap->buf.bytes + gap->end + 1,
	       gap->buf.size - gap->end - 1);
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
	if (unlikely(!buf_valid_pos(&gap->buf, gap->end + size)))
			return 1;

	memcpy(data, gap->buf.bytes + (gap->end + 1), size);

	return 0;
}

int
gap_copy_b(const Nit_gap *gap, void *data, size_t size)
{
	if (unlikely(!buf_valid_pos(&gap->buf, gap->start - size)))
			return 1;

	memcpy(data, gap->buf.bytes + gap->start - size, size);

	return 0;
}

int
gap_cut_f(Nit_gap *gap, void *data, size_t size)
{
	if (unlikely(gap_copy_f(gap, data, size)))
		return 1;

	gap->end += size;

	return 0;
}

int
nit_gap_cut_b(Nit_gap *gap, void *data, size_t size)
{
	if (unlikely(gap_copy_b(gap, data, size)))
		return 1;

	gap->start -= size;

	return 0;
}

int
gap_erase_f(Nit_gap *gap, size_t amount)
{
	size_t pos;

	if (unlikely(!buf_valid_pos(&gap->buf, pos = gap->end + amount)))
			return 1;

	gap->end = pos;

	return 0;
}

int
gap_erase_b(Nit_gap *gap, size_t amount)
{
	size_t pos;

	if (unlikely(!buf_valid_pos(&gap->buf, pos = gap->start - amount)))
			return 1;

	gap->start = pos;

	return 0;
}

int
gap_erase(Nit_gap *gap, ptrdiff_t amount)
{
	return (amount > 0) ? gap_erase_f(gap, amount)
		: gap_erase_b(gap, -1 * amount);
}

void
gap_empty(Nit_gap *gap)
{
	gap->start = 0;
	gap->end = gap->buf.size - 1;
}

