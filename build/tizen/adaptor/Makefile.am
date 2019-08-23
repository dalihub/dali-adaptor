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

# Build the Dali Adaptor library


############# INCLUDE FILE LISTS #############

# New structure includes
include ./module.list

pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = dali-adaptor.pc dali-adaptor-integration.pc

# sources
if UBUNTU_PROFILE
LIBDALI_ADAPTOR_LA_SOURCES = \
                  $(adaptor_accessibility_common_src_files) \
                  $(adaptor_accessibility_ubuntu_src_files) \
                  $(adaptor_adaptor_common_src_files) \
                  $(adaptor_adaptor_ubuntu_src_files) \
                  $(adaptor_clipboard_common_src_files) \
                  $(adaptor_clipboard_ubuntu_x11_src_files) \
                  $(adaptor_framework_generic_src_files) \
                  $(devel_api_src_files) \
                  $(adaptor_devel_api_text_abstraction_src_files) \
                  $(adaptor_graphics_common_src_files) \
                  $(adaptor_graphics_gles_src_files) \
                  $(adaptor_graphics_ubuntu_src_files) \
                  $(adaptor_haptics_common_src_files) \
                  $(adaptor_imaging_common_src_files) \
                  $(adaptor_imaging_ubuntu_x11_src_files) \
                  $(adaptor_input_common_src_files) \
                  $(adaptor_input_ubuntu_x11_src_files) \
                  $(adaptor_integration_api_src_files) \
                  $(adaptor_legacy_common_src_files) \
                  $(adaptor_network_common_src_files) \
                  $(adaptor_public_api_src_files) \
                  $(adaptor_sensor_common_src_files) \
                  $(adaptor_sensor_ubuntu_src_files) \
                  $(adaptor_styling_common_src_files) \
                  $(adaptor_system_common_src_files) \
                  $(adaptor_system_linux_src_files) \
                  $(adaptor_system_ubuntu_x11_src_files) \
                  $(adaptor_text_common_src_files) \
                  $(adaptor_text_ubuntu_src_files) \
                  $(adaptor_resampler_src_files) \
                  $(adaptor_vector_animation_common_src_files) \
                  $(adaptor_video_common_src_files) \
                  $(adaptor_web_engine_common_src_files) \
                  $(adaptor_window_system_common_src_files) \
                  $(adaptor_trace_common_src_files) \
                  $(adaptor_thread_common_src_files) \
                  $(adaptor_window_system_ubuntu_x11_src_files) \
                  $(devel_api_text_abstraction_src_files) \
                  $(static_libraries_glyphy_src_files) \
                  $(static_libraries_libunibreak_src_files) \
                  $(adaptor_dali_src_file)

if ENABLE_NETWORK_LOGGING
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_performance_logging_src_files)
endif

if ENABLE_TRACE
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_trace_ubuntu_src_files)
endif

endif


