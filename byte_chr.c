/*
 * $Log: byte_chr.c,v $
 * Revision 1.1  2014-09-01 20:02:36+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include "byte.h"

#ifndef lint
static char     sccsid[] = "$Id: byte_chr.c,v 1.1 2014-09-01 20:02:36+05:30 Cprogrammer Exp mbhangui $";
#endif

unsigned int
byte_chr(s, n, c)
	char           *s;
	register unsigned int n;
	int             c;
{
	register char   ch;
	register char  *t;

	ch = c;
	t = s;
	for (;;)
	{
		if (!n)
			break;
		if (*t == ch)
			break;
		++t;
		--n;
		if (!n)
			break;
		if (*t == ch)
			break;
		++t;
		--n;
		if (!n)
			break;
		if (*t == ch)
			break;
		++t;
		--n;
		if (!n)
			break;
		if (*t == ch)
			break;
		++t;
		--n;
	}
	return t - s;
}
