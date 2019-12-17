INCLUDE( common.cmake )

# Options
#           OPTION, VARIABLE, LIST_OF_VALUES, DESCRIPTION
ARG_ENABLE( ENABLE_EXPORTALL enable_exportall 1 "Enables the exporting of all the symbols in the library" )
ARG_ENABLE( ENABLE_DEBUG enable_debug ${ENABLE_VAL} "Turns on debugging" )
ARG_ENABLE( ENABLE_SHADERBINCACHE enable_shaderbincache ${ENABLE_VAL} "Enables shader binary cache" )
ARG_ENABLE( ENABLE_NETWORKLOGGING enable_networklogging ${ENABLE_VAL} "Enables network for debug tool" )

# Currently, dali-adaptor requires appfw on Tizen
# and does not require it on Ubuntu.
# So we should be able to enable/disable this option for dali-adaptor.
ARG_ENABLE( ENABLE_APPFW enable_appfw ${ENABLE_VAL} "Builds with Tizen App framework libraries, off by default" )

# Tizen Profile option
ARG_ENABLE( ENABLE_PROFILE enable_profile "${ENABLE_VAL};UBUNTU" "Select the variant of tizen" )

# Tizen Major version
ARG_ENABLE( ENABLE_TIZEN_MAJOR_VERSION enable_tizen_major_version "${ENABLE_VAL};0" "Specify the Tizen Major version for backwards compatibility" )

ARG_ENABLE( ENABLE_FEEDBACK enable_feedback 1 "Enable feedback plugin" )

ARG_ENABLE( ENABLE_WAYLAND enable_wayland "${ENABLE_VAL}" "Build on Wayland" )
ARG_ENABLE( ENABLE_ECORE_WAYLAND2 enable_ecore_wayland2 "${ENABLE_VAL}" "Build on Ecore Wayland2" )
ARG_ENABLE( ENABLE_RENAME_SO enable_rename_so "${ENABLE_VAL};1" "Specify whether so file is renamed or not" )
ARG_ENABLE( ENABLE_COVERAGE enable_coverage "${ENABLE_VAL}" "Enables coverage" )

# help option
ARG_ENABLE( PRINT_HELP print_help "${ENABLE_VAL}" "Prints help" )

IF( print_help )
  MESSAGE( STATUS ${HELP_ENABLES} )
  EXIT()
ENDIF()

IF( NOT enable_profile )
  IF( ANDROID )
    SET( enable_profile ANDROID )
  ELSE()
    SET( enable_profile UBUNTU )
  ENDIF()
ENDIF()

# Test for profile and exit if something wrong
SET( VALID_PROFILES COMMON MOBILE WEARABLE TV IVI UBUNTU ANDROID WINDOWS )
LIST( FIND VALID_PROFILES ${enable_profile} RESULT )
IF( RESULT EQUAL -1 )
  MESSAGE( FATAL_ERROR "Invalid profile!" )
ENDIF()

# Defines profile specific variable
SET( ${enable_profile}_PROFILE 1 )

# TODO check what version we really need for Android
IF( ANDROID_PROFILE )
  SET( FREETYPE_REQUIRED 6.16.0 )
  SET( FREETYPE_BITMAP_SUPPORT_VERSION 6.16.0 )
ELSE()
  SET( FREETYPE_REQUIRED 9.16.3 )
  SET( FREETYPE_BITMAP_SUPPORT_VERSION 17.1.11 )
ENDIF()

