/*
 * $Log: getln.h,v $
 * Revision 1.1  2014-09-01 20:08:05+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifndef GETLN_H
#define GETLN_H
#include "substdio.h"
#include "stralloc.h"

int             getln(substdio *, stralloc *, int *, int);
int             getln2(substdio *, stralloc *, char **, unsigned int *, int);

#endif
