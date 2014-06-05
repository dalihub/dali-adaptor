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

tizen_application_internal_src_dir = ../../../adaptors/tizen/internal
include ../../../adaptors/tizen/internal/application-file.list

tizen_application_public_api_src_dir = ../../../adaptors/tizen/public-api
include ../../../adaptors/tizen/public-api/adaptor-framework/application-file.list

if MOBILE_PROFILE
tizen_evas_plugin_internal_src_dir = ../../../adaptors/tizen/internal/mobile
tizen_native_buffer_plugin_internal_src_dir = ../../../adaptors/tizen/internal/mobile
include ../../../adaptors/tizen/internal/mobile/application-file.list

tizen_evas_plugin_public_api_src_dir = ../../../adaptors/tizen/public-api
tizen_native_buffer_plugin_public_api_src_dir = ../../../adaptors/tizen/public-api
include ../../../adaptors/tizen/public-api/adaptor-framework/mobile/application-file.list
endif

capi_public_api_src_dir = ../../../capi/dali/public-api
include ../../../capi/dali/public-api/file.list

lib_LTLIBRARIES = libdali-adaptor.la

libdali_adaptor_la_SOURCES = $(tizen_application_public_api_src_files)

if MOBILE_PROFILE
libdali_adaptor_la_SOURCES += $(tizen_evas_plugin_public_api_src_files) \
                              $(tizen_native_buffer_plugin_public_api_src_files)
endif

libdali_adaptor_la_SOURCES += $(tizen_application_internal_src_files)

if MOBILE_PROFILE
libdali_adaptor_la_SOURCES += $(tizen_evas_plugin_internal_src_files) \
                              $(tizen_native_buffer_plugin_internal_src_files)
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
                      -I../../../adaptors/ \
                      -I../../../adaptors/tizen \
                      -I../../../capi

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
                      $(DLOG_CFLAGS) \
                      $(XML_CFLAGS) \
                      $(VCONF_CFLAGS) \
                      $(EXIF_CFLAGS) \
                      $(TTS_CFLAGS) \
                      $(LIVEBOX_CLIENT_CFLAGS) \
                      $(LIVEBOX_SERVICE_CFLAGS)

libdali_adaptor_la_LIBADD = \
                      $(CAPI_APPFW_APPLICATION_LIBS) \
                      $(LIVEBOX_CLIENT_LIBS) \
                      $(LIVEBOX_SERVICE_LIBS)

if MOBILE_PROFILE
libdali_adaptor_la_CXXFLAGS += \
                      $(NATIVE_BUFFER_CFLAGS) \
                      $(NATIVE_BUFFER_POOL_CFLAGS)

libdali_adaptor_la_LIBADD += \
                      $(NATIVE_BUFFER_LIBS) \
                      $(NATIVE_BUFFER_POOL_LIBS)
endif



install-data-local:
	$(MKDIR_P) ${DESTDIR}/${daliUserFontCacheDir}

# Install resource log analyzer script
bin_SCRIPTS = ../../../adaptors/tizen/scripts/dalireslog.sh

# Install headers

tizenadaptorpublicapidir = $(devincludepath)/dali/public-api/adaptor-framework
tizenadaptorpublicapi_HEADERS = $(tizen_application_public_api_header_files)

# linking test

noinst_PROGRAMS = linker.test

linker_test_SOURCES = linker-test.cpp

linker_test_CXXFLAGS = \
  -I../../../adaptors/tizen \
  -I../../../capi \
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
