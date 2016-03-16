m4_define([dali_version],[0.1.0])
AC_INIT([dali], [dali_version])
AM_INIT_AUTOMAKE([-Wall foreign])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_MKDIR_P

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

LT_INIT

DALI_ADAPTOR_VERSION=dali_version
AC_SUBST(DALI_ADAPTOR_VERSION)

AM_CONDITIONAL([COMMON_PROFILE], [test x$enable_profile = xCOMMON])
AM_CONDITIONAL([MOBILE_PROFILE], [test x$enable_profile = xMOBILE])
AM_CONDITIONAL([WEARABLE_PROFILE], [test x$enable_profile = xWEARABLE])
AM_CONDITIONAL([TV_PROFILE], [test x$enable_profile = xTV])
AM_CONDITIONAL([UBUNTU_PROFILE], [test x$enable_profile = xUBUNTU])
AM_CONDITIONAL([WAYLAND], [test x$enable_wayland = xyes])

AM_CONDITIONAL([USE_FEEDBACK], [test x$enable_feedback = xyes])
AM_CONDITIONAL([USE_VIDEOPLAYER], [test x$enable_videoplayer = xyes])

if test "x$enable_wayland" = "xyes"; then
if test "x$enable_videoplayer" = "xyes"; then
if test "x$enable_profile" != "xUBUNTU"; then
PKG_CHECK_MODULES(CAPI_MEDIA_PLAYER, capi-media-player)
PKG_CHECK_MODULES(ECORE_WAYLAND, ecore-wayland)
fi
fi # enable_videoplayer
fi # enable_wayland

if test "x$enable_feedback" = "xyes"; then

if test "x$enable_profile" = "xCOMMON" || test "x$enable_profile" = "xTV"; then
PKG_CHECK_MODULES(HAPTIC, haptic)
fi

if test "x$enable_profile" = "xMOBILE"; then
PKG_CHECK_MODULES(DEVICED, deviced)
fi

PKG_CHECK_MODULES(FEEDBACK, feedback)
PKG_CHECK_MODULES(MMFSOUND, mm-sound)

fi # enable_feedback

AC_CONFIG_FILES(Makefile)

AC_OUTPUT
