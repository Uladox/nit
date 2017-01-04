/*    This file is part of nitlib.
 *
 *    Nitlib is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Foobar is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */


/* Include these
   #include <stdint.h>
   #include <stdlib.h>
 */

void
nit_get_u8(const char *charset, const char *src, size_t src_size,
	   uint8_t **result, size_t *result_size);

void
nit_get_encoded(const char *charset, const uint8_t *src, size_t src_size,
		char **result, size_t *result_size);

#if defined NIT_SHORT_NAMES || defined NIT_IO_SHORT_NAMES
# define get_u8(...)      nit_get_u8(__VA_ARGS__)
# define get_encoded(...) nit_get_encoded(__VA_ARGS__)
#endif
