/*    This file is part of nit.
 *
 *    Nit is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Nit is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with nit.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This file should not be required for any other headers */

/* Include these
   #include <stdlib.h>
   #include "macros.h"
 */

#define palloc(ptr)          (malloc(sizeof(*(ptr))))
#define palloc_a(ptr, size)  (malloc(sizeof(*(ptr)) * (size)))
#define palloc_0(ptr)        (calloc(1, sizeof(*(ptr))))
#define palloc_a0(ptr, size) (calloc((size), sizeof(*(ptr))))

#define pcheck(ptr, ret_val)						\
	do { if (unlikely(!(ptr))) return (ret_val); } while (0)

#define pcheck_c(ptr, ret_val, ...)					\
	do {								\
		if (unlikely(!(ptr))) {					\
			(__VA_ARGS__);					\
			return (ret_val);				\
		}							\
	} while (0)

#define pcheck_g(ptr, label)					\
	do { if (unlikely(!(ptr))) goto label; } while (0)

#define pcheck_gc(ptr, label, ...)					\
	do {								\
		if (unlikely(!(ptr))) {					\
			(__VA_ARGS__);					\
		        goto label;					\
		}							\
	} while (0)
