INCLUDE( common.cmake )

# Options
#           OPTION, VARIABLE, LIST_OF_VALUES, DESCRIPTION
ARG_ENABLE( ENABLE_EXPORTALL enable_exportall 1 "Enables the exporting of all the symbols in the library" )
ARG_ENABLE( ENABLE_DEBUG enable_debug ${ENABLE_VAL} "Turns on debugging" )
ARG_ENABLE( ENABLE_SHADERBINCACHE enable_shaderbincache ${ENABLE_VAL} "Enables shader binary cache" )
ARG_ENABLE( ENABLE_NETWORK_LOGGING enable_network_logging ${ENABLE_VAL} "Enables network for debug tool" )

# Currently, dali-adaptor requires appfw on Tizen
# and does not require it on Ubuntu.
# So we should be able to enable/disable this option for dali-adaptor.
ARG_ENABLE( ENABLE_APPFW enable_appfw ${ENABLE_VAL} "Builds with Tizen App framework libraries, off by default" )

# Tizen Profile option
ARG_ENABLE( ENABLE_PROFILE enable_profile "${ENABLE_VAL};UBUNTU" "Select the variant of tizen" )

# Graphics Backend option
ARG_ENABLE( ENABLE_GRAPHICS_BACKEND enable_graphics_backend "${ENABLE_VAL}" "Select the graphics backend to use (DYNAMIC/GLES/VULKAN)" )
ARG_ENABLE( ENABLE_VULKAN enable_vulkan "${ENABLE_VAL}" "Enables Vulkan build (Deprecated)") # Keep for backwards compatibility

# Tizen Major version
ARG_ENABLE( ENABLE_TIZEN_MAJOR_VERSION enable_tizen_major_version "${ENABLE_VAL};0" "Specify the Tizen Major version for backwards compatibility" )

ARG_ENABLE( ENABLE_FEEDBACK enable_feedback 1 "Enable feedback plugin" )
ARG_ENABLE( ENABLE_APPMODEL enable_appmodel 1 "Enable appmodel" )

ARG_ENABLE( ENABLE_WAYLAND enable_wayland "${ENABLE_VAL}" "Build on Wayland" )
ARG_ENABLE( ENABLE_RENAME_SO enable_rename_so "${ENABLE_VAL};1" "Specify whether so file is renamed or not" )
ARG_ENABLE( ENABLE_COVERAGE enable_coverage "${ENABLE_VAL}" "Enables coverage" )

# help option
ARG_ENABLE( PRINT_HELP print_help "${ENABLE_VAL}" "Prints help" )

IF( print_help )
  MESSAGE( STATUS ${HELP_ENABLES} )
  EXIT()
ENDIF()

IF( CONFIGURE_AUTOMATED_TESTS )
  # Configure automated tests
  CONFIGURE_FILE( ${ROOT_SRC_DIR}/automated-tests/CMakeLists.txt.in
                  ${ROOT_SRC_DIR}/automated-tests/CMakeLists.txt @ONLY )
ENDIF()


IF( NOT enable_profile )
  IF( ANDROID )
    SET( enable_profile ANDROID )
  ELSE()
    SET( enable_profile UBUNTU )
  ENDIF()
ENDIF()

# Test for profile and exit if something wrong
SET( VALID_PROFILES COMMON MOBILE WEARABLE TV IVI UBUNTU ANDROID WINDOWS MACOS LIBUV_X11 GLIB_X11)
LIST( FIND VALID_PROFILES ${enable_profile} RESULT )
IF( RESULT EQUAL -1 )
  MESSAGE( FATAL_ERROR "Invalid profile!" )
ENDIF()

# Defines profile specific variable
SET( ${enable_profile}_PROFILE 1 )

IF( LIBUV_X11_PROFILE OR GLIB_X11_PROFILE )
  SET( X11_REQUIRED 1 )
ENDIF()

IF( UBUNTU_PROFILE OR MACOS_PROFILE OR X11_REQUIRED )
  SET( DESKTOP_PROFILE 1 )
ELSE()
  SET( DEVICE_PROFILE 1 )
ENDIF()

IF( NOT enable_graphics_backend )
  IF( ANDROID_PROFILE OR MACOS_PROFILE OR WINDOWS_PROFILE )
    SET( enable_graphics_backend GLES )
  ELSE()
    SET( enable_graphics_backend DYNAMIC )
  ENDIF()
