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
#include <stdint.h>
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
	int len;
	Nit_joiner *jnr = palloc(jnr);

	pcheck(jnr, NULL);

	if (access(path, F_OK) >= 0 && unlink(path) < 0) {
		perror("unlink");

		return NULL;
	}

	jnr->sd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (jnr->sd < 0) {
		perror("socket");
		free(jnr);

	        return NULL;
	}

	jnr->socket.sun_family = AF_UNIX;
	strcpy(jnr->socket.sun_path, path);
	len = strlen(path) + sizeof(jnr->socket.sun_family);

	if (bind(jnr->sd, (struct sockaddr *) &jnr->socket, len) < 0) {
		perror("bind");
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
	Nit_joint *jnt = palloc(jnt);

	pcheck(jnt, NULL);
	jnt->sd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (jnt->sd < 0) {
		perror("socket");
		free(jnt);

		return NULL;
	}

	jnt->socket.sun_family = AF_UNIX;
	strcpy(jnt->socket.sun_path, path);
	jnt->len = strlen(path) + sizeof(jnt->socket.sun_family);

	pthread_mutex_init(&jnt->end_mutex, NULL);

	if (connect(jnt->sd, (struct sockaddr *) &jnt->socket,
		    jnt->len) < 0) {
		perror("connect");
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

	pcheck(jnt, NULL);

	if (listen(jnr->sd, 5) < 0) {
		perror("listen");
		free(jnt);

		return NULL;
	}

	jnt->len = sizeof(struct sockaddr);
	jnt->sd = accept(jnr->sd,
			(struct sockaddr *) &jnt->socket,
			&jnt->len);

	if (jnt->sd < 0) {
		perror("accept");
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
joint_end_mutate(Nit_joint *jnt, int value)
{
	pthread_mutex_lock(&jnt->end_mutex);
	jnt->end_bool = value;
	pthread_mutex_unlock(&jnt->end_mutex);
}

void
joint_kill(Nit_joint *jnt)
{
	int true_val = 1;

	pthread_mutex_lock(&jnt->end_mutex);
        joint_end_mutate(jnt, 1);
	setsockopt(jnt->sd, SOL_SOCKET, SO_REUSEADDR,
		   &true_val, sizeof(int));
	pthread_mutex_unlock(&jnt->end_mutex);
}

static void
set_zero_timeout(Nit_joint *jnt)
{
	FD_ZERO(&jnt->set);
	FD_SET(jnt->sd, &jnt->set);
	jnt->timeout.tv_sec = 0;
	jnt->timeout.tv_usec = 0;
}

static int
data_to_read(Nit_joint *jnt)
{
	set_zero_timeout(jnt);

	return select(FD_SETSIZE, &jnt->set, NULL, NULL, &jnt->timeout);
}

static int
resize_buf(char **buf, int32_t *old_size, int32_t size)
{
	if (size > *old_size) {
		free(*buf);
		*old_size = size;

		if (!(*buf = malloc(size))) {
			perror("malloc");

			return 0;
		}
	}

	return 1;
}

enum nit_join_status
joint_read(Nit_joint *jnt, char **buf, int32_t *old_size,
	   int32_t *msg_size, int32_t offset)
{
	int retval;
	int32_t size = 0;
	char test;

	pthread_mutex_lock(&jnt->end_mutex);

	retval = data_to_read(jnt);

	if (retval > 0) {
		if (!recv(jnt->sd, &test, 1, MSG_PEEK | MSG_DONTWAIT)) {
			retval = NIT_JOIN_CLOSED;
			goto end;
		}

		recv(jnt->sd, &size, sizeof(size), 0);

		if (!resize_buf(buf, old_size, offset + size)) {
			retval = NIT_JOIN_ERROR;
			goto end;
		}

		*msg_size = recv(jnt->sd, *buf, size, 0);

		if (*msg_size <= 0) {
			if (*msg_size < 0)
				perror("recv");

			jnt->end_bool = 1;
		}
	}
end:
	pthread_mutex_unlock(&jnt->end_mutex);

	return retval;
}

int
joint_send(Nit_joint *jnt, const void *msg, int32_t msg_size)
{
	if (joint_end_check(jnt))
		return 0;

	if (send(jnt->sd, &msg_size, sizeof(msg_size), MSG_NOSIGNAL) < 0 ||
	    send(jnt->sd, msg, msg_size, MSG_NOSIGNAL) < 0) {
		perror("send");

		return 0;
	}

	return 1;
}
