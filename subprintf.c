/*
 * $Log: subprintf.c,v $
 * Revision 1.1  2014-09-01 20:03:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "substdio.h"
#include <stdio.h>

#ifdef HAVE_STDARG_H
int
subprintf(substdio *ssout, char *fmt, ...)
#else
int
subprintf(va_alist)
	va_dcl
#endif
{
	va_list         ap;
	char           *ptr;
	char            buf[2048];
	int             i;
#ifdef SUN41
	int             len;
#else
	unsigned        len;
#endif
#ifndef HAVE_STDARG_H
	substdio       *ssout;
	char           *fmt;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, fmt);
#else
	va_start(ap);
	ssout = va_arg(ap, substdio *);	/* substdio descriptor */
	fmt = va_arg(ap, char *);
#endif
	(void) vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);
	for (len = 0, ptr = buf; *ptr++; len++);
	return ((i = substdio_put(ssout, buf, len)));
}

