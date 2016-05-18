#
# Copyright (c) 2015 Samsung Electronics Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

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

FREETYPE_REQUIRED=9.16.3
# 17.1.11 = Freetype version 2.5.2
FREETYPE_BITMAP_SUPPORT_VERSION=17.1.11

PKG_CHECK_MODULES(DALICORE, dali-core)
PKG_CHECK_MODULES(EXIF, libexif)
PKG_CHECK_MODULES(FREETYPE, [freetype2 >= $FREETYPE_REQUIRED])
PKG_CHECK_MODULES(FREETYPE_BITMAP_SUPPORT, [freetype2 >= $FREETYPE_BITMAP_SUPPORT_VERSION], [ freetype_bitmap_support=yes  ], [ freetype_bitmap_support=no ] )
PKG_CHECK_MODULES(FONTCONFIG, fontconfig)
PKG_CHECK_MODULES(PNG, libpng)
PKG_CHECK_MODULES(LIBEXIF, libexif)
PKG_CHECK_MODULES(LIBDRM, libdrm)
PKG_CHECK_MODULES(LIBCURL, libcurl)
PKG_CHECK_MODULES(HARFBUZZ, harfbuzz)
PKG_CHECK_MODULES(FRIBIDI, fribidi)
PKG_CHECK_MODULES(TTRACE,  ttrace, AC_DEFINE(ENABLE_TTRACE, 1, [ttrace available]),
                  [ AC_MSG_NOTICE([Tizen Trace not avaiable]) ]
                  )


# Currently, dali-adaptor always requires EFL on all platforms.
# (on Wayland & X11, on Tizen devices & Ubuntu PC).
enable_efl=yes
if test "x$enable_efl" = "xyes"; then
PKG_CHECK_MODULES(ECORE, ecore)
PKG_CHECK_MODULES(ECORE_IPC, ecore-ipc)
PKG_CHECK_MODULES(ECORE_IMF, [ecore-imf >= 1.13], [ecore_imf_1_13=yes], [ecore_imf_1_13=no])
PKG_CHECK_MODULES(ELEMENTARY, elementary)
# Check for EldBus.h in ECore
PKG_CHECK_MODULES(ELDBUS, eldbus, [ eldbus_available=yes ],  [ eldbus_available=no ] )
fi


DALI_ELDBUS_AVAILABLE=
if test "x$eldbus_available" = "xyes"; then
  DALI_ELDBUS_AVAILABLE=true
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DDALI_ELDBUS_AVAILABLE "
fi
AC_SUBST(DALI_ELDBUS_AVAILABLE)


PKG_CHECK_MODULES(TPKP_CURL, tpkp-curl, [ tpkp_curl_available=yes ], [ tpkp_curl_available=no ] )

if test "x$tpkp_curl_available" = "xyes"; then
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DTPK_CURL_ENABLED "
fi

PKG_CHECK_MODULES(UTILX, utilX, [ utilx_available=yes ], [ utilx_available=no ] )

DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DPLATFORM_TIZEN"

AC_ARG_ENABLE(exportall,
              [AC_HELP_STRING([--enable-exportall],
                              [enables the exporting of all the symbols in the library])],
              [enable_exportall=yes],
              [enable_exportall=no])

AC_ARG_ENABLE([debug],
              [AC_HELP_STRING([--enable-debug],
                              [Turns on debugging])],
              [enable_debug=$enableval],
              [enable_debug=no])

AC_ARG_ENABLE(shaderbincache,
              [AC_HELP_STRING([--enable-shaderbincache],
                              [enables shader binary cache])],
              [enable_shaderbincache=$enableval],
              [enable_shaderbincache=DISABLE])

AC_ARG_ENABLE(networklogging,
              [AC_HELP_STRING([--enable-networklogging],
                              [enables network for debug tool])],
              [enable_networklogging=$enableval],
              [enable_networklogging=no])


if test "x$enable_debug" = "xyes"; then
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DDEBUG_ENABLED"
fi

if test "x$enable_debug" = "xno" -a "x$enable_exportall" = "xno"; then
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -fvisibility=hidden -DHIDE_DALI_INTERNALS"
fi

if test "x$enable_shaderbincache" = "xENABLE"; then
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DSHADERBIN_CACHE_ENABLED"
fi

if test "x$enable_networklogging" = "xyes"; then
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DNETWORK_LOGGING_ENABLED"
fi

# If Ecore IMF version is greater than 1.13, then some structures are different
if test "x$ecore_imf_1_13" = "xyes"; then
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DECORE_IMF_1_13"
fi

AC_ARG_ENABLE([gles],
              [AC_HELP_STRING([--enable-gles],
                              [Specify the OpenGL ES version for backwards compatibility])],
              [enable_gles=$enableval],
              [enable_gles=30])

DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DDALI_GLES_VERSION=${enable_gles}"

