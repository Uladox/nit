/* Include these
   #include <stdlib.h> or something with size_t
   #include "buf.h"
   #include "gap.h"
*/


typedef struct {
	size_t pos;
	Nit_gap *gap;
} Nit_crs;

static inline void
nit_crs_init(Nit_crs *crs, Nit_gap *gap, size_t pos)
{
	crs->pos = pos;
	crs->gap = gap;
}

int
nit_crs_moveb(Nit_crs *crs);

int
nit_crs_movef(Nit_crs *crs);

static inline void
nit_crs_start(Nit_crs *crs)
{
	crs->pos = 0;
}

static inline void
nit_crs_end(Nit_crs *crs)
{
	crs->pos = gap_len(crs->gap);
}

int
nit_crs_to(Nit_crs *crs, size_t pos);

int
nit_crs_write(Nit_crs *crs, char dat);

int
nit_crs_read(Nit_crs *crs, char *dat);

int
nit_crs_erase(Nit_crs *crs);

static inline void
nit_crs_clear(Nit_crs *crs)
{
	gap_clear(crs->gap);
	crs->pos = 0;
}

#if defined NIT_SHORT_NAMES || defined NIT_CRS_SHORT_NAMES
# define crs_init(...)  nit_crs_init(__VA_ARGS__)
# define crs_moveb(...) nit_crs_moveb(__VA_ARGS__)
# define crs_movef(...) nit_crs_movef(__VA_ARGS__)
# define crs_start(...) nit_crs_start(__VA_ARGS__)
# define crs_end(...)   nit_crs_end(__VA_ARGS__)
# define crs_to(...)    nit_crs_to(__VA_ARGS__)
# define crs_write(...) nit_crs_write(__VA_ARGS__)
# define crs_read(...)  nit_crs_read(__VA_ARGS__)
# define crs_erase(...) nit_crs_erase(__VA_ARGS__)
# define crs_clear(...) nit_crs_clear(__VA_ARGS__)
#endif
