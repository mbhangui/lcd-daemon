/*
 * $Log: byte.h,v $
 * Revision 1.1  2014-09-01 20:05:17+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifndef BYTE_H
#define BYTE_H

unsigned int    byte_chr(char *, unsigned int, int);
unsigned int    byte_rchr(char *, unsigned int, int);
unsigned int    byte_cspn(char *, unsigned int, char *);
unsigned int    byte_rcspn(char *, unsigned int, char *);
void            byte_copy(char *, unsigned int, char *);
void            byte_copyr(char *, unsigned int, char *);
int             byte_diff(char *, unsigned int, char *);
void            byte_zero(char *, unsigned int);

#define byte_equal(s,n,t) (!byte_diff((s),(n),(t)))

#endif
