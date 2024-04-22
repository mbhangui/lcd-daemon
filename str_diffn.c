/*
 * $Log: str_diffn.c,v $
 * Revision 1.1  2024-04-22 08:02:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "str.h"

int
str_diffn(register const char *s, register const char *t, unsigned int len)
{
	register char   x;

	for (;;) {
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

void
getversion_str_diffn_c()
{
	static char    *x = "$Id: str_diffn.c,v 1.1 2024-04-22 08:02:34+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
