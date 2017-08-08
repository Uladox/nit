#include <stddef.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "buf.h"
#include "gap.h"
#include "crs.h"

int
crs_moveb(Nit_crs *crs)
{
	if (unlikely(!crs->pos))
		return -1;

	--crs->pos;
	return 0;
}

static size_t
gap_max(Nit_gap *gap)
{
	return gap->buf.size - gap->size;
}

int
crs_movef(Nit_crs *crs)
{
	if (unlikely(crs->pos == gap_max(crs->gap)))
		return -1;

	++crs->pos;
	return 0;
}

int
crs_to(Nit_crs *crs, size_t pos)
{
	if (unlikely(pos > gap_max(crs->gap)))
		return -1;

	crs->pos = pos;
	return 0;
}

int
crs_write(Nit_crs *crs, char dat)
{
	gap_to(crs->gap, crs->pos);

	if (gap_write(crs->gap, dat) < 0)
		return -1;

	++crs->pos;
	return 0;
}

static size_t
real_pos(Nit_crs *crs)
{
	return crs->pos <= crs->gap->start ? crs->pos : crs->pos + crs->gap->size;
}

int
crs_read(Nit_crs *crs, char *dat)
{
	size_t real = real_pos(crs);

	if (unlikely(!crs->pos))
		return -1;

	*dat = crs->gap->buf.bytes[real - 1];
	return 0;
}

int
crs_erase(Nit_crs *crs)
{
	gap_to(crs->gap, crs->pos);

        if (gap_erase(crs->gap) < 0)
		return -1;

	--crs->pos;
	return 0;
}
