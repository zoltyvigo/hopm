#ifndef INET_H
#define INET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifndef AF_INET6
#define AF_INET6 10
#endif

#ifndef HAVE_INET_PTON
extern int inet_pton(int, const char *, void *);
#endif
extern char *inetntop(int, const void *, char *, unsigned int);
extern struct hostent *bopm_gethostbyname(const char *);

#endif /* INET_H */
