/*
 * $Log: alloc.h,v $
 * Revision 1.1  2014-09-01 20:05:11+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifndef ALLOC_H
#define ALLOC_H

/* @null@ *//* @out@ */
char           *alloc(unsigned int);
void            alloc_free(char *);
#ifdef _ALLOC_
int             alloc_re();
#else
int             alloc_re(char *, unsigned int, unsigned int);
#endif

#endif
