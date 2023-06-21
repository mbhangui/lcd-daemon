/*
 * $Log: subprintf.h,v $
 * Revision 1.1  2014-09-01 20:08:21+05:30  Cprogrammer
 * Initial revision
 *
 *
 */

#ifndef _SUBPRINTF_H
#define _SUBPRINTF_H

#ifdef HAVE_STDARG_H
int             subprintf   __P((substdio *, char *, ...));
#else
int             subprintf   ();
#endif

#endif
