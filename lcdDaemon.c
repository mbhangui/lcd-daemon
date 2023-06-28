/*
 * $Id: lcdDaemon.c,v 1.12 2023-06-27 22:38:57+05:30 Cprogrammer Exp mbhangui $
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

#if HAVE_WIRINGPI_H
#include <wiringPi.h>
#endif
#if HAVE_LCD_H
#include <lcd.h>
#endif
#include "pilcd.h"
#include "substdio.h"
#include "stralloc.h"
#include "scan.h"
#include "getEnvConfig.h"
#include "env.h"
#include "subprintf.h"
#include "getln.h"
#include "error.h"
#include "str.h"
#include "timeoutread.h"
#include "ndelay.h"

/*-
LCD_PIN WIRINGPI_PIN RPI BOARD_PIN  LCD Pin Number */
                     /*             01 Ground */
                     /*             02 +5v */
                     /*             03 Contrast Adjustment */
#define PIN_RS 6     /*- 22         04 Register Select */
                     /*             05 Read Write Pin (connected to Ground) */
#define PIN_EN 5     /*- 18         06 Enable Pulse */
#define PIN_D0 0     /*-            07 Not connected for 4 bit mode */
#define PIN_D1 0     /*-            08 Not connected for 4 bit mode*/
#define PIN_D2 0     /*-            09 Not connected for 4 bit mode*/
#define PIN_D3 0     /*-            10 Not connected for 4 bit mode*/
#define PIN_D4 4     /*- 16         11 */
#define PIN_D5 0     /*- 11         12 */
#define PIN_D6 2     /*- 13         13 */
#define PIN_D7 3     /*- 15         14 */
                     /*             15 Anode of backlight LED */
                     /*             16 Cathode of backlight LED */
#ifndef HAVE_WIRINGPIDEV
#include <stdio.h>
#include <time.h>
#define lcdPutchar(h, c)         {putchar((c)); fflush(stdout);}
#define lcdPrintf(h, s1, s2, s3) {printf(s1, s2, s3); fflush(stdout);}
#define lcdPosition(h, col, row) {printf("\033[%d;%dH", (row), (col)); fflush(stdout);}
#define lcdClear(h)              {printf("\033[2J\033[H"); fflush(stdout);}
#endif
#define PORT 1806
typedef enum {
	fifo,
	sock,
} req_type;


static char     ssoutbuf[512], sserrbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
static int      childpid[4] = {-1,-1,-1,-1};
static int      lcd = -1, rows, cols, verbose, read_timeout = -1, port = -1;
static stralloc _dirbuf = { 0 };
static char     *startup_msg, *shutdown_msg;
#ifdef HAVE_WIRINGPIDEV
static long     pin_rs, pin_en, pin_d0, pin_d1, pin_d2, pin_d3,
				pin_d4, pin_d5, pin_d6, pin_d7;
#endif
req_type        fdtype;

int             lockfile(int, int);

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
	_exit(111);
}

static void
SigChild(void)
{
    int             status, i;
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
	for (i = 0; i < 4;i++) {
		if (pid == childpid[i])
			childpid[i] = -1;
	}
    (void) signal(SIGCHLD, (void (*)()) SigChild);
    return;
}

static void
SigTerm(void)
{
    int             i;

	if (shutdown_msg && lcd != -1) {
		i = lockfile(1, 0);
		lcdPosition(lcd, 0, 0);
		lcdPrintf(lcd, "%-*s", cols, shutdown_msg);
		lockfile(0, i);
	}
	subprintf(&sserr, "%d: ARGH!!! commiting suicide on SIGTERM\n", getpid());
	flush(0);
	for (i = 0; i < 4;i++) {
		if (childpid[i] != -1)
			kill(childpid[i], SIGTERM);
	}
	_exit (1);
}

static void
die1()
{
	subprintf(&sserr, "%d: ARGH!!! i have been murdered\n", getpid());
	flush(0);
	_exit (0);
}

static void
die2()
{
	flush(0);
	_exit (0);
}

#ifndef HAVE_WIRINGPI
static uint64_t epochMilli, epochMicro ;
/*-
 * initialiseEpoch:
 * Initialise our start-of-time variable to be the current unix
 * time in milliseconds and microseconds.
 */
