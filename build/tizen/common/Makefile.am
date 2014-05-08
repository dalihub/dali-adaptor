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

# Build the Dali Adaptor common intermediate library

# Base Adaptor
base_adaptor_src_dir = ../../../adaptors/base
include ../../../adaptors/base/file.list

# Platform Abstraction
slp_platform_abstraction_src_dir = ../../../platform-abstractions/slp
include ../../../platform-abstractions/slp/file.list

# Internal Common
tizen_adaptor_internal_src_dir = ../../../adaptors/tizen/internal/common
include ../../../adaptors/tizen/internal/common/file.list

if COMMON_PROFILE
tizen_adaptor_internal_common_src_files += $(tizen_adaptor_internal_common_profile_src_files)
slp_platform_abstraction_src_files += $(slp_assimp_stubs_src_files)
endif

if MOBILE_PROFILE
slp_platform_abstraction_src_files += $(slp_assimp_src_files)
include ../../../adaptors/tizen/internal/mobile/file.list
endif

if TV_PROFILE
slp_platform_abstraction_src_files += $(slp_assimp_src_files)
include ../../../adaptors/tizen/internal/tv/file.list
endif

# Public API
tizen_adaptor_public_api_src_dir = ../../../adaptors/tizen/public-api
include ../../../adaptors/tizen/public-api/file.list

# CAPI API
capi_src_dir = ../../../capi/dali/public-api
include ../../../capi/dali/public-api/file.list

# Plugins
plugin_themes_dir = ../../../adaptors/tizen/internal/common/feedback
dali_plugin_theme_files = $(plugin_themes_dir)/default-feedback-theme.json

dalifeedbackthemedir = ${dataReadOnlyDir}/themes/feedback-themes/
dalifeedbacktheme_DATA = ${dali_plugin_theme_files}

if TURBO_JPEG_IS_ON
  slp_platform_abstraction_src_files += $(slp_platform_abstraction_src_dir)/resource-loader/loader-jpeg-turbo.cpp
else
  slp_platform_abstraction_src_files += $(slp_platform_abstraction_src_dir)/resource-loader/loader-jpeg.cpp
endif

lib_LTLIBRARIES = libdali-adaptor-common.la

libdali_adaptor_common_la_SOURCES = \
                     $(base_adaptor_src_files) \
                     $(slp_platform_abstraction_src_files) \
                     $(tizen_adaptor_public_api_src_files) \
                     $(tizen_adaptor_internal_common_src_files)

libdali_adaptor_common_la_DEPENDENCIES =

libdali_adaptor_common_la_includes = \
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

libdali_adaptor_common_la_CXXFLAGS = \
                      -DDALI_DATA_RW_DIR="\"${daliReadWriteDir}\"" \
                      -DDALI_DATA_RO_DIR="\"${daliReadOnlyDir}\"" \
                      -DDALI_DEFAULT_FONT_CACHE_DIR="\"${daliDefaultFontCacheDir}\"" \
                      -DDALI_USER_FONT_CACHE_DIR="\"${daliUserFontCacheDir}\"" \
                      -DDALI_DEFAULT_THEME_DIR="\"${daliDefaultThemeDir}\"" \
                      -DFONT_PRELOADED_PATH="\"${fontPreloadedPath}\"" \
                      -DFONT_DOWNLOADED_PATH="\"${fontDownloadedPath}\"" \
                      -DFONT_APPLICATION_PATH="\"${fontApplicationPath}\"" \
                      -DFONT_CONFIGURATION_FILE_PATH="\"${fontConfigurationFilePath}\"" \
                      -DDALI_FEEDBACK_THEME_DIR="\"${dalifeedbackthemedir}\"" \
                      -DNON_POWER_OF_TWO_TEXTURES \
                      -DDALI_COMPILATION -DDALI_ADAPTOR_COMPILATION \
                      -Werror -Wall -lgcc \
                      $(libdali_adaptor_common_la_includes) \
                      $(DALI_ADAPTOR_CFLAGS) \
                      $(DALICORE_CFLAGS) \
                      $(OPENGLES20_CFLAGS) \
                      $(X11_CFLAGS) \
                      $(FREETYPE_CFLAGS) \
                      $(FONTCONFIG_CFLAGS) \
                      $(PNG_CFLAGS) \
                      $(CAPI_APPFW_APPLICATION_CFLAGS) \
                      $(ELEMENTARY_CFLAGS) \
                      $(EVAS_CFLAGS) \
                      $(ECORE_CFLAGS) \
                      $(ECORE_IPC_CFLAGS) \
                      $(DLOG_CFLAGS) \
                      $(XML_CFLAGS) \
                      $(VCONF_CFLAGS) \
                      $(EXIF_CFLAGS) \
                      $(MMFSOUND_CFLAGS) \
                      $(TTS_CFLAGS) \
                      $(SENSOR_CFLAGS) \
                      $(LIBDRM_CFLAGS) \
                      $(LIBEXIF_CFLAGS) \
                      $(CAPI_SYSTEM_SYSTEM_SETTINGS_CFLAGS)

