#
# Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
AM_PROG_AR

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

LT_INIT

DALI_ADAPTOR_VERSION=dali_version
AC_SUBST(DALI_ADAPTOR_VERSION)

AM_CONDITIONAL([COMMON_PROFILE], [test x$enable_profile = xCOMMON])
AM_CONDITIONAL([MOBILE_PROFILE], [test x$enable_profile = xMOBILE])
AM_CONDITIONAL([WEARABLE_PROFILE], [test x$enable_profile = xWEARABLE])
AM_CONDITIONAL([TV_PROFILE], [test x$enable_profile = xTV])
AM_CONDITIONAL([WAYLAND], [test x$enable_wayland = xyes -o x$enable_ecore_wayland2 = xyes ] )

AC_ARG_ENABLE([debug],
              [AC_HELP_STRING([--enable-debug],
                              [Turns on debugging])],
              [enable_debug=$enableval],
              [enable_debug=no])

if test "x$enable_debug" = "xyes"; then
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DDEBUG_ENABLED"
fi

if test "x$enable_trace" = "xyes"; then
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DTRACE_ENABLED"
fi

if test "x$enable_debug" = "xno" -a "x$enable_exportall" = "xno"; then
  DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -fvisibility=hidden -DHIDE_DALI_INTERNALS"
fi

AC_ARG_ENABLE([gles],
              [AC_HELP_STRING([--enable-gles],
                              [Specify the OpenGL ES version for backwards compatibility])],
              [enable_gles=$enableval],
              [enable_gles=30])

AC_ARG_ENABLE([vulkan],
              [AC_HELP_STRING([--enable-vulkan],
                              [Enable Vulkan support])],
              [enable_vulkan=$enableval],
              [enable_vulkan=no])

DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DDALI_GLES_VERSION=${enable_gles}"

AC_ARG_ENABLE(wayland,
              [  --enable-wayland       Build on Wayland],
              enable_wayland=yes,
              enable_wayland=no)

AC_ARG_ENABLE(ecore_wayland2,
              [  --enable-ecore-wayland2  Build on Ecore Wayland2],
              enable_ecore_wayland2=yes,
              enable_ecore_wayland2=no)

AC_ARG_ENABLE([cxx03_abi],
              [AC_HELP_STRING([--enable-cxx03-abi],
                              [Specify abi for the build])],
              [enable_cxx03_abi=$enableval],
              [enable_cxx03_abi=no])

AC_ARG_ENABLE([rename_so],
              [AC_HELP_STRING([--enable-rename-so],
                              [Specify whether so file is renamed or not])],
              [enable_rename_so=$enableval],
              [enable_rename_so=yes])

DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -DDALI_PROFILE_${enable_profile}"
DALI_PROFILE_CFLAGS=" -DDALI_PROFILE_${enable_profile}"

AM_CONDITIONAL([ENABLE_CXX03_ABI], [test x$enable_cxx03_abi = xyes])
AM_CONDITIONAL([ENABLE_RENAME_SO], [test x$enable_rename_so = xyes])


AS_IF([test "x$enable_cxx03_abi" = "xyes"],
            [PKG_CHECK_MODULES(DALI_CORE, dali-core-vk-cxx03)],
            [PKG_CHECK_MODULES(DALI_CORE, dali-core-vk)]
)

if test "x$enable_cxx03_abi" = "xyes"; then
 DALI_ADAPTOR_CFLAGS="$DALI_ADAPTOR_CFLAGS -D_GLIBCXX_USE_CXX11_ABI=0"
fi

if test x$TIZEN_PLATFORM_CONFIG_SUPPORTED != x; then
   PKG_CHECK_MODULES(DLOG, dlog)
  tizenPlatformConfigSupported=$TIZEN_PLATFORM_CONFIG_SUPPORTED
else
  tizenPlatformConfigSupported=0
fi

if test "x$enable_wayland" = "xyes"; then
if test "x$enable_ecore_wayland2" = "xyes"; then
PKG_CHECK_MODULES(WAYLAND, [ecore-wl2 egl wayland-egl wayland-egl-tizen wayland-client >= 1.2.0 xkbcommon libtbm])
else
PKG_CHECK_MODULES(WAYLAND, [ecore-wayland egl wayland-egl wayland-client >= 1.2.0 xkbcommon libtbm])
fi
else
PKG_CHECK_MODULES(ECORE_X, [ecore-x x11-xcb],
                  [DALI_USE_ECORE_X11=1],
                  [DALI_USE_ECORE_X11=0])
PKG_CHECK_MODULES(X11, [x11 x11-xcb],
                  [DALI_USE_X11=1],
                  [DALI_USE_X11=0])
fi


# vulkan support, includes xlib platform dependency
if test "x$enable_vulkan" != "xno"; then
    PKG_CHECK_MODULES(VULKAN, vulkan, AC_DEFINE(VULKAN_ENABLED, 1, [Vulkan available as package]),
                  [ AC_MSG_ERROR([Vulkan not available as a package]) ]
                  )

    if test "x$enable_wayland" = "xyes"; then
        DALI_ADAPTOR_CFLAGS+=" ${VULKAN_CFLAGS} -DVULKAN_HPP_NO_EXCEPTIONS -DVULKAN_WITH_WAYLAND"
        DALI_ADAPTOR_CXXFLAGS+=" -DVULKAN_WITH_WAYLAND "
        DALI_ADAPTOR_LDFLAGS+=${VULKAN_LIBS}
    else
        DALI_ADAPTOR_CFLAGS+=" ${VULKAN_CFLAGS} -DVULKAN_HPP_NO_EXCEPTIONS"

        # due to a bug in nvidia driver x11-xcb library is required
        DALI_ADAPTOR_LDFLAGS+=${VULKAN_LIBS}

        if test "x$DALI_HAS_X11_XCB" == "x1"; then
            echo "X11 libs: ${X11_XCB_LIBS}"
            DALI_ADAPTOR_LDFLAGS+=${X11_XCB_LIBS}
        fi
    fi
    AC_SUBST(VULKAN_SDK)
    AC_SUBST(VULKAN_LIBS)
    AC_SUBST(VULKAN_CFLAGS)
fi

AC_SUBST(DALI_CORE_CFLAGS)
AC_SUBST(DALI_CORE_LDFLAGS)
AC_SUBST(DALI_CORE_LIBS)

AC_SUBST(DALI_ADAPTOR_CFLAGS)
AC_SUBST(DALI_ADAPTOR_LDFLAGS)
AC_SUBST(DALI_ADAPTOR_LIBS)

AM_CONDITIONAL([VULKAN_ENABLED], [test x$enable_vulkan != xno])

# Specify the include directory for development headers
#devincludepath=${includedir}/dali/internal
devincludepath=${includedir}
AC_SUBST(devincludepath)

AC_CONFIG_FILES([Makefile])

# Note, Only useful as a separately configured library. Doesn't help
# for a library built in the same configuration...
if test "x$enable_cxx03_abi" = "xyes"; then
AC_CONFIG_FILES([dali-graphics-vk-cxx03.pc])
else
AC_CONFIG_FILES([dali-graphics-vk.pc])
fi

AC_OUTPUT


echo "
Configuration
-------------
  Prefix:                           $prefix
  Debug Build:                      $enable_debug
  Trace Build:                      $enable_trace
  Compile flags:                    $DALI_CORE_CFLAGS $DALI_ADAPTOR_CFLAGS
  Profile:                          $enable_profile
  Vulkan Support:                   ${enable_vulkan}"

if test "x$VULKAN_SDK" != "x"; then
    echo "  VulkanSDK:                        ${VULKAN_SDK}"
fi
echo ""
