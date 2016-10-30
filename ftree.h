/* Include these
 * #include "list.h"
 */

/* Branch size */
#define NIT_FTREE_BS 4

typedef struct {
	Nit_list deeper;
	short precnt, sufcnt;
	void *pre[NIT_FTREE_BS]; /* <- grows */
	void *suf[NIT_FTREE_BS]; /* grows -> */
} Nit_ftree;


Nit_ftree *
ftree_new(void);

void *
ftree_first(Nit_ftree *tree);

void *
ftree_last(Nit_ftree *tree);

int
ftree_prepend(Nit_ftree *tree, void *elem);


#if defined NIT_SHORT_NAMES || defined NIT_FTREE_SHORT_NAMES
#define FTREE_BS NIT_FTREE_BS
#endif
