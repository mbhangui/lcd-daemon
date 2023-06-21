@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

DEFS=@DEFS@ -Dlint -fno-strict-aliasing -I. -I/usr/include/wiringPi -I/usr/include/wiringPiDev -g

INSTALL_PROGRAM = ${INSTALL}          -c
install_sh_PROGRAM = $(install_sh)    -c
INSTALL_SCRIPT = ${INSTALL}           -c
install_sh_SCRIPT = $(install_sh)     -c
INSTALL_STRIP_PROGRAM = $(install_sh) -c $(STRIP_OPT)

edit = sed \
	-e 's}@PACKAGE\@}$(PACKAGE)}g' \
	-e 's}@PACKAGE_VERSION\@}$(PACKAGE_VERSION)}g' \
	-e 's}@version\@}$(VERSION)}g' \
	-e "s|@release\@|"`cat conf-release`"|g" \
	-e 's|@email\@|'"`cat conf-email`"'|g' \
	-e 's}@pkgconfigdir\@}$(pkgconfigdir)}g' \
	-e 's}@VERSION\@}$(VERSION)}g' \
	-e 's}@prefix\@}$(prefix)}g'

lcdbindir=@prefix@/bin
lcdbin_PROGRAMS = pilcd lcdDaemon
noinst_DATA = lcd-daemon.spec
man_MANS=pilcd.1 lcdDaemon.8
lcdlibexecdir=@libexecdir@/lcd-daemon
lcdlibexec_SCRIPTS=create_service

lcdDaemon_SOURCES = lcdDaemon.c
lcdDaemon_DEPENDENCIES = getln.lo getln2.lo scan_ulong.lo stralloc_catb.lo \
	stralloc_eady.lo subprintf.lo error_str.lo error.lo substdo.lo str_chr.lo \
	byte_chr.lo byte_copy.lo str_len.lo alloc.lo alloc_re.lo stralloc_opyb.lo substdi.lo byte_cr.lo \
	stralloc_pend.lo getEnvConfig.lo envread.lo scan_8long.lo str_diffn.lo
lcdDaemon_LDADD = getln.lo getln2.lo scan_ulong.lo stralloc_catb.lo stralloc_eady.lo error_str.lo \
	error.lo substdo.lo str_chr.lo subprintf.lo byte_chr.lo byte_copy.lo str_len.lo alloc.lo alloc_re.lo \
	stralloc_pend.lo substdi.lo stralloc_opyb.lo byte_cr.lo getEnvConfig.lo envread.lo scan_8long.lo \
	str_diffn.lo $(LIB_WIRINGPI) $(LIB_WIRINGPIDEV) -lpthread -lrt -lm -lcrypt

pilcd_SOURCES = pilcd.c
pilcd_LDADD = substdio.lo substdo.lo str_len.lo lcdPrint.lo error_str.lo error.lo \
	byte_copy.lo subprintf.lo getEnvConfig.lo envread.lo scan_8long.lo scan_ulong.lo str_diffn.lo \
	stralloc_catb.lo stralloc_eady.lo stralloc_opyb.lo stralloc_pend.lo alloc.lo alloc_re.lo
pilcd_DEPENDENCIES = substdio.lo substdo.lo str_len.lo lcdPrint.lo error_str.lo error.lo \
	byte_copy.lo subprintf.lo getEnvConfig.lo envread.lo scan_8long.lo scan_ulong.lo str_diffn.lo \
	stralloc_catb.lo stralloc_eady.lo stralloc_opyb.lo stralloc_pend.lo alloc.lo alloc_re.lo

lcd-daemon.spec: lcd-daemon.spec.in catChangeLog doc/ChangeLog conf-version \
conf-release conf-email
	(cat $@.in;./catChangeLog) | $(edit) > $@
lcd-daemon.changes: doc/ChangeLog conf-version conf-release conf-email
	./catChangeLog --changes doc/ChangeLog > $@

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

# AUTOMAKE_OPTIONS = foreign
#
