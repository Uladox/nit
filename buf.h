/* Include these
   #include <stdlib.h> or something with size_t
 */

typedef struct {
	size_t size;
	char *bytes;
} Nit_buf;

int
nit_buf_init(Nit_buf *buf, size_t size);

Nit_buf *
nit_buf_new(size_t size);

void
nit_buf_dispose(Nit_buf *buf);

void
nit_buf_free(Nit_buf *buf);

int
nit_buf_resize(Nit_buf *buf, size_t size);

int
nit_buf_expand(Nit_buf *buf, size_t amt);

#if defined NIT_SHORT_NAMES || defined NIT_BUF_SHORT_NAMES
# define buf_init(...)    nit_buf_init(__VA_ARGS__)
# define buf_new(...)     nit_buf_new(__VA_ARGS__)
# define buf_dispose(...) nit_buf_dispose(__VA_ARGS__)
# define buf_free(...)    nit_buf_free(__VA_ARGS__)
# define buf_resize(...)  nit_buf_resize(__VA_ARGS__)
# define buf_expand(...)  nit_buf_expand(__VA_ARGS__)
#endif
