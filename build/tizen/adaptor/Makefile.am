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

if USE_EFL
# ECore Common
adaptor_ecore_common_dir = ../../../adaptors/ecore/common
include ../../../adaptors/ecore/common/file.list
else
# If we're not using any EFL, then we need to use libuv mainloop
# for the frame work
adaptor_libuv_dir = ../../../adaptors/libuv
include ../../../adaptors/libuv/file.list
endif

# Wayland
if WAYLAND
if USE_EFL
## Use ecore_wayland
adaptor_ecore_wayland_dir = ../../../adaptors/ecore/wayland
include ../../../adaptors/ecore/wayland/file.list
else
## Use wayland
adaptor_wayland_dir = ../../../adaptors/wayland
include ../../../adaptors/wayland/file.list
endif
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

# WEARABLE
if WEARABLE_PROFILE
adaptor_wearable_dir = ../../../adaptors/wearable
include ../../../adaptors/wearable/file.list
endif

# TV
if TV_PROFILE
adaptor_tv_dir = ../../../adaptors/tv
include ../../../adaptors/tv/file.list
endif

# IVI
adaptor_mobile_dir = ../../../adaptors/mobile
include ../../../adaptors/mobile/file.list

# Public API
adaptor_public_api_dir = ../../../adaptors/public-api
include ../../../adaptors/public-api/file.list

# Devel API ( for use by Toolkit)
adaptor_devel_api_dir = ../../../adaptors/devel-api
include ../../../adaptors/devel-api/file.list


# Static libraries
static_libraries_libunibreak_src_dir = ../../../text/dali/internal/libunibreak
include ../../../text/dali/internal/libunibreak/file.list

static_libraries_glyphy_src_dir = ../../../text/dali/internal/glyphy
include ../../../text/dali/internal/glyphy/file.list

# Package doc
package_doxy_dir = ../../../doc
include ../../../doc/file.list

############# source files #############

# FRAMEWORK FILES for adaptor
# Either use ecore mainloop +  appcore / uiman
# Or libuv main loop with no appcore / uimain
if USE_EFL
adaptor_internal_src_files = $(adaptor_tizen_framework_efl_src_files) \
                             $(adaptor_ecore_common_internal_src_files)
else
adaptor_internal_src_files = $(adaptor_tizen_framework_libuv_src_files)
endif

# COMMON
if COMMON_PROFILE

adaptor_internal_src_files += $(adaptor_common_internal_src_files) \
                              $(adaptor_common_internal_default_profile_src_files) \
                              $(adaptor_tizen_internal_src_files) \
                              $(adaptor_tizen_internal_non_mobile_src_files) \
                              $(static_libraries_libunibreak_src_files)

if WAYLAND

if USE_ECORE_WAYLAND
adaptor_internal_src_files += $(adaptor_ecore_wayland_tizen_internal_src_files) \
                              $(adaptor_ecore_wayland_internal_default_profile_src_files)
else
adaptor_internal_src_files += $(adaptor_wayland_tizen_internal_src_files)
endif # USE_ECORE_WAYLAND

adaptor_internal_src_files += $(adaptor_tizen_internal_egl_extension_src_files) \
                              $(adaptor_tizen_internal_native_image_src_files)
else
adaptor_internal_src_files += $(adaptor_x11_tizen_internal_src_files) \
                              $(adaptor_common_internal_egl_extension_src_files) \
                              $(adaptor_x11_internal_default_profile_src_files)
endif # WAYLAND
endif # COMMON_PROFILE


# UBUNTU
if UBUNTU_PROFILE

adaptor_internal_src_files += $(adaptor_common_internal_src_files) \
                             $(adaptor_common_internal_default_profile_src_files) \
                             $(adaptor_ubuntu_internal_src_files) \
                             $(adaptor_x11_ubuntu_internal_src_files) \
                             $(adaptor_x11_internal_default_profile_src_files) \
                             $(static_libraries_libunibreak_src_files) \
                             $(adaptor_common_internal_egl_extension_src_files)

