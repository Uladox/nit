/* This file should not be required for any other headers */
/* Include these
   #include <stdlib.h>
   #include "macros.h"
 */

#define palloc(ptr)          (malloc(sizeof(*(ptr))))
#define palloc_a(ptr, size)  (malloc(sizeof(*(ptr)) * (size)))
#define palloc_0(ptr)        (calloc(1, sizeof(*(ptr))))
#define palloc_a0(ptr, size) (calloc((size), sizeof(*(ptr))))

#define pcheck(ptr, error)					\
	do { if (unlikely(!(ptr))) return (error); } while (0)
#define pcheck_c(ptr, error, code)					\
	do { if (unlikely(!(ptr))) { (code); return (error); } } while (0)
