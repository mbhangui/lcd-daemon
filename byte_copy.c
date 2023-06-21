/*
 * $Log: byte_copy.c,v $
 * Revision 1.1  2014-08-18 10:18:09+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include "byte.h"

#ifndef lint
static char     sccsid[] = "$Id: byte_copy.c,v 1.1 2014-08-18 10:18:09+05:30 Cprogrammer Exp mbhangui $";
#endif

void
byte_copy(to, n, from)
	register char  *to;
	register unsigned int n;
	register char  *from;
{
	for (;;)
	{
		if (!n)
			return;
		*to++ = *from++;
		--n;
		if (!n)
			return;
		*to++ = *from++;
		--n;
		if (!n)
			return;
		*to++ = *from++;
		--n;
		if (!n)
			return;
		*to++ = *from++;
		--n;
	}
}
