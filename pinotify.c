/*
 * $Id: pinotify.c,v 1.2 2025-05-18 21:59:58+05:30 Cprogrammer Exp mbhangui $
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
#include "str.h"

static char     ssoutbuf[512], sserrbuf[512];
static substdio ssout = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 1, ssoutbuf, sizeof ssoutbuf);
static substdio sserr = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 2, sserrbuf, sizeof sserrbuf);
stralloc summary = { 0 }, message = { 0 };

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
die_nomem()
{
	if (subprintf(&sserr, "out of memory\n") == -1)
		_exit(111);
	flush();
	_exit(111);
}

void
usage()
{
	if (subprintf(&sserr, "USAGE: pinotify summary arg1 [arg2 ...]\n") == -1)
		_exit (111);
	flush();
	_exit(100);
}

int
main(int argc, char **argv)
{
	int             i;

	if (argc < 3)
		usage();
	for (i = 2; i < argc; i++) {
		if (!stralloc_catb(&message, "body:", 5) ||
				!stralloc_catb(&message, argv[i], str_len(argv[i])) ||
				!stralloc_append(&message, "\n"))
			die_nomem();
	}
	if (!stralloc_catb(&message, "end:\n", 5) ||
			!stralloc_0(&message))
		die_nomem();
	message.len--;
	if (!stralloc_copyb(&summary, "summary:", 8) ||
			!stralloc_cats(&summary, argv[1]) ||
			!stralloc_append(&summary, "\n") ||
			!stralloc_0(&summary))
		die_nomem();
	summary.len--;
	return (sendMsg(&sserr, summary.s, message.len ? message.s : 0)); 
}

/*
 * $Log: pinotify.c,v $
 * Revision 1.2  2025-05-18 21:59:58+05:30  Cprogrammer
 * fix gcc14 errors
 *
 * Revision 1.1  2024-04-22 08:03:39+05:30  Cprogrammer
 * Initial revision
 *
 */
