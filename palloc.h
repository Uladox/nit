/* Include these
 * #include <stdlib.h>
 */

#define palloc(ptr) (malloc(sizeof(*(ptr))))
#define palloc_0(ptr) (calloc(1, sizeof(*(ptr))))
#define pcheck(ptr, error) do { if (ptr) return (error); } while (0)