ENDIF()

IF(enable_vulkan AND NOT MACOS_PROFILE AND NOT WINDOWS_PROFILE )
  SET(enable_graphics_backend VULKAN)
ENDIF()

# Test for graphics backend and exit if something wrong
SET( VALID_GRAPHICS_BACKENDS VULKAN GLES DYNAMIC)
LIST( FIND VALID_GRAPHICS_BACKENDS ${enable_graphics_backend} RESULT_GRAPHICS_BACKEND )
IF( RESULT_GRAPHICS_BACKEND EQUAL -1 )
  MESSAGE( FATAL_ERROR "Invalid graphics backend!" )
ENDIF()

# TODO check what version we really need for Android
IF( ANDROID_PROFILE )
  SET( FREETYPE_REQUIRED 6.16.0 )
  SET( FREETYPE_BITMAP_SUPPORT_VERSION 6.16.0 )
ELSE()
  SET( FREETYPE_REQUIRED 9.16.3 )
  SET( FREETYPE_BITMAP_SUPPORT_VERSION 17.1.11 )
ENDIF()

SET( ELDBUS_REQUIRED 1.16.0 )

# checking all possibly used modules (required and optionals)
CHECK_MODULE_AND_SET( EXIF exif exif_available )
CHECK_MODULE_AND_SET( FREETYPE freetype2>=${FREETYPE_REQUIRED} freetype_available )
CHECK_MODULE_AND_SET( FREETYPE_BITMAP_SUPPORT freetype2>=${FREETYPE_BITMAP_SUPPORT_VERSION} freetype_bitmap_support)
CHECK_MODULE_AND_SET( FONTCONFIG fontconfig fontconfig_available )
CHECK_MODULE_AND_SET( PNG libpng [] )
CHECK_MODULE_AND_SET( WEBP libwebp webp_available )
CHECK_MODULE_AND_SET( WEBP_DEMUX libwebpdemux [] )
CHECK_MODULE_AND_SET( LIBEXIF libexif [] )
CHECK_MODULE_AND_SET( LIBDRM libdrm [] )
CHECK_MODULE_AND_SET( LIBCURL libcurl [] )
CHECK_MODULE_AND_SET( HARFBUZZ harfbuzz [] )
CHECK_MODULE_AND_SET( HYPHEN libhyphen [] )
CHECK_MODULE_AND_SET( FRIBIDI fribidi [] )
CHECK_MODULE_AND_SET( CAIRO cairo [] )
CHECK_MODULE_AND_SET( EVAS evas [] )
CHECK_MODULE_AND_SET( TTRACE ttrace ENABLE_TTRACE )
CHECK_MODULE_AND_SET( ECORE ecore [] )
CHECK_MODULE_AND_SET( ECORE_IPC ecore-ipc [] )
CHECK_MODULE_AND_SET( ECORE_IMF ecore-imf [] )
CHECK_MODULE_AND_SET( ELDBUS eldbus>=${ELDBUS_REQUIRED} eldbus_available )
CHECK_MODULE_AND_SET( TPKP_CURL tpkp-curl tpkp_curl_available )
CHECK_MODULE_AND_SET( UTILX utilX utilx_available )
CHECK_MODULE_AND_SET( DLOG dlog [] )
CHECK_MODULE_AND_SET( TTS tts [] )
CHECK_MODULE_AND_SET( VCONF vconf [] )
CHECK_MODULE_AND_SET( LIBUV libuv [] )
CHECK_MODULE_AND_SET( GLIB glib-2.0 [] )
CHECK_MODULE_AND_SET( X11 x11 [] )
CHECK_MODULE_AND_SET( XCB x11-xcb [] )
CHECK_MODULE_AND_SET( XDAMAGE xdamage [] )
CHECK_MODULE_AND_SET( XFIXES xfixes [] )
CHECK_MODULE_AND_SET( XINPUT xi [] )
CHECK_MODULE_AND_SET( XRENDER xrender [] )
CHECK_MODULE_AND_SET( CAPI_SYSTEM_INFO capi-system-info [] )
CHECK_MODULE_AND_SET( CAPI_SYSTEM_SENSOR capi-system-sensor capi_system_sensor_support )
CHECK_MODULE_AND_SET( CAPI_SYSTEM_SYSTEM_SETTINGS capi-system-system-settings [] )
CHECK_MODULE_AND_SET( CAPI_APPFW_APPLICATION capi-appfw-application [] )
CHECK_MODULE_AND_SET( COMPONENT_BASED_CORE_BASE component-based-core-base [] )
CHECK_MODULE_AND_SET( ELEMENTARY elementary [] )
CHECK_MODULE_AND_SET( BUNDLE bundle [] )
CHECK_MODULE_AND_SET( SCREENCONNECTORPROVIDER screen_connector_provider [] )
CHECK_MODULE_AND_SET( APPFW_WATCH capi-appfw-watch-application watch_available )
CHECK_MODULE_AND_SET( APPCORE_WATCH appcore-watch [] )

