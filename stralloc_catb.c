/*
 * $Log: stralloc_catb.c,v $
 * Revision 1.1  2024-04-22 08:00:40+05:30  Cprogrammer
 * Initial revision
 *
 */
#define _STRALLOC_EADY
#include "stralloc.h"
#undef _STRALLOC_EADY
#include "byte.h"

#ifndef	lint
static char     sccsid[] = "$Id: stralloc_catb.c,v 1.1 2024-04-22 08:00:40+05:30 Cprogrammer Exp mbhangui $";
#endif

int
stralloc_catb(stralloc *sa, const char *s, unsigned int n)
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