# Currently, dali-adaptor always does not require libuv because it runs on ecore on all platforms.
# (on Wayland & X11, on Tizen devices & Ubuntu PC).
with_libuv=no

# Node.JS already has a libuv main loop running,so we have to integrate with it
AM_CONDITIONAL(LIB_UV_EVENT_LOOP, test x$with_libuv != xno)


build_for_libuv=no
if test "x$with_libuv" != "xno"; then
  AC_MSG_NOTICE("build with libuv mainloop (Node.JS support) == yes");
  [build_for_libuv=yes]
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DNODE_JS_SUPPORT  -I${with_libuv}"
else
 #not using libuv build
  AC_MSG_NOTICE("build with libuv mainloop == no (Node.JS not supported)");
fi

# Currently, dali-adaptor requires appfw on Tizen
# and does not require it on Ubuntu.
# So we should be able to enable/disable this option for dali-adaptor.
AC_ARG_ENABLE([appfw],
              [AC_HELP_STRING([--enable-appfw],
                              [Builds with Tizen App framework libraries, off by default])],
              [enable_appfw=$enableval],
              [enable_appfw=no])

# Option to allow building with Tizen SDK 2.2
AC_ARG_WITH([tizen-2-2-compatibility],
            [AC_HELP_STRING([--with-tizen-2-2-compatibility],
                            [Use Tizen SDK 2.2 compatibility])],
            [with_tizen_2_2_compatibility=$withval],
            [with_tizen_2_2_compatibility=no])

# Tizen Profile options
AC_ARG_ENABLE([profile],
              [AC_HELP_STRING([--enable-profile=COMMON,MOBILE,WEARABLE,TV,UBUNTU],
                            [Select the variant of tizen])],
              [enable_profile=$enableval],
              [enable_profile=COMMON])

# Ensure valid profile selected
if test "x$enable_profile" != "xCOMMON" -a "x$enable_profile" != "xMOBILE" -a "x$enable_profile" != "xWEARABLE" -a "x$enable_profile" != "xTV" -a "x$enable_profile" != "xUBUNTU"; then
  AC_MSG_ERROR([$enable_profile is an invalid profile])
fi

AC_ARG_ENABLE(wayland,
              [  --enable-wayland       Build on Wayland],
              enable_wayland=yes,
              enable_wayland=no)

DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DDALI_PROFILE_${enable_profile}"
DALI_PROFILE_CFLAGS=" -DDALI_PROFILE_${enable_profile}"
AM_CONDITIONAL([COMMON_PROFILE], [test x$enable_profile = xCOMMON])
AM_CONDITIONAL([MOBILE_PROFILE], [test x$enable_profile = xMOBILE])
AM_CONDITIONAL([WEARABLE_PROFILE], [test x$enable_profile = xWEARABLE])
AM_CONDITIONAL([TV_PROFILE], [test x$enable_profile = xTV])
AM_CONDITIONAL([UBUNTU_PROFILE], [test x$enable_profile = xUBUNTU])
AM_CONDITIONAL([WAYLAND], [test x$enable_wayland = xyes])
AM_CONDITIONAL([USE_EFL], [test x$enable_efl = xyes])
AM_CONDITIONAL([USE_APPFW], [test x$enable_appfw = xyes])

# Platforms with highp shader support can use vector based text
AM_CONDITIONAL([ENABLE_VECTOR_BASED_TEXT_RENDERING], [test x$enable_profile = xUBUNTU])

AM_CONDITIONAL([ENABLE_NETWORK_LOGGING], [test x$enable_networklogging = xyes])

# Platforms should either enable features or remove them, they
# should not disable features. This allows the developer to override
# features through the command line.

if test "x$enable_profile" = "xCOMMON"; then
PKG_CHECK_MODULES(OPENGLES20, glesv2 egl)
fi

if test "x$enable_profile" = "xMOBILE"; then
PKG_CHECK_MODULES(OPENGLES20, glesv2 egl)
enable_assimp=no
fi

if test "x$enable_profile" = "xLITE"; then
PKG_CHECK_MODULES(OPENGLES20, gles20)
fi

if test "x$enable_profile" = "xWEARABLE"; then
PKG_CHECK_MODULES(OPENGLES20, glesv2)
fi

if test "x$enable_profile" = "xTV"; then
PKG_CHECK_MODULES(OPENGLES20, glesv2)
fi

if test "x$enable_profile" = "xUBUNTU"; then
PKG_CHECK_MODULES(OPENGLES20, glesv2 egl)
else



PKG_CHECK_MODULES(DLOG, dlog)
PKG_CHECK_MODULES(TTS, tts)
PKG_CHECK_MODULES(VCONF, vconf)

