#include <config.h>

#ifndef HAVE_INET_NTOP

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifndef INET_ADDRSTRLEN
#	define INET_ADDRSTRLEN 16
#endif

/*
** This is an IPv4 only inet_ntop(3) replacement function.
*/

const char *inet_ntop(int af, const void *src, char *dst, size_t len) {
	const char *tmp = src;

	if (af != AF_INET) {
		errno = EAFNOSUPPORT;
		return (NULL);
	}

	if (len < INET_ADDRSTRLEN) {
		errno = ENOSPC;
		return (NULL);
	}

	snprintf(dst, len, "%u.%u.%u.%u", tmp[0], tmp[1], tmp[2], tmp[3]);

	return (dst);
}

#endif
