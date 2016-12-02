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

typedef struct nit_ftree Nit_ftree;

typedef struct {
	Nit_ftree *trees;
	Nit_fbnch *bnchs[];
} Nit_fmem;

typedef struct Nit_fdat {
	Nit_fmem *mem;
	void *ext;
	Nit_fnat nat;
	int max_elems;
}

enum nit_fmesd { NIT_FT_DAT, NIT_FT_ANO };

typedef int (*Nit_fsrch)(enum nit_fmesd des, void *acc,
			 void *subj, void *extra);

typedef int (*Nit_fnat)(enum nit_anop op, union nit_ano *subj,
			void *add, void *extra);

void
fbnch_inc_refs(Nit_fbnch *bnch);

Nit_fbnch *
fbnch_new(Nit_fdat *dat, void *elem, int depth);

Nit_fbnch *
fbnch_new_arr(Nit_fdat *dat, void **arr, int cnt, int depth);

void
fbnch_reduce(Nit_fdat *dat, Nit_fbnch *bnch, int depth);

Nit_fbnch *
fbnch_prepend(Nit_fdat *dat, Nit_fbnch *old, void *elem, int depth);

Nit_fbnch *
fbnch_append(Nit_fdat *dat, Nit_fbnch *old, void *elem, int depth);

void *
fbnch_pop(Nit_fdat *dat, Nit_fbnch **bnch, void *elem, int depth);

void *
fbnch_rpop(Nit_fdat *dat, Nit_fbnch **bnch, void *elem, int depth);
