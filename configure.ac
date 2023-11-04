dnl Process this file with autoconf to produce a configure script.

AC_CONFIG_MACRO_DIR([m4])

m4_define([v_maj], [0])
m4_define([v_min], [1])
m4_define([v_mic], [2])
m4_define([project_version], [v_maj.v_min.v_mic])
m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], [v_mic])
m4_define([lt_age], [v_min])

AC_INIT([geodata],[project_version])

version_info="lt_cur:lt_rev:lt_age"
AC_SUBST([version_info])

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

AX_CHECK_ENABLE_DEBUG([info], [DEBUG], [RELEASE], [IS-RELEASE])
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
dnl we do not depend on c++20 in just one place (glglobe),
dnl (did choose it for consistency across libs and programs)
dnl so if you happen to find yourself in the need to use
dnl a older compiler c++17, should work as well.
AX_CXX_COMPILE_STDCXX([20], [noext], [optional])

AC_DISABLE_STATIC

LT_INIT

AC_CANONICAL_HOST
AC_MSG_NOTICE([host_os $host_os])


PKG_CHECK_MODULES(GTKMM, [gtkmm-3.0])

PKG_CHECK_MODULES([GLIBMM], [glibmm-2.4 giomm-2.4])

PKG_CHECK_MODULES([GENERICGLM],[genericglm >= 0.2.4])
PKG_CHECK_MODULES([GENERICIMG],[genericimg >= 0.2.0])

PKG_CHECK_MODULES(LIBSOUP3, [libsoup-3.0])
PKG_CHECK_MODULES(JSONGLIB1, [json-glib-1.0])

if test "$host_os" = "mingw32" ; then
    AC_SUBST(EXTRA_LDFLAGS, "-no-undefined")
fi

AC_CONFIG_FILES([
Makefile
src/geodata.pc
src/Makefile
])
AC_OUTPUT
