# PROFILE: WINDOWS

# Set the sources
SET( SOURCES
        ${adaptor_accessibility_common_src_files}
        ${adaptor_accessibility_windows_src_files}
        ${adaptor_adaptor_common_src_files}
        ${adaptor_canvas_renderer_generic_src_files}
        ${adaptor_clipboard_windows_src_files}
        ${adaptor_drag_and_drop_generic_src_files}
        ${adaptor_framework_generic_src_files}
        ${devel_api_src_files}
        ${adaptor_devel_api_text_abstraction_src_files}
        ${adaptor_graphics_common_src_files}
        ${adaptor_graphics_gles_src_files}
        ${adaptor_haptics_common_src_files}
        ${adaptor_imaging_common_src_files}
        ${adaptor_input_common_src_files}
        ${adaptor_integration_api_src_files}
        ${adaptor_legacy_common_src_files}
        ${adaptor_network_common_src_files}
        ${adaptor_offscreen_common_src_files}
        ${adaptor_public_api_src_files}
        ${adaptor_sensor_common_src_files}
        ${adaptor_styling_common_src_files}
        ${adaptor_system_common_src_files}
        ${adaptor_system_windows_src_files}
        ${adaptor_text_common_src_files}
        ${adaptor_resampler_src_files}
        ${adaptor_vector_animation_common_src_files}
        ${adaptor_vector_image_common_src_files}
        ${adaptor_video_common_src_files}
        ${adaptor_camera_common_src_files}
        ${adaptor_web_engine_common_src_files}
        ${adaptor_window_system_common_src_files}
        ${adaptor_trace_common_src_files}
        ${adaptor_thread_common_src_files}
        ${adaptor_thread_windows_src_files}
        ${devel_api_text_abstraction_src_files}
        ${static_libraries_libunibreak_src_files}
        ${adaptor_windows_platform_src_files}
        ${adaptor_adaptor_windows_src_files}
        ${adaptor_window_system_windows_src_files}
        ${adaptor_graphics_windows_src_files}
        ${adaptor_input_windows_src_files}
        ${adaptor_imaging_windows_src_files}
        ${adaptor_addons_common_src_files}
        ${adaptor_addons_dummy_src_files}
)

IF(NOT ENABLE_VULKAN)
  SET(SOURCES ${SOURCES}
    ${adaptor_public_api_egl_src_files}
    ${adaptor_devel_api_egl_src_files}
    ${adaptor_system_common_egl_src_files}
    ${adaptor_window_system_common_egl_src_files}
  )
ENDIF()

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

# Builds the c files as c++
SET_SOURCE_FILES_PROPERTIES( ${static_libraries_libunibreak_src_files} PROPERTIES LANGUAGE CXX )

FIND_PACKAGE( pthreads REQUIRED )
FIND_PACKAGE( curl REQUIRED )
FIND_LIBRARY( GETOPT_LIBRARY NAMES getopt )
FIND_LIBRARY( EXIF_LIBRARY NAMES libexif )

FIND_PACKAGE( png REQUIRED )
FIND_PACKAGE( gif REQUIRED )
FIND_PACKAGE( jpeg REQUIRED )
FIND_LIBRARY( TURBO_JPEG_LIBRARY NAMES turbojpeg )

FIND_PACKAGE( unofficial-fontconfig REQUIRED )
FIND_PACKAGE( freetype REQUIRED )
FIND_PACKAGE( harfbuzz REQUIRED )
FIND_LIBRARY( FRIBIDI_LIBRARY NAMES fribidi )

FIND_PACKAGE( unofficial-angle REQUIRED )
FIND_PACKAGE( unofficial-cairo REQUIRED )

FIND_PACKAGE( WebP REQUIRED )
SET(DALI_WEBP_AVAILABLE 1)
ADD_DEFINITIONS( -DDALI_WEBP_AVAILABLE )

# Set the linker flags
SET( REQUIRED_LIBS
        PThreads4W::PThreads4W
        CURL::libcurl
        ${GETOPT_LIBRARY}
        ${EXIF_LIBRARY}
        ${PNG_LIBRARIES}
        ${GIF_LIBRARIES}
        JPEG::JPEG
        ${TURBO_JPEG_LIBRARY}
        unofficial::fontconfig::fontconfig
        Freetype::Freetype
        harfbuzz::harfbuzz
        ${FRIBIDI_LIBRARY}
        unofficial::angle::libEGL
        unofficial::angle::libGLESv2
        unofficial::cairo::cairo
        WebP::webp
        WebP::webpdemux
        dali2-core::dali2-core
)
