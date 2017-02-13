typedef enum { NIT_ARTR8, NIT_ARTR16,
	       NIT_ARTR48, NIT_ARTR256,
	       NIT_ARTR_EDGE, NIT_ARTR_EDGE_WITH_VAL } Nit_artr_type;

typedef struct nit_artr {
	Nit_artr_type type;
	unsigned int count;
	void *val; /* used as next for edge, unless with val */
} Nit_artr;

typedef struct nit_artr_node8 {
	Nit_artr artr;
	uint8_t keys[8];
	Nit_artr *sub[8];
} Nit_artr_node8;

typedef struct nit_artr_node16 {
	Nit_artr artr;
	uint8_t keys[16];
	Nit_artr *sub[16];
} Nit_artr_node16;

typedef struct nit_artr_node48 {
	Nit_artr artr;
	uint8_t keys[256];
	Nit_artr *sub[48];
} Nit_artr_node48;

typedef struct nit_artr_node256 {
	Nit_artr artr;
	Nit_artr *sub[256];
} Nit_artr_node256;

typedef struct nit_artr_edge {
	Nit_artr artr;
	uint8_t *str;
} Nit_artr_edge;

typedef struct nit_artr_iter {
	Nit_artr_type type;
	unsigned int count;
	Nit_artr *artr;
} Nit_artr_iter;

typedef struct nit_artr_reuse {
	Nit_artr_node8   *node8s;
	Nit_artr_node16  *node16s;
	Nit_artr_node48  *node48s;
	Nit_artr_node256 *node256s;
	Nit_artr_edge    *edges;
} Nit_artr_reuse;

#if defined NIT_SHORT_NAMES || defined NIT_ARTR_SHORT_NAMES
# define ARTR8 NIT_ARTR8
# define ARTR16 NIT_ARTR16
# define ARTR48 NIT_ARTR48
# define ARTR256 NIT_ARTR256
# define ARTR_EDGE NIT_ARTR_EDGE
# define ARTR_EDGE_WITH_VAL NIT_ARTR_EDGE_WITH_VAL
#endif
