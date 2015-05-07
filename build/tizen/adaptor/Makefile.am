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

# Build the Dali Adaptor library


############# INCLUDE FILE LISTS #############

# Base Adaptor
base_adaptor_src_dir = ../../../adaptors/base
include ../../../adaptors/base/file.list

# Platform Abstraction
tizen_platform_abstraction_src_dir = ../../../platform-abstractions/tizen
portable_platform_abstraction_src_dir = ../../../platform-abstractions/portable
include ../../../platform-abstractions/tizen/file.list

# Text Abstraction
text_src_dir = ../../../text
include ../../../text/file.list

# Integration
adaptor_integration_api_dir = ../../../adaptors/integration-api
include ../../../adaptors/integration-api/file.list

# Internal Common
adaptor_common_dir = ../../../adaptors/common
include ../../../adaptors/common/file.list

# Wayland
if WAYLAND
adaptor_wayland_dir = ../../../adaptors/wayland
include ../../../adaptors/wayland/file.list
else
# X11
adaptor_x11_dir = ../../../adaptors/x11
include ../../../adaptors/x11/file.list
endif

# Ubuntu
if UBUNTU_PROFILE
adaptor_ubuntu_dir = ../../../adaptors/ubuntu
include ../../../adaptors/ubuntu/file.list
else
# Tizen
adaptor_tizen_dir = ../../../adaptors/tizen
include ../../../adaptors/tizen/file.list
endif

# Mobile
adaptor_mobile_dir = ../../../adaptors/mobile
include ../../../adaptors/mobile/file.list

# TV
if TV_PROFILE
adaptor_tv_dir = ../../../adaptors/tv
include ../../../adaptors/tv/file.list
endif

# Public API
adaptor_public_api_dir = ../../../adaptors/public-api
include ../../../adaptors/public-api/file.list

# Devel API ( for use by Toolkit)
adaptor_devel_api_dir = ../../../adaptors/devel-api
include ../../../adaptors/devel-api/file.list


# Static libraries
static_libraries_libunibreak_src_dir = ../../../text/dali/internal/libunibreak
include ../../../text/dali/internal/libunibreak/file.list

############# source files #############

if ASSIMP_ENABLED
tizen_platform_abstraction_src_files += $(tizen_assimp_src_files)
else
tizen_platform_abstraction_src_files += $(tizen_assimp_stubs_src_files)
endif # ASSIMP_ENABLED

# COMMON
if COMMON_PROFILE

adaptor_internal_src_files = $(adaptor_common_internal_src_files) \
                             $(adaptor_common_internal_default_profile_src_files) \
                             $(adaptor_tizen_internal_src_files) \
                             $(static_libraries_libunibreak_src_files)

if WAYLAND
adaptor_internal_src_files += $(adaptor_wayland_tizen_internal_src_files) \
                              $(adaptor_wayland_internal_default_profile_src_files)
else
adaptor_internal_src_files += $(adaptor_x11_tizen_internal_src_files) \
                              $(adaptor_x11_internal_default_profile_src_files)
endif # WAYLAND
endif # COMMON_PROFILE


# UBUNTU
if UBUNTU_PROFILE

adaptor_internal_src_files = $(adaptor_common_internal_src_files) \
                             $(adaptor_common_internal_default_profile_src_files) \
                             $(adaptor_ubuntu_internal_src_files) \
                             $(adaptor_x11_ubuntu_internal_src_files) \
                             $(adaptor_x11_internal_default_profile_src_files) \
                             $(static_libraries_libunibreak_src_files)

endif # UBUNTU_PROFILE


# MOBILE
if MOBILE_PROFILE

adaptor_internal_src_files = $(adaptor_common_internal_src_files) \
                             $(adaptor_common_internal_mobile_profile_src_files) \
                             $(adaptor_tizen_internal_src_files) \
                             $(static_libraries_libunibreak_src_files)

if WAYLAND
adaptor_internal_src_files += $(adaptor_wayland_tizen_internal_src_files)
else
adaptor_internal_src_files += $(adaptor_x11_tizen_internal_src_files)
endif # WAYLAND