if test "x$enable_efl" = "xyes"; then
if test "x$with_tizen_2_2_compatibility" = "xno"; then
PKG_CHECK_MODULES(CAPI_SYSTEM_INFO, capi-system-info)
PKG_CHECK_MODULES(CAPI_SYSTEM_SENSOR, capi-system-sensor, [ capi_system_sensor_support=yes ], [ capi_system_sensor_support=no ] )

if test "x$capi_system_sensor_support" = "xyes"; then
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DCAPI_SYSTEM_SENSOR_SUPPORT"
fi

fi
fi

fi # ubuntu profile test

if test "x$enable_appfw" = "xyes"; then
PKG_CHECK_MODULES(CAPI_APPFW_APPLICATION, capi-appfw-application)
PKG_CHECK_MODULES(CAPI_SYSTEM_SYSTEM_SETTINGS, capi-system-system-settings)
fi

# Using EFL api's for  WAYLAND AND X11 to run on ecore mainloop
if test "x$enable_efl" = "xyes"; then
if test "x$enable_wayland" = "xyes"; then
PKG_CHECK_MODULES(WAYLAND, [ecore-wayland egl wayland-egl wayland-client >= 1.2.0 xkbcommon libtbm],
                  [DALI_USE_ECORE_WAYLAND=1],
                  [DALI_USE_ECORE_WAYLAND=0])

else
PKG_CHECK_MODULES(ECORE_X, [ecore-x],
                  [DALI_USE_ECORE_X11=1],
                  [DALI_USE_ECORE_X11=0])
PKG_CHECK_MODULES(X11, [x11],
                  [DALI_USE_X11=1],
                  [DALI_USE_X11=0])
fi
fi
# Using Wayland API directly  ( main loop agnostic, typically for running on libuv)
#  wayland-extension-client include xdg-shell-client
if test "x$enable_efl" = "xno"; then
if test "x$enable_wayland" = "xyes"; then
PKG_CHECK_MODULES(WAYLAND, [ egl wayland-egl wayland-client >= 1.2.0 xkbcommon libtbm],
                  [DALI_USE_WAYLAND=1],
                  [DALI_USE_WAYLAND=0])
fi
fi

# remove this when we update common repos
# common profile currently does not have wayland extensions like xdg-shell
if test "x$enable_wayland" = "xyes"; then
if test "x$enable_profile" != "xCOMMON"; then
PKG_CHECK_MODULES(WAYLAND_EXTENSION, xdg-shell-client)
fi
fi
AM_CONDITIONAL([USE_ECORE_WAYLAND], [test "$DALI_USE_ECORE_WAYLAND" -eq 1])

if test x$DALI_DATA_RW_DIR != x; then
  dataReadWriteDir=$DALI_DATA_RW_DIR
else
  dataReadWriteDir=${prefix}/share/dali/
fi

if test x$DALI_DATA_RO_DIR != x; then
  dataReadOnlyDir=$DALI_DATA_RO_DIR
else
  dataReadOnlyDir=${prefix}/share/dali/
fi

if test x$FONT_CONFIGURATION_FILE != x; then
  fontConfigurationFile=$FONT_CONFIGURATION_FILE
fi

AC_SUBST(dataReadWriteDir)
AC_SUBST(dataReadOnlyDir)
AC_SUBST(DALI_ADAPTOR_CFLAGS)
AC_SUBST(DALI_PROFILE_CFLAGS)
AC_SUBST(fontConfigurationFile)

# Specify the include directory for development headers
#devincludepath=${includedir}/dali/internal
devincludepath=${includedir}
AC_SUBST(devincludepath)

AC_CONFIG_FILES([
 Makefile
 dali-adaptor.pc
 dali-adaptor-integration.pc
])

if test "x$freetype_bitmap_support" = "xyes"; then
DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DFREETYPE_BITMAP_SUPPORT"
fi

AC_OUTPUT


echo "
Configuration
-------------
  Prefix:                           $prefix
  Debug Build:                      $enable_debug
  Compile flags                     $DALI_ADAPTOR_CFLAGS
  Freetype bitmap support (Emoji):  $freetype_bitmap_support
  Profile:                          $enable_profile
  Data Dir (Read/Write):            $dataReadWriteDir
  Data Dir (Read Only):             $dataReadOnlyDir
  Tizen SDK 2.2 compatibility:      $with_tizen_2_2_compatibility
  EldBus:                           $eldbus_available
  Shader Binary Cache:              $enable_shaderbincache
  Using LibUV mainloop (Node.JS)    $build_for_libuv
  Ecore Version At Least 1.13.0     $ecore_imf_1_13
  Network logging enabled:          $enable_networklogging
  Font config file:                 $fontConfigurationFile
  Building with EFL Libraries       $enable_efl
  Using Tizen APP FW libraries      $enable_appfw
"
# optional output of node.js source path if we're building with libuv
if test "x$build_for_libuv" != "xno"; then
echo "  LibUV header path         $with_libuv"
fi