CHECK_MODULE_AND_SET( CAPI_APP_CORE_UI_CPP app-core-ui-cpp [] )
CHECK_MODULE_AND_SET( CAPI_APP_COMMON appcore-common [] )
CHECK_MODULE_AND_SET( CAPI_APPFW_WIDGET_BASE appcore-widget-base [] )
CHECK_MODULE_AND_SET( CAPI_APPFW_COMMON capi-appfw-app-common [] )
CHECK_MODULE_AND_SET( CAPI_APPFW_CONTROL capi-appfw-app-control [] )

CHECK_MODULE_AND_SET( DALICORE dali2-core [] )

CHECK_MODULE_AND_SET( THORVG thorvg thorvg_support )

CHECK_MODULE_AND_SET( VULKAN vulkan VULKAN )
IF(VULKAN_ENABLED)
  CHECK_MODULE_AND_SET( GLSLANG glslang GLSLANG )
  CHECK_MODULE_AND_SET( SPIRVTOOLS SPIRV-Tools SPIRVTOOLS )
ELSE()
  FIND_PACKAGE(Vulkan)
  IF(Vulkan_FOUND)
    SET(VULKAN_ENABLED ON)
    SET(GLSLANG_ENABLED ON)
    SET(SPIRVTOOLS_ENABLED ON)
  ELSE()
    SET(GLSLANG_ENABLED OFF)
    SET(SPIRVTOOLS_ENABLED OFF)
    SET(enable_graphics_backend GLES)
  ENDIF()
ENDIF()

CHECK_MODULE_AND_SET( OPENGLES20 glesv2 [] )
CHECK_MODULE_AND_SET( EGL egl [] )

IF( thorvg_support )
  SET( THORVG_VERSION ${THORVG_VERSION_STRING} )
  ADD_DEFINITIONS( -DTHORVG_SUPPORT )
  IF( THORVG_VERSION VERSION_LESS "1.0.0" )
    ADD_DEFINITIONS( -DTHORVG_VERSION_0 )
  ELSE()
    ADD_DEFINITIONS( -DTHORVG_VERSION_1 )
  ENDIF()
ELSE()
  SET( THORVG_VERSION OFF )
ENDIF()

IF( ANDROID_PROFILE )
  INCLUDE_DIRECTORIES( ${ANDROID_NDK} )
  INCLUDE_DIRECTORIES( ${ANDROID_NDK}/sources )
  INCLUDE_DIRECTORIES( ${ANDROID_NDK}/sources/android )
  INCLUDE_DIRECTORIES( ${ANDROID_NDK}/sources/android/native_app_glue )
  INCLUDE_DIRECTORIES( ${CMAKE_SYSROOT}/usr )
  INCLUDE_DIRECTORIES( ${CMAKE_SYSROOT}/usr/include/android )
ENDIF()

IF( enable_wayland )
  PKG_CHECK_MODULES(WAYLAND ecore-wl2 egl wayland-egl wayland-egl-tizen wayland-client>=1.2.0 input-method-client xkbcommon libtbm )
  SET(WAYLAND 1)
ELSE()
  CHECK_MODULE_AND_SET( ECORE_X ecore-x DALI_USE_ECORE_X11 [] )
  CHECK_MODULE_AND_SET( X11 x11 DALI_USE_X11 [] )
ENDIF()

CHECK_MODULE_AND_SET( WAYLAND_EXTENSION xdg-shell-client text-client input-method-client [] )

# BUILD CONDITIONS
IF( watch_available AND WEARABLE_PROFILE )
  ADD_DEFINITIONS( -DAPPCORE_WATCH_AVAILABLE )
