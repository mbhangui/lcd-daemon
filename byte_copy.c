/*
 * $Log: byte_copy.c,v $
 * Revision 1.1  2024-04-22 07:55:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"

#ifndef lint
static char     sccsid[] = "$Id: byte_copy.c,v 1.1 2024-04-22 07:55:22+05:30 Cprogrammer Exp mbhangui $";
#endif

void
byte_copy(register char *to, register unsigned int n, register const char *from)
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
