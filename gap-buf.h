/* Include these
 * #include <stdint.h>
 * #include <stddef.h>
 */

typedef struct {
	char *bytes;
	size_t size;
	/* Used to store index of gap, not actual ptr dif. */
        ptrdiff_t start;
	ptrdiff_t end;
} Nit_gap;

/* Data setup */

int
nit_gap_init(Nit_gap *gap, size_t size);

int
nit_gap_clone(Nit_gap *clone, const Nit_gap *src);

static inline void
nit_gap_dispose(Nit_gap *gap)
{
	free(gap->bytes);
}

/* Text printing */

void
nit_gap_print(const Nit_gap *gap);

/* Data size */

size_t
nit_gap_hole_len(const Nit_gap *gap);

size_t
nit_gap_len(const Nit_gap *gap);

/* Movement */

int
nit_gap_move_f(Nit_gap *gap, size_t amount);

int
nit_gap_move_b(Nit_gap *gap, size_t amount);

int
nit_gap_move(Nit_gap *gap, ptrdiff_t amount);

void
nit_gap_rewind(Nit_gap *gap);

void
nit_gap_to_end(Nit_gap *gap);

/* Writing and Reading */

int
nit_gap_write(Nit_gap *gap, const void *data, size_t size);

void
nit_gap_read(const Nit_gap *gap, void *data);

void
nit_gap_read_str(const Nit_gap *gap, void *data);

char *
nit_gap_str(const Nit_gap *gap);

int
nit_gap_copy_f(const Nit_gap *gap, void *data, size_t size);

int
nit_gap_copy_b(const Nit_gap *gap, void *data, size_t size);

int
nit_gap_cut_f(Nit_gap *gap, void *data, size_t size);

int
nit_gap_cut_b(Nit_gap *gap, void *data, size_t size);

int
nit_gap_next(Nit_gap *gap, void *data, size_t size);

int
nit_gap_prev(Nit_gap *gap, void *data, size_t size);

/* Erasing */

int
nit_gap_erase_f(Nit_gap *gap, size_t amount);

int
nit_gap_erase_b(Nit_gap *gap, size_t amount);

int
nit_gap_erase(Nit_gap *gap, ptrdiff_t amount);

void
nit_gap_empty(Nit_gap *gap);

#if defined NIT_SHORT_NAMES || defined NIT_GAP_BUF_SHORT_NAMES
# define gap_init(...)     nit_gap_init(__VA_ARGS__)
# define gap_dispose(...)  nit_gap_dispose(__VA_ARGS__)
# define gap_print(...)    nit_gap_print(__VA_ARGS__)
# define gap_hole_len(...) nit_gap_hole_len(__VA_ARGS__)
# define gap_len(...)      nit_gap_len(__VA_ARGS__)
# define gap_move_f(...)   nit_gap_move_f(__VA_ARGS__)
# define gap_move_b(...)   nit_gap_move_b(__VA_ARGS__)
# define gap_move(...)     nit_gap_move(__VA_ARGS__)
# define gap_rewind(...)   nit_gap_rewind(__VA_ARGS__)
# define gap_to_end(...)   nit_gap_to_end(__VA_ARGS__)
# define gap_write(...)    nit_gap_write(__VA_ARGS__)
# define gap_read(...)     nit_gap_read(__VA_ARGS__)
# define gap_read_str(...) nit_gap_read_str(__VA_ARGS__)
# define gap_str(...)      nit_gap_str(__VA_ARGS__)
# define gap_copy_f(...)   nit_gap_copy_f(__VA_ARGS__)
# define gap_copy_b(...)   nit_gap_copy_b(__VA_ARGS__)
# define gap_cut_f(...)    nit_gap_cut_f(__VA_ARGS__)
# define gap_cut_b(...)    nit_gap_cut_b(__VA_ARGS__)
# define gap_next(...)     nit_gap_next(__VA_ARGS__)
# define gap_prev(...)     nit_gap_prev(__VA_ARGS__)
# define gap_erase_b(...)  nit_gap_erase_b(__VA_ARGS__)
# define gap_erase(...)    nit_gap_erase(__VA_ARGS__)
# define gap_empty(...)    nit_gap_empty(__VA_ARGS__)
#endif
