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
 * #include <stdint.h>
 * #include <stdio.h>
 * #include "hashmap.h"
 */

typedef struct {
	const char *headers;
	const char *compare;
	void (*key_print)(FILE *file, void *key, uint32_t key_size);
	void (*storage_print)(FILE *file, void *storage);
} Nit_hashmap_out;

void
nit_hashmap_out(Nit_hashmap *map, const char *name,
		Nit_hashmap_out *out, FILE *file);