libdali_adaptor_common_la_LIBADD = \
                      $(DALICORE_LIBS) \
                      $(X11_LIBS) \
                      $(OPENGLES20_LIBS) \
                      $(FREETYPE_LIBS) \
                      $(FONTCONFIG_LIBS) \
                      $(PNG_LIBS) \
                      $(ELEMENTARY_LIBS) \
                      $(ECORE_X_LIBS) \
                      $(ECORE_IPC_LIBS) \
                      $(DLOG_LIBS) \
                      $(XML_LIBS) \
                      $(VCONF_LIBS) \
                      $(EXIF_LIBS) \
                      $(TTS_LIBS) \
                      $(SENSOR_LIBS) \
                      $(LIBDRM_LIBS) \
                      $(LIBEXIF_LIBS) \
                      $(CAPI_SYSTEM_SYSTEM_SETTINGS_LIBS) \
                      -lgif \
                      -lboost_thread

if MOBILE_PROFILE
libdali_adaptor_common_la_CXXFLAGS += $(ASSIMP_CFLAGS) \
                      $(DEVICED_CFLAGS) \
                      $(NATIVE_BUFFER_CFLAGS) \
                      $(NATIVE_BUFFER_POOL_CFLAGS)

libdali_adaptor_common_la_LIBADD += $(ASSIMP_LIBS) \
                      $(NATIVE_BUFFER_LIBS) \
                      $(NATIVE_BUFFER_POOL_LIBS)
endif

if TV_PROFILE
libdali_adaptor_common_la_CXXFLAGS += $(ASSIMP_CFLAGS) \
                                      $(HAPTIC_CFLAGS)

libdali_adaptor_common_la_LIBADD += $(ASSIMP_LIBS)
endif

if COMMON_PROFILE
libdali_adaptor_common_la_CXXFLAGS += $(HAPTIC_CFLAGS)
endif

if TURBO_JPEG_IS_ON
libdali_adaptor_common_la_LIBADD += -lturbojpeg
CFLAGS += -D_TURBO_JPEG_LOADER
else
libdali_adaptor_common_la_LIBADD += -ljpeg
endif

tizenadaptorcommonpublicapidir = $(devincludepath)/dali/public-api/adaptor-framework/common
tizenadaptorcommonpublicapi_HEADERS = $(tizen_adaptor_public_api_common_header_files)

tizenadaptordaliheaderdir = $(devincludepath)/dali
tizenadaptordaliheader_HEADERS = $(tizen_adaptor_dali_header_file)

capicommonpublicapidir = $(devincludepath)/dali/public-api/adaptor-framework/common
capicommonpublicapi_HEADERS = $(capi_adaptor_framework_common_header_files)

capiheaderdir = $(devincludepath)/dali/public-api/
capiheader_HEADERS = $(capi_header_file)
