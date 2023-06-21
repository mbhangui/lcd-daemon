/*
 * $Log: stralloc_pend.c,v $
 * Revision 1.1  2014-08-18 10:22:03+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include "alloc.h"
#include "stralloc.h"
#include "gen_allocdefs.h"

#ifndef	lint
static char     sccsid[] = "$Id: stralloc_pend.c,v 1.1 2014-08-18 10:22:03+05:30 Cprogrammer Exp mbhangui $";
#endif

GEN_ALLOC_append(stralloc, char, s, len, a, i, n, x, 30, stralloc_readyplus, stralloc_append)
