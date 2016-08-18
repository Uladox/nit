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
 * #include <pthread.h>
 * #include <stdint.h>
 * #include <sys/select.h>
 * #include <sys/socket.h>
 * #include <sys/un.h>
 */

typedef struct {
	struct sockaddr_un socket;
	int sd;
} Nit_connecter;

typedef struct {
	struct sockaddr_un socket;
	int sd;

	fd_set set;
	struct timeval timeout;

	pthread_mutex_t end_mutex;
	int end_bool;
	unsigned int len;
} Nit_connection;

Nit_connecter *
nit_connecter_new(const char *path);

void
nit_connecter_free(Nit_connecter *cntr);

Nit_connection *
nit_connection_connect(const char *path);

Nit_connection *
nit_connecter_accept(Nit_connecter *cntr);

void
nit_connection_free(Nit_connection *cntn);

int
nit_connection_end_check(Nit_connection *cntn);

void
nit_connection_end_mutate(Nit_connection *cntn, int value);

void
nit_connection_kill(Nit_connection *cntn);

int
nit_connection_read(Nit_connection *cntn,
		    char **str, uint32_t *old_size,
		    int *message_size, uint32_t offset);

void
nit_connection_send(Nit_connection *cntn,
		    const void *msg, uint32_t msg_size);

#if defined NIT_SHORT_NAMES || defined NIT_SOCKET_SHORT_NAMES
# define connecter_new(...)         nit_connecter_new(__VA_ARGS__)
# define connecter_free(...)        nit_connecter_free(__VA_ARGS__)
# define connection_connect(...)    nit_connection_connect(__VA_ARGS__)
# define connecter_accept(...)      nit_connecter_accept(__VA_ARGS__)
# define connection_free(...)       nit_connection_free(__VA_ARGS__)
# define connection_end_check(...)  nit_connection_end_check(__VA_ARGS__)
# define connection_end_mutate(...) nit_connection_end_mutate(__VA_ARGS__)
# define connection_kill(...)       nit_connection_kill(__VA_ARGS__)
# define connection_read(...)       nit_connection_read(__VA_ARGS__)
# define connection_send(...)       nit_connection_send(__VA_ARGS__)
#endif
