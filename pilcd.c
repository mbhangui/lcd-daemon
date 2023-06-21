/*
 * $Log: pilcd.c,v $
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
#include "subprintf.h"
#include "scan.h"
#include "str.h"

static char     ssoutbuf[512], sserrbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
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
	if (subprintf(&sserr, "USAGE: pilcd [-s] [-C clear_type] -b bits -c cols -r rows -R rownum message\n") == -1)
		_exit (111);
	else
	if (subprintf(&sserr, "                       or\n") == -1)
		_exit (111);
	else
	if (subprintf(&sserr, "USAGE: pilcd -C clear_type -b bits\n") == -1)
		_exit (111);
	flush();
	return;
}

int
get_options(int argc, char **argv, int *clear, int *scroll, int *bits, int *cols, int *rows, int *rpos, char ***msgptr)
{
	int             c;

	*clear = *scroll = 0;
	*bits = *rows = *cols = *rpos = -1;
	*msgptr = (char **) 0;
	while ((c = getopt(argc, argv, "C:sb:r:c:R:")) != -1) {
		switch (c)
		{
		case 'C':
			scan_int(optarg, clear);
			break;
		case 'c':
			scan_int(optarg, cols);
			break;
		case 'r':
			scan_int(optarg, rows);
			break;
		case 'R':
			scan_int(optarg, rpos);
			break;
		case 'b':
			scan_int(optarg, bits);
			break;
		case 's':
			*scroll = 1;
			break;
		default:
			usage();
			return (1);
		}
	}
	if (optind < argc)
		*msgptr = argv + optind++;
	if (*clear == 4 || *clear == 5 || *clear == 6) {
		if (*bits == -1) {
			if (subprintf(&sserr, "LCD operation mode not specified (4bit or 8bit)\n") == -1)
				_exit (111);
			usage();
			return (1);
		} else
		if (!(*bits == 4 || *bits == 8)) {
			if (subprintf(&sserr, "LCD bits must be 4 or 8\n") == -1)
				_exit (111);
			usage();
			return (1);
		}
		return (lcdPrint(&sserr, 0, 0, 0, 0, 0, *clear, 0)); 
	}
	if (*cols == -1) {
		if (subprintf(&sserr, "LCD width not specified\n") == -1)
			_exit (111);
		usage(&sserr);
		return (1);
	} else
	if (!(*cols == 20 || *cols == 16)) {
		if (subprintf(&sserr, "LCD cols must be 16 or 20\n") == -1)
			_exit (111);
		usage();
		return (1);
	} else
	if (*rows == -1) {
		if (subprintf(&sserr, "LCD rows not specified\n") == -1)
			_exit (111);
		usage();
		return (1);
	} else
	if (!(*rows == 1 || *rows == 2 || *rows == 4)) {
		if (subprintf(&sserr, "LCD rows must be 1, 2 or 4\n") == -1)
			_exit (111);
		usage();
		return (1);
	} else
	if (*bits == -1) {
		if (subprintf(&sserr, "LCD operation mode not specified (4bit or 8bit)\n") == -1)
			_exit (111);
		usage();
		return (1);
	} else
	if (!(*bits == 4 || *bits == 8)) {
		if (subprintf(&sserr, "LCD bits must be 4 or 8\n") == -1)
			_exit (111);
		usage();
		return (1);
	} else
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
	int             scroll = 0, cols, rows, bits, rpos, clear;
	char          **ptr, **msg;

	if (get_options(argc, argv, &clear, &scroll, &bits, &cols, &rows, &rpos, &ptr))
		return (1);
	for (msg = ptr;msg && *msg; msg++) {
		stralloc_catb(&message, *msg, str_len(*msg));
		stralloc_append(&message, " ");
	}
	message.len--;
	stralloc_0(&message);
	return (lcdPrint(&sserr, scroll, cols, rows, bits, rpos, clear, message.len ? message.s : 0)); 
}