static void
initialiseEpoch (void)
{
  struct timespec ts ;

  clock_gettime (CLOCK_MONOTONIC_RAW, &ts) ;
  epochMilli = (uint64_t)ts.tv_sec * (uint64_t)1000    + (uint64_t)(ts.tv_nsec / 1000000L) ;
  epochMicro = (uint64_t)ts.tv_sec * (uint64_t)1000000 + (uint64_t)(ts.tv_nsec /    1000L) ;
}

/*-
 * millis:
 * Return a number of milliseconds as an unsigned int.
 * Wraps at 49 days.
 */
unsigned long
millis (void)
{
  uint64_t now ;

  struct  timespec ts ;

  clock_gettime (CLOCK_MONOTONIC_RAW, &ts) ;
  now  = (uint64_t)ts.tv_sec * (uint64_t)1000 + (uint64_t)(ts.tv_nsec / 1000000L) ;
  return (now - epochMilli) ;
}
#endif

#ifdef HAVE_FLOCK
int
lockfile(int type, int _lockfd)
{
    int             fd;

	if (type) {
		if ((fd = open(".lock", O_CREAT|O_TRUNC, 0600)) == -1) {
			subprintf(&sserr, "%d: flock: LOCK_EX: %s\n", getpid(), error_str(errno));
			flush(2);
			_exit (111);
		}
		if (flock(fd, LOCK_EX) == -1) {
			subprintf(&sserr, "%d: flock: LOCK_EX: %s\n", getpid(), error_str(errno));
			close(fd);
			flush(2);
			_exit (111);
		}
		return (fd);
	} else {
		if (flock(_lockfd, LOCK_UN) == -1) {
			close(_lockfd);
			subprintf(&sserr, "%d: flock: LOCK_UN: %s\n", getpid(), error_str(errno));
			flush(2);
			_exit (111);
		}
		close (_lockfd);
		return (0);
	}
}
#endif

void
scrollMessage(int handle, int col, int delay, int width, char *message)
{
	int             count, flag;
	uint64_t        i;
#if USE_MILLIS
	static uint64_t timer = 0;
#endif
	static int      j, position = 0, start_col;

	if (!start_col) {
		start_col = col + 1;
		j = col;
	}
	if (j) {
		for (count = 0, i = 0;i < j;i++, count++)
			lcdPutchar(handle, ' ');
	} else
		count = 0;
	for (flag = i = 0;i < width - count; i++) {
		if (flag || !message[i + position]) {
			flag = 1;
			lcdPutchar(handle, ' ');
		} else
			lcdPutchar(handle, message[i + position]);
	}
	if (j)
		j--;
	else {
		position++;
		if (!message[position]) {
			position = 0;
			start_col = 1;
			j = width - 1;
		}
	}
#if USE_MILLIS /* this consumes CPU. Have to figure out to make it consume less */
	if ((i = millis()) < timer)
		return;
	timer = millis() + delay ;
#else
	usleep(delay * 1000);
#endif
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
	if (subprintf(&sserr, "USAGE: lcdDaemon [-v] [-f fifo_path] [-m fifo_mode] [-d scroll_delay]\n") == -1 ||
			subprintf(&sserr, "         -b bits -c cols -r rows -t readtimeout -p port\n") == -1)
		_exit (111);
	flush(2);
	_exit(100);
}

