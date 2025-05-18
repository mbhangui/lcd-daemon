/*
 * $Log: pilcd.c,v $
 * Revision 1.8  2025-05-18 22:41:01+05:30  Cprogrammer
 * use libqmail qprintf.h for subprintf function
 *
 * Revision 1.7  2025-05-18 21:59:52+05:30  Cprogrammer
 * fix gcc14 errors
 *
 * Revision 1.6  2023-06-23 17:47:36+05:30  Cprogrammer
 * interchanged rownum, scroll arguments
 *
 * Revision 1.5  2023-06-22 23:40:20+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.4  2014-09-02 22:12:32+05:30  Cprogrammer
 * added scroll argument
 *
 * Revision 1.2  2014-09-02 00:18:48+05:30  Cprogrammer
 * fixed minor issues with usage
 *
 * Revision 1.1  2014-09-01 20:03:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#include "substdio.h"
#include "stralloc.h"
#include "lcdPrint.h"
#include "qprintf.h"
#include "scan.h"
#include "str.h"

static char     ssoutbuf[512], sserrbuf[512];
static substdio ssout = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 1, ssoutbuf, sizeof ssoutbuf);
static substdio sserr = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 2, sserrbuf, sizeof sserrbuf);
stralloc message = {0};

static void
flush()
{
	if (substdio_flush(&sserr))
		_exit (111);
	else
	if (substdio_flush(&ssout))
		_exit (111);
}

void
usage()
{
	if (subprintf(&sserr, "USAGE: pilcd [-s] [-c clear_type] -r rownum arg1 [arg2 ...]\n") == -1)
		_exit (111);
	else
	if (subprintf(&sserr, "                       or\n") == -1)
		_exit (111);
	else
	if (subprintf(&sserr, "USAGE: pilcd -c clear_type\n") == -1)
		_exit (111);
	flush();
	return;
}

int
get_options(int argc, char **argv, int *clear, int *scroll, int *rpos, char ***msgptr)
{
	int             c;

	*clear = *scroll = 0;
	*rpos = -1;
	*msgptr = (char **) 0;
	while ((c = getopt(argc, argv, "c:sr:")) != -1) {
		switch (c)
		{
		case 'c':
			scan_int(optarg, clear);
			break;
		case 's':
			*scroll = 1;
			break;
		case 'r':
			scan_int(optarg, rpos);
			break;
		default:
			usage();
			return (1);
		}
	}
	if (optind < argc)
		*msgptr = argv + optind++;
	if (*clear == 4 || *clear == 5 || *clear == 6) {
		return (lcdPrint(&sserr, 0, 0, *clear, 0)); 
	}
	if (*rpos == -1) {
		if (subprintf(&sserr, "row on which message to be put not specified\n") == -1)
			_exit (111);
		usage();
		return (1);
	} else
	if (!(*rpos == 0 || *rpos == 1 || *rpos == 2 || *rpos == 3)) {
		if (subprintf(&sserr, "LCD row postion must be 0, 1, 2 or 3\n") == -1)
			_exit (111);
		usage();
		return (1);
	} else
	if (!*msgptr) {
		if (subprintf(&sserr, "message not specified\n") == -1)
			_exit (111);
		usage();
		return (1);
	}
	return (0);
}

int
main(int argc, char **argv)
{
	int             scroll = 0, rpos, clear;
	char          **ptr, **msg;

	if (get_options(argc, argv, &clear, &scroll, &rpos, &ptr))
		return (1);
	for (msg = ptr;msg && *msg; msg++) {
		stralloc_catb(&message, *msg, str_len(*msg));
		stralloc_append(&message, " ");
	}
	message.len--;
	stralloc_0(&message);
	return (lcdPrint(&sserr, rpos, scroll, clear, message.len ? message.s : 0)); 
}
