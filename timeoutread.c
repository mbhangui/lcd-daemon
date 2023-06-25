/*
 * $Log: timeoutread.c,v $
 * Revision 1.2  2023-06-25 09:35:11+05:30  Cprogrammer
 * changed timeout to milliseconds
 *
 * Revision 1.1  2023-06-24 23:08:16+05:30  Cprogrammer
 * Initial revision
 *
 *
 * $Id: timeoutread.c,v 1.2 2023-06-25 09:35:11+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include "timeoutread.h"
#include "select.h"
#include "error.h"

ssize_t
timeoutread_fifo(long t, int fd, char *buf, size_t len)
{
	fd_set          rfds;
	struct timeval  tv;

	tv.tv_sec = 0;
	tv.tv_usec = 1000 * t;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	if (select(fd + 1, &rfds, (fd_set *) 0, (fd_set *) 0, &tv) == -1)
		return -1;
	if (FD_ISSET(fd, &rfds))
		return read(fd, buf, len);

	errno = error_timeout;
	return -1;
}

ssize_t
timeoutread_sock(long t, int fd, char *buf, size_t len)
{
	fd_set          rfds;
	struct timeval  tv;

	tv.tv_sec = 0;
	tv.tv_usec = 1000 * t;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	if (select(fd + 1, &rfds, (fd_set *) 0, (fd_set *) 0, &tv) == -1)
		return -1;
	if (FD_ISSET(fd, &rfds))
		return recv(fd, buf, len, MSG_DONTWAIT);

	errno = error_timeout;
	return -1;
}
