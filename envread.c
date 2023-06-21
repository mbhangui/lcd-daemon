/*
 * $Log: envread.c,v $
 * Revision 1.1  2014-08-18 10:19:13+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include "env.h"
#include "str.h"

#ifndef	lint
static char     sccsid[] = "$Id: envread.c,v 1.1 2014-08-18 10:19:13+05:30 Cprogrammer Exp mbhangui $";
#endif

char       *
env_get(s)
	char           *s;
{
	int             i;
	unsigned int    slen;
	char           *envi;

	slen = str_len(s);
	for (i = 0; (envi = environ[i]); ++i)
	{
		if ((!str_diffn(s, envi, slen)) && (envi[slen] == '='))
			return envi + slen + 1;
	}
	return 0;
}

char    *
env_pick()
{
	return environ[0];
}

char    *
env_findeq(s)
	char           *s;
{
	for (; *s; ++s)
		if (*s == '=')
			return s;
	return 0;
}