endif # UBUNTU_PROFILE


# MOBILE
if MOBILE_PROFILE

adaptor_internal_src_files += $(adaptor_common_internal_src_files) \
                             $(adaptor_common_internal_mobile_profile_src_files) \
                             $(adaptor_tizen_internal_src_files) \
                             $(static_libraries_libunibreak_src_files)

if WAYLAND

if USE_ECORE_WAYLAND
adaptor_internal_src_files += $(adaptor_ecore_wayland_tizen_internal_src_files)
else
adaptor_internal_src_files += $(adaptor_wayland_tizen_internal_src_files)
endif # USE_ECORE_WAYLAND

adaptor_internal_src_files += $(adaptor_tizen_internal_egl_extension_src_files) \
                              $(adaptor_tizen_internal_native_image_src_files) \
                              $(adaptor_common_internal_wayland_mobile_profile_src_files)
else
adaptor_internal_src_files += $(adaptor_x11_tizen_internal_src_files) \
                              $(adaptor_common_internal_egl_extension_src_files) \
                              $(adaptor_common_internal_x_mobile_profile_src_files)
endif # WAYLAND

endif # MOBILE_PROFILE

# WEARABLE
if WEARABLE_PROFILE

adaptor_internal_src_files += $(adaptor_common_internal_src_files) \
                             $(adaptor_common_internal_mobile_profile_src_files) \
                             $(adaptor_tizen_internal_src_files) \
                             $(static_libraries_libunibreak_src_files)
if WAYLAND

if USE_ECORE_WAYLAND
adaptor_internal_src_files += $(adaptor_ecore_wayland_tizen_internal_src_files)
else
adaptor_internal_src_files += $(adaptor_wayland_tizen_internal_src_files)
endif # USE_ECORE_WAYLAND

adaptor_internal_src_files += $(adaptor_tizen_internal_egl_extension_src_files) \
                              $(adaptor_tizen_internal_native_image_src_files) \
                              $(adaptor_internal_wearable_profile_src_files)
else
adaptor_internal_src_files += $(adaptor_x11_tizen_internal_src_files) \
                              $(adaptor_common_internal_egl_extension_src_files)
endif # WAYLAND

endif # WEARABLE


# TV
if TV_PROFILE

adaptor_internal_src_files += $(adaptor_common_internal_src_files) \
                             $(adaptor_common_internal_tv_profile_src_files) \
                             $(adaptor_tizen_internal_src_files) \
                             $(adaptor_tizen_internal_non_mobile_src_files) \
                             $(static_libraries_libunibreak_src_files)
if WAYLAND

if USE_ECORE_WAYLAND
adaptor_internal_src_files += $(adaptor_ecore_wayland_tizen_internal_src_files)
else
adaptor_internal_src_files += $(adaptor_wayland_tizen_internal_src_files)
endif # USE_ECORE_WAYLAND

adaptor_internal_src_files += $(adaptor_tizen_internal_egl_extension_src_files) \
                              $(adaptor_tizen_internal_native_image_src_files)
else
adaptor_internal_src_files += $(adaptor_x11_tv_internal_src_files) \
                              $(adaptor_x11_internal_tv_profile_key_src_files) \
                              $(adaptor_common_internal_egl_extension_src_files)
endif # WAYLAND

endif

# IVI
if IVI_PROFILE

adaptor_internal_src_files += $(adaptor_common_internal_src_files) \
                             $(adaptor_common_internal_mobile_profile_src_files) \
                             $(adaptor_tizen_internal_src_files) \
                             $(static_libraries_libunibreak_src_files)

if WAYLAND

if USE_ECORE_WAYLAND
adaptor_internal_src_files += $(adaptor_ecore_wayland_tizen_internal_src_files)
else
adaptor_internal_src_files += $(adaptor_wayland_tizen_internal_src_files)
endif # USE_ECORE_WAYLAND

adaptor_internal_src_files += $(adaptor_tizen_internal_egl_extension_src_files) \
                              $(adaptor_tizen_internal_native_image_src_files)
