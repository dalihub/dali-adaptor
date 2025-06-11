# PROFILE: MACOS

# Set the sources
SET( SOURCES
        ${adaptor_accessibility_common_src_files}
        ${adaptor_accessibility_macos_src_files}
        ${adaptor_adaptor_common_src_files}
        ${adaptor_canvas_renderer_generic_src_files}
        ${adaptor_adaptor_macos_src_files}
        ${adaptor_clipboard_macos_src_files}
        ${adaptor_drag_and_drop_generic_src_files}
        ${adaptor_framework_generic_src_files}
        ${devel_api_src_files}
        ${adaptor_devel_api_text_abstraction_src_files}
        ${adaptor_graphics_common_src_files}
        ${adaptor_graphics_gles_src_files}
        ${adaptor_graphics_macos_src_files}
        ${adaptor_haptics_common_src_files}
        ${adaptor_imaging_common_src_files}
        ${adaptor_imaging_macos_src_files}
        ${adaptor_input_common_src_files}
        ${adaptor_input_macos_src_files}
        ${adaptor_integration_api_src_files}
        ${adaptor_legacy_common_src_files}
        ${adaptor_network_common_src_files}
        ${adaptor_offscreen_common_src_files}
        ${adaptor_public_api_src_files}
	${adaptor_widget_public_api_src_files}
        ${adaptor_sensor_common_src_files}
        ${adaptor_sensor_macos_src_files}
        ${adaptor_styling_common_src_files}
        ${adaptor_system_common_src_files}
        ${adaptor_system_macos_src_files}
        ${adaptor_system_macos_src_files}
        ${adaptor_text_common_src_files}
        ${adaptor_text_macos_src_files}
        ${adaptor_resampler_src_files}
        ${adaptor_vector_animation_common_src_files}
        ${adaptor_vector_image_common_src_files}
        ${adaptor_video_common_src_files}
        ${adaptor_camera_common_src_files}
        ${adaptor_web_engine_common_src_files}
        ${adaptor_window_system_common_src_files}
        ${adaptor_macos_platform_src_files}
        ${adaptor_trace_common_src_files}
        ${adaptor_thread_common_src_files}
        ${adaptor_thread_linux_src_files}
        ${adaptor_window_system_macos_src_files}
        ${devel_api_text_abstraction_src_files}
        ${adaptor_addons_common_src_files}
        ${adaptor_addons_macos_src_files}
        ${static_libraries_libunibreak_src_files}
)

IF( ENABLE_VECTOR_BASED_TEXT_RENDERING )
    SET( SOURCES ${SOURCES}
         ${static_libraries_glyphy_src_files}
    )
ENDIF()

IF( NOT thorvg_support)
    SET( SOURCES ${SOURCES}
         ${static_libraries_nanosvg_src_files}
    )
ENDIF()

IF( ENABLE_NETWORK_LOGGING )
    SET( SOURCES ${SOURCES}
          ${adaptor_performance_logging_src_files}
    )
ENDIF()

IF( ENABLE_TRACE )
    SET( SOURCES ${SOURCES}
        ${adaptor_trace_generic_src_files}
      )
ENDIF()

FIND_PACKAGE( curl REQUIRED )
FIND_LIBRARY( EXIF_LIBRARY NAMES libexif REQUIRED )

FIND_PACKAGE( png REQUIRED )
FIND_PACKAGE( gif REQUIRED )
FIND_PACKAGE( jpeg REQUIRED )
FIND_LIBRARY( TURBO_JPEG_LIBRARY NAMES turbojpeg REQUIRED )

FIND_PACKAGE( freetype REQUIRED )
FIND_PACKAGE( harfbuzz REQUIRED )
FIND_LIBRARY( FRIBIDI_LIBRARY NAMES fribidi REQUIRED )

FIND_PACKAGE( unofficial-angle REQUIRED )
FIND_PACKAGE( unofficial-cairo REQUIRED )
FIND_PACKAGE( dali2-core REQUIRED)

FIND_PACKAGE( WebP REQUIRED )
SET(DALI_WEBP_AVAILABLE 1)
ADD_DEFINITIONS( -DDALI_WEBP_AVAILABLE )

FIND_LIBRARY(OpenGL OpenGL)
FIND_LIBRARY(IOSurface IOSurface)
FIND_LIBRARY(QuartzCore QuartzCore)
FIND_LIBRARY(Foundation Foundation)
FIND_LIBRARY(Cocoa Cocoa)

# Set the linker flags
SET(REQUIRED_LIBS
  CURL::libcurl
  ${GETOPT_LIBRARY}
  ${EXIF_LIBRARY}
  ${PNG_LIBRARIES}
  ${GIF_LIBRARIES}
  ${FONTCONFIG_LDFLAGS}
  ${CAIRO_LDFLAGS}
  JPEG::JPEG
  ${TURBO_JPEG_LIBRARY}
  Freetype::Freetype
  harfbuzz::harfbuzz
  ${FRIBIDI_LIBRARY}
  unofficial::angle::libEGL
  unofficial::angle::libGLESv2
  unofficial::cairo::cairo
  WebP::webp
  WebP::webpdemux
  dali2-core::dali2-core
  ${OpenGL}
  ${IOSurface}
  ${Foundation}
  ${QuartzCore}
  ${Cocoa}
  -pthread
)

# Set compile options
ADD_COMPILE_OPTIONS(
  ${FONTCONFIG_CFLAGS}
  ${CAIRO_CFLAGS}
)