if MOBILE_PROFILE
LIBDALI_ADAPTOR_LA_SOURCES = \
                  $(adaptor_accessibility_common_src_files) \
                  $(adaptor_accessibility_tizen_wayland_src_files) \
                  $(adaptor_accessibility_tizen_mobile_src_files) \
                  $(adaptor_adaptor_common_src_files) \
                  $(adaptor_adaptor_tizen_wayland_src_files) \
                  $(adaptor_clipboard_common_src_files) \
                  $(adaptor_clipboard_tizen_wayland_src_files) \
                  $(adaptor_framework_generic_src_files) \
                  $(devel_api_src_files) \
                  $(adaptor_devel_api_text_abstraction_src_files) \
                  $(adaptor_graphics_common_src_files) \
                  $(adaptor_graphics_gles_src_files) \
                  $(adaptor_graphics_tizen_src_files) \
                  $(adaptor_haptics_common_src_files) \
                  $(adaptor_imaging_common_src_files) \
                  $(adaptor_imaging_tizen_src_files) \
                  $(adaptor_input_common_src_files) \
                  $(adaptor_input_tizen_wayland_src_files) \
                  $(adaptor_integration_api_src_files) \
                  $(adaptor_legacy_common_src_files) \
                  $(adaptor_network_common_src_files) \
                  $(adaptor_public_api_src_files) \
                  $(adaptor_sensor_common_src_files) \
                  $(adaptor_sensor_tizen_src_files) \
                  $(adaptor_styling_common_src_files) \
                  $(adaptor_system_common_src_files) \
                  $(adaptor_system_linux_src_files) \
                  $(adaptor_system_tizen_wayland_src_files) \
                  $(adaptor_text_common_src_files) \
                  $(adaptor_resampler_src_files) \
                  $(adaptor_vector_animation_common_src_files) \
                  $(adaptor_video_common_src_files) \
                  $(adaptor_web_engine_common_src_files) \
                  $(adaptor_window_system_common_src_files) \
                  $(adaptor_window_system_tizen_src_files) \
                  $(adaptor_window_system_tizen_wayland_src_files) \
                  $(adaptor_trace_common_src_files) \
                  $(adaptor_thread_common_src_files) \
                  $(devel_api_text_abstraction_src_files) \
                  $(static_libraries_glyphy_src_files) \
                  $(static_libraries_libunibreak_src_files) \
                  $(adaptor_dali_src_file)

if ECORE_WAYLAND2
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_window_system_ecore_wl2_src_files)
else
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_window_system_ecore_wl_src_files)
endif

if ENABLE_NETWORK_LOGGING
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_performance_logging_src_files)
endif

if ENABLE_TRACE
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_trace_tizen_src_files)
endif

endif


if IVI_PROFILE
LIBDALI_ADAPTOR_LA_SOURCES = \
                  $(adaptor_accessibility_common_src_files) \
                  $(adaptor_accessibility_tizen_wayland_src_files) \
                  $(adaptor_accessibility_tizen_ivi_src_files) \
                  $(adaptor_adaptor_common_src_files) \
                  $(adaptor_adaptor_tizen_wayland_src_files) \
                  $(adaptor_clipboard_common_src_files) \
                  $(adaptor_clipboard_tizen_wayland_src_files) \
                  $(adaptor_framework_generic_src_files) \
                  $(devel_api_src_files) \
                  $(adaptor_devel_api_text_abstraction_src_files) \
                  $(adaptor_graphics_common_src_files) \
                  $(adaptor_graphics_gles_src_files) \
                  $(adaptor_graphics_tizen_src_files) \
                  $(adaptor_haptics_common_src_files) \
                  $(adaptor_haptics_tizen_src_files) \
                  $(adaptor_imaging_common_src_files) \
                  $(adaptor_imaging_tizen_src_files) \
                  $(adaptor_input_common_src_files) \
                  $(adaptor_input_tizen_wayland_src_files) \
                  $(adaptor_integration_api_src_files) \
                  $(adaptor_legacy_common_src_files) \
                  $(adaptor_network_common_src_files) \
                  $(adaptor_public_api_src_files) \
                  $(adaptor_sensor_common_src_files) \
                  $(adaptor_sensor_tizen_src_files) \
                  $(adaptor_styling_common_src_files) \
                  $(adaptor_system_common_src_files) \
                  $(adaptor_system_linux_src_files) \
                  $(adaptor_system_tizen_wayland_src_files) \
                  $(adaptor_text_common_src_files) \
                  $(adaptor_resampler_src_files) \
                  $(adaptor_vector_animation_common_src_files) \
                  $(adaptor_video_common_src_files) \
                  $(adaptor_web_engine_common_src_files) \
                  $(adaptor_window_system_common_src_files) \
                  $(adaptor_window_system_tizen_src_files) \
                  $(adaptor_window_system_tizen_wayland_src_files) \
                  $(adaptor_trace_common_src_files) \
                  $(adaptor_thread_common_src_files) \
                  $(devel_api_text_abstraction_src_files) \
                  $(static_libraries_glyphy_src_files) \
                  $(static_libraries_libunibreak_src_files) \
                  $(adaptor_dali_src_file)

