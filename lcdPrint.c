/*
 * $Log: lcdPrint.c,v $
 * Revision 1.4  2014-09-23 14:10:28+05:30  Cprogrammer
 * added documentation for lcdPrint()
 *
 * Revision 1.3  2014-09-02 22:09:08+05:30  Cprogrammer
 * added scroll argument
 *
 * Revision 1.2  2014-09-02 00:18:35+05:30  Cprogrammer
 * lcd daemon version
 *
 * Revision 1.1  2014-09-01 19:47:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "substdio.h"
#include "error.h"
#include "str.h"
#include "pilcd.h"
#include "getEnvConfig.h"
#include "subprintf.h"

#ifndef lint
static char     sccsid[] = "$Id: lcdPrint.c,v 1.4 2014-09-23 14:10:28+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * scroll - scroll the text
 * cols   - no of columns (16 or 20)
 * rows   - no of rows (2 or 4)
 * bits   - 4 bit or 8 bit data mode
 * rownum - 0 to 3, the row on which to display the text
 * clear  - 1 - clear lcd screen
 * clear  - 2 - clear and initialize lcd screen
 * clear  - 3 - initialize lcd screen
 */
#ifdef HAVE_STDARG_H
#include <stdio.h>
int
lcdPrint(substdio *sserr, int scroll, int cols, int rows, int bits, int rownum, int clear, char *fmt, ...)
#else
int
lcdPrint(va_alist)
	va_dcl
#endif
{
	va_list         ap;
	char           *ptr;
	char            buf[2048];
	static char     outbuf[512];
	int             fd;
	static substdio ssout;
#ifdef SUN41
	int             len;
#else
	unsigned        len;
#endif
#ifndef HAVE_STDARG_H
	substdio       *sserr;
	int             cols, rows, bits, rownum, clear;
	char           *fmt;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, fmt);
#else
	va_start(ap);
	sserr = va_arg(ap, substdio *);	/* substdio descriptor */
	scroll = va_arg(ap, int);
	cols = va_arg(ap, int);
	rows = va_arg(ap, int);
	bits = va_arg(ap, int);
	rownum = va_arg(ap, int);
	clear = va_arg(ap, int);
	fmt = va_arg(ap, char *);
#endif
	if (fmt)
		(void) vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);
	getEnvConfigStr(&ptr, "LCDFIFO", LCDFIFO);
	if (access(ptr, F_OK)) {
		if (subprintf(sserr, "access: %s: %s\n", ptr, error_str(errno)) == -1)
			_exit (111);
		else
		if (substdio_flush(sserr))
			_exit (111);
		return (1);
	}
	if ((fd = open(ptr, O_RDWR|O_NDELAY)) == -1) {
		if (errno == error_wouldblock)
			return (1);
		if (subprintf(sserr, "open: %s: %s\n", ptr, error_str(errno)) == -1)
			_exit (111);
		else
		if (substdio_flush(sserr))
			_exit (111);
		return (1);
	}
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	for (len = 0, ptr = buf; *ptr++; len++);
	if (subprintf(&ssout, "%d %d %d %d %d %d :%s\n",
		scroll || (len > 20 ? 1: 0), cols, rows, bits, rownum, clear, fmt ? buf : "clear screen") == -1) {
		close(fd);
		_exit (111);
	} else
	if (substdio_flush(&ssout) == -1) {
		close(fd);
		_exit (111);
	}
	return (close(fd));
}
