/*
 * $Log: getEnvConfig.c,v $
 * Revision 1.1  2014-08-18 07:11:38+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "env.h"
#include "scan.h"

#ifndef	lint
static char     sccsid[] = "$Id: getEnvConfig.c,v 1.1 2014-08-18 07:11:38+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * getEnvConfigStr
 */
void
getEnvConfigStr(char **source, char *envname, char *defaultValue)
{
	if (!(*source = env_get(envname)))
		*source = defaultValue;
	return;
}

void
getEnvConfigInt(long *source, char *envname, long defaultValue)
{
	char           *value;

	if (!(value = env_get(envname)))
		*source = defaultValue;
	else
		scan_long(value, source);
	return;
}
