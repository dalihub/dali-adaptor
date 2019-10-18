# dependencies

# sources
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

if( DEFINED ENABLE_NETWORK_LOGGING )
    SET( DALI_ADAPTOR_LA_SOURCES ${DALI_ADAPTOR_LA_SOURCES}
          ${adaptor_performance_logging_src_files}
    )
endif()

if ( DEFINED ENABLE_TRACE )
    SET( DALI_ADAPTOR_LA_SOURCES ${DALI_ADAPTOR_LA_SOURCES}
        ${adaptor_trace_ubuntu_src_files}
    )
endif()

# headers
SET( INCLUDE_HEADERS
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
        )

# definitions
SET( DEFINITIONS -DDALI_PROFILE_UBUNTU )