if ECORE_WAYLAND2
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_window_system_ecore_wl2_src_files)
else
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_window_system_ecore_wl_src_files)
endif

if ENABLE_NETWORK_LOGGING
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_performance_logging_src_files)
endif

if ENABLE_TRACE
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_trace_tizen_src_files)
endif

endif


if TV_PROFILE
LIBDALI_ADAPTOR_LA_SOURCES = \
                  $(adaptor_accessibility_common_src_files) \
                  $(adaptor_accessibility_tizen_wayland_src_files) \
                  $(adaptor_accessibility_tizen_tv_src_files) \
                  $(adaptor_adaptor_common_src_files) \
                  $(adaptor_adaptor_tizen_wayland_src_files) \
                  $(adaptor_clipboard_common_src_files) \
                  $(adaptor_clipboard_tizen_wayland_src_files) \
                  $(adaptor_framework_generic_src_files) \
                  $(devel_api_src_files) \
                  $(adaptor_devel_api_text_abstraction_src_files) \
                  $(adaptor_graphics_common_src_files) \
                  $(adaptor_graphics_gles_src_files) \
                  $(adaptor_graphics_tizen_src_files) \
                  $(adaptor_haptics_common_src_files) \
                  $(adaptor_imaging_common_src_files) \
                  $(adaptor_imaging_tizen_src_files) \
                  $(adaptor_input_common_src_files) \
                  $(adaptor_input_tizen_wayland_src_files) \
                  $(adaptor_integration_api_src_files) \
                  $(adaptor_legacy_common_src_files) \
                  $(adaptor_network_common_src_files) \
                  $(adaptor_public_api_src_files) \
                  $(adaptor_sensor_common_src_files) \
                  $(adaptor_sensor_tizen_src_files) \
                  $(adaptor_styling_common_src_files) \
                  $(adaptor_system_common_src_files) \
                  $(adaptor_system_linux_src_files) \
                  $(adaptor_system_tizen_wayland_src_files) \
                  $(adaptor_text_common_src_files) \
                  $(adaptor_resampler_src_files) \
                  $(adaptor_vector_animation_common_src_files) \
                  $(adaptor_video_common_src_files) \
                  $(adaptor_web_engine_common_src_files) \
                  $(adaptor_window_system_common_src_files) \
                  $(adaptor_window_system_tizen_src_files) \
                  $(adaptor_window_system_tizen_wayland_src_files) \
                  $(adaptor_trace_common_src_files) \
                  $(adaptor_thread_common_src_files) \
                  $(devel_api_text_abstraction_src_files) \
                  $(static_libraries_glyphy_src_files) \
                  $(static_libraries_libunibreak_src_files) \
                  $(adaptor_dali_src_file)

if ECORE_WAYLAND2
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_window_system_ecore_wl2_src_files)
else
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_window_system_ecore_wl_src_files)
endif

if ENABLE_NETWORK_LOGGING
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_performance_logging_src_files)
endif

if ENABLE_TRACE
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_trace_tizen_src_files)
endif

endif

