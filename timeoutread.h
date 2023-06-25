/*
 * $Log: timeoutread.h,v $
 * Revision 1.1  2023-06-24 23:08:18+05:30  Cprogrammer
 * Initial revision
 *
 *
 * $Id: timeoutread.h,v 1.1 2023-06-24 23:08:18+05:30 Cprogrammer Exp mbhangui $
 */
#ifndef TIMEOUTREAD_H
#define TIMEOUTREAD_H
#include <sys/types.h>

ssize_t         timeoutread_fifo(long, int, char *, size_t);
ssize_t         timeoutread_sock(long, int, char *, size_t);

#endif
