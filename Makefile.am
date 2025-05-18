@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

DEFS=@DEFS@ -Dlint -fno-strict-aliasing -I. -I/usr/include/qmail -I/usr/include/wiringPi -I/usr/include/wiringPiDev -g

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
lcdsbindir=@prefix@/sbin
lcdbin_PROGRAMS = pilcd pinotify
lcdsbin_PROGRAMS = lcd-daemon notify-daemon
noinst_DATA = lcd-daemon.spec
man_MANS=pilcd.1 pinotify.1 lcd-daemon.8 notify-daemon.8
lcdlibexecdir=@libexecdir@/lcd-daemon
lcdlibexec_SCRIPTS=create_service

lcd_daemon_SOURCES = lcd-daemon.c
lcd_daemon_DEPENDENCIES = timeoutread.lo
lcd_daemon_LDADD = timeoutread.lo $(LIB_QMAIL) $(LIB_WIRINGPI) $(LIB_WIRINGPIDEV) -lpthread -lrt -lm -lcrypt

notify_daemon_SOURCES = notify-daemon.c
notify_daemon_DEPENDENCIES = timeoutread.lo
notify_daemon_LDADD = timeoutread.lo $(LIB_QMAIL)

pilcd_SOURCES = pilcd.c
pilcd_LDADD = lcdPrint.lo $(LIB_QMAIL)
pilcd_DEPENDENCIES = lcdPrint.lo

pinotify_SOURCES = pinotify.c
pinotify_LDADD = lcdPrint.lo $(LIB_QMAIL)
pinotify_DEPENDENCIES = lcdPrint.lo

select.h: trysysel.c select.h1 select.h2
	( $(COMPILE) -c trysysel.c >/dev/null 2>&1 \
	&& cat select.h2 || cat select.h1 ) > select.h
	rm -f trysysel.o trysysel

timeoutread.lo: select.h

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
