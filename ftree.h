/* Include these
 * #include <stdint.h>
 * #include "list.h"
 * #include "fbnch.h"
 */
typedef struct nit_ftree {
	Nit_list deeper;
	union nit_ano ano;
	Nit_fbnch *pre, *suf;
	uint32_t refs;
} Nit_ftree;

Nit_ftree *
nit_ftree_new(Nit_fdat *dat);

Nit_ftree *
nit_ftree_copy(Nit_fdat *dat, Nit_ftree *tree);

void
nit_ftree_reduce(Nit_fdat *dat, Nit_ftree *tree, int depth);

void *
nit_ftree_first(const Nit_ftree *tree);

void *
nit_ftree_last(const Nit_ftree *tree);

int
nit_ftree_prepend(Nit_fdat *dat, Nit_ftree *tree, void *elem, int depth);

int
nit_ftree_append(Nit_fdat *dat, Nit_ftree *tree, void *elem, int depth);

void *
nit_ftree_pop(Nit_fdat *dat, Nit_ftree *tree, int depth);

void *
nit_ftree_rpop(Nit_fdat *dat, Nit_ftree *tree, int depth);

/* Nit_ftree * */
/* nit_ftree_concat(Nit_fnat nat, Nit_ftree *left, Nit_ftree *right, void *extra); */

/* void * */
/* nit_ftree_search(Nit_fsrch srch, Nit_ftree *tree, void *acc, void *extra); */

#if defined NIT_SHORT_NAMES || defined NIT_FTREE_SHORT_NAMES
# define ftree_new(...)     nit_ftree_new(__VA_ARGS__)
# define ftree_copy(...)    nit_ftree_copy(__VA_ARGS__)
# define ftree_reduce(...)  nit_ftree_reduce(__VA_ARGS__)
# define ftree_first(...)   nit_ftree_first(__VA_ARGS__)
# define ftree_last(...)    nit_ftree_last(__VA_ARGS__)
# define ftree_prepend(...) nit_ftree_prepend(__VA_ARGS__)
# define ftree_append(...)  nit_ftree_append(__VA_ARGS__)
# define ftree_pop(...)     nit_ftree_pop(__VA_ARGS__)
# define ftree_rpop(...)    nit_ftree_rpop(__VA_ARGS__)
# define ftree_concat(...)  nit_ftree_concat(__VA_ARGS__)
# define ftree_search(...)  nit_ftree_search(__VA_ARGS__)
#endif
