/*
 * $Log: stralloc_eady.c,v $
 * Revision 1.1  2014-08-18 10:21:44+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#define _ALLOC_
#include "alloc.h"
#undef _ALLOC_
#define _STRALLOC_EADY
#include "stralloc.h"
#undef _STRALLOC_EADY
#include "gen_allocdefs.h"

#ifndef	lint
static char     sccsid[] = "$Id: stralloc_eady.c,v 1.1 2014-08-18 10:21:44+05:30 Cprogrammer Exp mbhangui $";
#endif

GEN_ALLOC_ready(stralloc, char, s, len, a, i, n, x, 30, stralloc_ready)
GEN_ALLOC_readyplus(stralloc, char, s, len, a, i, n, x, 30, stralloc_readyplus)