ENDIF()

IF( eldbus_available )
  SET(DALI_ELDBUS_AVAILABLE 1)
  ADD_DEFINITIONS( -DDALI_ELDBUS_AVAILABLE )
ENDIF()

IF( webp_available )
  SET(DALI_WEBP_AVAILABLE 1)
  ADD_DEFINITIONS( -DDALI_WEBP_AVAILABLE )
ENDIF()

ADD_DEFINITIONS( -DPLATFORM_TIZEN )

IF( enable_debug )
  ADD_DEFINITIONS( -DDEBUG_ENABLED )
  SET( ENABLE_EXPORTALL ON )
ENDIF()

IF( NOT ${ENABLE_EXPORTALL} )
  ADD_DEFINITIONS( "-fvisibility=hidden -DHIDE_DALI_INTERNALS" )
ENDIF()

IF( enable_shaderbincache )
  ADD_DEFINITIONS( -DSHADERBIN_CACHE_ENABLED )
ENDIF()

IF( enable_network_logging )
  ADD_DEFINITIONS( -DNETWORK_LOGGING_ENABLED )
ENDIF()

IF( watch_available AND WEARABLE_PROFILE )
  ADD_DEFINITIONS( -DAPPCORE_WATCH_AVAILABLE )
ENDIF()

IF( capi_system_sensor_support AND (NOT UBUNTU_PROFILE) )
  ADD_DEFINITIONS( -DCAPI_SYSTEM_SENSOR_SUPPORT )
ENDIF()

IF( enable_appfw AND (enable_tizen_major_version EQUAL 3) )
  ADD_DEFINITIONS( -DWIDGET_AVAILABLE )
ENDIF()

IF( enable_wayland AND (NOT COMMON_PROFILE) )
  PKG_CHECK_MODULES( WAYLAND_EXTENSION [xdg-shell-client text-client input-method-client] )
ENDIF()

IF( freetype_bitmap_support )
  ADD_DEFINITIONS( -DFREETYPE_BITMAP_SUPPORT )
ENDIF()

IF( ENABLE_COVERAGE OR "$ENV{CXXFLAGS}" MATCHES --coverage )
  ADD_COMPILE_OPTIONS( --coverage )
  SET(ENABLE_COVERAGE ON)
  SET(COVERAGE --coverage)
ENDIF()

IF( DEFINED ENV{DALI_DATA_RW_DIR} )
  SET( dataReadWriteDir $ENV{DALI_DATA_RW_DIR} )
ELSE()
  SET( dataReadWriteDir ${CMAKE_INSTALL_PREFIX}/share/dali/ )
ENDIF()

IF( DEFINED ENV{DALI_DATA_RO_DIR} )
  SET( dataReadOnlyDir $ENV{DALI_DATA_RO_DIR} )
ELSE()
  SET( dataReadOnlyDir ${CMAKE_INSTALL_PREFIX}/share/dali/ )
ENDIF()

IF( DEFINED ENV{TIZEN_PLATFORM_CONFIG_SUPPORTED} )
  SET( tizenPlatformConfigSupported $ENV{TIZEN_PLATFORM_CONFIG_SUPPORTED} )
  SET( TIZEN_PLATFORM_CONFIG_SUPPORTED_LOGMSG "YES" )
ELSE()
  SET( tizenPlatformConfigSupported 0 )
  SET( TIZEN_PLATFORM_CONFIG_SUPPORTED_LOGMSG "NO" )
ENDIF()

IF( DEFINED ENV{FONT_CONFIGURATION_FILE} )
  SET( fontConfigurationFile $ENV{FONT_CONFIGURATION_FILE} )
ENDIF()

IF( DESKTOP_PROFILE )
  SET( cachePath $ENV{HOME} )
ELSEIF(ANDROID_PROFILE)
  SET( cachePath ${CMAKE_INSTALL_PREFIX}/share/dali/.cache )
ELSE()
  SET( cachePath /home/owner )
ENDIF()

IF( enable_appfw AND (enable_tizen_major_version GREATER_EQUAL 8) )
  ADD_DEFINITIONS( -DUI_THREAD_AVAILABLE )
ENDIF()

IF( enable_appfw )
  ADD_DEFINITIONS( -DUSE_APPFW -DCOMPONENT_APPLICATION_SUPPORT)
ENDIF()

