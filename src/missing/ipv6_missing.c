/*
** ipv6.c - Compatibility functions.
**
** This file includes getaddrinfo(), freeaddrinfo() and gai_strerror().
** These funtions are defined in rfc2133.
**
** But these functions are not implemented correctly. The minimum subset
** is implemented for ssh use only. For exapmle, this routine assumes
** that ai_family is AF_INET. Don't use it for another purpose.
**
** This code was taken from OpenSSH. It is distributed
** under a two-term BSD license.
**
** Modifications are Copyright (C) 2001-2006 Ryan McCabe <ryan@numb.org>
*/

#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>
#include <netdb.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <oidentd_missing.h>

#ifndef HAVE_GAI_STRERROR

char *gai_strerror(int ecode) {
	switch (ecode) {
		case EAI_NODATA:
			return ("no address associated with hostname");
		case EAI_MEMORY:
			return ("memory allocation failure");
		default:
			return ("unknown error");
	}
}

#endif

#ifndef HAVE_FREEADDRINFO

void freeaddrinfo(struct addrinfo *ai) {

	while (ai != NULL) {
		struct addrinfo *next = ai->ai_next;

		free(ai);
		ai = next;
	}
}

#endif

#ifndef HAVE_GETADDRINFO

static struct addrinfo *malloc_ai(int port, in_addr_t addr) {
	struct addrinfo *ai;

	ai = malloc(sizeof(struct addrinfo) + sizeof(struct sockaddr_in));
	if (ai == NULL)
		return (NULL);

	memset(ai, 0, sizeof(struct addrinfo) + sizeof(struct sockaddr_in));

	ai->ai_addr = (struct sockaddr *) (ai + 1);
	ai->ai_addrlen = sizeof(struct sockaddr_in);
	ai->ai_addr->sa_family = ai->ai_family = AF_INET;

	((struct sockaddr_in *) (ai)->ai_addr)->sin_port = port;
	((struct sockaddr_in *) (ai)->ai_addr)->sin_addr.s_addr = addr;

	return (ai);
}

int getaddrinfo(const char *hostname, const char *servname,
				const struct addrinfo *hints, struct addrinfo **res)
{
	struct addrinfo *cur, *prev = NULL;
	struct hostent *hp;
	struct in_addr in;
	int i, port;

	if (servname)
		port = htons(atoi(servname));
	else
		port = 0;

	if (hints != NULL && hints->ai_flags & AI_PASSIVE) {
		*res = malloc_ai(port, htonl(0x00000000));
		if (*res != NULL)
			return (0);
		else
			return (EAI_MEMORY);
	}

	if (hostname == NULL) {
		*res = malloc_ai(port, htonl(0x7f000001));
		if (*res != NULL)
			return (0);
		else
			return (EAI_MEMORY);
	}

	if (inet_aton(hostname, &in) != 0) {
		*res = malloc_ai(port, in.s_addr);
		if (*res != NULL)
			return (0);
		else
			return (EAI_MEMORY);
	}

	hp = gethostbyname(hostname);
	if (hp != NULL && hp->h_name != NULL && hp->h_name[0] != 0 &&
		hp->h_addr_list[0] != NULL)
	{
		for (i = 0 ; hp->h_addr_list[i] ; i++) {
			cur = malloc_ai(port,
							((struct in_addr *) hp->h_addr_list[i])->s_addr);

			if (cur == NULL) {
				if (*res != NULL)
					freeaddrinfo(*res);

				return (EAI_MEMORY);
			}

			if (prev != NULL)
				prev->ai_next = cur;
			else
				*res = cur;

			prev = cur;
		}

		return (0);
	}

	return (EAI_NODATA);
}

#endif

#ifndef HAVE_GETNAMEINFO

int getnameinfo(const struct sockaddr *sa, size_t salen, char *host,
				size_t hostlen, char *serv, size_t servlen, int flags)
{
	struct sockaddr_in *sin4 = (struct sockaddr_in *) sa;
	struct hostent *hp;
	char tmpserv[16];

	(void) salen;

	if (serv != NULL) {
		snprintf(tmpserv, sizeof(tmpserv), "%d", ntohs(sin4->sin_port));
		if (strlen(tmpserv) >= servlen)
			return (EAI_MEMORY);
		else
			strcpy(serv, tmpserv);
	}

	if (host != NULL) {
		if (flags & NI_NUMERICHOST) {
			if (strlen(inet_ntoa(sin4->sin_addr)) >= hostlen)
				return (EAI_MEMORY);

			strcpy(host, inet_ntoa(sin4->sin_addr));
			return (0);
		} else {
			hp = gethostbyaddr((char *) &sin4->sin_addr,
					sizeof(struct in_addr), AF_INET);
			if (hp == NULL)
				return (EAI_NODATA);

			if (strlen(hp->h_name) >= hostlen)
				return (EAI_MEMORY);

			strcpy(host, hp->h_name);
			return (0);
		}
	}

	return (0);
}

#endif
