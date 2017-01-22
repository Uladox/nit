/* Include these
   #include <stdlib.h> or something else with size_t
*/

#define nit_lvec_foreach(lvec, ptr, type)					\
	ptr = (type *) lvec->dat;					\
	for (type *nit_lvec_past = (type *) (lvec->dat + lvec->size);	\
	     ptr != nit_lvec_past; ++ptr)

typedef struct nit_lvec {
	size_t size;
	size_t max;
	char *dat;
} Nit_lvec;

int
nit_lvec_init(Nit_lvec *lvec, size_t max);

void
nit_lvec_dispose(Nit_lvec *lvec);

Nit_lvec *
nit_lvec_new(size_t max);

void
nit_lvec_free(Nit_lvec *lvec);

int
nit_lvec_insert(Nit_lvec *lvec, void *dat, size_t num, size_t size);

int
nit_lvec_insert_ptr(Nit_lvec *lvec, void *ptr, size_t num);

int
nit_lvec_push(Nit_lvec *lvec, void *dat, size_t size);

int
nit_lvec_push_ptr(Nit_lvec *lvec, void *ptr);

void *
nit_lvec_get(Nit_lvec *lvec, size_t num, size_t size);

void *
nit_lvec_get_last(Nit_lvec *lvec, size_t size);

void *
nit_lvec_get_ptr(Nit_lvec *lvec, size_t num);

void *
nit_lvec_get_last_ptr(Nit_lvec *lvec);

int
nit_lvec_remove(Nit_lvec *lvec, size_t num, size_t size);

int
nit_lvec_remove_ptr(Nit_lvec *lvec, size_t num);

void *
nit_lvec_pop_ptr(Nit_lvec *lvec);

# if defined NIT_SHORT_NAMES || defined NIT_LVEC_SHORT_NAMES
# define lvec_foreach(...)      nit_lvec_foreach(__VA_ARGS__)
# define lvec_init(...)         nit_lvec_init(__VA_ARGS__)
# define lvec_dispose(...)      nit_lvec_dispose(__VA_ARGS__)
# define lvec_new(...)          nit_lvec_new(__VA_ARGS__)
# define lvec_free(...)         nit_lvec_free(__VA_ARGS__)
# define lvec_insert(...)       nit_lvec_insert(__VA_ARGS__)
# define lvec_insert_ptr(...)   nit_lvec_insert_ptr(__VA_ARGS__)
# define lvec_push(...)         nit_lvec_push(__VA_ARGS__)
# define lvec_push_ptr(...)     nit_lvec_push_ptr(__VA_ARGS__)
# define lvec_get(...)          nit_lvec_get(__VA_ARGS__)
# define lvec_get_last(...)     nit_lvec_get_last(__VA_ARGS__)
# define lvec_get_ptr(...)      nit_lvec_get_ptr(__VA_ARGS__)
# define lvec_get_last_ptr(...) nit_lvec_get_last_ptr(__VA_ARGS__)
# define lvec_remove(...)       nit_lvec_remove(__VA_ARGS__)
# define lvec_remove_ptr(...)   nit_lvec_remove_ptr(__VA_ARGS__)
# define lvec_pop_ptr(...)      nit_lvec_pop_ptr(__VA_ARGS__)
#endif
