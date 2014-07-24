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

# Builds the Dali Adaptor + all applications library

# Common
adaptor_common_dir = ../../../adaptors/common
include ../../../adaptors/common/file.list

# Public API
adaptor_public_api_dir = ../../../adaptors/public-api
include ../../../adaptors/public-api/file.list

if WAYLAND
adaptor_wayland_dir = ../../../adaptors/wayland
include ../../../adaptors/wayland/file.list
else
adaptor_x11_dir = ../../../adaptors/x11
include ../../../adaptors/x11/file.list
endif

lib_LTLIBRARIES = libdali-adaptor.la

libdali_adaptor_la_SOURCES = $(adaptor_application_src_files)

libdali_adaptor_la_SOURCES += $(adaptor_application_internal_src_files)

if WAYLAND
libdali_adaptor_la_SOURCES += $(adaptor_wayland_application_src_files)
else
libdali_adaptor_la_SOURCES += $(adaptor_x11_application_src_files)
endif

libdali_adaptor_la_DEPENDENCIES =

# List include directories with more platform-specific (slp) before portable
# so platform-specific files can override portable versions:
libdali_adaptor_includes = \
                      -I../../.. \
                      -I../../../platform-abstractions/ \
                      -I../../../platform-abstractions/slp \
                      -I../../../platform-abstractions/slp/resource-loader \
                      -I../../../platform-abstractions/portable \
                      -I../../../adaptors/public-api \
                      -I../../../adaptors/common \
                      -I../../../adaptors/ 

if WAYLAND
libdali_adaptor_includes += \
                      -I../../../adaptors/wayland
else
libdali_adaptor_includes += \
                      -I../../../adaptors/x11
endif

daliUserFontCacheDir = ${dataReadWriteDir}/glyphcache/
daliDefaultThemeDir  = ${dataReadWriteDir}/theme/

libdali_adaptor_la_CXXFLAGS = \
                      -DDALI_DATA_RW_DIR="\"${daliReadWriteDir}\"" \
                      -DDALI_DATA_RO_DIR="\"${daliReadOnlyDir}\"" \
                      -DDALI_USER_FONT_CACHE_DIR="\"${daliUserFontCacheDir}\"" \
                      -DDALI_DEFAULT_THEME_DIR="\"${daliDefaultThemeDir}\"" \
                      -DFONT_PRELOADED_PATH="\"${fontPreloadedPath}\"" \
                      -DFONT_DOWNLOADED_PATH="\"${fontDownloadedPath}\"" \
                      -DFONT_CONFIGURATION_FILE_PATH="\"${fontConfigurationFilePath}\"" \
                      -DNON_POWER_OF_TWO_TEXTURES \
                      -DDALI_COMPILATION -DDALI_ADAPTOR_COMPILATION \
                      -Werror -Wall \
                      $(libdali_adaptor_includes) \
                      $(DALI_ADAPTOR_CFLAGS) \
                      $(DALICORE_CFLAGS) \
                      $(OPENGLES20_CFLAGS) \
                      $(X11_CFLAGS) \
                      $(FREETYPE_CFLAGS) \
                      $(FONTCONFIG_CFLAGS) \
                      $(PNG_CFLAGS) \
                      $(CAPI_APPFW_APPLICATION_CFLAGS) \
                      $(ELEMENTARY_CFLAGS) \
                      $(ECORE_IPC_CFLAGS) \
                      $(DLOG_CFLAGS) \
                      $(XML_CFLAGS) \
                      $(VCONF_CFLAGS) \
                      $(EXIF_CFLAGS) \
                      $(TTS_CFLAGS)

libdali_adaptor_la_LIBADD = \
                      $(CAPI_APPFW_APPLICATION_LIBS)

install-data-local:
	$(MKDIR_P) ${DESTDIR}/${daliUserFontCacheDir}

# Install resource log analyzer script
bin_SCRIPTS = ../../../adaptors/scripts/dalireslog.sh

# Install headers

tizenadaptorpublicapidir = $(devincludepath)/dali/public-api/
tizenadaptorpublicapi_HEADERS = $(public_api_application_header_files)

# linking test

noinst_PROGRAMS = linker.test

linker_test_SOURCES = linker-test.cpp

linker_test_CXXFLAGS = \
  -I../../../adaptors/common \
  -I../../../adaptors/public-api \
  $(DALI_ADAPTOR_CFLAGS) \
  $(DALICORE_CFLAGS) \
  $(DALIX11_CFLAGS) \
  -Werror -Wall

linker_test_DEPENDENCIES = libdali-adaptor.la
linker_test_LDADD = \
  $(DALICORE_LIBS) \
  libdali-adaptor.la \
  ../common/libdali-adaptor-common.la \
  -L${prefix}/lib