#######################################################

ADD_DEFINITIONS( -DDALI_PROFILE_${enable_profile})
SET( DALI_PROFILE_CFLAGS -DDALI_PROFILE_${enable_profile} )

# Platforms with highp shader support can use vector based text
IF( NOT DEFINED ENABLE_VECTOR_BASED_TEXT_RENDERING )
  SET(ENABLE_VECTOR_BASED_TEXT_RENDERING ON)
ENDIF()

IF( ENABLE_VECTOR_BASED_TEXT_RENDERING )
  ADD_DEFINITIONS( "-DENABLE_VECTOR_BASED_TEXT_RENDERING" )
ENDIF()

CONDITIONAL( WAYLAND enable_wayland )

# set lowercase profile name
STRING( REPLACE "_" "-" PROFILE_DASH ${ENABLE_PROFILE})
STRING( TOLOWER ${PROFILE_DASH} PROFILE_LCASE )

##########################################################
# Default CFLAGS of packages
#
SET( DALI_CFLAGS
  ${DALI_ADAPTOR_CFLAGS}
  ${DALICORE_CFLAGS}
  ${OPENGLES20_CFLAGS}
  ${FREETYPE_CFLAGS}
  ${FONTCONFIG_CFLAGS}
  ${CAIRO_CFLAGS}
  ${THORVG_CFLAGS}
  ${PNG_CFLAGS}
  ${WEBP_CFLAGS}
  ${WEBP_DEMUX_CFLAGS}
  ${DLOG_CFLAGS}
  ${VCONF_CFLAGS}
  ${EXIF_CFLAGS}
  ${MMFSOUND_CFLAGS}
  ${TTS_CFLAGS}
  ${CAPI_SYSTEM_SENSOR_CFLAGS}
  ${LIBDRM_CFLAGS}
  ${LIBEXIF_CFLAGS}
  ${LIBCURL_CFLAGS}
  ${UTILX_CFLAGS}
  ${VULKAN_CFLAGS}
  -Wall
)

IF (NOT APPLE)
  # Default set of linked libraries
  SET( DALI_LDFLAGS
    ${DALICORE_LDFLAGS}
    ${OPENGLES20_LDFLAGS}
    ${FREETYPE_LDFLAGS}
    ${FONTCONFIG_LDFLAGS}
    ${CAIRO_LDFLAGS}
    ${THORVG_LDFLAGS}
    ${PNG_LDFLAGS}
    ${WEBP_LDFLAGS}
    ${WEBP_DEMUX_LDFLAGS}
    ${DLOG_LDFLAGS}
    ${VCONF_LDFLAGS}
    ${EXIF_LDFLAGS}
    ${TTS_LDFLAGS}
    ${CAPI_SYSTEM_SENSOR_LDFLAGS}
    ${LIBDRM_LDFLAGS}
    ${LIBEXIF_LDFLAGS}
    ${LIBCURL_LDFLAGS}
    ${LIBCRYPTO_LDFLAGS}
    ${HARFBUZZ_LDFLAGS}
    ${UTILX_LDFLAGS}
    -lgif
    -lturbojpeg
    -ljpeg
  )

if( NOT ANDROID_PROFILE )
  SET( DALI_LDFLAGS ${DALI_LDFLAGS}
    -lhyphen
  )
ENDIF()

ENDIF()

# Android includes pthread with android lib
IF( NOT ANDROID_PROFILE )
  SET( DALI_LDFLAGS ${DALI_LDFLAGS}
    -lpthread
  )
ENDIF()

# You need to include manually private and Android libs deps in order to link for Android
IF( ANDROID_PROFILE )
  CHECK_MODULE_AND_SET( PIXMAN pixman-1 [] )
  CHECK_MODULE_AND_SET( EXPAT expat [] )

  SET( DALI_CFLAGS ${DALI_CFLAGS}
    ${PIXMAN_CFLAGS}
    ${EXPAT_CFLAGS}
    ${FRIBIDI_CFLAGS}
  )

  LIST( APPEND DALI_LDFLAGS
    ${FRIBIDI_LDFLAGS}
    ${FREETYPE_LDFLAGS}
    ${PNG_LDFLAGS}
    ${PIXMAN_LDFLAGS}
    ${EXPAT_LDFLAGS}
    z
    android
    log
  )

  IF(NOT OPENGLES20_LDFLAGS)
    SET(OPENGLES20_LDFLAGS GLESv3)
  ENDIF()

  IF(NOT EGL_LDFLAGS)
    SET(EGL_LDFLAGS EGL)
  ENDIF()
