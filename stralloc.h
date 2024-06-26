/*
 * $Log: stralloc.h,v $
 * Revision 1.2  2024-04-22 08:00:55+05:30  Cprogrammer
 * updated prototypes for stralloc_cats, stralloc_catb
 *
 * Revision 1.1  2014-09-01 20:05:55+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifndef STRALLOC_H
#define STRALLOC_H

#include "gen_alloc.h"

GEN_ALLOC_typedef(stralloc, char, s, len, a)
#ifndef _STRALLOC_EADY
int             stralloc_ready(stralloc *, int);
int             stralloc_readyplus(stralloc *, int);
int             stralloc_copy(stralloc *, stralloc *);
int             stralloc_copys(stralloc *, char *);
int             stralloc_copyb(stralloc *, char *, int);
int             stralloc_cat(stralloc *, stralloc *);
int             stralloc_cats(stralloc *, const char *);
int             stralloc_catb(stralloc *, const char *, int);
int             stralloc_append(stralloc *, char *); /*- beware: this takes a pointer to 1 char */
int             stralloc_starts(stralloc *, char *);
int             stralloc_catulong0(stralloc *, unsigned long, unsigned int);
int             stralloc_catlong0(stralloc *, long, unsigned int);
#else
int             stralloc_ready();
int             stralloc_readyplus();
int             stralloc_copy();
int             stralloc_copys();
int             stralloc_copyb();
int             stralloc_cat();
int             stralloc_cats();
int             stralloc_catb();
int             stralloc_append();
int             stralloc_starts();
int             stralloc_catulong0();
int             stralloc_catlong0();
#endif

#define stralloc_0(sa) stralloc_append(sa,"")
#define stralloc_catlong(sa,l) (stralloc_catlong0((sa),(l),0))
#define stralloc_catuint0(sa,i,n) (stralloc_catulong0((sa),(unsigned long) (i),(n)))
#define stralloc_catint0(sa,i,n) (stralloc_catlong0((sa),(long) (i),(n)))
#define stralloc_catint(sa,i) (stralloc_catlong0((sa),(long) (i),0))

#endif