endif # MOBILE_PROFILE

# WEARABLE
if WEARABLE_PROFILE

adaptor_internal_src_files = $(adaptor_common_internal_src_files) \
                             $(adaptor_common_internal_mobile_profile_src_files) \
                             $(adaptor_tizen_internal_src_files) \
                             $(static_libraries_libunibreak_src_files)
if WAYLAND
adaptor_internal_src_files += $(adaptor_wayland_tizen_internal_src_files)
else
adaptor_internal_src_files += $(adaptor_x11_tizen_internal_src_files)
endif # WAYLAND

endif # WEARABLE


# TV
if TV_PROFILE

adaptor_internal_src_files = $(adaptor_common_internal_src_files) \
                             $(adaptor_common_internal_tv_profile_src_files) \
                             $(adaptor_tizen_internal_src_files) \
                             $(static_libraries_libunibreak_src_files)
if WAYLAND
adaptor_internal_src_files += $(adaptor_wayland_tizen_internal_src_files)
else
adaptor_internal_src_files += $(adaptor_x11_tv_internal_src_files) \
                             $(adaptor_x11_internal_tv_profile_key_src_files)
endif # WAYLAND

endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = dali-adaptor-integration.pc

# Feedback Plugin
if USE_FEEDBACK
plugin_themes_dir = ../../../adaptors/common/feedback
dali_plugin_theme_files = $(plugin_themes_dir)/default-feedback-theme.json
dalifeedbackthemedir = ${dataReadOnlyDir}/themes/feedback-themes/
dalifeedbacktheme_DATA = ${dali_plugin_theme_files}
endif

if TURBO_JPEG_IS_ON
  tizen_platform_abstraction_src_files += $(tizen_turbo_jpeg_loader)
else
  tizen_platform_abstraction_src_files += $(tizen_jpeg_loader)
endif

lib_LTLIBRARIES = libdali-adaptor.la

libdali_adaptor_la_SOURCES = \
                     $(base_adaptor_src_files) \
                     $(tizen_platform_abstraction_src_files) \
                     $(text_abstraction_src_files) \
                     $(devel_api_src_files) \
                     $(public_api_src_files) \
                     $(adaptor_internal_src_files)

libdali_adaptor_la_DEPENDENCIES =

# List include directories with more platform-specific (tizen) before portable root:
libdali_adaptor_la_includes = \
                      -I../../.. \
                      -I../../../platform-abstractions/tizen \
                      -I../../../platform-abstractions/tizen/resource-loader \
                      -I../../../platform-abstractions/portable \
                      -I../../../platform-abstractions/ \
                      -I../../../adaptors/public-api \
                      -I../../../adaptors/integration-api \
                      -I../../../adaptors/public-api/adaptor-framework \
                      -I../../../adaptors/devel-api/adaptor-framework \
                      -I../../../adaptors/common \
                      -I../../../adaptors/base/interfaces \
                      -I../../../adaptors/ \
                      -I../../../text \
                      -I../../../text/dali/internal/libunibreak

if WAYLAND
libdali_adaptor_la_includes += \
                      -I../../../adaptors/wayland \
                      -I../../../adaptors/integration-api/wayland
else
libdali_adaptor_la_includes += \
                      -I../../../adaptors/x11 \
                      -I../../../adaptors/integration-api/x11
endif

if UBUNTU_PROFILE
libdali_adaptor_la_includes += \
                      -I../../../adaptors/ubuntu
else
libdali_adaptor_la_includes += \
                      -I../../../adaptors/tizen
endif

daliDefaultThemeDir  = ${dataReadWriteDir}/theme/
daliShaderbinCacheDir = ${dataReadOnlyDir}/core/shaderbin/

