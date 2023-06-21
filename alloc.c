/*
 * $Log: alloc.c,v $
 * Revision 1.1  2014-08-18 10:17:41+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include "alloc.h"
#include "error.h"
#include <stdlib.h>

#define ALIGNMENT 16			/*- XXX: assuming that this alignment is enough */
#define SPACE 4096				/*- must be multiple of ALIGNMENT */

#ifndef lint
static char     sccsid[] = "$Id: alloc.c,v 1.1 2014-08-18 10:17:41+05:30 Cprogrammer Exp mbhangui $";
#endif

typedef union
{
	char            irrelevant[ALIGNMENT];
	double          d;
} aligned;

static aligned  realspace[SPACE / ALIGNMENT];
#define space ((char *) realspace)
static unsigned int avail = SPACE;	/*- multiple of ALIGNMENT; 0<=avail<=SPACE */

/*@null@*//*@out@*/char *alloc(n)
	unsigned int    n;
{
	char           *x;
	unsigned int    m = n;

	if ((n = ALIGNMENT + n - (n & (ALIGNMENT - 1))) < m)	/*- handle overflow */
	{
		errno = error_nomem;
		return 0;
	}
	if (n <= avail)
	{
		avail -= n;
		return space + avail;
	}
	if (!(x = malloc(n)))
		errno = error_nomem;
	return x;
}

void
alloc_free(x)
	char           *x;
{
	if (x >= space && (x < space + SPACE))
		return;				/*- XXX: assuming that pointers are flat */
	free(x);
}