else
adaptor_internal_src_files += $(adaptor_x11_tizen_internal_src_files) \
                              $(adaptor_common_internal_egl_extension_src_files)
endif # WAYLAND

endif # IVI_PROFILE



# Node JS support for using an external libuv main loop. If not enabled then just use e-core as normal
# Used for things like callbacks, file-monintors, x input handling
if LIB_UV_EVENT_LOOP
main_loop_integration_src_files = $(adaptor_common_internal_uv_src_files)
input_event_handler_src_files = $(adaptor_uv_x_event_handler_internal_src_files)
else
main_loop_integration_src_files = $(adaptor_common_internal_ecore_src_files)
input_event_handler_src_files = $(adaptor_ecore_x_event_handler_internal_src_files)
endif

if ENABLE_VECTOR_BASED_TEXT_RENDERING
adaptor_internal_src_files += $(static_libraries_glyphy_src_files)
DALI_ADAPTOR_CFLAGS += -DENABLE_VECTOR_BASED_TEXT_RENDERING
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = dali-adaptor.pc dali-adaptor-integration.pc

lib_LTLIBRARIES = libdali-adaptor.la

libdali_adaptor_la_SOURCES = \
                     $(base_adaptor_src_files) \
                     $(main_loop_integration_src_files) \
                     $(tizen_platform_abstraction_src_files) \
                     $(text_abstraction_src_files) \
                     $(devel_api_src_files) \
                     $(public_api_src_files) \
                     $(adaptor_internal_src_files) \
                     $(input_event_handler_src_files)


if ENABLE_NETWORK_LOGGING
libdali_adaptor_la_SOURCES += \
  $(base_adaptor_networking_src_files)
endif

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
libdali_adaptor_la_includes += -I../../../adaptors/integration-api/wayland

if USE_ECORE_WAYLAND
libdali_adaptor_la_includes += \
                      -I../../../adaptors/ecore/common \
                      -I../../../adaptors/ecore/wayland
else
libdali_adaptor_la_includes += -I../../../adaptors/wayland \
                               -I../../../adaptors/wayland/input/text/imf \
                               -I../../../adaptors/wayland/clipboard
endif # USE_ECORE_WAYLADN

libdali_adaptor_la_includes += \
                      -I../../../adaptors/tizen
else
libdali_adaptor_la_includes += \
                      -I../../../adaptors/ecore/common \
                      -I../../../adaptors/x11 \
                      -I../../../adaptors/integration-api/x11
endif # WAYLAND

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
                      -DFONT_CONFIGURATION_FILE="\"${fontConfigurationFile}\"" \
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
                      $(DLOG_CFLAGS) \
                      $(VCONF_CFLAGS) \
                      $(EXIF_CFLAGS) \
                      $(MMFSOUND_CFLAGS) \
                      $(TTS_CFLAGS) \
                      $(CAPI_SYSTEM_SENSOR_CFLAGS) \
                      $(LIBDRM_CFLAGS) \
                      $(LIBEXIF_CFLAGS) \
                      $(LIBCURL_CFLAGS) \
                      $(TPKP_CURL_CFLAGS) \
                      $(UTILX_CFLAGS)

# Todo, as soon as common repos are updated on build server remove this.
if !COMMON_PROFILE
libdali_adaptor_la_CXXFLAGS += -DWAYLAND_EXTENSIONS_SUPPORTED
endif

libdali_adaptor_la_CFLAGS = $(libdali_adaptor_la_CXXFLAGS)

libdali_adaptor_la_LIBADD = \
                      $(DALICORE_LIBS) \
                      $(OPENGLES20_LIBS) \
                      $(FREETYPE_LIBS) \
                      $(FONTCONFIG_LIBS) \
                      $(PNG_LIBS) \
                      $(DLOG_LIBS) \
                      $(VCONF_LIBS) \
                      $(EXIF_LIBS) \
                      $(TTS_LIBS) \
                      $(CAPI_SYSTEM_SENSOR_LIBS) \
                      $(LIBDRM_LIBS) \
                      $(LIBEXIF_LIBS) \
                      $(LIBCURL_LIBS) \
                      $(HARFBUZZ_LIBS) \
                      $(TPKP_CURL_LIBS) \
                      $(UTILX_LIBS) \
                      $(APPCORE_WATCH_LIBS) \
                      -lgif \
                      -lpthread \
                      -lturbojpeg \
                      -ljpeg