int
get_options(int argc, char **argv, char **fifo_path, unsigned long *fifo_mode,
		int *bits, int *cols, int *rows, int *delay)
{
	int             c;
	char            *ptr;

	verbose = 0;
	*fifo_path = (char *) 0;
	*fifo_mode = -1;
	*bits = *rows = *cols = *delay = -1;
	read_timeout = port = -1;
	while ((c = getopt(argc, argv, "vf:m:c:r:b:d:i:s:u:t:p:")) != -1) {
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
		case 'c':
			scan_int(optarg, cols);
			break;
		case 'r':
			scan_int(optarg, rows);
			break;
		case 'b':
			scan_int(optarg, bits);
			break;
		case 'd':
			scan_int(optarg, delay);
			break;
		case 't':
			scan_int(optarg, &read_timeout);
			break;
		case 'p':
			scan_int(optarg, &port);
			break;
		default:
			usage();
		}
	}
	if (*bits == -1) {
		if (!(ptr = env_get("BIT_MODE"))) {
			if (subprintf(&sserr, "LCD bit mode not specified (4 or 7 for 4bit or 8bit mode)\n") == -1)
				_exit (111);
			usage();
		} else
			scan_int(ptr, bits);
	}
	if (!(*bits == 4 || *bits == 8)) {
		if (subprintf(&sserr, "LCD bits must be 4 or 7 for 4 or 8 bit mode\n") == -1)
			_exit (111);
		usage();
	}
	if (*cols == -1) {
		if (!(ptr = env_get("LCD_WIDTH"))) {
			if (subprintf(&sserr, "LCD width not specified\n") == -1)
				_exit (111);
			usage();
		} else
			scan_int(ptr, cols);
	}
	if (!(*cols == 20 || *cols == 16)) {
		if (subprintf(&sserr, "LCD width must be 16 or 20\n") == -1)
			_exit (111);
		usage();
	}
	if (*rows == -1) {
		if (!(ptr = env_get("LCD_ROWS"))) {
			if (subprintf(&sserr, "LCD rows not specified\n") == -1)
				_exit (111);
			usage();
		} else
			scan_int(ptr, rows);
	}
	if (!(*rows == 1 || *rows == 2 || *rows == 4)) {
		if (subprintf(&sserr, "LCD rows must be 1, 2 or 4\n") == -1)
			_exit (111);
		usage();
	}
	if (!*fifo_path && !(*fifo_path = env_get("LCDFIFO"))) {
		if (!access("/run", F_OK))
			*fifo_path = "/run/lcd-daemon/lcdfifo";
		else
		if (!access("/var/run", F_OK))
			*fifo_path = "/var/run/lcd-daemon/lcdfifo";
		else
			*fifo_path = LCDFIFO;
	}
	if (*fifo_mode == -1) {
		if ((ptr = env_get("FIFO_MODE")))
			scan_8long(ptr, fifo_mode);
		else
			*fifo_mode = 0666;
	}
	if (*delay == -1) {
		if ((ptr = env_get("SCROLL_DELAY")))
			scan_int(ptr, delay);
		else
			*delay = 200;
	}
	if (read_timeout == -1) {
		if ((ptr = env_get("DATA_TIMEOUT")))
			scan_int(ptr, &read_timeout);
		else
			read_timeout = 300;
	}
	if (port == -1) {
		if ((ptr = env_get("PORT")))
			scan_int(ptr, &port);
		else
			port = PORT;
	}
	return (0);
}

