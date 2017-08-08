#include <stdlib.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "buf.h"

int
buf_init(Nit_buf *buf, size_t size)
{
	pcheck(buf->bytes = malloc(buf->size = size), -1);
	return 0;
}

Nit_buf *
buf_new(size_t size)
{
	Nit_buf *buf = palloc(buf);

	pcheck(buf, NULL);

	if (buf_init(buf, size) < 0) {
		free(buf);
		return NULL;
	}

	return buf;
}

void
buf_dispose(Nit_buf *buf)
{
	free(buf->bytes);
}

void
buf_free(Nit_buf *buf)
{
	buf_dispose(buf);
	free(buf);
}

int
buf_resize(Nit_buf *buf, size_t size)
{
	char *new_bytes = realloc(buf->bytes, size);

	pcheck(new_bytes, -1);
	buf->size = size;
	buf->bytes = new_bytes;
	return 0;
}

int
buf_expand(Nit_buf *buf, size_t amt)
{
	return buf_resize(buf, buf->size + amt);
}
