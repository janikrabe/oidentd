#ifndef __IPV6_MISSING_H
#define __IPV6_MISSING_H

#ifndef EAI_NODATA
#	define EAI_NODATA	1
#	define EAI_MEMORY	2
#endif

#ifndef AI_PASSIVE
#	define AI_PASSIVE		1
#	define AI_CANONNAME		2
#endif

#ifndef NI_NUMERICHOST
#	define NI_NUMERICHOST	2
#	define NI_NAMEREQD		4
#	define NI_NUMERICSERV	8
#endif

#ifndef HAVE_STRUCT_ADDRINFO

struct addrinfo {
	int	ai_flags;
	int	ai_family;
	int	ai_socktype;
	int	ai_protocol;
	size_t ai_addrlen;
	char *ai_canonname;
	struct sockaddr *ai_addr;
	struct addrinfo *ai_next;
};

#endif

#ifndef HAVE_GETADDRINFO
int getaddrinfo(const char *hostname, const char *servname, 
				const struct addrinfo *hints, struct addrinfo **res);
#endif

#ifndef HAVE_GAI_STRERROR
char *gai_strerror(int ecode);
#endif /* !HAVE_GAI_STRERROR */

#ifndef HAVE_FREEADDRINFO
void freeaddrinfo(struct addrinfo *ai);
#endif

#ifndef HAVE_GETNAMEINFO
int getnameinfo(const struct sockaddr *sa, size_t salen, char *host, 
				size_t hostlen, char *serv, size_t servlen, int flags);
#endif

#ifndef NI_MAXSERV
#	define NI_MAXSERV 32
#endif

#ifndef NI_MAXHOST
#	define NI_MAXHOST 1025
#endif

#endif
