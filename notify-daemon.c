/*
 * $Id: notify-daemon.c,v 1.2 2025-02-18 11:14:22+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_FLOCK
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT EINVAL
#endif

#include "substdio.h"
#include "stralloc.h"
#include "scan.h"
#include "getEnvConfig.h"
#include "env.h"
#include "subprintf.h"
#include "getln.h"
#include "error.h"
#include "str.h"
#include "fmt.h"
#include "timeoutread.h"
#include "ndelay.h"

#define PORT 1807
typedef enum {
	fifo,
	sock,
} req_type;

#define NOTIFYFIFO "/tmp/notify-desktop/notify-fifo"

static char     ssoutbuf[512], sserrbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
static int      verbose, read_timeout = -1, port = -1, display_timeout = -1;
static stralloc _dirbuf = { 0 };
static stralloc summary = { 0}, body = { 0 };
static char     *startup_msg, *shutdown_msg;
static char      strnum[FMT_ULONG];
req_type        fdtype;


void
print_message()
{
	pid_t           pid;
	int             status;
	char           *(args[6]);

	args[0] = "notify-send";
	if (display_timeout != -1) {
		fmt_ulong(strnum, display_timeout);
		args[1] = "-t";
		args[2] = strnum;
		if (summary.len)
			args[3] = summary.s;
		else
			args[3] = "notify-desktop";
		if (!body.len)
			return;
		args[4] = body.s;
		args[5] = NULL;
	} else {
		if (summary.len)
			args[1] = summary.s;
		else
			args[1] = "notify-desktop";
		if (!body.len)
			return;
		args[2] = body.s;
		args[3] = NULL;
		args[4] = NULL;
		args[5] = NULL;
	}
	switch (pid = fork())
	{
	case 0:
		execv("/usr/bin/notify-send", args);
		if (subprintf(&sserr, "execv: notify-send: %s\n", error_str(errno)) == -1)
			_exit (111);
		_exit(111);
	default:
		wait(&status);
		break;
	}
	return;
}

static void
flush(int type)
{
	if ((!type || type == 2) && substdio_flush(&sserr))
		_exit (111);
	if ((!type || type == 1) && substdio_flush(&ssout))
		_exit (111);
}

void
die_nomem()
{
	if (subprintf(&sserr, "out of memory\n") == -1)
		_exit(111);
	flush(2);
	_exit(111);
}

static void
SigChild(void)
{
    int             status;
    pid_t           pid;

    for (;(pid = waitpid(-1, &status, WNOHANG | WUNTRACED));) {
#ifdef ERESTART
        if (pid == -1 && (errno == EINTR || errno == ERESTART))
#else   
        if (pid == -1 && errno == EINTR)
#endif
            continue;
        break;
    } /*- for (; pid = waitpid(-1, &status, WNOHANG | WUNTRACED);) -*/
    (void) signal(SIGCHLD, (void (*)()) SigChild);
    return;
}

static void
SigTerm(void)
{
	if (shutdown_msg) {
		if (!stralloc_copyb(&summary, "notify-desktop", 14) ||
				!stralloc_0(&summary) ||
				!stralloc_copys(&body, shutdown_msg) ||
				!stralloc_0(&body))
			die_nomem();
		print_message();
	}
	subprintf(&sserr, "%d: ARGH!!! commiting suicide on SIGTERM\n", getpid());
	flush(0);
	_exit (1);
}

char *
get_octal(mode_t mode)
{
	int             a[4];
	int             i, d, count;
	static char     octal[5];

	d = mode;
	for (count = i = 0;d != 0 && i < 4;++i) {
		a[i] = (d % 8) + '0';
		d /= 8;
		count++;
	}
	d = 0;
	octal[d++] = '0';
	for (i = count - 1;i >= 0;--i)
		octal[d++] = a[i];
	octal[d] = 0;
	return octal;
}

int
r_mkdir(char *dir, mode_t mode)
{
	char           *ptr;
	int             i;

	for (ptr = _dirbuf.s + 1; *ptr; ptr++) {
		if (*ptr == '/') {
			*ptr = 0;
			if (access(_dirbuf.s, F_OK)) {
				if ((i = mkdir(_dirbuf.s, mode)) == -1) {
					if (subprintf(&sserr, "mkdir: %s: %s\n", _dirbuf.s, error_str(errno)) == -1)
						_exit (111);
					flush(2);
					_exit (111);
				}
			}
			*ptr = '/';
		}
	}
	if (access(_dirbuf.s, F_OK)) {
		if ((i = mkdir(_dirbuf.s, mode)) == -1) {
			if (subprintf(&sserr, "mkdir: %s: %s\n", _dirbuf.s, error_str(errno)) == -1)
				_exit (111);
			flush(2);
			_exit (111);
		}
	}
	return (0);
}

