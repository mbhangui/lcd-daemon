/*
 * $Log: str.h,v $
 * Revision 1.1  2014-08-20 18:06:29+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifndef STR_H
#define STR_H

unsigned int    str_copy(char *, char *);
unsigned int    str_copyb(char *, char *, unsigned int);
int             str_diff(char *, char *);
int             str_diffn(char *, char *, unsigned int);
unsigned int    str_len(char *);
unsigned int    str_chr(char *, int);
unsigned int    str_rchr(char *, int);
int             str_start(char *, char *);
char           *str_chrn(char *, int, int);
char           *str_str(char *, char *);
#include <sys/types.h>
size_t          str_cspn(const char *, register const char *);

#define str_equal(s,t) (!str_diff((s),(t)))

#endif