libdali_adaptor_la_CXXFLAGS = \
                      -DDALI_DATA_RW_DIR="\"${daliReadWriteDir}\"" \
                      -DDALI_DATA_RO_DIR="\"${daliReadOnlyDir}\"" \
                      -DDALI_DEFAULT_FONT_CACHE_DIR="\"${daliDefaultFontCacheDir}\"" \
                      -DDALI_USER_FONT_CACHE_DIR="\"${daliUserFontCacheDir}\"" \
                      -DDALI_SHADERBIN_DIR="\"${daliShaderbinCacheDir}\"" \
                      -DDALI_DEFAULT_THEME_DIR="\"${daliDefaultThemeDir}\"" \
                      -DFONT_PRELOADED_PATH="\"${fontPreloadedPath}\"" \
                      -DFONT_DOWNLOADED_PATH="\"${fontDownloadedPath}\"" \
                      -DFONT_APPLICATION_PATH="\"${fontApplicationPath}\"" \
                      -DFONT_CONFIGURATION_FILE_PATH="\"${fontConfigurationFilePath}\"" \
                      -DDALI_FEEDBACK_THEME_DIR="\"${dalifeedbackthemedir}\"" \
                      -DNON_POWER_OF_TWO_TEXTURES \
                      -DDALI_COMPILATION -DDALI_ADAPTOR_COMPILATION \
                      -Werror -Wall -lgcc \
                      $(libdali_adaptor_la_includes) \
                      $(DALI_ADAPTOR_CFLAGS) \
                      $(DALICORE_CFLAGS) \
                      $(OPENGLES20_CFLAGS) \
                      $(FREETYPE_CFLAGS) \
                      $(FONTCONFIG_CFLAGS) \
                      $(PNG_CFLAGS) \
                      $(CAPI_APPFW_APPLICATION_CFLAGS) \
                      $(CAPI_SYSTEM_INFO_CFLAGS) \
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
                      $(LIBCURL_CFLAGS) \
                      $(ASSIMP_CFLAGS) \
                      $(CAPI_SYSTEM_SYSTEM_SETTINGS_CFLAGS)

libdali_adaptor_la_LIBADD = \
                      $(DALICORE_LIBS) \
                      $(OPENGLES20_LIBS) \
                      $(FREETYPE_LIBS) \
                      $(FONTCONFIG_LIBS) \
                      $(PNG_LIBS) \
                      $(ELEMENTARY_LIBS) \
                      $(ECORE_IPC_LIBS) \
                      $(DLOG_LIBS) \
                      $(XML_LIBS) \
                      $(VCONF_LIBS) \
                      $(EXIF_LIBS) \
                      $(TTS_LIBS) \
                      $(SENSOR_LIBS) \
                      $(LIBDRM_LIBS) \
                      $(LIBEXIF_LIBS) \
                      $(LIBCURL_LIBS) \
                      $(CAPI_SYSTEM_SYSTEM_SETTINGS_LIBS) \
                      $(CAPI_APPFW_APPLICATION_LIBS) \
                      $(HARFBUZZ_LIBS) \
                      $(CAPI_SYSTEM_INFO_LIBS) \
                      -lgif \
                      -lboost_thread

if WAYLAND
libdali_adaptor_la_CXXFLAGS += $(WAYLAND_CFLAGS)
libdali_adaptor_la_LIBADD += $(WAYLAND_LIBS)
else
libdali_adaptor_la_CXXFLAGS += $(X11_CFLAGS)
libdali_adaptor_la_LIBADD += $(X11_LIBS)
libdali_adaptor_la_LIBADD += $(ECORE_X_LIBS)
endif

if COMMON_PROFILE
libdali_adaptor_la_CXXFLAGS += $(HAPTIC_CFLAGS)
endif

if MOBILE_PROFILE
libdali_adaptor_la_CXXFLAGS += \
                      $(DEVICED_CFLAGS) \
                      $(EFL_ASSIST_CFLAGS) \
                      $(NATIVE_BUFFER_CFLAGS) \
                      $(NATIVE_BUFFER_POOL_CFLAGS)

libdali_adaptor_la_LIBADD += \
                      $(EFL_ASSIST_LIBS) \
                      $(NATIVE_BUFFER_LIBS) \
                      $(NATIVE_BUFFER_POOL_LIBS)
endif

