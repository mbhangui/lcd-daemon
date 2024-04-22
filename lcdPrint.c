/*
 * $Log: lcdPrint.c,v $
 * Revision 1.7  2024-04-22 07:59:25+05:30  Cprogrammer
 * added sendMsg function for pinotify
 *
 * Revision 1.6  2023-06-23 17:47:20+05:30  Cprogrammer
 * interchanged rownum, scroll arguments
 *
 * Revision 1.5  2023-06-22 23:42:54+05:30  Cprogrammer
 * refactored code
 *
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
#include "env.h"
#include "subprintf.h"

#ifndef lint
static char     sccsid[] = "$Id: lcdPrint.c,v 1.7 2024-04-22 07:59:25+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * rownum - 0 to 3, the row on which to display the text
 * scroll - scroll the text
 * clear  - 1 - clear lcd screen
 * clear  - 2 - clear and initialize lcd screen
 * clear  - 3 - initialize lcd screen
 * message
 */
#ifdef HAVE_STDARG_H
#include <stdio.h>
int
lcdPrint(substdio *sserr, int rownum, int scroll, int clear, char *fmt, ...)
#else
int
lcdPrint(va_alist)
	va_dcl
#endif
{
	va_list         ap;
	char           *ptr, *fifo_path;
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

#ifndef HAVE_STDARG_H
	va_start(ap);
	sserr = va_arg(ap, substdio *);	/* substdio descriptor */
	rownum = va_arg(ap, int);
	scroll = va_arg(ap, int);
	clear = va_arg(ap, int);
	fmt = va_arg(ap, char *);
#else
	va_start(ap, fmt);
#endif
	if (fmt)
		(void) vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);
	if (!(fifo_path = env_get("LCDFIFO"))) {
		if (!access("/run", F_OK))
			fifo_path = "/run/lcd-daemon/lcdfifo";
		else
		if (!access("/var/run", F_OK))
			fifo_path = "/var/run/lcd-daemon/lcdfifo";
		else
			fifo_path = LCDFIFO;
	}
	if (access(fifo_path, F_OK)) {
		if (subprintf(sserr, "access: %s: %s\n", fifo_path, error_str(errno)) == -1)
			_exit (111);
		else
		if (substdio_flush(sserr))
			_exit (111);
		return (1);
	}
	if ((fd = open(fifo_path, O_RDWR|O_NDELAY)) == -1) {
		if (errno == error_wouldblock)
			return (1);
		if (subprintf(sserr, "open: %s: %s\n", fifo_path, error_str(errno)) == -1)
			_exit (111);
		else
		if (substdio_flush(sserr))
			_exit (111);
		return (1);
	}
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	for (len = 0, ptr = buf; *ptr++; len++);
	/*-
	 * scroll rownum clear string
	 */
	if (subprintf(&ssout, "%d %d %d:%s\n",
		rownum, scroll || (len > 20 ? 1: 0), clear, fmt ? buf : "clear screen") == -1) {
		close(fd);
		_exit (111);
	} else
	if (substdio_flush(&ssout) == -1) {
		close(fd);
		_exit (111);
	}
	return (close(fd));
}

#ifdef HAVE_STDARG_H
#include <stdio.h>
int
sendMsg(substdio *sserr, char *summary, char *fmt, ...)
#else
int
sendMsg(va_alist)
	va_dcl
#endif
{
	va_list         ap;
	char           *ptr, *fifo_path;
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
	char           *summary;
	char           *fmt;
#endif

#ifndef HAVE_STDARG_H
	va_start(ap);
	sserr = va_arg(ap, substdio *);	/* substdio descriptor */
	summary = va_arg(ap, char *);
	fmt = va_arg(ap, char *);
#else
	va_start(ap, fmt);
#endif
	if (fmt)
		(void) vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);
	if (!(fifo_path = env_get("NOTIFYFIFO"))) {
		if (!access("/run", F_OK))
			fifo_path = "/run/notify-desktop/notify-fifo";
		else
		if (!access("/var/run", F_OK))
			fifo_path = "/var/run/notify-desktop/notify-fifo";
		else
			fifo_path = LCDFIFO;
	}
	if (access(fifo_path, F_OK)) {
		if (subprintf(sserr, "access: %s: %s\n", fifo_path, error_str(errno)) == -1)
			_exit (111);
		else
		if (substdio_flush(sserr))
			_exit (111);
		return (1);
	}
	if ((fd = open(fifo_path, O_RDWR|O_NDELAY)) == -1) {
		if (errno == error_wouldblock)
			return (1);
		if (subprintf(sserr, "open: %s: %s\n", fifo_path, error_str(errno)) == -1)
			_exit (111);
		else
		if (substdio_flush(sserr))
			_exit (111);
		return (1);
	}
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	for (len = 0, ptr = buf; *ptr++; len++);
	/*-
	 * scroll rownum clear string
	 */
	if (subprintf(&ssout, "%s\n%s", summary, buf) == -1) {
		close(fd);
		_exit (111);
	} else
	if (substdio_flush(&ssout) == -1) {
		close(fd);
		_exit (111);
	}
	return (close(fd));
}
