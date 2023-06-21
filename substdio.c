/*
 * $Log: substdio.c,v $
 * Revision 1.1  2014-09-01 19:48:14+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include "substdio.h"
#include <sys/types.h>

#ifndef lint
static char     sccsid[] = "$Id: substdio.c,v 1.1 2014-09-01 19:48:14+05:30 Cprogrammer Exp mbhangui $";
#endif

void
substdio_fdbuf(s, op, fd, buf, len)
	register substdio *s;
	register ssize_t (*op) ();
	register int    fd;
	register char  *buf;
	register int    len;
{
	s->x = buf;
	s->fd = fd;
	s->op = op;
	s->p = 0;
	s->n = len;
}