ENDIF()


IF(X11_REQUIRED)

  SET( DALI_CFLAGS ${DALI_CFLAGS}
    ${X11_CFLAGS}
    ${XDAMAGE_CFLAGS}
    ${XFIXES_CFLAGS}
    ${XINPUT_CFLAGS}
    ${XRENDER_CFLAGS}
    )

  SET( DALI_LDFLAGS ${DALI_LDFLAGS}
    ${X11_LDFLAGS}
    ${XDAMAGE_LDFLAGS}
    ${XFIXES_LDFLAGS}
    ${XINPUT_LDFLAGS}
    ${XRENDER_LDFLAGS}
    )

ELSE()

  # EVAS used indicator
  SET( DALI_CFLAGS ${DALI_CFLAGS}
    ${EVAS_CFLAGS}
    ${ECORE_CFLAGS}
    ${ECORE_IPC_CFLAGS}
    ${ELDBUS_CFLAGS}
    )

  SET( DALI_LDFLAGS ${DALI_LDFLAGS}
    ${ECORE_IPC_LDFLAGS}
    ${ELDBUS_LDFLAGS}
    )

  # APPFW
  IF( enable_appfw )
    SET( DALI_CFLAGS ${DALI_CFLAGS}
      ${CAPI_APPFW_APPLICATION_CFLAGS}
      ${CAPI_APP_CORE_UI_CPP_CFLAGS}
      ${CAPI_APP_COMMON_CFLAGS}
      ${CAPI_SYSTEM_SYSTEM_SETTINGS_CFLAGS}
      ${CAPI_SYSTEM_INFO_CFLAGS}
      ${TTS_CFLAGS}
      ${SENSOR_CFLAGS}
      ${BUNDLE_CFLAGS}
      ${CAPI_APPFW_COMMON_CFLAGS}
      ${CAPI_APPFW_CONTROL_CFLAGS}
      ${CAPI_APPFW_WIDGET_BASE_CFLAGS}
      ${ECORE_IMF_CFLAGS}
      ${FRIBIDI_CFLAGS}
      ${COMPONENT_BASED_CORE_BASE_CFLAGS}
      ${SCREENCONNECTORPROVIDER_CFLAGS}
      )

    SET( DALI_LDFLAGS ${DALI_LDFLAGS}
      ${CAPI_APPFW_APPLICATION_LDFLAGS}
      ${CAPI_APP_CORE_UI_CPP_LDFLAGS}
      ${CAPI_APP_COMMON_LDFLAGS}
      ${CAPI_SYSTEM_SYSTEM_SETTINGS_LDFLAGS}
      ${CAPI_SYSTEM_INFO_LDFLAGS}
      ${TTS_LDFLAGS}
      ${SENSOR_LDFLAGS}
      ${BUNDLE_LDFLAGS}
      ${CAPI_APPFW_COMMON_LDFLAGS}
      ${CAPI_APPFW_CONTROL_LDFLAGS}
      ${CAPI_APPFW_WIDGET_BASE_LDFLAGS}
      ${ECORE_IMF_LDFLAGS}
      ${FRIBIDI_LDFLAGS}
      ${COMPONENT_BASED_CORE_BASE_LDFLAGS}
      ${SCREENCONNECTORPROVIDER_LDFLAGS}
      )
  ELSE()
    SET( DALI_CFLAGS ${DALI_CFLAGS}
      ${ELEMENTARY_CFLAGS}
      )

    SET( DALI_LDFLAGS ${DALI_LDFLAGS}
      ${ELEMENTARY_LDFLAGS}
      )
  ENDIF()

ENDIF() # X11_REQUIRED

# WAYLAND
IF( WAYLAND )
  SET( DALI_CFLAGS ${DALI_CFLAGS}
    -DWL_EGL_PLATFORM
    ${WAYLAND_CFLAGS}
  )
  SET( DALI_LDFLAGS ${DALI_LDFLAGS}
    ${WAYLAND_LDFLAGS}
  )
ENDIF()