# checking all possibly used modules (required and optionals)
CHECK_MODULE_AND_SET( EXIF exif exif_available )
CHECK_MODULE_AND_SET( FREETYPE freetype2>=${FREETYPE_REQUIRED} freetype_available )
CHECK_MODULE_AND_SET( FREETYPE_BITMAP_SUPPORT freetype2>=${FREETYPE_BITMAP_SUPPORT_VERSION} freetype_bitmap_support)
CHECK_MODULE_AND_SET( FONTCONFIG fontconfig fontconfig_available )
CHECK_MODULE_AND_SET( PNG libpng [] )
CHECK_MODULE_AND_SET( LIBEXIF libexif [] )
CHECK_MODULE_AND_SET( LIBDRM libdrm [] )
CHECK_MODULE_AND_SET( LIBCURL libcurl [] )
CHECK_MODULE_AND_SET( HARFBUZZ harfbuzz [] )
CHECK_MODULE_AND_SET( FRIBIDI fribidi [] )
CHECK_MODULE_AND_SET( CAIRO cairo [] )
CHECK_MODULE_AND_SET( EVAS evas [] )
CHECK_MODULE_AND_SET( TTRACE ttrace ENABLE_TTRACE )
CHECK_MODULE_AND_SET( ECORE ecore [] )
CHECK_MODULE_AND_SET( ECORE_IPC ecore-ipc [] )
CHECK_MODULE_AND_SET( ECORE_IMF ecore-imf [] )
CHECK_MODULE_AND_SET( ELDBUS eldbus eldbus_available )
CHECK_MODULE_AND_SET( TPKP_CURL tpkp-curl tpkp_curl_available )
CHECK_MODULE_AND_SET( UTILX utilX utilx_available )
CHECK_MODULE_AND_SET( OPENGLES20 glesv2 [] )
CHECK_MODULE_AND_SET( EGL egl [] )
CHECK_MODULE_AND_SET( DLOG dlog [] )
CHECK_MODULE_AND_SET( TTS tts [] )
CHECK_MODULE_AND_SET( VCONF vconf [] )

CHECK_MODULE_AND_SET( CAPI_SYSTEM_INFO capi-system-info [] )
CHECK_MODULE_AND_SET( CAPI_SYSTEM_SENSOR capi-system-sensor capi_system_sensor_support )
CHECK_MODULE_AND_SET( CAPI_SYSTEM_SYSTEM_SETTINGS capi-system-system-settings [] )
CHECK_MODULE_AND_SET( CAPI_APPFW_APPLICATION capi-appfw-application [] )

CHECK_MODULE_AND_SET( ELEMENTARY elementary [] )
CHECK_MODULE_AND_SET( BUNDLE bundle [] )
CHECK_MODULE_AND_SET( SCREENCONNECTORPROVIDER screen_connector_provider [] )
CHECK_MODULE_AND_SET( APPFW_WATCH capi-appfw-watch-application watch_available )
CHECK_MODULE_AND_SET( APPCORE_WATCH appcore-watch [] )

CHECK_MODULE_AND_SET( CAPI_APPFW_APPLICATION appcore-ui [] )
CHECK_MODULE_AND_SET( CAPI_APPFW_WIDGET_BASE appcore-widget-base [] )
CHECK_MODULE_AND_SET( CAPI_APPFW_COMMON capi-appfw-app-common [] )
CHECK_MODULE_AND_SET( CAPI_APPFW_CONTROL capi-appfw-app-control [] )

CHECK_MODULE_AND_SET( DALICORE dali-core [] )

IF( ANDROID_PROFILE )
  INCLUDE_DIRECTORIES( ${ANDROID_NDK} )
  INCLUDE_DIRECTORIES( ${ANDROID_NDK}/sources )
  INCLUDE_DIRECTORIES( ${ANDROID_NDK}/sources/android )
  INCLUDE_DIRECTORIES( ${ANDROID_NDK}/sources/android/native_app_glue )
  INCLUDE_DIRECTORIES( ${ANDROID_NDK}/sysroot/usr )
  INCLUDE_DIRECTORIES( ${ANDROID_NDK}/sysroot/usr/include/android )
ENDIF()

IF( enable_wayland )
  IF( enable_ecore_wayland2 )
    PKG_CHECK_MODULES(WAYLAND ecore-wl2 egl wayland-egl wayland-egl-tizen wayland-client>=1.2.0 xkbcommon libtbm )
  ELSE()
    PKG_CHECK_MODULES(WAYLAND ecore-wayland egl wayland-egl wayland-client>=1.2.0 xkbcommon libtbm)
  ENDIF()
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