if COMMON_PROFILE
LIBDALI_ADAPTOR_LA_SOURCES = \
                  $(adaptor_accessibility_common_src_files) \
                  $(adaptor_accessibility_tizen_wayland_src_files) \
                  $(adaptor_accessibility_tizen_common_src_files) \
                  $(adaptor_adaptor_common_src_files) \
                  $(adaptor_adaptor_tizen_wayland_src_files) \
                  $(adaptor_clipboard_common_src_files) \
                  $(adaptor_clipboard_tizen_wayland_src_files) \
                  $(adaptor_framework_generic_src_files) \
                  $(devel_api_src_files) \
                  $(adaptor_devel_api_text_abstraction_src_files) \
                  $(adaptor_graphics_common_src_files) \
                  $(adaptor_graphics_gles_src_files) \
                  $(adaptor_graphics_tizen_src_files) \
                  $(adaptor_haptics_common_src_files) \
                  $(adaptor_imaging_common_src_files) \
                  $(adaptor_imaging_tizen_src_files) \
                  $(adaptor_input_common_src_files) \
                  $(adaptor_input_tizen_wayland_src_files) \
                  $(adaptor_integration_api_src_files) \
                  $(adaptor_legacy_common_src_files) \
                  $(adaptor_network_common_src_files) \
                  $(adaptor_public_api_src_files) \
                  $(adaptor_sensor_common_src_files) \
                  $(adaptor_sensor_tizen_src_files) \
                  $(adaptor_styling_common_src_files) \
                  $(adaptor_system_common_src_files) \
                  $(adaptor_system_linux_src_files) \
                  $(adaptor_system_tizen_wayland_src_files) \
                  $(adaptor_text_common_src_files) \
                  $(adaptor_resampler_src_files) \
                  $(adaptor_vector_animation_common_src_files) \
                  $(adaptor_video_common_src_files) \
                  $(adaptor_web_engine_common_src_files) \
                  $(adaptor_window_system_common_src_files) \
                  $(adaptor_window_system_tizen_src_files) \
                  $(adaptor_window_system_tizen_wayland_src_files) \
                  $(adaptor_trace_common_src_files) \
                  $(adaptor_thread_common_src_files) \
                  $(devel_api_text_abstraction_src_files) \
                  $(static_libraries_glyphy_src_files) \
                  $(static_libraries_libunibreak_src_files) \
                  $(adaptor_dali_src_file)

if ECORE_WAYLAND2
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_window_system_ecore_wl2_src_files)
else
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_window_system_ecore_wl_src_files)
endif

if ENABLE_NETWORK_LOGGING
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_performance_logging_src_files)
endif

if ENABLE_TRACE
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_trace_tizen_src_files)
endif

endif



if WEARABLE_PROFILE
LIBDALI_ADAPTOR_LA_SOURCES = \
                  $(adaptor_accessibility_common_src_files) \
                  $(adaptor_accessibility_tizen_wayland_src_files) \
                  $(adaptor_accessibility_tizen_wearable_src_files) \
                  $(adaptor_adaptor_common_src_files) \
                  $(adaptor_adaptor_tizen_wayland_src_files) \
                  $(adaptor_adaptor_tizen_wearable_src_files) \
                  $(adaptor_clipboard_common_src_files) \
                  $(adaptor_clipboard_tizen_wayland_src_files) \
                  $(adaptor_framework_generic_src_files) \
                  $(devel_api_src_files) \
                  $(adaptor_devel_api_text_abstraction_src_files) \
                  $(adaptor_graphics_common_src_files) \
                  $(adaptor_graphics_gles_src_files) \
                  $(adaptor_graphics_tizen_src_files) \
                  $(adaptor_haptics_common_src_files) \
                  $(adaptor_imaging_common_src_files) \
                  $(adaptor_imaging_tizen_src_files) \
                  $(adaptor_input_common_src_files) \
                  $(adaptor_input_tizen_wayland_src_files) \
                  $(adaptor_integration_api_src_files) \
                  $(adaptor_legacy_common_src_files) \
                  $(adaptor_network_common_src_files) \
                  $(adaptor_public_api_src_files) \
                  $(adaptor_sensor_common_src_files) \
                  $(adaptor_sensor_tizen_src_files) \
                  $(adaptor_styling_common_src_files) \
                  $(adaptor_system_common_src_files) \
                  $(adaptor_system_linux_src_files) \
                  $(adaptor_system_tizen_wearable_src_files) \
                  $(adaptor_text_common_src_files) \
                  $(adaptor_resampler_src_files) \
                  $(adaptor_vector_animation_common_src_files) \
                  $(adaptor_video_common_src_files) \
                  $(adaptor_web_engine_common_src_files) \
                  $(adaptor_window_system_common_src_files) \
                  $(adaptor_window_system_tizen_src_files) \
                  $(adaptor_window_system_tizen_wayland_src_files) \
                  $(adaptor_trace_common_src_files) \
                  $(adaptor_thread_common_src_files) \
                  $(devel_api_text_abstraction_src_files) \
                  $(static_libraries_glyphy_src_files) \
                  $(static_libraries_libunibreak_src_files) \
                  $(adaptor_dali_src_file)

