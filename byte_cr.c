/*
 * $Log: byte_cr.c,v $
 * Revision 1.1  2024-04-22 07:57:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"

void
byte_copyr(register char *to, register unsigned int n, register const char *from)
{
	to += n;
	from += n;
	for (;;) {
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

void
getversion_byte_cr_c()
{
	static char    *x = "$Id: byte_cr.c,v 1.1 2024-04-22 07:57:58+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
