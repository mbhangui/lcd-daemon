/*
 * $Log: str_diffn.c,v $
 * Revision 1.1  2014-08-18 10:22:24+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include "str.h"

#ifndef	lint
static char     sccsid[] = "$Id: str_diffn.c,v 1.1 2014-08-18 10:22:24+05:30 Cprogrammer Exp mbhangui $";
#endif

int
str_diffn(s, t, len)
	register char  *s;
	register char  *t;
	unsigned int    len;
{
	register char   x;

	for (;;)
	{
		if (!len--)
			return 0;
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
		if (!len--)
			return 0;
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
		if (!len--)
			return 0;
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
		if (!len--)
			return 0;
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
	}
	return ((int) (unsigned int) (unsigned char) x) - ((int) (unsigned int) (unsigned char) *t);
}
