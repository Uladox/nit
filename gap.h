/* Include these
   #include <stdlib.h> or something with size_t
   #include "buf.h"
*/

typedef struct {
	size_t start;
	size_t size;
	Nit_buf buf;
} Nit_gap;

/* size must atleast be 1. */
int
nit_gap_init(Nit_gap *gap, size_t size);

Nit_gap *
nit_gap_new(size_t size);

void
nit_gap_dispose(Nit_gap *gap);

void
nit_gap_free(Nit_gap *gap);

size_t
nit_gap_len(Nit_gap *gap); /* len of stuff in gap buf, not size of gap. */

int
nit_gap_resize(Nit_gap *gap, size_t size);

int
nit_gap_expand(Nit_gap *gap, size_t size);

int
nit_gap_moveb(Nit_gap *gap);

int
nit_gap_movef(Nit_gap *gap);

void
nit_gap_start(Nit_gap *gap);

void
nit_gap_end(Nit_gap *gap);

int
nit_gap_to(Nit_gap *gap, size_t pos);

int
nit_gap_write(Nit_gap *gap, char dat);

int
nit_gap_read(Nit_gap *gap, char *dat);

int
nit_gap_erase(Nit_gap *gap);

void
nit_gap_clear(Nit_gap *gap);

void
nit_gap_copy(Nit_gap *gap, void *dst);

int
nit_gap_to_buf(Nit_gap *gap, Nit_buf *buf); /* copies stuff from gap buf to buf. */

int
nit_gap_from_buf(Nit_gap *gap, Nit_buf *buf);

#if defined NIT_SHORT_NAMES || defined NIT_GAP_SHORT_NAMES
# define gap_init(...)     nit_gap_init(__VA_ARGS__)
# define gap_new(...)      nit_gap_new(__VA_ARGS__)
# define gap_dispose(...)  nit_gap_dispose(__VA_ARGS__)
# define gap_free(...)     nit_gap_free(__VA_ARGS__)
# define gap_len(...)      nit_gap_len(__VA_ARGS__)
# define gap_resize(...)   nit_gap_resize(__VA_ARGS__)
# define gap_expand(...)   nit_gap_expand(__VA_ARGS__)
# define gap_moveb(...)    nit_gap_moveb(__VA_ARGS__)
# define gap_movef(...)    nit_gap_movef(__VA_ARGS__)
# define gap_start(...)    nit_gap_start(__VA_ARGS__)
# define gap_end(...)      nit_gap_end(__VA_ARGS__)
# define gap_to(...)       nit_gap_to(__VA_ARGS__)
# define gap_write(...)    nit_gap_write(__VA_ARGS__)
# define gap_read(...)     nit_gap_read(__VA_ARGS__)
# define gap_erase(...)    nit_gap_erase(__VA_ARGS__)
# define gap_clear(...)    nit_gap_clear(__VA_ARGS__)
# define gap_copy(...)     nit_gap_copy(__VA_ARGS__)
# define gap_to_buf(...)   nit_gap_to_buf(__VA_ARGS__)
# define gap_from_buf(...) nit_gap_from_buf(__VA_ARGS__)
#endif