IF( tpkp_curl_available )
  ADD_DEFINITIONS( -DTPK_CURL_ENABLED )
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

IF( enable_networklogging )
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

IF( UBUNTU_PROFILE )
  SET( cachePath $ENV{HOME} )
ELSE()
  SET( cachePath /home/owner )
ENDIF()

#######################################################

ADD_DEFINITIONS( -DDALI_PROFILE_${enable_profile})
SET( DALI_PROFILE_CFLAGS -DDALI_PROFILE_${enable_profile} )

# Platforms with highp shader support can use vector based text
CONDITIONAL( ENABLE_VECTOR_BASED_TEXT_RENDERING UBUNTU_PROFILE )
CONDITIONAL( WAYLAND enable_wayland )

# set lowercase profile name
STRING( TOLOWER ${ENABLE_PROFILE} PROFILE_LCASE )

##########################################################3
# Default CFLAGS of packages
#
SET( DALI_CFLAGS
  ${DALI_ADAPTOR_CFLAGS}
  ${DALICORE_CFLAGS}
  ${OPENGLES20_CFLAGS}
  ${FREETYPE_CFLAGS}
  ${FONTCONFIG_CFLAGS}
  ${CAIRO_CFLAGS}
  ${PNG_CFLAGS}
  ${DLOG_CFLAGS}
  ${VCONF_CFLAGS}
  ${EXIF_CFLAGS}
  ${MMFSOUND_CFLAGS}
  ${TTS_CFLAGS}
  ${CAPI_SYSTEM_SENSOR_CFLAGS}
  ${LIBDRM_CFLAGS}
  ${LIBEXIF_CFLAGS}
  ${LIBCURL_CFLAGS}
  ${TPKP_CURL_CFLAGS}
  ${UTILX_CFLAGS}
  -Wall
)

# Default set of linked librarires
SET( DALI_LDFLAGS
  ${DALICORE_LDFLAGS}
  ${OPENGLES20_LDFLAGS}
  ${FREETYPE_LDFLAGS}
  ${FONTCONFIG_LDFLAGS}
  ${CAIRO_LDFLAGS}
  ${PNG_LDFLAGS}
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
  ${TPKP_CURL_LDFLAGS}
  ${UTILX_LDFLAGS}
  -lgif
  -lturbojpeg
  -ljpeg
)

# Android includes pthread with android lib
if( NOT ANDROID_PROFILE )
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
    EGL
    GLESv3
  )
ENDIF()

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
  )

  SET( DALI_LDFLAGS ${DALI_LDFLAGS}
    ${CAPI_APPFW_APPLICATION_LDFLAGS}
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
  )
ELSE()
  SET( DALI_CFLAGS ${DALI_CFLAGS}
    ${ELEMENTARY_CFLAGS}
  )

  SET( DALI_LDFLAGS ${DALI_LDFLAGS}
    ${ELEMENTARY_LDFLAGS}
  )
ENDIF()

# WAYLAND
IF( WAYLAND )
  SET( DALI_CFLAGS ${DALI_CFLAGS}
    -DWL_EGL_PLATFORM
    ${WAYLAND_CFLAGS}
  )
  SET( DALI_LDFLAGS ${DALI_LDFLAGS}
    ${WAYLAND_LDFLAGS}
  )
ELSE()
  SET( DALI_CFLAGS ${DALI_CFLAGS}
       ${X11_CFLAGS}
       )
  SET( DALI_LDFLAGS ${DALI_LDFLAGS}
       ${X11_LDFLAGS}
       ${ECORE_X_LDFLAGS}
       )
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
    ${SCREENCONNECTORPROVIDER_CFLAGS}
    ${APPCORE_WATCH_CFLAGS}
  )
  SET( DALI_LDFLAGS ${DALI_LDFLAGS}
    ${SCREENCONNECTORPROVIDER_LDFLAGS}
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
  SET( DALI_LDFLAGS ${DALI_LDFLAGS} -ljpeg )
ENDIF()

IF( NOT COMMON_PROFILE )
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
