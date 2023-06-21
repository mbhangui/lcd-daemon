/*
 * $Log: byte_cr.c,v $
 * Revision 1.1  2014-08-23 09:05:10+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include "byte.h"

#ifndef lint
static char     sccsid[] = "$Id: byte_cr.c,v 1.1 2014-08-23 09:05:10+05:30 Cprogrammer Exp mbhangui $";
#endif

void
byte_copyr(to, n, from)
	register char  *to;
	register unsigned int n;
	register char  *from;
{
	to += n;
	from += n;
	for (;;)
	{
		if (!n)
			return;
		*--to = *--from;
		--n;
		if (!n)
			return;
		*--to = *--from;
		--n;
		if (!n)
			return;
		*--to = *--from;
		--n;
		if (!n)
			return;
		*--to = *--from;
		--n;
	}
}
