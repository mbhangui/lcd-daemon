/*
 * $Log: lcdPrint.h,v $
 * Revision 1.3  2023-06-22 23:43:00+05:30  Cprogrammer
 * refactored lcdPrint()
 *
 * Revision 1.2  2014-09-02 22:09:19+05:30  Cprogrammer
 * added scroll argument
 *
 * Revision 1.1  2014-09-01 20:08:09+05:30  Cprogrammer
 * Initial revision
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STDARG_H
int             lcdPrint    __P((substdio *, int, int, int, char *, ...));
#else
int             lcdPrint    ();
#endif

