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
} Nit_joiner;

typedef struct {
	struct sockaddr_un socket;
	int sd;

	fd_set set;
	struct timeval timeout;

	pthread_mutex_t end_mutex;
	int end_bool;
	unsigned int len;
} Nit_joint;

enum nit_join_status {
	NIT_JOIN_CLOSED = -2,
	NIT_JOIN_ERROR,
	NIT_JOIN_NONE,
	NIT_JOIN_OK,
};

Nit_joiner *
nit_joiner_new(const char *path);

void
nit_joiner_free(Nit_joiner *jnr);

Nit_joint *
nit_joint_connect(const char *path);

Nit_joint *
nit_joiner_accept(Nit_joiner *jnr);

void
nit_joint_free(Nit_joint *jnt);

int
nit_joint_end_check(Nit_joint *jnt);

void
nit_joint_end_mutate(Nit_joint *jnt, int value);

void
nit_joint_kill(Nit_joint *jnt);

enum nit_join_status
nit_joint_read(Nit_joint *jnt, char **buf, int32_t *old_size,
	       int32_t *msg_size, int32_t offset);

int
nit_joint_send(Nit_joint *jnt, const void *msg, int32_t msg_size);

#if defined NIT_SHORT_NAMES || defined NIT_SOCKET_SHORT_NAMES
# define joiner_new(...)       nit_joiner_new(__VA_ARGS__)
# define joiner_free(...)      nit_joiner_free(__VA_ARGS__)
# define joint_connect(...)    nit_joint_connect(__VA_ARGS__)
# define joiner_accept(...)    nit_joiner_accept(__VA_ARGS__)
# define joint_free(...)       nit_joint_free(__VA_ARGS__)
# define joint_end_check(...)  nit_joint_end_check(__VA_ARGS__)
# define joint_end_mutate(...) nit_joint_end_mutate(__VA_ARGS__)
# define joint_kill(...)       nit_joint_kill(__VA_ARGS__)
# define joint_read(...)       nit_joint_read(__VA_ARGS__)
# define joint_send(...)       nit_joint_send(__VA_ARGS__)
#endif
