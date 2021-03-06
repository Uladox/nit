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

#if defined(__GNUC__) || defined(__clang__)
# define likely(val)   __builtin_expect(!!(val), 1)
# define unlikely(val) __builtin_expect(!!(val), 0)
#else
# define likely(val)   (val)
# define unlikely(val) (val)
#endif

#define ARRAY_UNITS(array) (sizeof(array) / sizeof(*array))

#define QUOTE(...) #__VA_ARGS__

#define ANY_TYPE(val) ((void *) (val))

/* If used, #include <stddef.h> */
#define CONTAINER(ptr, type, member)		\
	((type *) ((char *)(ptr) - offsetof(type, member))
