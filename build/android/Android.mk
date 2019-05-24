LOCAL_PATH := $(call my-dir)
SRC_PATH := $(LOCAL_PATH)/../../..

include $(CLEAR_VARS)
include ./defines.mk

LOCAL_MODULE := libdali-adaptor

INCLUDE_PATH=$(LOCAL_PATH)/../../../dali-env/android/include

LOCAL_EXPORT_C_INCLUDES += $(INCLUDE_PATH)

cppfiles :=

adaptor_public_api_dir=$(SRC_PATH)/dali-adaptor/dali/public-api
include $(adaptor_public_api_dir)/file.list

adaptor_devel_api_dir=$(SRC_PATH)/dali-adaptor/dali/devel-api
include $(adaptor_devel_api_dir)/file.list

adaptor_adaptor_dir=$(SRC_PATH)/dali-adaptor/dali/internal/adaptor
include $(adaptor_adaptor_dir)/file.list

adaptor_accessibility_dir=$(SRC_PATH)/dali-adaptor/dali/internal/accessibility
include $(adaptor_accessibility_dir)/file.list

adaptor_haptics_dir=$(SRC_PATH)/dali-adaptor/dali/internal/haptics
include $(adaptor_haptics_dir)/file.list

adaptor_integration_api_dir=$(SRC_PATH)/dali-adaptor/dali/integration-api
include $(adaptor_integration_api_dir)/file.list

adaptor_legacy_dir=$(SRC_PATH)/dali-adaptor/dali/internal/legacy
include $(adaptor_legacy_dir)/file.list

adaptor_system_dir=$(SRC_PATH)/dali-adaptor/dali/internal/system
include $(adaptor_system_dir)/file.list

adaptor_trace_dir=$(SRC_PATH)/dali-adaptor/dali/internal/trace
include $(adaptor_trace_dir)/file.list

adaptor_thread_dir=$(SRC_PATH)/dali-adaptor/dali/internal/thread
include $(adaptor_thread_dir)/file.list

adaptor_network_dir=$(SRC_PATH)/dali-adaptor/dali/internal/network
include $(adaptor_network_dir)/file.list

adaptor_window_system_dir=$(SRC_PATH)/dali-adaptor/dali/internal/window-system
include $(adaptor_window_system_dir)/file.list

adaptor_graphics_dir=$(SRC_PATH)/dali-adaptor/dali/internal/graphics
include $(adaptor_graphics_dir)/file.list

adaptor_input_dir=$(SRC_PATH)/dali-adaptor/dali/internal/input
include $(adaptor_input_dir)/file.list

adaptor_clipboard_dir=$(SRC_PATH)/dali-adaptor/dali/internal/clipboard
include $(adaptor_clipboard_dir)/file.list

adaptor_imaging_dir=$(SRC_PATH)/dali-adaptor/dali/internal/imaging
include $(adaptor_imaging_dir)/file.list

adaptor_styling_dir=$(SRC_PATH)/dali-adaptor/dali/internal/styling
include $(adaptor_styling_dir)/file.list

adaptor_text_dir=$(SRC_PATH)/dali-adaptor/dali/internal/text
include $(adaptor_text_dir)/file.list

adaptor_thirdparty_dir=$(SRC_PATH)/dali-adaptor/third-party
include $(adaptor_thirdparty_dir)/file.list

adaptor_vector_animation_dir=$(SRC_PATH)/dali-adaptor/dali/internal/vector-animation
include $(adaptor_vector_animation_dir)/file.list

cppfiles += $(public_api_header_files) \
            $(public_api_adaptor_framework_header_files) \
            $(adaptor_public_api_src_files) \
            $(adaptor_devel_api_src_files) \
            $(devel_api_adaptor_framework_header_files) \
            $(devel_api_text_abstraction_src_files) \
            $(adaptor_adaptor_common_src_files) \
            $(adaptor_adaptor_android_src_files) \
            $(adaptor_legacy_common_src_files) \
            $(adaptor_integration_api_header_files) \
            $(adaptor_integration_api_src_files) \
            $(adaptor_system_common_src_files) \
            $(adaptor_system_android_src_files) \
            $(adaptor_text_android_src_files) \
            $(adaptor_trace_common_src_files) \
            $(adaptor_trace_android_src_files) \
            $(adaptor_thread_common_src_files) \
            $(adaptor_network_common_src_files) \
            $(adaptor_window_system_common_src_files) \
            $(adaptor_window_system_android_src_files) \
            $(adaptor_graphics_gles_src_files) \
            $(adaptor_graphics_android_src_files) \
            $(adaptor_input_common_src_files) \
            $(adaptor_input_android_src_files) \
            $(adaptor_clipboard_common_src_files) \
            $(adaptor_clipboard_android_src_files) \
            $(adaptor_imaging_common_src_files) \
            $(adaptor_styling_common_src_files) \
            $(adaptor_accessibility_common_src_files) \
            $(adaptor_accessibility_android_src_files) \
            $(adaptor_haptics_common_src_files) \
            $(static_libraries_libunibreak_src_files) \
            $(static_libraries_glyphy_src_files) \
            $(adaptor_resampler_src_files) \
            $(adaptor_vector_animation_common_src_files) \
            $(public_api_src_files) \
            $(devel_api_src_files) \

LOCAL_CFLAGS += -DDALI_ADAPTOR_COMPILATION
LOCAL_CFLAGS += -DHIDE_DALI_INTERNALS

LOCAL_CFLAGS += -I$(SRC_PATH)/dali-adaptor
LOCAL_CFLAGS += -I$(SRC_PATH)/dali-adaptor/dali
LOCAL_CFLAGS += -I$(SRC_PATH)/dali-adaptor/dali/integration-api
LOCAL_CFLAGS += -I$(SRC_PATH)/dali-adaptor/dali/devel-api
LOCAL_CFLAGS += -I$(SRC_PATH)/dali-adaptor/dali/public-api

LOCAL_CFLAGS += -I$(ANDROID_NDK)/sysroot/usr/include
LOCAL_CFLAGS += -I$(ANDROID_NDK)/sysroot/usr/include/android
LOCAL_CFLAGS += -I$(ANDROID_NDK)/sources/android/native_app_glue

LOCAL_CFLAGS += -I$(SRC_PATH)/android-dependencies/libexif
LOCAL_CFLAGS += -I$(SRC_PATH)/android-dependencies/libfontconfig
LOCAL_CFLAGS += -I$(SRC_PATH)/android-dependencies/libfreetype/include
LOCAL_CFLAGS += -I$(SRC_PATH)/android-dependencies/libfreetype/include/freetype
LOCAL_CFLAGS += -I$(SRC_PATH)/android-dependencies/libfreetype/include/freetype/config
LOCAL_CFLAGS += -I$(SRC_PATH)/android-dependencies/libgif
LOCAL_CFLAGS += -I$(SRC_PATH)/android-dependencies/libharfbuzz
LOCAL_CFLAGS += -I$(SRC_PATH)/android-dependencies/libjpeg-turbo
LOCAL_CFLAGS += -I$(SRC_PATH)/android-dependencies/libpng

LOCAL_CFLAGS += -I$(INCLUDE_PATH)

LOCAL_SRC_FILES := $(cppfiles)

include $(BUILD_STATIC_LIBRARY)

DALI_PUBLIC_INCLUDES := $(INCLUDE_PATH)/dali


