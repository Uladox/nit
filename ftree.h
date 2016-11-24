/* Include these
 * #include <stdint.h>
 * #include "list.h"
 */

/* Branch size */
#define NIT_FTREE_BS 8

typedef void *(*Nit_fmes)(void **acum, void *val, void *extra);

typedef struct {
	void *ano;
	int refs;
        short cnt, max;
	void *elems[];
} Nit_fbnch;

typedef struct {
	Nit_list deeper;
	void *ano;
        void *pre[NIT_FTREE_BS];
	void *suf[NIT_FTREE_BS];
	uint32_t refs;
	uint8_t precnt, sufcnt;
} Nit_ftree;

typedef struct {
	Nit_ftree *trees;
	Nit_fbnch *bnchs[NIT_FTREE_BS - 1];
} Nit_fmem;

Nit_ftree *
nit_ftree_new(void);

Nit_ftree *
nit_ftree_copy(Nit_ftree *tree, int depth);

void
nit_ftree_reduce(Nit_ftree *tree, int depth);

void *
nit_ftree_first(const Nit_ftree *tree);

void *
nit_ftree_last(const Nit_ftree *tree);

int
nit_ftree_prepend(Nit_ftree *tree, void *elem, int depth);

int
nit_ftree_append(Nit_ftree *tree, void *elem, int depth);

void *
nit_ftree_pop(Nit_ftree *tree, int depth);

void *
nit_ftree_rpop(Nit_ftree *tree, int depth);

Nit_ftree *
nit_ftree_concat(Nit_ftree *left, Nit_ftree *right);

#if defined NIT_SHORT_NAMES || defined NIT_FTREE_SHORT_NAMES
# define FTREE_BS NIT_FTREE_BS
# define ftree_new(...)     n1it_ftree_new(__VA_ARGS__)
# define ftree_copy(...)    nit_ftree_copy(__VA_ARGS__)
# define ftree_reduce(...)  nit_ftree_reduce(__VA_ARGS__)
# define ftree_first(...)   nit_ftree_first(__VA_ARGS__)
# define ftree_last(...)    nit_ftree_last(__VA_ARGS__)
# define ftree_prepend(...) nit_ftree_prepend(__VA_ARGS__)
# define ftree_append(...)  nit_ftree_append(__VA_ARGS__)
# define ftree_pop(...)     nit_ftree_pop(__VA_ARGS__)
# define ftree_rpop(...)    nit_ftree_rpop(__VA_ARGS__)
# define ftree_concat(...)  nit_ftree_concat(__VA_ARGS__)
#endif