#ifdef HAVE_WIRINGPIDEV
int
lcd_initialize(int bits, int cols, int row, int rs, int en,
		int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7)
{
	int             i;
	extern struct  lcdDataStruct *lcds[];

	for (i = 0 ; i < MAX_LCDS ; ++i)
		lcds [i] = NULL ;
	if (bits == 4) {
		lcd = lcdInit(rows, cols, 4, rs, en, d4,d5,d6,d7, 0, 0, 0, 0);
		if (verbose && subprintf(&ssout, "LCD init rows=%d, cols=%d, bits=%d, RS=%d, EN=%d Data Pins (%d,%d,%d,%d,0,0,0,0)\n",
			rows, cols, bits, rs, en, d4,d5,d6,d7) == -1)
		_exit (111);
	} else {
		lcd = lcdInit(rows, cols, 8, rs, en, d0,d1,d2,d3,d4,d5,d6,d7);
		if (verbose && subprintf(&ssout, "LCD init rows=%d, cols=%d, bits=%d, RS=%d, EN=%d Data Pins (%d,%d,%d,%d,%d,%d,%d,%d)\n",
			rows, cols, bits, rs, en, d0,d1,d2,d3, d4,d5,d6,d7) == -1)
			_exit (111);
	}
	flush(1);
	if (lcd < 0) {
		if (subprintf(&sserr, "lcdDaemon: lcdInit failed\n") == -1)
			_exit (111);
		flush(2);
		return (-1);
	}
	return 0;
}
#endif

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
read_data(int fd, int bits, int cols, int rows, int delay)
{
	static char     ssinbuf[512];
	substdio        ssin;
	stralloc        line = {0};
	int             i, lockfd, match, scroll = 0, clear, rpos;
	char           *ptr, *message;

	substdio_fdbuf(&ssin, saferead, fd, ssinbuf, sizeof ssinbuf);
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
		/* process the line */
		i = str_chr(line.s, ':');
		if (!line.s[i])
			continue;
		message = line.s + i + 1;
		ptr = line.s;
		scan_int(ptr, &rpos);
		i = str_chr(ptr, ' ');
		if (!ptr[i])
			continue;
		for (ptr += (i + 1);*ptr && isspace(*ptr); ptr++);
		if (!*ptr)
			continue;
		scan_int(ptr, &scroll);
		i = str_chr(ptr, ' ');
		if (!ptr[i])
			continue;
		for (ptr += (i + 1);*ptr && isspace(*ptr); ptr++);
		if (!*ptr)
			continue;
		scan_int(ptr, &clear);
		/*-
	 	* clear == 1 - clear lcd screen
	 	* clear == 2 - clear and initialize lcd screen
	 	* clear == 3 - initialize lcd screen
	 	* clear == 4 - clear lcd screen
	 	* clear == 5 - clear and initialize screen
	 	* clear == 6 - initialize screen
	 	*/
#ifdef HAVE_WIRINGPIDEV
		if (clear == 2 || clear == 3 || clear == 5 || clear == 6) {
			lcd = lcd_initialize(bits, cols, rows, pin_rs, pin_en,
					pin_d0, pin_d1, pin_d2, pin_d3, pin_d4, pin_d5, pin_d6, pin_d7);
			if (lcd < 0)
				_exit(111);
		}
#else
		lcd = 0;
#endif
		if (str_len(message) > cols)
			scroll = 1;
		if (verbose && subprintf(&ssout, "%d: Command scroll %d cols %d, rows %d, bits %d, rownum %d, clear %d pid %d [%s]\n",
			lcd, scroll, cols, rows, bits, rpos, clear, childpid[rpos], message) == -1)
			_exit (111);
		flush(1);
		if (rpos >= rows) {
			subprintf(&sserr, "rownum cannot be greater than %d\n", rows - 1);
			return 1;
		}
		if (clear == 1 || clear == 2 || clear == 4 || clear == 5)
			lcdClear(lcd);
		if (clear == 4 || clear == 5 || clear == 6)
			continue;
#ifdef HAVE_FLOCK
		if (childpid[rpos] != -1) {
			if (verbose && subprintf(&ssout, "Kill childpid[%d]=%d\n", rpos, childpid[rpos]) == -1)
				_exit(111);
			flush(1);
			if (kill(childpid[rpos], SIGUSR1)) {
				if (subprintf(&sserr, "kill childpid[%d]=%d: %s\n", rpos, childpid[rpos], error_str(errno)) == -1)
					_exit (111);
				flush(2);
				_exit (1);
			}
		}
#else
		for (i = 0; i < 4;i++) {
			if (childpid[i] != -1) {
				if (verbose && subprintf(&ssout, "Kill childpid[%d]=%d\n", i, childpid[i]) == -1)
					_exit(111);
				flush(1);
				if (kill(childpid[i], SIGUSR1)) {
					if (subprintf(&sserr, "kill childpid[%d]=%d: %s\n", i, childpid[i], error_str(errno)) == -1)
						_exit (111);
					flush(2);
					_exit (1);
				}
			}
		}
#endif
		if (!scroll) {
			lockfd = lockfile(1, 0);
			lcdPosition(lcd, 0, rpos);
			lcdPrintf(lcd, "%-*s", cols, message);
			lockfile(0, lockfd);
		} else {
			switch ((childpid[rpos] = fork()))
			{
			case -1:
				if (subprintf(&sserr, "fork: %s\n", error_str(errno)) == -1)
					_exit (111);
				flush(2);
				return (1);
			case 0:
				(void) signal(SIGTERM, (void (*)()) die1);
				(void) signal(SIGUSR1, (void (*)()) die2);
				close(fd);
				for (i = 0;;) {
					lockfd = lockfile(1, 0);
					if (!i) {
						lcdPosition(lcd, 0, rpos);
						lcdPrintf(lcd, "%-*s", cols, " ");
					}
					lcdPosition(lcd, 0, rpos);
					scrollMessage(lcd, 0, delay, cols, message);
					lockfile(0, lockfd);
					i = 1;
				}
				break;
			default:
				if (verbose && subprintf(&ssout, "childpid[%d]=%d\n", rpos, childpid[rpos]) == -1)
					_exit (111);
				flush(1);
				break;
			} /* switch ((childpid[rpos] = fork())) */
		} /*- scroll */
	}
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
	int             i, bits, fifofd, sockfd, lockfd, delay;
	unsigned long   fifo_mode;
	char           *fifo_path;
	fd_set          rfds;	/*- File descriptor mask for select -*/

	if (get_options(argc, argv, &fifo_path, &fifo_mode, &bits, &cols, &rows, &delay))
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
#ifdef HAVE_WIRINGPI
	if (wiringPiSetup() == -1) {
		if (subprintf(&sserr, "wiringPiSetup: %s\n", error_str(errno)) == -1)
			_exit (111);
		flush(2);
		return (1);
	}
