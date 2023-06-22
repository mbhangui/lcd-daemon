/*
 * $Log: env.h,v $
 * Revision 1.2  2023-06-22 23:39:17+05:30  Cprogrammer
 * fixed compilation error
 *
 * Revision 1.1  2014-09-01 20:05:24+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifndef ENV_H
#define ENV_H

extern int      env_isinit;
extern char   **environ;

int             env_init(void);
int             env_put(char *);
int             env_put2(char *, char *);
int             env_unset(char *);
void            restore_env(void);
char           *env_get(char *);
char           *env_pick(void);
void            env_clear(void);
char           *env_findeq(char *);

#endif
