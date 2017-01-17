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

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define NIT_SHORT_NAMES
#include "macros.h"
#include "palloc.h"
#include "socket.h"

Nit_joiner *
joiner_new(const char *path)
{
	socklen_t len;
	Nit_joiner *jnr = palloc(jnr);

	pcheck(jnr, NULL);
	jnr->sd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (jnr->sd < 0) {
		free(jnr);
	        return NULL;
	}

	memset(&jnr->socket, 0, sizeof(jnr->socket));
	jnr->socket.sun_family = AF_UNIX;
	strcpy(jnr->socket.sun_path + 1, path);
	len = strlen(path) + sizeof(jnr->socket.sun_family) + 1;

	if (bind(jnr->sd, (struct sockaddr *) &jnr->socket, len) < 0) {
		free(jnr);
	        return NULL;
	}

	return jnr;
}

void
joiner_free(Nit_joiner *jnr)
{
	close(jnr->sd);
	free(jnr);
}

Nit_joint *
joint_connect(const char *path)
{
	socklen_t len;
	Nit_joint *jnt = palloc(jnt);

	pcheck(jnt, NULL);
	jnt->sd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (jnt->sd < 0) {
		free(jnt);
		return NULL;
	}

	memset(&jnt->socket, 0, sizeof(jnt->socket));
	jnt->socket.sun_family = AF_UNIX;
	strcpy(jnt->socket.sun_path + 1, path);
        len = strlen(path) + sizeof(jnt->socket.sun_family) + 1;

	pthread_mutex_init(&jnt->end_mutex, NULL);

	if (connect(jnt->sd, (struct sockaddr *) &jnt->socket, len) < 0) {
		free(jnt);
		return NULL;
	}

	jnt->end_bool = 0;
	return jnt;
}

Nit_joint *
joiner_accept(Nit_joiner *jnr)
{
	Nit_joint *jnt = palloc(jnt);
	socklen_t addrlen = sizeof(struct sockaddr);

	pcheck(jnt, NULL);

	if (listen(jnr->sd, 5) < 0) {
		free(jnt);

		return NULL;
	}

	jnt->sd = accept(jnr->sd, (struct sockaddr *) &jnt->socket, &addrlen);

	if (jnt->sd < 0) {
		free(jnt);

		return NULL;
	}

	pthread_mutex_init(&jnt->end_mutex, NULL);
	jnt->end_bool = 0;
	return jnt;
}

void
joint_free(Nit_joint *jnt)
{
	pthread_mutex_destroy(&jnt->end_mutex);
	close(jnt->sd);
	free(jnt);
}

int
joint_end_check(Nit_joint *jnt)
{
	int value;

	pthread_mutex_lock(&jnt->end_mutex);
	value = jnt->end_bool;
	pthread_mutex_unlock(&jnt->end_mutex);
	return value;
}

void
joint_end(Nit_joint *jnt)
{
	pthread_mutex_lock(&jnt->end_mutex);
	jnt->end_bool = 1;
	pthread_mutex_unlock(&jnt->end_mutex);
}

void
joint_kill(Nit_joint *jnt)
{
	int true_val = 1;

	pthread_mutex_lock(&jnt->end_mutex);
        joint_end(jnt);
	setsockopt(jnt->sd, SOL_SOCKET, SO_REUSEADDR,
		   &true_val, sizeof(int));
	pthread_mutex_unlock(&jnt->end_mutex);
}

int
joint_ready(Nit_joint *jnt)
{
	fd_set set;
	struct timeval timeout = {
		.tv_sec = 0,
		.tv_usec = 0
	};


	FD_ZERO(&set);
	FD_SET(jnt->sd, &set);
	return select(FD_SETSIZE, &set, NULL, NULL, &timeout);
}

static int
resize_buf(char **buf, size_t *old_size, size_t size)
{
	if (size > *old_size) {
		free(*buf);
		*old_size = size;

		if (!(*buf = malloc(size)))
			return 0;
	}

	return 1;
}

enum nit_join_status
joint_read(Nit_joint *jnt, char **buf, size_t *old_size,
	   ssize_t *msg_size, size_t offset)
{
	int state = joint_ready(jnt);
	size_t size = 0;
	char test;

	if (!state)
	        return NIT_JOIN_NONE;

	if (state < 0) {
		joint_end(jnt);
	        return NIT_JOIN_ERROR;
	}

	if (!recv(jnt->sd, &test, 1, MSG_PEEK | MSG_DONTWAIT)) {
	        joint_end(jnt);
	        return NIT_JOIN_CLOSED;
	}

	recv(jnt->sd, &size, sizeof(size), 0);

	if (!resize_buf(buf, old_size, offset + size)) {
		joint_end(jnt);
	        return NIT_JOIN_ERROR;
	}

	*msg_size = recv(jnt->sd, *buf, size, 0);

	if (*msg_size < 0) {
	        joint_end(jnt);
	        return NIT_JOIN_ERROR;
	}

	if (!*msg_size)
		joint_end(jnt);

	return NIT_JOIN_OK;
}

int
joint_send(Nit_joint *jnt, const void *msg, size_t msg_size)
{
	if (joint_end_check(jnt))
		return 0;

	if (send(jnt->sd, &msg_size, sizeof(msg_size), MSG_NOSIGNAL) < 0 ||
	    send(jnt->sd, msg, msg_size, MSG_NOSIGNAL) < 0) {
		return 0;
	}

	return 1;
}