#else
	initialiseEpoch();
#endif
	(void) signal(SIGTERM, (void (*)()) SigTerm);
	(void) signal(SIGCHLD, (void (*)()) SigChild);
#ifdef HAVE_WIRINGPIDEV
	getEnvConfigInt(&pin_rs, "PIN_RS", PIN_RS);
	getEnvConfigInt(&pin_en, "PIN_EN", PIN_EN);
	getEnvConfigInt(&pin_d0, "PIN_D0", PIN_D0);
	getEnvConfigInt(&pin_d1, "PIN_D1", PIN_D1);
	getEnvConfigInt(&pin_d2, "PIN_D2", PIN_D2);
	getEnvConfigInt(&pin_d3, "PIN_D3", PIN_D3);
	getEnvConfigInt(&pin_d4, "PIN_D4", PIN_D4);
	getEnvConfigInt(&pin_d5, "PIN_D5", PIN_D5);
	getEnvConfigInt(&pin_d6, "PIN_D6", PIN_D6);
	getEnvConfigInt(&pin_d7, "PIN_D7", PIN_D7);
#endif
#ifdef HAVE_WIRINGPIDEV
	lcd = lcd_initialize(bits, cols, rows, pin_rs, pin_en,
			pin_d0, pin_d1, pin_d2, pin_d3, pin_d4, pin_d5, pin_d6, pin_d7);
	if (lcd < 0)
		_exit(111);
#else
	lcd = 0;
#endif
	if ((sockfd = initialize_socket()) == -1) {
		_exit(111);
	}
	if (startup_msg && lcd != -1) {
		lcdClear(lcd);
		lockfd = lockfile(1, 0);
		lcdPosition(lcd, 0, 0);
		lcdPrintf(lcd, "%-*s", cols, startup_msg);
		lockfile(0, lockfd);
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
			read_data(fifofd, bits, cols, rows, delay);
		}
		if (FD_ISSET(sockfd, &rfds)) {
			fdtype = sock;
			read_data(sockfd, bits, cols, rows, delay);
		}
	} /*- for(;;) */
	return 0;
}

/*
 * $Log: lcdDaemon.c,v $
 * Revision 1.12  2023-06-27 22:38:57+05:30  Cprogrammer
 * moved sleep to the end of function
 *
 * Revision 1.11  2023-06-25 11:59:23+05:30  Cprogrammer
 * automatically scroll if length is greater than lcd width
 *
 * Revision 1.10  2023-06-25 09:38:11+05:30  Cprogrammer
 * added udp listener
 *
 * Revision 1.9  2023-06-23 17:43:22+05:30  Cprogrammer
 * interchanged rownum and scroll fields
 * prevent row number to be greater than rows supported by LCD
 *
 * Revision 1.8  2023-06-23 12:30:09+05:30  Cprogrammer
 * added startup, shutdown message
 *
 * Revision 1.7  2023-06-22 23:39:36+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.6  2014-09-03 13:12:25+05:30  Cprogrammer
 * BUG. Mode was not specified for O_CREAT
 *
 * Revision 1.5  2014-09-03 10:53:16+05:30  Cprogrammer
 * flush stdout when wiringPi is not available
 *
 * Revision 1.4  2014-09-02 22:12:57+05:30  Cprogrammer
 * use flock() to allow simultaneous scrolling of multiple rows
 *
 * Revision 1.2  2014-09-02 00:18:10+05:30  Cprogrammer
 * fix for missing wiringPiDev lib
 *
 * Revision 1.1  2014-09-01 20:03:14+05:30  Cprogrammer
 * Initial revision
 *
 */
