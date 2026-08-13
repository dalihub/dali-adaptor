# PROFILE: WINDOWS

# Set the sources
SET( SOURCES
        ${adaptor_accessibility_common_src_files}
        ${adaptor_accessibility_windows_src_files}
        ${adaptor_adaptor_common_src_files}
        ${adaptor_canvas_renderer_generic_src_files}
        ${adaptor_clipboard_common_src_files}
        ${adaptor_clipboard_windows_src_files}
        ${adaptor_drag_and_drop_windows_src_files}
        ${adaptor_framework_windows_src_files}
        ${devel_api_src_files}
        ${adaptor_devel_api_text_abstraction_src_files}
        ${adaptor_graphics_common_src_files}
        ${adaptor_graphics_gles_src_files}
        ${adaptor_haptics_common_src_files}
        ${adaptor_imaging_common_src_files}
        ${adaptor_input_common_src_files}
        ${adaptor_integration_api_src_files}
        ${adaptor_integration_api_file_download_plugin_src_files}
        ${adaptor_extension_api_src_files}
        ${adaptor_legacy_common_src_files}
        ${adaptor_network_windows_src_files}
        ${adaptor_offscreen_common_src_files}
        ${adaptor_public_api_src_files}
        ${adaptor_widget_devel_api_src_files}
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

# Builds the c files as c++
SET_SOURCE_FILES_PROPERTIES( ${static_libraries_libunibreak_src_files} PROPERTIES LANGUAGE CXX )

IF( ENABLE_NETWORK_LOGGING )
    SET( SOURCES ${SOURCES}
         ${adaptor_performance_logging_src_files}
    )
    ADD_DEFINITIONS( -DNETWORK_LOGGING_ENABLED )
ENDIF()

# Prevent windows.h from pulling in the legacy Winsock 1 header before the
# native network backend includes winsock2.h.
ADD_DEFINITIONS( -DWIN32_LEAN_AND_MEAN -DNOMINMAX )

FIND_PACKAGE( pthreads REQUIRED )
FIND_LIBRARY( GETOPT_LIBRARY NAMES getopt )
FIND_LIBRARY( EXIF_LIBRARY NAMES libexif )

FIND_PACKAGE( PNG REQUIRED )
FIND_PACKAGE( GIF REQUIRED )
FIND_PACKAGE( JPEG REQUIRED )
FIND_LIBRARY( TURBO_JPEG_LIBRARY NAMES turbojpeg )

FIND_PACKAGE( unofficial-fontconfig REQUIRED )
FIND_PACKAGE( Freetype REQUIRED )
# Windows uses the same CBDT/CBLC color-bitmap emoji path as the target builds.
# Enable BGRA bitmap glyph handling when FreeType is found.
ADD_DEFINITIONS( -DFREETYPE_BITMAP_SUPPORT )
FIND_PACKAGE( harfbuzz REQUIRED )
FIND_LIBRARY( FRIBIDI_LIBRARY NAMES fribidi )

FIND_PACKAGE( unofficial-angle REQUIRED )
FIND_PACKAGE( unofficial-cairo REQUIRED )

FIND_PACKAGE( WebP REQUIRED )
SET(DALI_WEBP_AVAILABLE 1)
ADD_DEFINITIONS( -DDALI_WEBP_AVAILABLE )

IF( thorvg_support )
    # Windows skips the pkg-config dependency discovery in deps-check.cmake.
    # Resolve TizenVG through the active CMake search paths instead. TizenVG
    # preserves ThorVG's public API and installs unversioned header/library
    # names; retain the versioned suffix as a compatibility fallback.
    FIND_PATH( THORVG_INCLUDE_DIR NAMES thorvg.h PATH_SUFFIXES thorvg-1 REQUIRED )
    FIND_LIBRARY( THORVG_LIBRARY NAMES thorvg thorvg-1 REQUIRED )

    # ThorVG 1.0 and later publish compile-time version macros in thorvg.h.
    # Pre-1.0 headers do not, so their absence selects DALi's legacy API path.
    FILE(STRINGS "${THORVG_INCLUDE_DIR}/thorvg.h" _THORVG_VERSION_DEFINES
         REGEX "^#define[ \t]+TVG_VERSION_(MAJOR|MINOR|MICRO)[ \t]+[0-9]+")
    FOREACH(_THORVG_COMPONENT MAJOR MINOR MICRO)
        FOREACH(_THORVG_DEFINE ${_THORVG_VERSION_DEFINES})
            IF(_THORVG_DEFINE MATCHES "TVG_VERSION_${_THORVG_COMPONENT}[ \t]+([0-9]+)")
                SET(THORVG_VERSION_${_THORVG_COMPONENT} "${CMAKE_MATCH_1}")
            ENDIF()
        ENDFOREACH()
    ENDFOREACH()

    IF(DEFINED THORVG_VERSION_MAJOR AND
       DEFINED THORVG_VERSION_MINOR AND
       DEFINED THORVG_VERSION_MICRO)
        SET(THORVG_VERSION
            "${THORVG_VERSION_MAJOR}.${THORVG_VERSION_MINOR}.${THORVG_VERSION_MICRO}")
    ELSE()
        SET(THORVG_VERSION "pre-1.0")
        SET(THORVG_VERSION_MAJOR 0)
    ENDIF()

    ADD_DEFINITIONS( -DTHORVG_SUPPORT )
    IF(THORVG_VERSION_MAJOR LESS 1)
        ADD_DEFINITIONS( -DTHORVG_VERSION_0 )
    ELSE()
        ADD_DEFINITIONS( -DTHORVG_VERSION_1 )
    ENDIF()
ELSE()
    SET(THORVG_VERSION OFF)
ENDIF()

# Set the linker flags
SET( REQUIRED_LIBS
        dali-windows-dependencies::dali-windows-dependencies
        PThreads4W::PThreads4W
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
        user32
        imm32
        ole32
        uuid
        ws2_32
)

IF( thorvg_support )
    LIST( APPEND REQUIRED_LIBS ${THORVG_LIBRARY} )
    INCLUDE_DIRECTORIES( ${THORVG_INCLUDE_DIR} )
ENDIF()
