/*
 * $Log: stralloc_cats.c,v $
 * Revision 1.1  2024-04-22 07:57:15+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"
#include "str.h"
#include "stralloc.h"

int
stralloc_cats(stralloc *sa, const char *s)
{
	return stralloc_catb(sa, s, str_len(s));
}

void
getversion_stralloc_cats_c()
{
	static char    *x = "$Id: stralloc_cats.c,v 1.1 2024-04-22 07:57:15+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
