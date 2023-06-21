/*
 * $Log: str_chr.c,v $
 * Revision 1.1  2014-08-18 10:22:12+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include "str.h"

#ifndef	lint
static char     sccsid[] = "$Id: str_chr.c,v 1.1 2014-08-18 10:22:12+05:30 Cprogrammer Exp mbhangui $";
#endif

unsigned int
str_chr(s, c)
	register char  *s;
	int             c;
{
	register char   ch;
	register char  *t;

	ch = c;
	t = s;
	for (;;)
	{
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
	}
	return t - s;
}
