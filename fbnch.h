/* Include these
 * #include <stdint.h>
 */

enum nit_anop {
	NIT_FT_RESET,
	NIT_FT_DEC,
	NIT_FT_MES_DAT,
	NIT_FT_MES_ANO,
	NIT_FT_COPY
};

union nit_ano {
	float flt;
	intptr_t num;
	void *ptr;
};

typedef struct {
	union nit_ano ano;
	int refs;
        short cnt, max;
	void *elems[];
} Nit_fbnch;

struct nit_ftree;

typedef struct {
        struct nit_ftree *trees;
	Nit_fbnch *bnchs[];
} Nit_fmem;

enum nit_fmesd { NIT_FT_DAT, NIT_FT_ANO };

typedef int (*Nit_fsrch)(enum nit_fmesd des, void *acc,
			 void *subj, void *extra);

typedef int (*Nit_fnat)(enum nit_anop op, union nit_ano *subj,
			void *add, void *extra);

typedef struct {
	Nit_fmem *mem;
	void *ext;
	Nit_fnat nat;
	int max_elems;
} Nit_fdat;

static inline void
nit_fdat_set(Nit_fdat *dat, Nit_fmem *mem, void *ext,
	     Nit_fnat nat, int max_elems)
{
	dat->mem = mem;
	dat->ext = ext;
	dat->nat = nat;
	dat->max_elems = max_elems;
}

void
nit_fbnch_inc_refs(Nit_fbnch *bnch);

Nit_fbnch *
nit_fbnch_new(Nit_fdat *dat, void *elem, int depth);

Nit_fbnch *
nit_fbnch_new_arr(Nit_fdat *dat, void **arr, int cnt, int depth);

void
nit_fbnch_reduce(Nit_fdat *dat, Nit_fbnch *bnch, int depth);

void *
nit_fbnch_first(Nit_fbnch *bnch);

void *
nit_fbnch_last(Nit_fbnch *bnch);

int
nit_fbnch_prepend(Nit_fdat *dat, Nit_fbnch **bnch, void *elem, int depth);

int
nit_fbnch_append(Nit_fdat *dat, Nit_fbnch **bnch, void *elem, int depth);

void *
nit_fbnch_pop(Nit_fdat *dat, Nit_fbnch **bnch, int depth);

void *
nit_fbnch_rpop(Nit_fdat *dat, Nit_fbnch **bnch, int depth);

void *
nit_fbnch_search(Nit_fsrch srch, Nit_fbnch *bnch,
		 void *acc, void *ext, int depth);

#if defined NIT_SHORT_NAMES || NIT_FBNCH_SHORT_NAMES
# define FT_RESET    NIT_FT_RESET
# define FT_INC      NIT_FT_INC
# define FT_DEC      NIT_FT_DEC
# define FT_MES_DAT  NIT_FT_MES_DAT
# define FT_MES_ANO  NIT_FT_MES_ANO
# define FT_COPY     NIT_FT_COPY
# define FT_DAT NIT_FT_DAT
# define FT_ANO NIT_FT_ANO
# define fdat_set(...)       nit_fdat_set(__VA_ARGS__)
# define fbnch_inc_refs(...) nit_fbnch_inc_refs(__VA_ARGS__)
# define fbnch_new(...)      nit_fbnch_new(__VA_ARGS__)
# define fbnch_new_arr(...)  nit_fbnch_new_arr(__VA_ARGS__)
# define fbnch_reduce(...)   nit_fbnch_reduce(__VA_ARGS__)
# define fbnch_first(...)    nit_fbnch_first(__VA_ARGS__)
# define fbnch_last(...)     nit_fbnch_last(__VA_ARGS__)
# define fbnch_prepend(...)  nit_fbnch_prepend(__VA_ARGS__)
# define fbnch_append(...)   nit_fbnch_append(__VA_ARGS__)
# define fbnch_pop(...)      nit_fbnch_pop(__VA_ARGS__)
# define fbnch_rpop(...)     nit_fbnch_rpop(__VA_ARGS__)
# define fbnch_search(...)   nit_fbnch_search(__VA_ARGS__)
#endif
