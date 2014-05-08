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

# Build the Dali Adaptor + Evas Plugin library

tizen_evas_plugin_internal_src_dir = ../../../adaptors/tizen/internal/mobile
include ../../../adaptors/tizen/internal/mobile/application-file.list

tizen_evas_plugin_public_api_src_dir = ../../../adaptors/tizen/public-api
include ../../../adaptors/tizen/public-api/adaptor-framework/mobile/application-file.list

capi_public_api_src_dir = ../../../capi/dali/public-api
include ../../../capi/dali/public-api/file.list

lib_LTLIBRARIES = libdali-evas-plugin.la

libdali_evas_plugin_la_SOURCES = \
                     $(tizen_evas_plugin_public_api_src_files) \
                     $(tizen_evas_plugin_internal_src_files)

libdali_evas_plugin_la_DEPENDENCIES =

libdali_evas_plugin_includes = \
                      -I../../.. \
                      -I../../../platform-abstractions/ \
                      -I../../../platform-abstractions/slp \
                      -I../../../platform-abstractions/slp/resource-loader \
                      -I../../../adaptors/ \
                      -I../../../adaptors/tizen \
                      -I../../../capi/

daliDefaultFontCacheDir = ${dataReadOnlyDir}/glyphcache/
daliUserFontCacheDir = ${dataReadWriteDir}/glyphcache/
daliDefaultThemeDir  = ${dataReadWriteDir}/theme/

libdali_evas_plugin_la_CXXFLAGS = \
                      -DDALI_DATA_RW_DIR="\"${daliReadWriteDir}\"" \
                      -DDALI_DATA_RO_DIR="\"${daliReadOnlyDir}\"" \
                      -DDALI_DEFAULT_FONT_CACHE_DIR="\"${daliDefaultFontCacheDir}\"" \
                      -DDALI_USER_FONT_CACHE_DIR="\"${daliUserFontCacheDir}\"" \
                      -DDALI_DEFAULT_THEME_DIR="\"${daliDefaultThemeDir}\"" \
                      -DFONT_PRELOADED_PATH="\"${fontPreloadedPath}\"" \
                      -DFONT_DOWNLOADED_PATH="\"${fontDownloadedPath}\"" \
                      -DFONT_CONFIGURATION_FILE_PATH="\"${fontConfigurationFilePath}\"" \
                      -DNON_POWER_OF_TWO_TEXTURES \
                      -DDALI_COMPILATION -DDALI_ADAPTOR_COMPILATION \
                      -Werror -Wall \
                      $(libdali_evas_plugin_includes) \
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
                      $(EVAS_CFLAGS) \
                      $(ECORE_CFLAGS) \
                      $(DLOG_CFLAGS) \
                      $(XML_CFLAGS) \
                      $(VCONF_CFLAGS) \
                      $(EXIF_CFLAGS) \
                      $(MMFSOUND_CFLAGS) \
                      $(DEVICED_CFLAGS) \
                      $(TTS_CFLAGS)

libdali_evas_plugin_la_LIBADD = \
                      $(ELEMENTARY_LIBS) \
                      $(EVAS_LIBS)

tizenadaptordaliheaderdir = $(devincludepath)/dali/public-api/adaptor-framework
tizenadaptordaliheader_HEADERS = $(tizen_evas_plugin_public_api_header_files) \
                                 $(capi_evas_plugin_public_api_header_files)

# linking test

noinst_PROGRAMS = linker.test

linker_test_SOURCES = linker-test.cpp

linker_test_CXXFLAGS = \
  -I../../../adaptors/tizen \
  -I../../../capi \
  $(DALI_ADAPTOR_CFLAGS) \
  $(DALICORE_CFLAGS) \
  $(DALIX11_CFLAGS) \
  $(ELEMENTARY_CFLAGS) \
  $(EVAS_CFLAGS) \
  $(ECORE_CFLAGS) \
  -Werror -Wall

linker_test_DEPENDENCIES = libdali-evas-plugin.la
linker_test_LDADD = \
  $(DALICORE_LIBS) \
  libdali-evas-plugin.la \
  ../common/libdali-adaptor-common.la \
  -L${prefix}/lib
