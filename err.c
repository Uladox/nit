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

#include <errno.h>

#include "err.h"

enum nit_err
nit_err_code(void)
{
	return errno;
}

const char *
nit_err_str(void)
{
	switch (nit_err_code()) {
	case NIT_ERR_MEM:
		return "out of memory";
	}

	return "error getting error string";
}
