/*
 * $Log: alloc_re.c,v $
 * Revision 1.1  2014-08-18 10:18:00+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#define _ALLOC_
#include "alloc.h"
#undef _ALLOC_
#include "byte.h"

#ifndef lint
static char     sccsid[] = "$Id: alloc_re.c,v 1.1 2014-08-18 10:18:00+05:30 Cprogrammer Exp mbhangui $";
#endif

int
alloc_re(x, old_size, new_size)
	char          **x;
	unsigned int    old_size;
	unsigned int    new_size;
{
	char           *y;

	y = alloc(new_size);
	if (!y)
		return 0;
	byte_copy(y, old_size, *x);
	alloc_free(*x);
	*x = y;
	return 1;
}