# TODO: Clang is a lot more strict with warnings, we should address
# those issues at some point.
# Matches "Clang" or "AppleClang"
IF( NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" )
  ADD_COMPILE_OPTIONS(-Werror)
ENDIF()

IF(VULKAN_ENABLED)
  ADD_DEFINITIONS( -DVULKAN_HPP_NO_EXCEPTIONS )
#gcc_flags = -Wno-return-local-addr -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override \
#            -Wstack-usage=256 -Wunsafe-loop-optimizations -Wzero-as-null-pointer-constant -Wuseless-cast
# -Wfloat-equal causes issues with vulkan.hpp, removed for now
#cxx_more_warnings = -Wold-style-cast -Woverloaded-virtual -Wdouble-promotion -Wswitch-enum \
#                    -Wshadow \
#                    -Wcast-qual -Wcast-align \
#                    -Wconversion -Wsign-conversion
#-Wlarger-than=1024  # Conflicts with --coverage
# the following warnings should not be enforced
#cxx_warnings_to_remove = \
#                         -Wno-c++98-compat \
#                         -Wno-unused-parameter \
#                         -Wno-unknown-warning-option \
#                         -Wno-switch \
#                         -Wno-switch-enum \
#                         -Wno-error=switch \
#                         -Wno-error=switch-enum
#the following warnings should be added back when possible
#cxx_warnings_to_preserve = \
#                           -Wno-weak-vtables
  # Warnings that cause issues with vulkan.hpp. Double check when we upgrade.
  ADD_COMPILE_OPTIONS($<$<COMPILE_LANGUAGE:CXX>:-Wno-init-list-lifetime>)
  ADD_COMPILE_OPTIONS(-Wno-deprecated-declarations)
  INCLUDE(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG(-Wno-class-memaccess HAVE_NO_CLASS_MEMACCESS)
  IF (HAVE_NO_CLASS_MEMACCESS)
    ADD_COMPILE_OPTIONS( $<$<COMPILE_LANGUAGE:CXX>:-Wno-class-memaccess>)
  ENDIF()
  IF(X11_REQUIRED)
    SET(DALI_CFLAGS ${DALI_CFLAGS} ${XCB_CFLAGS} )
    SET(DALI_LDFLAGS ${DALI_LDFLAGS} ${XCB_LDFLAGS} )
  ENDIF()
  SET(DALI_CFLAGS ${DALI_CFLAGS} ${VULKAN_CFLAGS} )

  IF(NOT VULKAN_LDFLAGS)
    SET(VULKAN_LDFLAGS vulkan)
  ENDIF()

  SET(DALI_LDFLAGS ${DALI_LDFLAGS} ${VULKAN_LDFLAGS} )

  # glsllang-dev package on Ubuntu seems to be broken and doesn't
  # provide valid cmake config files. On Tizen cmake files are valid
  # but there's no pkg-config files so we handle both ways of obtaining
  # package configuration.
  IF(NOT GLSLANG_LDFLAGS)
    FIND_PACKAGE(glslang)
    SET(GLSLANG_LDFLAGS glslang::glslang glslang::SPIRV glslang::glslang-default-resource-limits)
  ELSE()
    # On Ubuntu 22.04 glslang seems to be horribly broken, pc file doesn't include
    # all needed deps and SPIRV-Tools package is needed
    SET(GLSLANG_LDFLAGS ${GLSLANG_LDFLAGS} -lSPIRV ${SPIRVTOOLS_LDFLAGS} -lglslang-default-resource-limits)
  ENDIF()
ENDIF()

IF(LIBUV_X11_PROFILE)
  SET(DALI_CFLAGS ${DALI_CFLAGS} ${LIBUV_CFLAGS})
  SET(DALI_LDFLAGS ${DALI_LDFLAGS} ${LIBUV_LDFLAGS})
ELSEIF(GLIB_X11_PROFILE)
  SET(DALI_CFLAGS ${DALI_CFLAGS} ${GLIB_CFLAGS})
  SET(DALI_LDFLAGS ${DALI_LDFLAGS} ${GLIB_LDFLAGS})
ENDIF()


# COMMON PROFILE
IF( COMMON_PROFILE )
  SET( DALI_CFLAGS ${DALI_CFLAGS} ${HAPTIC_CFLAGS} )
ENDIF()

# MOBILE PROFILE
IF( MOBILE_PROFILE )
  SET( DALI_CFLAGS ${DALI_CFLAGS}
      ${DEVICED_CFLAGS}
      ${EFL_ASSIST_CFLAGS}
      ${NATIVE_BUFFER_CFLAGS}
      ${NATIVE_BUFFER_POOL_CFLAGS}
      )

  SET( DALI_LDFLAGS ${DALI_LDFLAGS}
      ${EFL_ASSIST_LDFLAGS}
      ${NATIVE_BUFFER_LDFLAGS}
      ${NATIVE_BUFFER_POOL_LDFLAGS}
  )
ENDIF()

# WEARABLE PROFILE
IF( WEARABLE_PROFILE )
  SET( DALI_CFLAGS ${DALI_CFLAGS}
    ${HAPTIC_CFLAGS}
    ${EFL_ASSIST_CFLAGS}
    ${APPCORE_WATCH_CFLAGS}
  )
  SET( DALI_LDFLAGS ${DALI_LDFLAGS}
    ${APPCORE_WATCH_LDFLAGS}
  )
ENDIF()

# TV PROFILE
IF( TV_PROFILE )
  SET( DALI_CFLAGS ${DALI_CFLAGS} ${HAPTIC_CFLAGS} )
ENDIF()

# IVI PROFILE
IF( IVI_PROFILE )
  SET( DALI_CFLAGS ${DALI_CFLAGS}
      ${DEVICED_CFLAGS}
      ${EFL_ASSIST_CFLAGS}
      ${NATIVE_BUFFER_CFLAGS}
      ${NATIVE_BUFFER_POOL_CFLAGS}
  )
  SET( DALI_LDFLAGS ${DALI_LDFLAGS}
      ${EFL_ASSIST_LDFLAGS}
      ${NATIVE_BUFFER_LDFLAGS}
      ${NATIVE_BUFFER_POOL_LDFLAGS}
  )
ENDIF()

# UBUNTU PROFILE
IF( UBUNTU_PROFILE )
  SET( DALI_CFLAGS ${DALI_CFLAGS} -fPIC )
  SET( DALI_LDFLAGS ${DALI_LDFLAGS} -ljpeg)
ENDIF()

# Dali depends on shm_open, but librt is usually included from framework (ecore, libuv etc)
IF( GLIB_X11_PROFILE)
  SET( DALI_LDFLAGS ${DALI_LDFLAGS} -lrt)
ENDIF()

IF( NOT COMMON_PROFILE AND NOT X11_REQUIRED)
  ADD_DEFINITIONS( -DWAYLAND_EXTENSIONS_SUPPORTED )
ENDIF()

# had to do it here for Android by some reason, even if it is set above already
IF( ANDROID_PROFILE )
  SET( daliReadOnlyDir $ENV{DALI_DATA_RO_DIR} )
  SET( daliReadWriteDir $ENV{DALI_DATA_RW_DIR} )
ENDIF()

SET( daliDefaultThemeDir ${dataReadWriteDir}/theme/ )
SET( fontPreloadedPath $ENV{FONT_PRELOADED_PATH} )
SET( fontDownloadedPath $ENV{FONT_DOWNLOADED_PATH} )
SET( fontApplicationPath $ENV{FONT_APPLICATION_PATH} )

# Configure paths
ADD_DEFINITIONS(  -DDALI_DATA_RW_DIR="${daliReadWriteDir}"
                  -DDALI_DATA_RO_DIR="${daliReadOnlyDir}"
                  -DDALI_DEFAULT_FONT_CACHE_DIR="${daliDefaultFontCacheDir}"
                  -DDALI_USER_FONT_CACHE_DIR="${daliUserFontCacheDir}"
                  -DDALI_SHADERBIN_DIR="${daliShaderbinCacheDir}"
                  -DDALI_DEFAULT_THEME_DIR="${daliDefaultThemeDir}"
                  -DFONT_PRELOADED_PATH="${fontPreloadedPath}"
                  -DFONT_DOWNLOADED_PATH="${fontDownloadedPath}"
                  -DFONT_APPLICATION_PATH="${fontApplicationPath}"
                  -DFONT_CONFIGURATION_FILE="${fontConfigurationFile}"
                  -DTIZEN_PLATFORM_CONFIG_SUPPORTED=${tizenPlatformConfigSupported}
)


MESSAGE(STATUS "DALI_CFLAGS: ${DALI_CFLAGS}")
