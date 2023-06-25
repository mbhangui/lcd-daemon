/*
 * $Log: str_rchr.c,v $
 * Revision 1.1  2023-06-24 23:09:47+05:30  Cprogrammer
 * Initial revision
 *
 *
 * $Id: str_rchr.c,v 1.1 2023-06-24 23:09:47+05:30 Cprogrammer Exp mbhangui $
 */
#include "str.h"

unsigned int
str_rchr(register char *s, int c)
{
	register char   ch;
	register const char  *t, *u;

	ch = c;
	t = s;
	u = 0;
	for (;;)
	{
		if (!*t)
			break;
		if (*t == ch)
			u = t;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			u = t;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			u = t;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			u = t;
		++t;
	}
	if (!u)
		u = t;
	return u - s;
}

void
getversion_str_rchr_c()
{
	static char    *x = "$Id: str_rchr.c,v 1.1 2023-06-24 23:09:47+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
