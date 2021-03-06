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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "list.h"
#include "hset.h"
#include "hmap.h"
#include "hmap-out.h"


static inline void
entry_gen(Nit_hentry *entry, FILE *file,
	  void (*dat_print)(FILE *file, void *dat, uint32_t key_size))
{
	int entry_num = 0;

	foreach (entry) {
		++entry_num;
		fputs("&(Nit_hentry) {", file);
		fprintf(file, "\n\t\t.key_size = %"PRIu32, entry->key_size);

		fputs(",\n\t\t.dat = ", file);
		dat_print(file, entry->dat, entry->key_size);

		fputs(",\n\t\t.next.next = ", file);
	}
	fputs("NULL", file);
	for (; entry_num; --entry_num)
		fputc('}', file);
	fputc(',', file);
}

static inline void
bins_gen(Nit_hbin *bin, FILE *file, int bin_num,
	 void (*dat_print)(FILE *file, void *dat, uint32_t key_size))
{
	int i = 0;

	fprintf(file, ",\n.bins = (Nit_hbin[%i]) {", bin_num);

	for (; i != bin_num; ++i, ++bin) {
		fprintf(file, "\n\t[%i].first = ", i);
		entry_gen(bin->first, file, dat_print);
	}

	fputs("\n}\n", file);
}

void
nit_hmap_out(Nit_hmap *map, const char *name,
	     Nit_hmap_out *out, FILE *file)
{
	fputs("/* Do not edit this file, it is generated! */\n"
	      "#include <stdio.h>\n"
	      "#include <stdlib.h>\n"
	      "#include <stdint.h>\n"
	      "\n", file);
	fputs(out->headers, file);
	fputc('\n', file);
	fputs(out->compare, file);
	fputc('\n', file);

	fprintf(file, "\nconst Nit_hmap %s = {", name);

	fprintf(file, ",\n.bin_num = %u", map->bin_num);
	fprintf(file, ",\n.entry_num = %i", map->entry_num);
	fprintf(file, ",\n.primes_pointer = &(int) { %i }",
		*map->primes_pointer);
	bins_gen(map->bins, file, map->bin_num, out->dat_print);

	fputs("\n};", file);

}