if ECORE_WAYLAND2
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_window_system_ecore_wl2_src_files)
else
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_window_system_ecore_wl_src_files)
endif

if ENABLE_NETWORK_LOGGING
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_performance_logging_src_files)
endif

if ENABLE_TRACE
LIBDALI_ADAPTOR_LA_SOURCES += \
                  $(adaptor_trace_tizen_src_files)
endif

endif

# Package doc
package_doxy_dir = ../../../doc
include ../../../doc/file.list

libdali_adaptor_la_DEPENDENCIES =

# List include directories with more platform-specific (tizen) before portable root:
LIBDALI_ADAPTOR_LA_includes = \
                       -I../../../

daliDefaultThemeDir  = ${dataReadWriteDir}/theme/
daliShaderbinCacheDir = ${dataReadOnlyDir}/core/shaderbin/

LIBDALI_ADAPTOR_LA_CXXFLAGS = \
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
                      -DTIZEN_PLATFORM_CONFIG_SUPPORTED=${tizenPlatformConfigSupported} \
                      -DNON_POWER_OF_TWO_TEXTURES \
                      -DDALI_COMPILATION -DDALI_ADAPTOR_COMPILATION \
                      -Werror -Wall -lgcc -Wold-style-cast\
                      $(LIBDALI_ADAPTOR_LA_includes) \
                      $(DALI_ADAPTOR_CFLAGS) \
                      $(DALICORE_CFLAGS) \
                      $(OPENGLES20_CFLAGS) \
                      $(FREETYPE_CFLAGS) \
                      $(FONTCONFIG_CFLAGS) \
                      $(CAIRO_CFLAGS) \
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
                      $(LIBCRYPTO_CFLAGS) \
                      $(TPKP_CURL_CFLAGS) \
                      $(UTILX_CFLAGS)

# Todo, as soon as common repos are updated on build server remove this.
if !COMMON_PROFILE
LIBDALI_ADAPTOR_LA_CXXFLAGS += -DWAYLAND_EXTENSIONS_SUPPORTED
endif

LIBDALI_ADAPTOR_LA_CFLAGS = \
                      -Werror -Wall \
                      -DDALI_COMPILATION -DDALI_ADAPTOR_COMPILATION \
                      $(DALI_ADAPTOR_CFLAGS)

LIBDALI_ADAPTOR_LA_LIBADD = \
                      $(DALICORE_LIBS) \
                      $(OPENGLES20_LIBS) \
                      $(FREETYPE_LIBS) \
                      $(FONTCONFIG_LIBS) \
                      $(CAIRO_LIBS) \
                      $(PNG_LIBS) \
                      $(DLOG_LIBS) \
                      $(VCONF_LIBS) \
                      $(EXIF_LIBS) \
                      $(TTS_LIBS) \
                      $(CAPI_SYSTEM_SENSOR_LIBS) \
                      $(LIBDRM_LIBS) \
                      $(LIBEXIF_LIBS) \
                      $(LIBCURL_LIBS) \
                      $(LIBCRYPTO_LIBS) \
                      $(HARFBUZZ_LIBS) \
                      $(TPKP_CURL_LIBS) \
                      $(UTILX_LIBS) \
                      -lgif \
                      -lpthread \
                      -lturbojpeg \
                      -ljpeg

# EVAS used indicator
LIBDALI_ADAPTOR_LA_CXXFLAGS += $(EVAS_CFLAGS) \
                               $(ECORE_CFLAGS) \
                               $(ECORE_IPC_CFLAGS) \
                               $(ELDBUS_CFLAGS)