void
usage()
{
	if (subprintf(&sserr, "USAGE: notify-daemon [-v -f fifo_path -m fifo_mode -t readtimeout -p port]\n") == -1)
		_exit (111);
	flush(2);
	_exit(100);
}

int
get_options(int argc, char **argv, char **fifo_path, unsigned long *fifo_mode)
{
	int             c;
	char            *ptr;

	verbose = 0;
	*fifo_path = (char *) 0;
	*fifo_mode = -1;
	read_timeout = display_timeout = port = -1;
	while ((c = getopt(argc, argv, "vf:m:i:s:t:T:p:")) != -1) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'i':
			startup_msg = optarg;
			break;
		case 's':
			shutdown_msg = optarg;
			break;
		case 'f':
			*fifo_path = optarg;
			break;
		case 'm':
			scan_8long(optarg, fifo_mode);
			break;
		case 't':
			scan_int(optarg, &read_timeout);
			break;
		case 'T':
			scan_int(optarg, &display_timeout);
			break;
		case 'p':
			scan_int(optarg, &port);
			break;
		default:
			usage();
		}
	}
	if (!*fifo_path && !(*fifo_path = env_get("NOTIFYFIFO"))) {
		if (!access("/run", F_OK))
			*fifo_path = "/run/notify-desktop/notify-fifo";
		else
		if (!access("/var/run", F_OK))
			*fifo_path = "/var/run/notify-desktop/notify-fifo";
		else
			*fifo_path = NOTIFYFIFO;
	}
	if (*fifo_mode == -1) {
		if ((ptr = env_get("FIFO_MODE")))
			scan_8long(ptr, fifo_mode);
		else
			*fifo_mode = 0666;
	}
	if (read_timeout == -1) {
		if ((ptr = env_get("DATA_TIMEOUT")))
			scan_int(ptr, &read_timeout);
		else
			read_timeout = 300;
	}
	if (display_timeout == -1) {
		if ((ptr = env_get("DISPLAY_TIMEOUT")))
			scan_int(ptr, &display_timeout);
	}
	if (port == -1) {
		if ((ptr = env_get("PORT")))
			scan_int(ptr, &port);
		else
			port = PORT;
	}
	return (0);
}

ssize_t
saferead(int fd, char *buf, int len)
{
	int             r;

	if ((r = (fdtype == sock ? timeoutread_sock : timeoutread_fifo)(read_timeout, fd, buf, len)) == -1) {
		if (errno == error_timeout)
			return 0;
	} else
	if (r <= 0) {
		if (r) {
			if (subprintf(&sserr, "read: %s\n", error_str(errno)) == -1)
				_exit(111);
			flush(2);
			_exit(111);
		}
	}
	return r;
}

int
read_data(int fd)
{
	static char     ssinbuf[512];
	substdio        ssin;
	stralloc        line = {0};
	int             i, match;

	substdio_fdbuf(&ssin, saferead, fd, ssinbuf, sizeof ssinbuf);
	body.len = 0;
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			if (subprintf(&sserr, "getln: %s\n", error_str(errno)) == -1)
				_exit (111);
			flush(2);
			_exit (111);
		}
		if (!match && line.len == 0)
			break;
		line.len--;
		if (!stralloc_0(&line))
			die_nomem();
		line.len--;
		/* process the line */
		i = str_chr(line.s, ':');
		if (!line.s[i])
			continue;
		if (!str_diffn(line.s, "summary:", 8)) {
			if (!stralloc_copyb(&summary, line.s + i + 1, line.len - 8) ||
					!stralloc_0(&summary))
				die_nomem();
			summary.len--;
			if (verbose && subprintf(&ssout, "summary [%s][%d]\n", summary.s, line.len - 8) == -1)
				_exit (111);
		} else
		if (!str_diffn(line.s, "body:", 5)) {
			if (!stralloc_catb(&body, line.s + i + 1, line.len - 5) ||
					!stralloc_append(&body, "\n"))
				die_nomem();
			if (verbose && subprintf(&ssout, "body [%s][%d]\n", line.s + i + 1, line.len - 5) == -1)
				_exit (111);
		} else
		if (!str_diffn(line.s, "end:", 4)) {
			if (verbose && subprintf(&ssout, "end\n") == -1)
				_exit (111);
			break;
		}
		flush(1);
	}
	if (!stralloc_0(&body))
		die_nomem();
	body.len--;
	print_message();
	return (0);
}

