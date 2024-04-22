/*
 * $Log: str_len.c,v $
 * Revision 1.1  2024-04-22 08:01:53+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "str.h"

unsigned int
str_len(register const char *s)
{
	register const char *t;

	t = s;
	for (;;) {
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

void
getversion_str_len_c()
{
	static char    *x = "$Id: str_len.c,v 1.1 2024-04-22 08:01:53+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
