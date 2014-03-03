#
# Copyright (c) 2014 Samsung Electronics Co., Ltd.
#
# Licensed under the Flora License, Version 1.0 (the License);
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://floralicense.org/license/
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Builds the Dali Adaptor + all applications library

tizen_adaptor_internal_src_dir = ../../../adaptors/tizen/internal
include ../../../adaptors/tizen/internal/application-file.list

tizen_adaptor_public_api_src_dir = ../../../adaptors/tizen/public-api
include ../../../adaptors/tizen/public-api/adaptor-framework/application-file.list

capi_public_api_src_dir = ../../../capi/dali/public-api
include ../../../capi/dali/public-api/file.list

lib_LTLIBRARIES = libdali-adaptor.la

libdali_adaptor_la_SOURCES = \
                     $(tizen_adaptor_public_api_src_files) \
                     $(tizen_adaptor_internal_src_files)

libdali_adaptor_la_DEPENDENCIES =

libdali_adaptor_includes = \
                      -I../../.. \
                      -I../../../platform-abstractions/ \
                      -I../../../platform-abstractions/slp \
                      -I../../../platform-abstractions/slp/resource-loader \
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
                      $(ASSIMP_CFLAGS) \
                      $(CAPI_APPFW_APPLICATION_CFLAGS) \
                      $(ELEMENTARY_CFLAGS) \
                      $(DLOG_CFLAGS) \
                      $(XML_CFLAGS) \
                      $(VCONF_CFLAGS) \
                      $(EXIF_CFLAGS) \
                      $(TTS_CFLAGS) \
                      $(LIVEBOX_CLIENT_CFLAGS) \
                      $(LIVEBOX_SERVICE_CFLAGS) \
                      $(NATIVE_BUFFER_CFLAGS) \
                      $(NATIVE_BUFFER_POOL_CFLAGS)

libdali_adaptor_la_LIBADD = \
                      $(CAPI_APPFW_APPLICATION_LIBS) \
                      $(LIVEBOX_CLIENT_LIBS) \
                      $(LIVEBOX_SERVICE_LIBS) \
                      $(NATIVE_BUFFER_LIBS) \
                      $(NATIVE_BUFFER_POOL_LIBS)

install-data-local:
	$(MKDIR_P) ${DESTDIR}/${daliUserFontCacheDir}

# Install resource log analyzer script
bin_SCRIPTS = ../../../adaptors/tizen/scripts/dalireslog.sh

# Install headers

tizenadaptorpublicapidir = $(devincludepath)/dali/public-api/adaptor-framework
tizenadaptorpublicapi_HEADERS = $(tizen_adaptor_public_api_header_files)


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
