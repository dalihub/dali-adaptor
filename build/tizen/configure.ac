#
# Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_MKDIR_P

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

LT_INIT

DALI_ADAPTOR_VERSION=dali_version
AC_SUBST(DALI_ADAPTOR_VERSION)

FREETYPE_REQUIRED=9.16.3

PKG_CHECK_MODULES(CAPI_APPFW_APPLICATION, capi-appfw-application)
PKG_CHECK_MODULES(DALICORE, dali-core)
PKG_CHECK_MODULES(DLOG, dlog)
PKG_CHECK_MODULES(ELEMENTARY, elementary)
PKG_CHECK_MODULES(ECORE, ecore)
PKG_CHECK_MODULES(ECORE_X, ecore-x)
PKG_CHECK_MODULES(ECORE_IPC, ecore-ipc)
PKG_CHECK_MODULES(EXIF, libexif)
PKG_CHECK_MODULES(FREETYPE, [freetype2 >= $FREETYPE_REQUIRED])
PKG_CHECK_MODULES(FEEDBACK, feedback)
PKG_CHECK_MODULES(FONTCONFIG, fontconfig)
PKG_CHECK_MODULES(MMFSOUND, mm-sound)
PKG_CHECK_MODULES(PNG, libpng)
PKG_CHECK_MODULES(SENSOR, sensor)
PKG_CHECK_MODULES(TTS, tts)
PKG_CHECK_MODULES(VCONF, vconf)
PKG_CHECK_MODULES(X11, x11)
PKG_CHECK_MODULES(XML, libxml-2.0)
PKG_CHECK_MODULES(LIBDRM, libdrm)
PKG_CHECK_MODULES(LIBEXIF, libexif)
PKG_CHECK_MODULES(CAPI_SYSTEM_SYSTEM_SETTINGS, capi-system-system-settings)

# Check for availability of BulletPhysics
PKG_CHECK_EXISTS(bullet, [
  BULLET_CFLAGS=`pkg-config --cflags bullet`
  BULLET_LIBS=`pkg-config --libs bullet`
  BULLET=yes
], [
  AC_MSG_WARN([BulletPhysics development files not found, not building bullet plugin])
  BULLET_CFLAGS=
  BULLET_LIBS=
  BULLET=no
])

AC_SUBST(BULLET_CFLAGS)
AC_SUBST(BULLET_LIBS)
AM_CONDITIONAL([USE_BULLET], [test x$BULLET = xyes])

DALI_ADAPTOR_CFLAGS=-DPLATFORM_SLP

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

if test "x$enable_debug" = "xyes"; then
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DDEBUG_ENABLED"
fi

if test "x$enable_debug" = "xno" -a "x$enable_exportall" = "xno"; then
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -fvisibility=hidden -DHIDE_DALI_INTERNALS"
fi

AC_ARG_ENABLE([gles],
              [AC_HELP_STRING([--enable-gles],
                              [Specify the OpenGL ES version for backwards compatibility])],
              [enable_gles=$enableval],
              [enable_gles=30])

DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DDALI_GLES_VERSION=${enable_gles}"

AC_ARG_WITH([jpeg-turbo],
            [AC_HELP_STRING([--with-jpeg-turbo],
                            [Use the JPEG Turbo library])],
            [with_jpeg_turbo=$withval],
            [with_jpeg_turbo=yes])

AM_CONDITIONAL(TURBO_JPEG_IS_ON, test $with_jpeg_turbo = yes)

# Tizen Profile options
AC_ARG_ENABLE([profile],
              [AC_HELP_STRING([--enable-profile=COMMON,MOBILE,TV],
                            [Select the variant of tizen])],
              [dali_profile=$enableval],
              [dali_profile=COMMON])

if test "x$enable_profile" = "xyes"; then
  dali_profile=COMMON
fi
if test "x$enable_profile" = "x"; then
  dali_profile=COMMON
fi

DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DDALI_PROFILE_${dali_profile}"
DALI_PROFILE_CFLAGS=" -DDALI_PROFILE_${dali_profile}"
AM_CONDITIONAL([COMMON_PROFILE], [test x$dali_profile = xCOMMON])
AM_CONDITIONAL([MOBILE_PROFILE], [test x$dali_profile = xMOBILE])
AM_CONDITIONAL([TV_PROFILE], [test x$dali_profile = xTV])

if test "x$enable_profile" = "xMOBILE"; then
PKG_CHECK_MODULES(OPENGLES20, opengl-es-20)
PKG_CHECK_MODULES(ASSIMP, assimp)
PKG_CHECK_MODULES(DEVICED, deviced)
PKG_CHECK_MODULES(EVAS, evas)
PKG_CHECK_MODULES(NATIVE_BUFFER, native-buffer)
PKG_CHECK_MODULES(NATIVE_BUFFER_POOL, native-buffer-pool)
fi

if test "x$enable_profile" = "xTV"; then
PKG_CHECK_MODULES(OPENGLES20, gles20)
PKG_CHECK_MODULES(ASSIMP, assimp)
PKG_CHECK_MODULES(HAPTIC, haptic)
fi

if test "x$enable_profile" = "xCOMMON"; then
PKG_CHECK_MODULES(OPENGLES20, gles20)
PKG_CHECK_MODULES(HAPTIC, haptic)
fi

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

if test x$FONT_PRELOADED_PATH != x; then
  fontPreloadedPath=$FONT_PRELOADED_PATH
else
  fontPreloadedPath=/usr/share/fonts/
fi

if test x$FONT_DOWNLOADED_PATH != x; then
  fontDownloadedPath=$FONT_DOWNLOADED_PATH
else
  fontDownloadedPath=/opt/share/fonts/
fi

if test x$FONT_APPLICATION_PATH != x; then
  fontApplicationPath=$FONT_APPLICATION_PATH
else
  fontApplicationPath=~/.fonts/
fi

if test x$FONT_CONFIGURATION_FILE != x; then
  fontConfigurationFilePath=$FONT_CONFIGURATION_FILE
fi

AC_SUBST(dataReadWriteDir)
AC_SUBST(dataReadOnlyDir)
AC_SUBST(fontPreloadedPath)
AC_SUBST(fontDownloadedPath)
AC_SUBST(fontApplicationPath)
AC_SUBST(fontConfigurationFilePath)
AC_SUBST(DALI_ADAPTOR_CFLAGS)
AC_SUBST(DALI_PROFILE_CFLAGS)

# Specify the include directory for development headers
#devincludepath=${includedir}/dali/internal
devincludepath=${includedir}
AC_SUBST(devincludepath)


AC_CONFIG_FILES([
 Makefile
 common/Makefile
 dali-adaptor/Makefile
 dali.pc
 dali-application/Makefile
 dali-application.pc
 dali-evas/Makefile
 dali-evas.pc
 dali-native-buffer/Makefile
 dali-native-buffer.pc
 plugins/Makefile
])

AC_OUTPUT

echo "
Configuration
-------------
  Prefix:                           $prefix
  Debug Build:                      $enable_debug
  Compile flags                     $DALI_ADAPTOR_CFLAGS
  Using JPEG Turbo Library:         $with_jpeg_turbo
  Profile:                          $dali_profile
  Data Dir (Read/Write):            $dataReadWriteDir
  Data Dir (Read Only):             $dataReadOnlyDir
  Font Path (Preloaded):            $fontPreloadedPath
  Font Path (Downloaded):           $fontDownloadedPath
  Font Path (Application):          $fontApplicationPath
  Font Configuration File:          $fontConfigurationFilePath
"
