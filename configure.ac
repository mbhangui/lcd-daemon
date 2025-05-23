#
# $Log: configure.ac,v $
# Revision 1.5  2025-05-18 22:39:11+05:30  Cprogrammer
# added check for libqmail
#
# Revision 1.4  2023-06-24 22:36:18+05:30  Cprogrammer
# updated AC_CHECK_HEADERS
#
# Revision 1.3  2023-06-23 12:29:48+05:30  Cprogrammer
# fixed support address
#
# Revision 1.2  2023-06-22 23:38:30+05:30  Cprogrammer
# updated configure.ac
#
# Revision 1.1  2014-09-02 22:13:49+05:30  Cprogrammer
# Initial revision
#
#
# $Id: configure.ac,v 1.5 2025-05-18 22:39:11+05:30 Cprogrammer Exp mbhangui $
#
AC_PREREQ([2.64])
AC_INIT([lcd-daemon],[m4_normalize(m4_include(conf-version))],[m4_normalize(m4_include(conf-email))],[lcd-daemon],[https://github.com/mbhangui/lcd-daemon])
AC_CANONICAL_TARGET
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
LT_INIT
AM_INIT_AUTOMAKE([foreign silent-rules no-dist])
AM_CONFIG_HEADER(config.h)
AC_DEFINE_UNQUOTED(PACKAGE_BUGREPORT, "lcd-daemon@indimail.org")
if test "$GCC" = yes ; then
	CXXFLAGS="$CXXFLAGS -Wall"
	CFLAGS="$CFLAGS -O4 -Wall"
fi

dnl Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_MAKE_SET

dnl Checks for header files.
CPPFLAGS="$CPPFLAGS -I/usr/include/wiringPi -I/usr/include/wiringPiDev"
dnl AC_ISC_POSIX
AC_CHECK_HEADERS(ctype.h errno.h stdarg.h unistd.h fcntl.h signal.h inttypes.h)
AC_CHECK_HEADERS(sys/types.h sys/wait.h sys/file.h sys/socket.h netinet/in.h)
AC_CHECK_HEADERS(wiringPi.h lcd.h)

AC_CHECK_LIB(qmail, substdio_fdbuf, [AC_SUBST([LIB_QMAIL], ["-lqmail"]) AC_DEFINE([HAVE_QMAIL], [1],[qmail Library])],noqmail=t,)
if test " $noqmail" = " t"
then
  AC_MSG_ERROR(Could not find qmail library.)
fi
AC_CHECK_LIB(wiringPi,    millis,    [AC_SUBST([LIB_WIRINGPI],   ["-lwiringPi"])    AC_DEFINE([HAVE_WIRINGPI],    [1],[WiringPi Library])],,-lwiringPi)
AC_CHECK_LIB(wiringPiDev, lcdPrintf, [AC_SUBST([LIB_WIRINGPIDEV],["-lwiringPiDev"]) AC_DEFINE([HAVE_WIRINGPIDEV], [1],[WiringPiDev Library])],,-lwiringPiDev)
AC_CHECK_FUNC(flock)
if test "$ac_cv_func_flock" = yes ; then
	AC_DEFINE([HAVE_FLOCK], [1], [flock function])
fi

dnl Does not work on RHEL 5, CentOS 5 64 bit systems
RPATH_LIB="$acl_libdirstem"
if test -f /usr/bin/lsb_release
then
DISTRIB_ID=$(lsb_release -si)
DISTRIB_VERSION=$(lsb_release -sr)
else
DISTRIB_ID=$(sh ./lsb_release -si)
DISTRIB_VERSION=$(sh ./lsb_release -sr)
fi
AC_SUBST(DISTRIB_ID)
AC_SUBST(DISTRIB_VERSION)
defaultprefix="/usr"
LIBVER=0
AC_SUBST([LIBTOOL_DEPS])
AC_SUBST(LIBVER)
AC_SUBST(STRIP_OPT)
AC_SUBST(RPATH_LIB)
AC_SUBST(defaultprefix)

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args -prefix=$prefix"
fi

AC_OUTPUT(Makefile)