if WEARABLE_PROFILE
libdali_adaptor_la_CXXFLAGS += \
                      $(HAPTIC_CFLAGS) \
                      $(EFL_ASSIST_CFLAGS)
endif

if TV_PROFILE
libdali_adaptor_la_CXXFLAGS += $(HAPTIC_CFLAGS)
libdali_adaptor_la_LIBADD +=
endif

if TURBO_JPEG_IS_ON
libdali_adaptor_la_LIBADD += -lturbojpeg
CFLAGS += -D_TURBO_JPEG_LOADER
if UBUNTU_PROFILE
libdali_adaptor_la_LIBADD += -ljpeg
CFLAGS += -fPIC
endif
else
libdali_adaptor_la_LIBADD += -ljpeg
endif

tizenadaptorpublicapidir = $(devincludepath)/dali/public-api
tizenadaptorpublicapi_HEADERS = $(public_api_header_files)

tizenadaptordevelapidir= $(devincludepath)/dali/devel-api

tizenadaptorintegrationapidir = $(devincludepath)/dali/integration-api/adaptors
tizenadaptorintegrationapi_HEADERS = $(adaptor_integration_api_header_files)

if WAYLAND
tizenadaptorintegrationwaylandapidir = $(devincludepath)/dali/integration-api/adaptors
tizenadaptorintegrationwaylandapi_HEADERS = $(adaptor_integration_wayland_api_header_files)
else
tizenadaptorintegrationx11apidir = $(devincludepath)/dali/integration-api/adaptors
tizenadaptorintegrationx11api_HEADERS = $(adaptor_integration_x11_api_header_files)
endif

tizenadaptorframeworkpublicapidir = $(tizenadaptorpublicapidir)/adaptor-framework
tizenadaptorframeworkpublicapi_HEADERS = $(public_api_adaptor_framework_header_files)

tizenadaptorframeworkdevelapidir = $(tizenadaptordevelapidir)/adaptor-framework
tizenadaptorframeworkdevelapi_HEADERS = $(devel_api_adaptor_framework_header_files)

tizentextabstractiondevelapidir = $(tizenadaptordevelapidir)/text-abstraction
tizentextabstractiondevelapi_HEADERS = $(text_abstraction_header_files)

if !UBUNTU_PROFILE

if !WAYLAND
tizenadaptorframeworkpublicapi_HEADERS += $(public_api_adaptor_tizen_x11_header_files)
endif # NOT WAYLAND

endif # NOT UBUNTU_PROFILE

tizenadaptordaliheaderdir = $(devincludepath)/dali
tizenadaptordaliheader_HEADERS = $(adaptor_dali_header_file)

install-data-local:
	$(MKDIR_P) ${DESTDIR}/${daliUserFontCacheDir} ${DESTDIR}/${daliShaderbinCacheDir}

# Install resource log analyzer script
bin_SCRIPTS = ../../../adaptors/scripts/dalireslog.sh

# linking test

noinst_PROGRAMS = linker.test

linker_test_SOURCES = linker-test.cpp

linker_test_CXXFLAGS = \
  -I../../../adaptors/common \
  -I../../../adaptors/public-api \
    -I../../../adaptors/integration-api \
  -I../../../adaptors/base/interfaces \
  -I../../../adaptors/public-api/adaptor-framework \
  -I../../../adaptors/devel-api/adaptor-framework \
  $(DALI_ADAPTOR_CFLAGS) \
  $(DALICORE_CFLAGS) \
  $(VCONF_CFLAGS) \
  $(DALIX11_CFLAGS) \
  -Werror -Wall

if WAYLAND
linker_test_CXXFLAGS += \
  -I../../../adaptors/integration-api/wayland
else

linker_test_CXXFLAGS += \
  -I../../../adaptors/integration-api/x11
endif


linker_test_DEPENDENCIES = libdali-adaptor.la
linker_test_LDADD = \
  $(DALICORE_LIBS) \
  $(VCONF_LIBS) \
  libdali-adaptor.la \
  $(HARFBUZZ_LIBS) \
  -L${prefix}/lib
