/*
 * $Log: stralloc_opyb.c,v $
 * Revision 1.1  2014-08-18 10:21:50+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#define _STRALLOC_EADY
#include "stralloc.h"
#undef _STRALLOC_EADY
#include "byte.h"

#ifndef	lint
static char     sccsid[] = "$Id: stralloc_opyb.c,v 1.1 2014-08-18 10:21:50+05:30 Cprogrammer Exp mbhangui $";
#endif

int
stralloc_copyb(sa, s, n)
	stralloc       *sa;
	char           *s;
	unsigned int    n;
{
	if (!stralloc_ready(sa, n + 1))
		return 0;
	byte_copy(sa->s, n, s);
	sa->len = n;
	sa->s[n] = 'Z';				/*- ``offensive programming'' */
	return 1;
}
