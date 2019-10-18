# PROFILE: UBUNTU

# Set the sources
SET( SOURCES
        ${adaptor_accessibility_common_src_files}
        ${adaptor_accessibility_ubuntu_src_files}
        ${adaptor_adaptor_common_src_files}
        ${adaptor_adaptor_ubuntu_src_files}
        ${adaptor_clipboard_common_src_files}
        ${adaptor_clipboard_ubuntu_x11_src_files}
        ${adaptor_framework_generic_src_files}
        ${devel_api_src_files}
        ${adaptor_devel_api_text_abstraction_src_files}
        ${adaptor_graphics_common_src_files}
        ${adaptor_graphics_gles_src_files}
        ${adaptor_graphics_ubuntu_src_files}
        ${adaptor_haptics_common_src_files}
        ${adaptor_imaging_common_src_files}
        ${adaptor_imaging_ubuntu_x11_src_files}
        ${adaptor_input_common_src_files}
        ${adaptor_input_ubuntu_x11_src_files}
        ${adaptor_integration_api_src_files}
        ${adaptor_legacy_common_src_files}
        ${adaptor_network_common_src_files}
        ${adaptor_public_api_src_files}
        ${adaptor_sensor_common_src_files}
        ${adaptor_sensor_ubuntu_src_files}
        ${adaptor_styling_common_src_files}
        ${adaptor_system_common_src_files}
        ${adaptor_system_linux_src_files}
        ${adaptor_system_ubuntu_x11_src_files}
        ${adaptor_text_common_src_files}
        ${adaptor_text_ubuntu_src_files}
        ${adaptor_resampler_src_files}
        ${adaptor_vector_animation_common_src_files}
        ${adaptor_video_common_src_files}
        ${adaptor_web_engine_common_src_files}
        ${adaptor_window_system_common_src_files}
        ${adaptor_trace_common_src_files}
        ${adaptor_thread_common_src_files}
        ${adaptor_window_system_ubuntu_x11_src_files}
        ${devel_api_text_abstraction_src_files}
        ${static_libraries_glyphy_src_files}
        ${static_libraries_libunibreak_src_files}
)

IF( ECORE_WAYLAND2 )
    SET( SOURCES ${SOURCES}
      ${adaptor_window_system_ecore_wl2_src_files}
    )
ENDIF()

IF( ENABLE_NETWORK_LOGGING )
    SET( SOURCES ${SOURCES}
          ${adaptor_performance_logging_src_files}
    )
ENDIF()

IF( ENABLE_TRACE )
    SET( SOURCES ${SOURCES}
        ${adaptor_trace_ubuntu_src_files}
      )
ENDIF()

# Set the header directories
SET( PROFILE_INCLUDE_DIRECTORIES
        ${ECORE_INCLUDE_DIRS}
        ${EXIF_INCLUDE_DIRS}
        ${FREETYPE_INCLUDE_DIRS}
        ${FREETYPE_BITMAP_SUPPORT}
        ${FONTCONFIG_INCLUDE_DIRS}
        ${PNG_INCLUDE_DIRS}
        ${LIBEXIF_INCLUDE_DIRS}
        ${LIBDRM_INCLUDE_DIRS}
        ${LIBCURL_INCLUDE_DIRS}
        ${LIBCRYPTO_INCLUDE_DIRS}
        ${HARFBUZZ_INCLUDE_DIRS}
        ${FRIBIDI_INCLUDE_DIRS}
        ${CAIRO_INCLUDE_DIRS}
        ${EVAS_INCLUDE_DIRS}
        ${ECORE_IPC_INCLUDE_DIRS}
        ${ECORE_IMF_INCLUDE_DIRS}
        ${ECORE_IMF_INCLUDE_DIRS}
        ${CAPI_APPFW_APPLICATION_INCLUDE_DIRS}
        ${ELEMENTARY_INCLUDE_DIRS}
        ${ECORE_X_INCLUDE_DIRS}
        ${X11_INCLUDE_DIRS}
        ${DALICORE_INCLUDE_DIRS}
        )

# Set compile options
ADD_COMPILE_OPTIONS(
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
  ${LIBCRYPTO_CFLAGS}
  ${TPKP_CURL_CFLAGS}
  ${UTILX_CFLAGS}
)

# Set the linker flags
SET(REQUIRED_LIBS ${ECORE_LDFLAGS}
  ${EXIF_LDFLAGS}
  ${FREETYPE_LDFLAGS}
  ${FREETYPE_BITMAP_SUPPORT}
  ${FONTCONFIG_LDFLAGS}
  ${PNG_LDFLAGS}
  ${LIBEXIF_LDFLAGS}
  ${LIBDRM_LDFLAGS}
  ${LIBCURL_LDFLAGS}
  ${LIBCRYPTO_LDFLAGS}
  ${HARFBUZZ_LDFLAGS}
  ${FRIBIDI_LDFLAGS}
  ${CAIRO_LDFLAGS}
  ${EVAS_LDFLAGS}
  ${ECORE_IPC_LDFLAGS}
  ${ECORE_IMF_LDFLAGS}
  ${ECORE_IMF_LDFLAGS}
  ${CAPI_APPFW_APPLICATION_LDFLAGS}
  ${ELEMENTARY_LDFLAGS}
  ${ECORE_X_LDFLAGS}
  ${X11_LDFLAGS}
  ${DALICORE_LDFLAGS}
  ${OPENGLES20_LDFLAGS}
  ${EGL_LDFLAGS}
  ${EFL_ASSIST_LIBS}
  -lgif
  -lpthread
  -lturbojpeg
  -ljpeg
)