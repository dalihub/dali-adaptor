# PROFILE: ANDROID

include( ${adaptor_integration_api_dir}/adaptor-framework/android/file.list )

# Set the sources
SET( SOURCES
        ${adaptor_accessibility_common_src_files}
        ${adaptor_accessibility_android_src_files}
        ${adaptor_adaptor_common_src_files}
        ${adaptor_canvas_renderer_generic_src_files}
        ${adaptor_clipboard_android_src_files}
        ${adaptor_drag_and_drop_generic_src_files}
        ${adaptor_framework_android_src_files}
        ${devel_api_src_files}
        ${adaptor_devel_api_text_abstraction_src_files}
        ${adaptor_graphics_common_src_files}
        ${adaptor_graphics_common_dynamic_src_files}
        ${adaptor_graphics_common_dynamic_android_src_files}
        ${adaptor_haptics_common_src_files}
        ${adaptor_imaging_common_src_files}
        ${adaptor_imaging_common_dynamic_src_files}
        ${adaptor_imaging_android_src_files}
        ${adaptor_input_common_src_files}
        ${adaptor_input_generic_src_files}
        ${adaptor_integration_api_src_files}
        ${adaptor_integration_api_android_src_files}
        ${adaptor_legacy_common_src_files}
        ${adaptor_network_common_src_files}
        ${adaptor_offscreen_common_src_files}
        ${adaptor_public_api_src_files}
        ${adaptor_widget_public_api_src_files}
        ${adaptor_sensor_common_src_files}
        ${adaptor_sensor_android_src_files}
        ${adaptor_styling_common_src_files}
        ${adaptor_system_common_src_files}
        ${adaptor_system_android_src_files}
        ${adaptor_text_common_src_files}
        ${adaptor_resampler_src_files}
        ${adaptor_vector_animation_common_src_files}
        ${adaptor_vector_image_common_src_files}
        ${adaptor_video_common_src_files}
        ${adaptor_camera_common_src_files}
        ${adaptor_web_engine_common_src_files}
        ${adaptor_window_system_common_src_files}
        ${adaptor_window_system_common_dynamic_src_files}
        ${adaptor_window_system_android_src_files}
        ${adaptor_trace_common_src_files}
        ${adaptor_thread_common_src_files}
        ${adaptor_thread_linux_src_files}
        ${devel_api_text_abstraction_src_files}
        ${static_libraries_libunibreak_src_files}
        ${adaptor_addons_common_src_files}
        ${adaptor_addons_android_src_files}
)

SET(ADAPTOR_GRAPHICS_GLES_SOURCES
        ${adaptor_graphics_gles_src_files}
        ${adaptor_graphics_egl_android_src_files}
        ${adaptor_imaging_egl_android_src_files}
        ${adaptor_graphics_library_common_src_files}
        ${adaptor_window_system_android_graphics_backend_library_src_files}
)

SET(ADAPTOR_GRAPHICS_VULKAN_SOURCES
        ${adaptor_graphics_vulkan_src_files}
        ${adaptor_graphics_vulkan_android_src_files}
        ${adaptor_imaging_vulkan_android_src_files}
        ${adaptor_libraries_spirv_reflect_src_files}
        ${adaptor_graphics_library_common_src_files}
        ${adaptor_window_system_android_graphics_backend_library_src_files}
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

IF( ENABLE_ANDROIDJNI_FRAMEWORK )
    SET( SOURCES ${SOURCES}
      ${adaptor_adaptor_androidjni_src_files}
    )
ELSE()
    SET( SOURCES ${SOURCES}
      ${adaptor_adaptor_android_src_files}
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
