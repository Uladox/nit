/* Include these
   #include <stdint.h>
   #include <stdlib.h> or something else with size_t
   #include "list.h"
   #include "hset.h"
   #include "hmap.h"
 */

typedef struct nit_radix Nit_radix;
typedef struct nit_redge Nit_redge;

typedef void(*Nit_radix_free)(void *dat);

struct nit_radix {
	void *dat;
	Nit_hmap map;
};

struct nit_redge {
	Nit_radix *radix;
	size_t len;
	char str[];
};

enum { NIT_T_RADIX, NIT_T_REDGE };

typedef struct {
	int type;
	int pos;

	union {
		Nit_radix *radix;
		Nit_redge *redge;
	} d;
} Nit_radix_iter;

Nit_redge **
nit_radix_get_ref(Nit_radix *radix, char c);

Nit_redge *
nit_radix_get(Nit_radix *radix, char c);

int
nit_radix_add(Nit_radix *radix, char c, Nit_redge *edge);

void *
nit_radix_lookup(Nit_radix *radix, const void *str, size_t len);

void
nit_radix_init(Nit_radix *radix, void *dat);

void
nit_radix_dispose(Nit_radix *radix, Nit_radix_free dat_free);

void
nit_radix_free(Nit_radix *radix, Nit_radix_free dat_free);

Nit_radix *
nit_radix_new(void *dat);

Nit_redge *
nit_redge_new(Nit_radix *radix, const void *pre, size_t len);

int
nit_redge_split(Nit_redge **old_ref, const void *key,
		size_t key_size, void *dat);

int
nit_radix_insert(Nit_radix *radix, const void *key,
		 size_t key_size, void *dat);

void
nit_radix_iter_set(Nit_radix_iter *iter, Nit_radix *radix);

/* returns how much has been moved */
size_t
nit_radix_iter_move(Nit_radix_iter *iter, const void *key, size_t len);

void *
nit_radix_iter_get(Nit_radix_iter *iter);

#if defined NIT_SHORT_NAMES || defined NIT_RADIX_SHORT_NAMES
# define radix_get_ref(...)   nit_radix_get_ref(__VA_ARGS__)
# define radix_get(...)       nit_radix_get(__VA_ARGS__)
# define radix_add(...)       nit_radix_add(__VA_ARGS__)
# define radix_lookup(...)    nit_radix_lookup(__VA_ARGS__)
# define radix_init(...)      nit_radix_init(__VA_ARGS__)
# define radix_dispose(...)   nit_radix_dispose(__VA_ARGS__)
# define radix_free(...)      nit_radix_free(__VA_ARGS__)
# define radix_new(...)       nit_radix_new(__VA_ARGS__)
# define redge_new(...)       nit_redge_new(__VA_ARGS__)
# define redge_split(...)     nit_redge_split(__VA_ARGS__)
# define radix_insert(...)    nit_radix_insert(__VA_ARGS__)
# define radix_iter_set(...)  nit_radix_iter_set(__VA_ARGS__)
# define radix_iter_move(...) nit_radix_iter_move(__VA_ARGS__)
# define radix_iter_get(...)  nit_radix_iter_get(__VA_ARGS__)
#endif
