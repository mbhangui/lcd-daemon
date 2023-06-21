/*
 * $Log: stralloc_catb.c,v $
 * Revision 1.1  2014-08-18 10:21:15+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#define _STRALLOC_EADY
#include "stralloc.h"
#undef _STRALLOC_EADY
#include "byte.h"

#ifndef	lint
static char     sccsid[] = "$Id: stralloc_catb.c,v 1.1 2014-08-18 10:21:15+05:30 Cprogrammer Exp mbhangui $";
#endif

int
stralloc_catb(sa, s, n)
	stralloc       *sa;
	char           *s;
	unsigned int    n;
{
	if (!sa->s)
		return stralloc_copyb(sa, s, n);
	if (!stralloc_readyplus(sa, n + 1))
		return 0;
	byte_copy(sa->s + sa->len, n, s);
	sa->len += n;
	sa->s[sa->len] = 'Z';		/*- ``offensive programming'' */
	return 1;
}