int
initialize_socket()
{
	int             s, i, ipv6 = 1;
	struct sockaddr_in6 in6 = {0};
	struct sockaddr_in  in4 = {0};

	if ((s = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		if (errno == EINVAL || errno == EAFNOSUPPORT) {
			ipv6 = 0;
			if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
				if (subprintf(&sserr, "socket: AF_INET: SOCK_DGRAM: %s\n", error_str(errno)) == -1)
					_exit(111);
				flush(2);
				_exit(111);
			}
		} else {
			if (subprintf(&sserr, "socket: PF_INET6: SOCK_DGRAM: %s\n", error_str(errno)) == -1)
				_exit(111);
			flush(2);
			_exit(111);
		}
	}
	if (ndelay_on(s) == -1) {
		if (subprintf(&sserr, "ndelay: %s\n", error_str(errno)) == -1)
			_exit(111);
		_exit(111);
	}
	if (ipv6) {
		in6.sin6_family = AF_INET6;
		in6.sin6_addr = in6addr_any;
		in6.sin6_port = htons(port);
	} else {
		in4.sin_family = AF_INET;
		in4.sin_addr.s_addr = INADDR_ANY;
		in4.sin_port = htons(port);
	}
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
	if (bind(s, ipv6 ? (struct sockaddr *) &in6 : (struct sockaddr *) &in4,
				ipv6 ? sizeof(in6) : sizeof(in4)) < 0 ) {
		if (subprintf(&sserr, "bind: %s\n", error_str(errno)) == -1)
			_exit(111);
		flush(2);
		_exit(111);
	}
	return s;
}

int
main(int argc, char **argv)
{
	int             i, fifofd, sockfd;
	unsigned long   fifo_mode;
	char           *fifo_path;
	fd_set          rfds;	/*- File descriptor mask for select -*/

	if (get_options(argc, argv, &fifo_path, &fifo_mode))
		return 1;
	i = str_rchr(fifo_path, '/');
	if (fifo_path[i]) {
		if (!stralloc_copyb(&_dirbuf, fifo_path, i) ||
				!stralloc_0(&_dirbuf))
			die_nomem();
		if (access(_dirbuf.s, F_OK) && r_mkdir(_dirbuf.s, 0755))
			_exit(111);
		/*- restore _dirbuf which was modified by r_mkdir() */
		if (!stralloc_copyb(&_dirbuf, fifo_path, i) ||
				!stralloc_0(&_dirbuf))
			die_nomem();
		if (chdir(_dirbuf.s) == -1) {
			if (subprintf(&sserr, "chdir: %s: %s\n", _dirbuf.s, error_str(errno)) == -1)
				_exit (111);
			_exit(111);
		}
	}
	if (access(fifo_path, F_OK) && mkfifo(fifo_path, 0666)) {
		if (subprintf(&sserr, "mkfifo: %s: %s\n", fifo_path, error_str(errno)) == -1)
			_exit (111);
		flush(2);
		return (1);
	}
	if (chmod(fifo_path, (mode_t) fifo_mode)) {
		if (subprintf(&sserr, "chmod: %o %s: %s\n", get_octal((mode_t) fifo_mode), fifo_path, error_str(errno)) == -1)
			_exit (111);
		flush(2);
		_exit (111);
	}
	if ((fifofd = open(fifo_path, O_RDWR)) == -1) {
		if (subprintf(&sserr, "open: %s: %s\n", fifo_path, error_str(errno)) == -1)
			_exit (111);
		flush(2);
		_exit (111);
	}
	if (ndelay_on(fifofd) == -1) {
		if (subprintf(&sserr, "ndelay-fifo: %s\n", error_str(errno)) == -1)
			_exit(111);
		_exit (111);
	}
	(void) signal(SIGTERM, (void (*)()) SigTerm);
	(void) signal(SIGCHLD, (void (*)()) SigChild);
	if ((sockfd = initialize_socket()) == -1)
		_exit(111);
	if (startup_msg) {
		if (!stralloc_copyb(&summary, "notify-desktop", 14) ||
				!stralloc_0(&summary) ||
				!stralloc_copys(&body, startup_msg) ||
				!stralloc_0(&body))
			die_nomem();
		print_message();
	}
	for (;;) {
		FD_ZERO(&rfds);
		FD_SET(fifofd, &rfds);
		FD_SET(sockfd, &rfds);
		if ((i = select(sockfd + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, NULL)) < 0) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			if (subprintf(&sserr, "select: %s\n", error_str(errno)) == -1)
				_exit(111);
			_exit(111);
		}
		if (FD_ISSET(fifofd, &rfds)) {
			fdtype = fifo;
			read_data(fifofd);
		}
		if (FD_ISSET(sockfd, &rfds)) {
			fdtype = sock;
			read_data(sockfd);
		}
	} /*- for(;;) */
	return 0;
}

/*
 * $Log: notify-daemon.c,v $
 * Revision 1.2  2025-02-18 11:14:22+05:30  Cprogrammer
 * added feature to set display time for notify-send
 *
 * Revision 1.1  2024-04-22 07:52:39+05:30  Cprogrammer
 * Initial revision
 *
 */
