/*
 * $Log: gen_alloc.h,v $
 * Revision 1.1  2014-09-01 20:05:46+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifndef GEN_ALLOC_H
#define GEN_ALLOC_H

#define GEN_ALLOC_typedef(ta,type,field,len,a) \
  typedef struct ta { type *field; unsigned int len; unsigned int a; } ta;

#endif
