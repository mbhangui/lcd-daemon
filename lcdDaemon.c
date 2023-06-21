/*
 * $Log: lcdDaemon.c,v $
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
#include "subprintf.h"
#include "getln.h"
#include "error.h"
#include "str.h"

/*
 * LCD_PIN WIRINGPI_PIN BOARD_PIN
 */
#define PIN_EN 5  /*- 18 */
#define PIN_RS 6  /*- 22 */
#define PIN_D0 0  /*- ?? */
#define PIN_D1 0  /*- ?? */
#define PIN_D2 0  /*- ?? */
#define PIN_D3 0  /*- ?? */
#define PIN_D4 4  /*- 16 */
#define PIN_D5 0  /*- 11 */
#define PIN_D6 2  /*- 13 */
#define PIN_D7 3  /*- 15 */

static char     ssoutbuf[512], sserrbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
static char     ssinbuf[512];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static int      childpid[4] = {-1,-1,-1,-1};

static void
flush(int type)
{
	if ((!type || type == 2) && substdio_flush(&sserr))
		_exit (111);
	if ((!type || type == 1) && substdio_flush(&ssout))
		_exit (111);
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

#ifndef HAVE_WIRINGPIDEV
#include <stdio.h>
#define lcdPutchar(h, c)         {putchar((c)); fflush(stdout);}
#define lcdPrintf(h, s1, s2, s3) {printf(s1, s2, s3); fflush(stdout);}
#define lcdPosition(h, col, row) {printf("\033[%d;%dH", (row), (col)); fflush(stdout);}
#define lcdClear(h)              {printf("\033[2J\033[H"); fflush(stdout);}
#endif

#ifdef HAVE_FLOCK
int
lockfile(int type, int lockfd)
{
    int             fd;

	if (type) {
		if ((fd = open("/tmp/lcdDaemon.lock", O_CREAT|O_TRUNC, 0600)) == -1) {
			subprintf(&sserr, "%d: open: LOCK_EX: %s\n", getpid(), error_str(errno));
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
		if (flock(lockfd, LOCK_UN) == -1) {
			close(lockfd);
			subprintf(&sserr, "%d: flock: LOCK_UN: %s\n", getpid(), error_str(errno));
			flush(2);
			_exit (111);
		}
		close (lockfd);
		return (0);
	}
}
#endif

void
scrollMessage(int handle, int col, int delay, int width, char *message)
{
	int             i, count, flag;
	static int      j, position = 0, start_col;
#ifdef HAVE_WIRINGPIDEV
	static int      timer = 0;
#endif

#ifdef HAVE_WIRINGPIDEV
	if ((i = millis ()) < timer)
		usleep(timer - i);
	timer += delay ;
#else
	usleep(delay);
#endif
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
}

int
main(int argc, char **argv)
{
	int             i, scroll = 0, cols, rows, bits, rpos, clear, lcd, fd, lockfd, match;
	stralloc        line = {0};
	char           *message, *ptr;
#ifdef HAVE_WIRINGPIDEV
	long            pin_rs, pin_en, pin_d0, pin_d1, pin_d2, pin_d3, pin_d4, pin_d5, pin_d6, pin_d7;
	extern struct  lcdDataStruct *lcds[];
#endif

	getEnvConfigStr(&ptr, "LCDFIFO", LCDFIFO);
	if (access(ptr, F_OK) && mkfifo(ptr, 0666)) {
		if (subprintf(&sserr, "mkfifo: %s: %s\n", ptr, error_str(errno)) == -1)
			_exit (111);
		flush(2);
		return (1);
	}
	close(0);
	if ((fd = open(ptr, O_RDWR)) == -1) {
		if (subprintf(&sserr, "open: %s: %s\n", ptr, error_str(errno)) == -1)
			_exit (111);
		flush(2);
		_exit (111);
	}
#ifdef HAVE_WIRINGPIDEV
	if (wiringPiSetup() == -1) {
		if (subprintf(&sserr, "wiringPiSetup: %s\n", error_str(errno)) == -1)
		flush(2);
		return (1);
	}
#endif
	(void) signal(SIGTERM, (void (*)()) SigTerm);
	(void) signal(SIGCHLD, (void (*)()) SigChild);
	for (lcd = -1;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			if (subprintf(&sserr, "getln: %s\n", error_str(errno)) == -1)
				_exit (111);
			flush(2);
			_exit (1);
		}
		if (!match && line.len == 0)
			break;
		line.len--;
		if (!stralloc_0(&line)) {
			if (subprintf(&sserr, "out of memory: %s\n", error_str(errno)) == -1)
				_exit (111);
			flush(2);
			_exit (1);
		}
		i = str_chr(line.s, ':');
		if (!line.s[i])
			continue;
		message = line.s + i + 1;
		ptr = line.s;
		scan_int(ptr, &scroll);
		i = str_chr(ptr, ' ');
		if (!ptr[i])
			continue;
		for (ptr += (i + 1);*ptr && isspace(*ptr); ptr++);
		if (!*ptr)
			continue;
		scan_int(ptr, &cols);
		i = str_chr(ptr, ' ');
		if (!ptr[i])
			continue;
		for (ptr += (i + 1);*ptr && isspace(*ptr); ptr++);
		if (!*ptr)
			continue;
		scan_int(ptr, &rows);
		i = str_chr(ptr, ' ');
		if (!ptr[i])
			continue;
		for (ptr += (i + 1);*ptr && isspace(*ptr); ptr++);
		if (!*ptr)
			continue;
		scan_int(ptr, &bits);
		i = str_chr(ptr, ' ');
		if (!ptr[i])
			continue;
		for (ptr += (i + 1);*ptr && isspace(*ptr); ptr++);
		if (!*ptr)
			continue;
		scan_int(ptr, &rpos);
		i = str_chr(ptr, ' ');
		if (!ptr[i])
			continue;
		for (ptr += (i + 1);*ptr && isspace(*ptr); ptr++);
		if (!*ptr)
			continue;
		scan_int(ptr, &clear);
		/*
		 * clear == 1 - clear lcd screen
		 * clear == 2 - clear and initialize lcd screen
		 * clear == 3 - initialize lcd screen
		 * clear == 4 - clear lcd screen
		 * clear == 5 - clear and initialize screen
		 * clear == 6 - initialize screen
		 */
#ifdef HAVE_WIRINGPIDEV
		if (lcd == -1 || clear == 2 || clear == 3 || clear == 5 || clear == 6) {
			if (lcd == -1) {
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
			}
			for (i = 0 ; i < MAX_LCDS ; ++i)
				lcds [i] = NULL ;
			if (bits == 4) {
				lcd = lcdInit(rows, cols, 4, pin_rs, pin_en, pin_d4,pin_d5,pin_d6,pin_d7, 0, 0, 0, 0);
				if (subprintf(&ssout, "LCD init rows=%d, cols=%d, bits=%d, RS=%d, EN=%d Data Pins (%d,%d,%d,%d,0,0,0,0)\n",
					rows, cols, bits, pin_rs, pin_en, pin_d4,pin_d5,pin_d6,pin_d7) == -1)
					_exit (111);
			} else {
				lcd = lcdInit(rows, cols, 8, pin_rs, pin_en, pin_d0,pin_d1,pin_d2,pin_d3,pin_d4,pin_d5,pin_d6,pin_d7);
				if (subprintf(&ssout, "LCD init rows=%d, cols=%d, bits=%d, RS=%d, EN=%d Data Pins (%d,%d,%d,%d,%d,%d,%d,%d)\n",
					rows, cols, bits, pin_rs, pin_en, pin_d0,pin_d1,pin_d2,pin_d3, pin_d4,pin_d5,pin_d6,pin_d7) == -1)
					_exit (111);
			}
			flush(1);
			if (lcd < 0) {
				if (subprintf(&sserr, "%s: lcdInit failed\n", argv[0]) == -1)
					_exit (111);
				flush(2);
				return (1);
			}
		}
#else
		lcd = 0;
#endif
		if (subprintf(&ssout, "%d: Command scroll %d cols %d, rows %d, bits %d, rownum %d, clear %d pid %d [%s]\n",
			lcd, scroll, cols, rows, bits, rpos, clear, childpid[rpos], message) == -1)
			_exit (111);
		flush(1);
		if (clear == 1 || clear == 2 || clear == 4 || clear == 5)
			lcdClear(lcd);
		if (clear == 4 || clear == 5 || clear == 6)
			continue;
#ifdef HAVE_FLOCK
		if (childpid[rpos] != -1) {
			if (subprintf(&ssout, "Kill child %d row %d\n", childpid[rpos], rpos) == -1)
				_exit(111);
			flush(1);
			if (kill(childpid[rpos], SIGUSR1)) {
				if (subprintf(&sserr, "kill: %d: %s\n", childpid[rpos], error_str(errno)) == -1)
					_exit (111);
				flush(2);
				_exit (1);
			}
		}
#else
		for (i = 0; i < 4;i++) {
			if (childpid[i] != -1) {
				if (subprintf(&ssout, "Kill child %d row %d\n", childpid[i], i) == -1)
					_exit(111);
				flush(1);
				if (kill(childpid[i], SIGUSR1)) {
					if (subprintf(&sserr, "kill: %d: %s\n", childpid[rpos], error_str(errno)) == -1)
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
#ifndef HAVE_FLOCK
			for (i = 0; i < 4;i++) {
				if (childpid[i] != -1) {
					if (subprintf(&ssout, "%d: Kill child %d row %d\n", getpid(), childpid[i], i) == -1)
						_exit(111);
					flush(1);
					if (kill(childpid[i], SIGUSR1)) {
						if (subprintf(&sserr, "%d: kill: %d: %s\n", getpid(), childpid[rpos], error_str(errno)) == -1)
							_exit (111);
						flush(2);
						_exit (1);
					}
				}
			}
#endif
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
					scrollMessage(lcd, 0, 1000, cols, message);
					lockfile(0, lockfd);
					i = 1;
				}
				break;
			default:
				if (subprintf(&ssout, "fork pid=%d, rpos=%d\n", childpid[rpos], rpos) == -1)
					_exit (111);
				flush(1);
				break;
			}
		} 
	}
	return (0);
}
