/* Include these
   #include <stdlib.h> or something else with size_t
*/

typedef struct nit_vec {
	size_t size;
	size_t max;
	char *dat;
} Nit_vec;

int
nit_vec_init(Nit_vec *vec, size_t max);

void
nit_vec_dispose(Nit_vec *vec);

Nit_vec *
nit_vec_new(size_t max);

void
nit_vec_free(Nit_vec *vec);

int
nit_vec_push(Nit_vec *vec, void *dat, size_t size);

int
nit_vec_push_ptr(Nit_vec *vec, void *ptr);

void *
nit_vec_get(Nit_vec *vec, size_t num, size_t size);

void *
nit_vec_get_last(Nit_vec *vec, size_t size);

void *
nit_vec_get_ptr(Nit_vec *vec, size_t num);

void *
nit_vec_get_last_ptr(Nit_vec *vec);

# if defined NIT_SHORT_NAMES || defined NIT_VEC_SHORT_NAMES
# define vec_init(...)         nit_vec_init(__VA_ARGS__)
# define vec_dispose(...)      nit_vec_dispose(__VA_ARGS__)
# define vec_new(...)          nit_vec_new(__VA_ARGS__)
# define vec_free(...)         nit_vec_free(__VA_ARGS__)
# define vec_push(...)         nit_vec_push(__VA_ARGS__)
# define vec_push_ptr(...)     nit_vec_push_ptr(__VA_ARGS__)
# define vec_get(...)          nit_vec_get(__VA_ARGS__)
# define vec_get_last(...)     nit_vec_get_last(__VA_ARGS__)
# define vec_get_ptr(...)      nit_vec_get_ptr(__VA_ARGS__)
# define vec_get_last_ptr(...) nit_vec_get_last_ptr(__VA_ARGS__)
#endif