LIBDALI_ADAPTOR_LA_LIBADD += $(ECORE_IPC_LIBS) \
                             $(ELDBUS_LIBS)

if USE_APPFW

LIBDALI_ADAPTOR_LA_CXXFLAGS += $(CAPI_APPFW_APPLICATION_CFLAGS) \
                               $(CAPI_SYSTEM_SYSTEM_SETTINGS_CFLAGS) \
                               $(CAPI_SYSTEM_INFO_CFLAGS) \
                               $(TTS_CFLAGS) \
                               $(SENSOR_CFLAGS) \
                               $(BUNDLE_CFLAGS) \
                               $(CAPI_APPFW_COMMON_CFLAGS) \
                               $(CAPI_APPFW_CONTROL_CFLAGS) \
                               $(CAPI_APPFW_WIDGET_BASE_CFLAGS) \
                               $(ECORE_IMF_CFLAGS) \
                               $(FRIBIDI_CFLAGS)

LIBDALI_ADAPTOR_LA_LIBADD += $(CAPI_APPFW_APPLICATION_LIBS) \
                             $(CAPI_SYSTEM_SYSTEM_SETTINGS_LIBS) \
                             $(CAPI_SYSTEM_INFO_LIBS) \
                             $(TTS_LIBS) \
                             $(SENSOR_LIBS) \
                             $(BUNDLE_LIBS) \
                             $(CAPI_APPFW_COMMON_LIBS) \
                             $(CAPI_APPFW_CONTROL_LIBS) \
                             $(CAPI_APPFW_WIDGET_BASE_LIBS) \
                             $(ECORE_IMF_LIBS) \
                             $(FRIBIDI_LIBS)

else
LIBDALI_ADAPTOR_LA_CXXFLAGS += $(ELEMENTARY_CFLAGS)

LIBDALI_ADAPTOR_LA_LIBADD += $(ELEMENTARY_LIBS)
endif

if WAYLAND
# This is to avoid having to include wayland-egl.h everywhere
LIBDALI_ADAPTOR_LA_CXXFLAGS += -DWL_EGL_PLATFORM
LIBDALI_ADAPTOR_LA_CXXFLAGS += $(WAYLAND_CFLAGS)
LIBDALI_ADAPTOR_LA_LIBADD += $(WAYLAND_LIBS)
else
LIBDALI_ADAPTOR_LA_CXXFLAGS += $(X11_CFLAGS)
LIBDALI_ADAPTOR_LA_LIBADD += $(X11_LIBS)
LIBDALI_ADAPTOR_LA_LIBADD += $(ECORE_X_LIBS)
endif

if COMMON_PROFILE
LIBDALI_ADAPTOR_LA_CXXFLAGS += $(HAPTIC_CFLAGS)
endif

if MOBILE_PROFILE
LIBDALI_ADAPTOR_LA_CXXFLAGS += \
                      $(DEVICED_CFLAGS) \
                      $(EFL_ASSIST_CFLAGS) \
                      $(NATIVE_BUFFER_CFLAGS) \
                      $(NATIVE_BUFFER_POOL_CFLAGS)

LIBDALI_ADAPTOR_LA_LIBADD += \
                      $(EFL_ASSIST_LIBS) \
                      $(NATIVE_BUFFER_LIBS) \
                      $(NATIVE_BUFFER_POOL_LIBS)
endif

if WEARABLE_PROFILE
LIBDALI_ADAPTOR_LA_CXXFLAGS += \
                      $(HAPTIC_CFLAGS) \
                      $(EFL_ASSIST_CFLAGS) \
                      $(SCREENCONNECTORPROVIDER_CFLAGS) \
                      $(APPCORE_WATCH_CFLAGS)

LIBDALI_ADAPTOR_LA_LIBADD += \
                      $(SCREENCONNECTORPROVIDER_LIBS) \
                      $(APPCORE_WATCH_LIBS)
endif

