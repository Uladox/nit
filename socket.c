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

#include "macros.h"
#include "palloc.h"
#include "socket.h"

Nit_connecter *
nit_connecter_new(const char *path)
{
	int len;
	Nit_connecter *cntr = palloc(cntr);

	pcheck(cntr, NULL);
	cntr->sd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (cntr->sd == -1) {
		perror("socket");
		free(cntr);
		exit(1);
	}

	cntr->socket.sun_family = AF_UNIX;
	strcpy(cntr->socket.sun_path, path);
	unlink(path);
	len = strlen(path) + sizeof(cntr->socket.sun_family);

	if (bind(cntr->sd, (struct sockaddr *)&cntr->socket, len) == -1) {
		perror("bind");
		free(cntr);
		exit(1);
	}

	return cntr;
}

void
nit_connecter_free(Nit_connecter *cntr)
{
	close(cntr->sd);
	free(cntr);
}

Nit_connection *
nit_connection_connect(const char *path)
{
	Nit_connection *cntn = palloc(cntn);

	pcheck(cntn, NULL);
	cntn->sd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (cntn->sd == -1) {
		perror("socket");
		free(cntn);
		return NULL;
	}

	cntn->socket.sun_family = AF_UNIX;
	strcpy(cntn->socket.sun_path, path);

	cntn->len = strlen(path) + sizeof(cntn->socket.sun_family);

	pthread_mutex_init(&cntn->end_mutex, NULL);

	if (connect(cntn->sd, (struct sockaddr *)&cntn->socket,
		    cntn->len) == -1) {
		perror("connect");
		free(cntn);
		return NULL;
	}

	cntn->end_bool = 0;

	return cntn;
}

Nit_connection *
nit_connecter_accept(Nit_connecter *cntr)
{
	Nit_connection *cntn = palloc(cntn);

	pcheck(cntn, NULL);

	if (listen(cntr->sd, 5) == -1) {
		perror("listen");
		return NULL;
	}

	cntn->len = sizeof(struct sockaddr);
	cntn->sd = accept(cntr->sd,
			  (struct sockaddr *) &cntn->socket,
			  &cntn->len);
	if (cntn->sd == -1) {
		perror("accept");
		free(cntn);
		return NULL;
	}

	pthread_mutex_init(&cntn->end_mutex, NULL);
	cntn->end_bool = 0;

	return cntn;
}

void
nit_connection_free(Nit_connection *cntn)
{
	pthread_mutex_destroy(&cntn->end_mutex);
	close(cntn->sd);
	free(cntn);
}

int
nit_connection_end_check(Nit_connection *cntn)
{
	int value;

	pthread_mutex_lock(&cntn->end_mutex);
	value = cntn->end_bool;
	pthread_mutex_unlock(&cntn->end_mutex);
	return value;
}

void
nit_connection_end_mutate(Nit_connection *cntn, int value)
{
	pthread_mutex_lock(&cntn->end_mutex);
	cntn->end_bool = value;
	pthread_mutex_unlock(&cntn->end_mutex);
}

void
nit_connection_kill(Nit_connection *cntn)
{
	int true_val = 1;

	pthread_mutex_lock(&cntn->end_mutex);
	nit_connection_end_mutate(cntn, 1);
	setsockopt(cntn->sd, SOL_SOCKET, SO_REUSEADDR,
		   &true_val, sizeof(int));
	pthread_mutex_unlock(&cntn->end_mutex);
}

int
nit_connection_read(Nit_connection *cntn,
		    char **str, uint32_t *old_size,
		    int *msg_size, uint32_t offset)
{
	int retval;
	uint32_t size = 0;
	uint32_t offset_size = offset;

	pthread_mutex_lock(&cntn->end_mutex);

	FD_ZERO(&cntn->set);
	FD_SET(cntn->sd,
	       &cntn->set);
	cntn->timeout.tv_sec = 0;
	cntn->timeout.tv_usec = 0;

	retval = select(FD_SETSIZE, &cntn->set, NULL, NULL,
			&cntn->timeout);
	if (retval == 1) {
		recv(cntn->sd, &size, sizeof(uint32_t), 0);
		offset_size += size;

		if (offset_size > *old_size) {
			free(*str);
			*str = malloc(offset_size);
			*old_size = offset_size;
		}

		*msg_size = recv(cntn->sd, *str, size, 0);

		if (*msg_size <= 0) {
			if (*msg_size < 0)
				perror("recv");
			cntn->end_bool = 1;
		}
	}

	pthread_mutex_unlock(&cntn->end_mutex);
	return retval;
}

void
nit_connection_send(Nit_connection *cntn,
		    const void *msg, uint32_t msg_size)
{
	if (!nit_connection_end_check(cntn))
		if (send(cntn->sd, msg, msg_size, MSG_NOSIGNAL) < 0) {
			perror("send");
			nit_connection_end_mutate(cntn, 1);
		}
}
