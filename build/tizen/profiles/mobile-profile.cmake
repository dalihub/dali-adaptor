# PROFILE: UBUNTU

# Set the sources
SET( SOURCES
        ${adaptor_accessibility_common_src_files}
        ${adaptor_accessibility_tizen_wayland_src_files}
        ${adaptor_accessibility_tizen_mobile_src_files}
        ${adaptor_adaptor_common_src_files}
        ${adaptor_adaptor_tizen_wayland_src_files}
        ${adaptor_clipboard_common_src_files}
        ${adaptor_clipboard_tizen_wayland_src_files}
        ${adaptor_framework_generic_src_files}
        ${devel_api_src_files}
        ${adaptor_devel_api_text_abstraction_src_files}
        ${adaptor_graphics_common_src_files}
        ${adaptor_graphics_gles_src_files}
        ${adaptor_graphics_tizen_src_files}
        ${adaptor_haptics_common_src_files}
        ${adaptor_imaging_common_src_files}
        ${adaptor_imaging_tizen_src_files}
        ${adaptor_input_common_src_files}
        ${adaptor_input_tizen_wayland_src_files}
        ${adaptor_integration_api_src_files}
        ${adaptor_legacy_common_src_files}
        ${adaptor_network_common_src_files}
        ${adaptor_public_api_src_files}
        ${adaptor_sensor_common_src_files}
        ${adaptor_sensor_tizen_src_files}
        ${adaptor_styling_common_src_files}
        ${adaptor_system_common_src_files}
        ${adaptor_system_linux_src_files}
        ${adaptor_system_tizen_wayland_src_files}
        ${adaptor_text_common_src_files}
        ${adaptor_resampler_src_files}
        ${adaptor_vector_animation_common_src_files}
        ${adaptor_video_common_src_files}
        ${adaptor_web_engine_common_src_files}
        ${adaptor_window_system_common_src_files}
        ${adaptor_window_system_tizen_src_files}
        ${adaptor_window_system_tizen_wayland_src_files}
        ${adaptor_trace_common_src_files}
        ${adaptor_thread_common_src_files}
        ${devel_api_text_abstraction_src_files}
        ${static_libraries_glyphy_src_files}
        ${static_libraries_libunibreak_src_files}
)
IF( enable_ecore_wayland2 )
    SET( SOURCES ${SOURCES}
         ${adaptor_window_system_ecore_wl2_src_files}
         )
ELSE()
    SET( SOURCES ${SOURCES}
         ${adaptor_window_system_ecore_wl_src_files}
         )
ENDIF()

IF( ENABLE_NETWORK_LOGGING )
    SET( SOURCES ${SOURCES}
          ${adaptor_performance_logging_src_files}
    )
ENDIF()

IF( ENABLE_TRACE )
    SET( SOURCES ${SOURCES}
        ${adaptor_trace_tizen_src_files}
      )
ENDIF()