if USE_EFL
# EVAS used indicator
libdali_adaptor_la_CXXFLAGS += $(ELEMENTARY_CFLAGS) \
                               $(EVAS_CFLAGS) \
                               $(ECORE_CFLAGS) \
                               $(ECORE_IPC_CFLAGS) \
                               $(ELDBUS_CFLAGS) \
                               -DUSE_EFL


libdali_adaptor_la_LIBADD += $(ELEMENTARY_LIBS) \
                             $(ECORE_IPC_LIBS) \
                             $(ELDBUS_LIBS)

endif

if USE_APPFW

libdali_adaptor_la_CXXFLAGS += $(CAPI_APPFW_APPLICATION_CFLAGS) \
                               $(CAPI_SYSTEM_SYSTEM_SETTINGS_CFLAGS) \
                               $(CAPI_SYSTEM_INFO_CFLAGS) \
                               $(TTS_CFLAGS) \
                               $(SENSOR_CFLAGS)

libdali_adaptor_la_LIBADD += $(CAPI_APPFW_APPLICATION_LIBS) \
                             $(CAPI_SYSTEM_SYSTEM_SETTINGS_LIBS) \
                             $(CAPI_SYSTEM_INFO_LIBS) \
                             $(TTS_LIBS) \
                             $(SENSOR_LIBS)

endif
if WAYLAND
# This is to avoid having to include wayland-egl.h everywhere
libdali_adaptor_la_CXXFLAGS += -DWL_EGL_PLATFORM
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

if IVI_PROFILE
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

if UBUNTU_PROFILE
libdali_adaptor_la_LIBADD += -ljpeg
CFLAGS += -fPIC
endif

if !UBUNTU_PROFILE
if WAYLAND
else
# X11
libdali_adaptor_la_CXXFLAGS += $(UTILX_CFLAGS)
libdali_adaptor_la_LIBADD += $(UTILX_LIBS)
endif
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
tizenadaptorframeworkpublicapi_HEADERS += $(public_api_adaptor_tizen_header_files)

if !WAYLAND
tizenadaptorframeworkdevelapi_HEADERS += $(devel_api_adaptor_tizen_x11_header_files)
endif # NOT WAYLAND

endif # NOT UBUNTU_PROFILE

tizenadaptordaliheaderdir = $(devincludepath)/dali
tizenadaptordaliheader_HEADERS = $(adaptor_dali_header_file)

if WEARABLE_PROFILE
tizenadaptorpublicapi_HEADERS += $(adaptor_dali_wearable_header_file)

tizenwatchpublicapidir = $(tizenadaptorpublicapidir)/watch
tizenwatchpublicapi_HEADERS = $(public_dali_watch_header_files)
endif

install-data-local:
	$(MKDIR_P) ${DESTDIR}/${daliUserFontCacheDir} ${DESTDIR}/${daliShaderbinCacheDir}

# Install resource log analyzer script
bin_SCRIPTS = ../../../adaptors/scripts/dalireslog.sh

# package doxygen file (contains doxygen grouping information)
packagedoxydir = $(devincludepath)/dali/doc
packagedoxy_HEADERS = $(package_doxy_files)

# linking test

# turn off the linker test if were building for libuv
# We can't link to LibUV becase it is statically linked to Node.JS (by default)
if !LIB_UV_EVENT_LOOP
noinst_PROGRAMS = linker.test
endif # NOT LIB_UV_EVENT_LOOP

linker_test_SOURCES = linker-test.cpp

linker_test_CXXFLAGS = \
  -DDALI_ADAPTOR_COMPILATION \
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
