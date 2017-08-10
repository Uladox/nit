#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "buf.h"
#include "gap.h"

int
gap_init(Nit_gap *gap, size_t size)
{
	gap->start = 0;
	gap->size = size;
	return buf_init(&gap->buf, size);
}

Nit_gap *
gap_new(size_t size)
{
	Nit_gap *gap = palloc(gap);

	pcheck(gap, NULL);

	if (unlikely(gap_init(gap, size) < 0)) {
		free(gap);
		return NULL;
	}

	return gap;
}

void
gap_dispose(Nit_gap *gap)
{
	buf_dispose(&gap->buf);
}

void
gap_free(Nit_gap *gap)
{
	gap_dispose(gap);
	free(gap);
}

size_t
gap_len(Nit_gap *gap)
{
	return gap->buf.size - gap->size;
}

static char *
after_gap(Nit_gap *gap)
{
	return gap->buf.bytes + gap->start + gap->size;
}

static size_t
size_after_gap(Nit_gap *gap)
{
	return gap->buf.size - (gap->start + gap->size);
}

static void
copy_to_gap(char *dat, Nit_gap *gap)
{
	memcpy(dat, gap->buf.bytes, gap->start);
}

static void
copy_after_gap(char *dat, Nit_gap *gap)
{
	memcpy(dat, after_gap(gap), size_after_gap(gap));
}

int
gap_resize(Nit_gap *gap, size_t size)
{
	size_t new_size = gap->buf.size - gap->size + size;
	char *new_bytes = malloc(new_size);

	pcheck(new_bytes, -1);
	copy_to_gap(new_bytes, gap);
	/* creating the actual gap */
	copy_after_gap(new_bytes + gap->start + size, gap);
	free(gap->buf.bytes);
	gap->buf.size = new_size;
	gap->buf.bytes = new_bytes;
	gap->size = size;
	return 0;
}

int
gap_expand(Nit_gap *gap, size_t size)
{
	return gap_resize(gap, gap->size + size);
}

int
gap_moveb(Nit_gap *gap)
{
	if (unlikely(!gap->start))
		return -1;

        gap->buf.bytes[gap->start + gap->size - 1] =
		gap->buf.bytes[gap->start - 1];
	--gap->start;
	return 0;
}

int
gap_movef(Nit_gap *gap)
{
	if (unlikely(gap->start + gap->size == gap->buf.size))
		return -1;

	gap->buf.bytes[gap->start] =
		gap->buf.bytes[gap->start + gap->size];
	++gap->start;
	return 0;
}

void
nit_gap_start(Nit_gap *gap)
{
	while (likely(gap_moveb(gap) == 0));
}

void
nit_gap_end(Nit_gap *gap)
{
	while (likely(gap_movef(gap) == 0));
}

int
gap_to(Nit_gap *gap, size_t pos)
{
	if (unlikely(pos > gap_len(gap)))
		return -1;

	if (pos > gap->start) {
		while (gap->start != pos)
			gap_movef(gap);

		return 0;
	}

	while (gap->start != pos)
		gap_moveb(gap);

	return 0;
}

int
gap_write(Nit_gap *gap, char dat)
{
	if (unlikely(!gap->size && gap_expand(gap, gap->buf.size) < 0))
		return -1;

	gap->buf.bytes[gap->start] = dat;
	++gap->start;
	--gap->size;
	return 0;
}

int
gap_read(Nit_gap *gap, char *dat)
{
	if (unlikely(!gap->start))
		return -1;

	*dat = gap->buf.bytes[gap->start - 1];
	return 0;
}

int
gap_erase(Nit_gap *gap)
{
	if (unlikely(!gap->start))
		return -1;

	--gap->start;
	++gap->size;
	return 0;
}

void
gap_clear(Nit_gap *gap)
{
	gap->start = 0;
	gap->size = gap->buf.size;
}

void
gap_copy(Nit_gap *gap, void *dst)
{
	copy_to_gap(dst, gap);
	copy_after_gap((char *) dst + gap->start, gap);
}

int
gap_to_buf(Nit_gap *gap, Nit_buf *buf)
{
	if (unlikely(buf_resize(buf, gap_len(gap)) < 0))
		return -1;

	gap_copy(gap, buf->bytes);
	return 0;
}

int
gap_from_buf(Nit_gap *gap, Nit_buf *buf)
{
	if (gap->buf.size < buf->size && gap_expand(gap, buf->size * 1.25) < 0)
		return -1;

        gap->start = 0;
	gap->size = gap->buf.size - buf->size;
	memcpy(after_gap(gap), buf->bytes, buf->size);
	return 0;
}
