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

AC_ARG_WITH([libuv],
              [AC_HELP_STRING([--with-libuv],
                              [Path that contains libuv headers. Setting this configures DALi to work with LibUV mainloop used in Node.JS.
                              For example /usr/tmp/downloads/node/deps/uv/include/ ])],
              [with_libuv=$withval],
              [with_libuv=no])

AC_ARG_ENABLE([feedback],
              [AC_HELP_STRING([ --enable-feedback],
                              [Enable feedback plugin])],
              [enable_feedback=yes],
              [enable_feedback=no])

AC_ARG_ENABLE([videoplayer],
              [AC_HELP_STRING([ --enable-videoplayer],
                              [Enable video player plugin])],
              [enable_videoplayer=yes],
              [enable_videoplayer=no])

AC_CONFIG_SUBDIRS(adaptor)
if test "x$with_libuv" != "xno"; then
  # build dali-adaptor & dali-adaptor-uv
  AC_CONFIG_SUBDIRS(adaptor-uv)
fi

if test "x$enable_feedback" = "xyes" || test "x$enable_videoplayer" = "xyes"; then
  # build dali-adaptor & dali-adaptor-uv & plugins
  AC_CONFIG_SUBDIRS(plugins)
fi
#else
  # build dali-adaptor only

AC_CONFIG_FILES(Makefile)

AC_OUTPUT
