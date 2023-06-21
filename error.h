/*
 * $Log: error.h,v $
 * Revision 1.1  2014-09-01 20:05:30+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifndef ERROR_H
#define ERROR_H

#include <errno.h>

extern int      error_ebadf;
extern int      error_intr;
extern int      error_nomem;
extern int      error_noent;
extern int      error_txtbsy;
extern int      error_io;
extern int      error_exist;
extern int      error_timeout;
extern int      error_inprogress;
extern int      error_wouldblock;
extern int      error_again;
extern int      error_dquot;
extern int      error_pipe;
extern int      error_perm;
extern int      error_acces;
extern int      error_nodevice;
extern int      error_proto;
extern int      error_isdir;
extern int      error_connrefused;
extern int      error_hostdown;
extern int      error_netunreach;
extern int      error_hostunreach;

char           *error_str(int);
char           *estack(int, const char *);
int             error_temp(int);

#endif