if TV_PROFILE
LIBDALI_ADAPTOR_LA_CXXFLAGS += $(HAPTIC_CFLAGS)
LIBDALI_ADAPTOR_LA_LIBADD +=
endif

if IVI_PROFILE
LIBDALI_ADAPTOR_LA_CXXFLAGS += \
                      $(DEVICED_CFLAGS) \
                      $(EFL_ASSIST_CFLAGS) \
                      $(NATIVE_BUFFER_CFLAGS) \
                      $(NATIVE_BUFFER_POOL_CFLAGS)

LIBDALI_ADAPTOR_LA_LIBADD += \
                      $(EFL_ASSIST_LIBS) \
                      $(NATIVE_BUFFER_LIBS) \
                      $(NATIVE_BUFFER_POOL_LIBS)
endif

if UBUNTU_PROFILE
LIBDALI_ADAPTOR_LA_LIBADD += -ljpeg
CFLAGS += -fPIC
endif

tizenadaptorpublicapidir = $(devincludepath)/dali/public-api
tizenadaptorpublicapi_HEADERS = $(public_api_header_files)

tizenadaptordevelapidir= $(devincludepath)/dali/devel-api

tizenadaptorintegrationapidir = $(devincludepath)/dali/integration-api/adaptors
tizenadaptorintegrationapi_HEADERS = $(adaptor_integration_api_header_files)

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

tizenadaptorpublicapi_HEADERS += $(adaptor_dali_wearable_header_file)

tizenwatchpublicapidir = $(tizenadaptorpublicapidir)/watch
tizenwatchpublicapi_HEADERS = $(public_dali_watch_header_files)

tizencapturepublicapidir = $(tizenadaptorpublicapidir)/capture
tizencapturepublicapi_HEADERS = $(public_dali_capture_header_files)

install-data-local:
	$(MKDIR_P) ${DESTDIR}/${daliUserFontCacheDir} ${DESTDIR}/${daliShaderbinCacheDir}

# Install resource log analyzer script
bin_SCRIPTS = ../../../adaptors/scripts/dalireslog.sh

# package doxygen file (contains doxygen grouping information)
packagedoxydir = $(devincludepath)/dali/doc
packagedoxy_HEADERS = $(package_doxy_files)


lib_LTLIBRARIES = libdali-adaptor-cxx11.la

libdali_adaptor_cxx11_la_SOURCES = $(LIBDALI_ADAPTOR_LA_SOURCES)
libdali_adaptor_cxx11_la_includes = $(LIBDALI_ADAPTOR_LA_INCLUDES)
libdali_adaptor_cxx11_la_CFLAGS = $(LIBDALI_ADAPTOR_LA_CFLAGS)
libdali_adaptor_cxx11_la_CXXFLAGS = $(LIBDALI_ADAPTOR_LA_CXXFLAGS)
libdali_adaptor_cxx11_la_LIBADD = $(LIBDALI_ADAPTOR_LA_LIBADD)

# linking test
noinst_PROGRAMS = linker.test

linker_test_SOURCES = linker-test.cpp

linker_test_CXXFLAGS = \
  -DDALI_ADAPTOR_COMPILATION \
  -I../../../ \
  $(DALI_ADAPTOR_CFLAGS) \
  $(DALICORE_CFLAGS) \
  $(VCONF_CFLAGS) \
  $(DALIX11_CFLAGS) \
  -Werror -Wall

linker_test_LDADD = \
  $(DALICORE_LIBS) \
  $(VCONF_LIBS) \
  $(HARFBUZZ_LIBS) \
  -L${prefix}/lib

linker_test_LDADD += libdali-adaptor-cxx11.la
linker_test_DEPENDENCIES = libdali-adaptor-cxx11.la

if ENABLE_RENAME_SO
#rename
install: install-am
	rm -rf $(libdir)/libdali-adaptor-cxx11.so
	rm -rf $(libdir)/libdali-adaptor.so
	ln -s $(libdir)/libdali-adaptor-cxx11.so.0.0.* $(libdir)/libdali-adaptor.so
endif

