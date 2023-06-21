/*
 * $Log: str_len.c,v $
 * Revision 1.1  2014-08-18 10:22:33+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include "str.h"

#ifndef	lint
static char     sccsid[] = "$Id: str_len.c,v 1.1 2014-08-18 10:22:33+05:30 Cprogrammer Exp mbhangui $";
#endif

unsigned int
str_len(s)
	register char  *s;
{
	register char  *t;

	t = s;
	for (;;)
	{
		if (!*t)
			return t - s;
		++t;
		if (!*t)
			return t - s;
		++t;
		if (!*t)
			return t - s;
		++t;
		if (!*t)
			return t - s;
		++t;
	}
}
