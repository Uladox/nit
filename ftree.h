/* Include these
 * #include "list.h"
 */

/* Branch size */
#define NIT_FTREE_BS 4

typedef struct {
	Nit_list deeper;
	short precnt, sufcnt;
	void *pre[NIT_FTREE_BS]; /* grows -> */
	void *suf[NIT_FTREE_BS]; /* grows -> */
} Nit_ftree;

Nit_ftree *
nit_ftree_new(void);

void *
nit_ftree_first(const Nit_ftree *tree);

void *
nit_ftree_last(const Nit_ftree *tree);

int
nit_ftree_prepend(Nit_ftree *tree, void *elem);

int
nit_ftree_append(Nit_ftree *tree, void *elem);

void *
nit_ftree_pop(Nit_ftree *tree);

#if defined NIT_SHORT_NAMES || defined NIT_FTREE_SHORT_NAMES
# define FTREE_BS NIT_FTREE_BS
# define ftree_new(...)     nit_ftree_new(__VA_ARGS__)
# define ftree_first(...)   nit_ftree_first(__VA_ARGS__)
# define ftree_last(...)    nit_ftree_last(__VA_ARGS__)
# define ftree_prepend(...) nit_ftree_prepend(__VA_ARGS__)
# define ftree_append(...)  nit_ftree_append(__VA_ARGS__)
# define ftree_pop(...)     nit_ftree_pop(__VA_ARGS__)
#endif
