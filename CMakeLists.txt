cmake_minimum_required(VERSION 3.11)

if (WIN32)

PROJECT (dali-adaptor)

SET(adaptor_devel_api_dir dali/devel-api)

SET( devel_api_src_files
  ${adaptor_devel_api_dir}/adaptor-framework/accessibility-adaptor.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/application-devel.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/application-extensions.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/bitmap-saver.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/clipboard.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/clipboard-event-notifier.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/color-controller.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/drag-and-drop-detector.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/environment-variable.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/event-feeder.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/event-thread-callback.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/feedback-player.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/file-loader.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/image-loading.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/gif-loading.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/input-method-context.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/input-method-options.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/orientation.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/performance-logger.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/physical-keyboard.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/pixel-buffer.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/singleton-service.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/sound-player.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/style-monitor.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/tilt-sensor.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/lifecycle-controller.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/video-player.cpp
  ${adaptor_devel_api_dir}/adaptor-framework/virtual-keyboard.cpp
  )

SET( devel_api_adaptor_framework_header_files
  ${adaptor_devel_api_dir}/adaptor-framework/accessibility-adaptor.h
  ${adaptor_devel_api_dir}/adaptor-framework/accessibility-action-handler.h
  ${adaptor_devel_api_dir}/adaptor-framework/accessibility-gesture-handler.h
  ${adaptor_devel_api_dir}/adaptor-framework/application-devel.h
  ${adaptor_devel_api_dir}/adaptor-framework/application-extensions.h
  ${adaptor_devel_api_dir}/adaptor-framework/bitmap-saver.h
  ${adaptor_devel_api_dir}/adaptor-framework/clipboard-event-notifier.h
  ${adaptor_devel_api_dir}/adaptor-framework/clipboard.h
  ${adaptor_devel_api_dir}/adaptor-framework/color-controller.h
  ${adaptor_devel_api_dir}/adaptor-framework/drag-and-drop-detector.h
  ${adaptor_devel_api_dir}/adaptor-framework/environment-variable.h
  ${adaptor_devel_api_dir}/adaptor-framework/event-feeder.h
  ${adaptor_devel_api_dir}/adaptor-framework/event-thread-callback.h
  ${adaptor_devel_api_dir}/adaptor-framework/feedback-plugin.h
  ${adaptor_devel_api_dir}/adaptor-framework/feedback-player.h
  ${adaptor_devel_api_dir}/adaptor-framework/file-loader.h
  ${adaptor_devel_api_dir}/adaptor-framework/image-loading.h
  ${adaptor_devel_api_dir}/adaptor-framework/gif-loading.h
  ${adaptor_devel_api_dir}/adaptor-framework/imf-manager.h
  ${adaptor_devel_api_dir}/adaptor-framework/input-method-devel.h
  ${adaptor_devel_api_dir}/adaptor-framework/input-method-options.h
  ${adaptor_devel_api_dir}/adaptor-framework/lifecycle-controller.h
  ${adaptor_devel_api_dir}/adaptor-framework/orientation.h
  ${adaptor_devel_api_dir}/adaptor-framework/performance-logger.h
  ${adaptor_devel_api_dir}/adaptor-framework/pixel-buffer.h
  ${adaptor_devel_api_dir}/adaptor-framework/render-surface.h
  ${adaptor_devel_api_dir}/adaptor-framework/singleton-service.h
  ${adaptor_devel_api_dir}/adaptor-framework/sound-player.h
  ${adaptor_devel_api_dir}/adaptor-framework/style-monitor.h
  ${adaptor_devel_api_dir}/adaptor-framework/tilt-sensor.h
  ${adaptor_devel_api_dir}/adaptor-framework/video-player.h
  ${adaptor_devel_api_dir}/adaptor-framework/video-player-plugin.h
  ${adaptor_devel_api_dir}/adaptor-framework/key-extension-plugin.h
  ${adaptor_devel_api_dir}/adaptor-framework/virtual-keyboard.h
  ${adaptor_devel_api_dir}/adaptor-framework/physical-keyboard.h
  ${adaptor_devel_api_dir}/adaptor-framework/key-devel.h
  )
  
SET ( devel_api_text_abstraction_src_files
   ${adaptor_devel_api_dir}/text-abstraction/bidirectional-support.cpp
   ${adaptor_devel_api_dir}/text-abstraction/font-client.cpp
   ${adaptor_devel_api_dir}/text-abstraction/font-list.cpp
   ${adaptor_devel_api_dir}/text-abstraction/font-metrics.cpp
   ${adaptor_devel_api_dir}/text-abstraction/glyph-info.cpp
   ${adaptor_devel_api_dir}/text-abstraction/script.cpp
   ${adaptor_devel_api_dir}/text-abstraction/segmentation.cpp
   ${adaptor_devel_api_dir}/text-abstraction/shaping.cpp
   )

SET( text_abstraction_header_files
   ${adaptor_devel_api_dir}/text-abstraction/bidirectional-support.h
   ${adaptor_devel_api_dir}/text-abstraction/font-client.h
   ${adaptor_devel_api_dir}/text-abstraction/font-list.h
   ${adaptor_devel_api_dir}/text-abstraction/font-metrics.h
   ${adaptor_devel_api_dir}/text-abstraction/glyph-info.h
   ${adaptor_devel_api_dir}/text-abstraction/script.h
   ${adaptor_devel_api_dir}/text-abstraction/segmentation.h
   ${adaptor_devel_api_dir}/text-abstraction/shaping.h
   ${adaptor_devel_api_dir}/text-abstraction/text-abstraction.h
   ${adaptor_devel_api_dir}/text-abstraction/text-abstraction-definitions.h
   )

SET( SOURCES ${SOURCES}
  ${devel_api_src_files}
  ${devel_api_text_abstraction_src_files}
)

SET( INCLUDE ${INCLUDE}
  ${devel_api_adaptor_framework_header_files}
  ${text_abstraction_header_files}
)

SET(adaptor_integration_api_dir dali/integration-api)

SET(adaptor_integration_api_header_files
  ${adaptor_integration_api_dir}/adaptor.h
  ${adaptor_integration_api_dir}/egl-interface.h
  ${adaptor_integration_api_dir}/log-factory-interface.h
  ${adaptor_integration_api_dir}/thread-synchronization-interface.h
  ${adaptor_integration_api_dir}/trigger-event-interface.h
  ${adaptor_integration_api_dir}/trigger-event-factory-interface.h
  ${adaptor_integration_api_dir}/trigger-event-factory.h)

SET (adaptor_integration_win_api_header_files
  ${adaptor_integration_api_dir}/windows/ecore-win-types.h
  ${adaptor_integration_api_dir}/windows/ecore-win-render-surface.h
  ${adaptor_integration_api_dir}/windows/pixmap-render-surface.h
  ${adaptor_integration_api_dir}/windows/imf-manager-impl.h
  ${adaptor_integration_api_dir}/pixmap-render-surface-factory.h)

SET( INCLUDE ${INCLUDE}
${adaptor_integration_api_header_files}
${adaptor_integration_win_api_header_files}
)

SET( adaptor_internal_dir dali/internal/ )

SET( SOURCES ${SOURCES}
  #accessibility
  ${adaptor_internal_dir}/accessibility/common/tts-player-factory.cpp
  ${adaptor_internal_dir}/accessibility/common/tts-player-impl.cpp
  ${adaptor_internal_dir}/accessibility/common/accessibility-adaptor-impl.cpp
  ${adaptor_internal_dir}/accessibility/common/accessibility-gesture-detector.cpp
  ${adaptor_internal_dir}/accessibility/windows/accessibility-adaptor-impl-win.cpp

  #adaptor
  ${adaptor_internal_dir}/adaptor/common/adaptor.cpp
  ${adaptor_internal_dir}/adaptor/common/lifecycle-controller-impl.cpp
  ${adaptor_internal_dir}/adaptor/common/adaptor-impl.cpp
  ${adaptor_internal_dir}/adaptor/common/adaptor.cpp
  ${adaptor_internal_dir}/adaptor/common/application-impl.cpp
  ${adaptor_internal_dir}/adaptor/common/combined-update-render-controller.cpp
  ${adaptor_internal_dir}/adaptor/windows/adaptor-impl-win.cpp
  ${adaptor_internal_dir}/adaptor/windows/framework-win.cpp
  
  #clipboard
  ${adaptor_internal_dir}/clipboard/common/clipboard-event-notifier-impl.cpp
  ${adaptor_internal_dir}/clipboard/windows/clipboard-impl-win.cpp
  
  #graphics
  ${adaptor_internal_dir}/graphics/common/render-helper.cpp
  ${adaptor_internal_dir}/graphics/gles20/egl-debug.cpp
  ${adaptor_internal_dir}/graphics/gles20/egl-factory.cpp
  ${adaptor_internal_dir}/graphics/gles20/egl-implementation.cpp
  ${adaptor_internal_dir}/graphics/gles20/egl-sync-implementation.cpp
  ${adaptor_internal_dir}/graphics/gles20/gl-extensions.cpp
  ${adaptor_internal_dir}/graphics/gles20/gl-proxy-implementation.cpp
  ${adaptor_internal_dir}/graphics/windows/vsync-monitor-win.cpp
  ${adaptor_internal_dir}/graphics/windows-gl/egl-image-extensions.cpp
  
  #haptics
  ${adaptor_internal_dir}/haptics/common/feedback-player-impl.cpp
  ${adaptor_internal_dir}/haptics/common/feedback-plugin-proxy.cpp
  
  #imaging
  ${adaptor_internal_dir}/imaging/common/native-bitmap-buffer-impl.cpp
  ${adaptor_internal_dir}/imaging/common/native-image-source-factory.cpp
  ${adaptor_internal_dir}/imaging/common/native-image-source-impl.cpp
  ${adaptor_internal_dir}/imaging/common/pixel-buffer-impl.cpp
  ${adaptor_internal_dir}/imaging/common/alpha-mask.cpp
  ${adaptor_internal_dir}/imaging/common/gaussian-blur.cpp
  ${adaptor_internal_dir}/imaging/common/http-utils.cpp
  ${adaptor_internal_dir}/imaging/common/image-loader.cpp
  ${adaptor_internal_dir}/imaging/common/image-operations.cpp
  ${adaptor_internal_dir}/imaging/common/loader-astc.cpp
  ${adaptor_internal_dir}/imaging/common/loader-bmp.cpp
  ${adaptor_internal_dir}/imaging/common/loader-gif.cpp
  ${adaptor_internal_dir}/imaging/common/loader-ico.cpp
  ${adaptor_internal_dir}/imaging/common/loader-jpeg-turbo.cpp
  ${adaptor_internal_dir}/imaging/common/loader-ktx.cpp
  ${adaptor_internal_dir}/imaging/common/loader-png.cpp
  ${adaptor_internal_dir}/imaging/common/loader-wbmp.cpp
  ${adaptor_internal_dir}/imaging/common/pixel-manipulation.cpp
  ${adaptor_internal_dir}/imaging/windows/file-download-win.cpp
  ${adaptor_internal_dir}/imaging/windows/CurlEnvironment-win.cpp
  ${adaptor_internal_dir}/input/common/drag-and-drop-detector-impl.cpp
  ${adaptor_internal_dir}/input/common/gesture-manager.cpp
  ${adaptor_internal_dir}/input/common/input-method-context-impl.cpp
  ${adaptor_internal_dir}/input/common/key-grab.cpp
  ${adaptor_internal_dir}/input/common/key-impl.cpp
  ${adaptor_internal_dir}/input/common/long-press-gesture-detector.cpp
  ${adaptor_internal_dir}/input/common/pan-gesture-detector-base.cpp
  ${adaptor_internal_dir}/input/common/pan-gesture-detector.cpp
  ${adaptor_internal_dir}/input/common/physical-keyboard-impl.cpp
  ${adaptor_internal_dir}/input/common/pinch-gesture-detector.cpp
  ${adaptor_internal_dir}/input/common/tap-gesture-detector.cpp
  ${adaptor_internal_dir}/input/windows/input-method-context-factory-win.cpp
  ${adaptor_internal_dir}/input/windows/input-method-context-impl-win.cpp
  ${adaptor_internal_dir}/input/windows/key-mapping-win.cpp
  ${adaptor_internal_dir}/input/windows/virtual-keyboard-impl-win.cpp
  
  #legacy
  ${adaptor_internal_dir}/legacy/common/tizen-platform-abstraction.cpp
  ${adaptor_internal_dir}/legacy/tizen/data-compression.cpp
  
  #network
  ${adaptor_internal_dir}/network/common/socket-factory.cpp
  ${adaptor_internal_dir}/network/common/socket-impl.cpp

  #sensor
  ${adaptor_internal_dir}/sensor/common/tilt-sensor-factory.cpp
  ${adaptor_internal_dir}/sensor/common/tilt-sensor-impl.cpp
  
  #styling
  ${adaptor_internal_dir}/styling/common/style-monitor-impl.cpp
  
  #system
  ${adaptor_internal_dir}/system/common/abort-handler.cpp
  ${adaptor_internal_dir}/system/common/command-line-options.cpp
  ${adaptor_internal_dir}/system/common/environment-options.cpp
  ${adaptor_internal_dir}/system/common/fps-tracker.cpp
  ${adaptor_internal_dir}/system/common/frame-time-stamp.cpp
  ${adaptor_internal_dir}/system/common/frame-time-stats.cpp
  ${adaptor_internal_dir}/system/common/kernel-trace.cpp
  ${adaptor_internal_dir}/system/common/locale-utils.cpp
  ${adaptor_internal_dir}/system/common/object-profiler.cpp
  ${adaptor_internal_dir}/system/common/performance-interface-factory.cpp
  ${adaptor_internal_dir}/system/common/performance-logger-impl.cpp
  ${adaptor_internal_dir}/system/common/performance-marker.cpp
  ${adaptor_internal_dir}/system/common/performance-server.cpp
  ${adaptor_internal_dir}/system/common/singleton-service-impl.cpp
  ${adaptor_internal_dir}/system/common/sound-player-impl.cpp
  ${adaptor_internal_dir}/system/common/stat-context.cpp
  ${adaptor_internal_dir}/system/common/stat-context-manager.cpp
  ${adaptor_internal_dir}/system/common/system-trace.cpp
  ${adaptor_internal_dir}/system/common/thread-controller.cpp
  ${adaptor_internal_dir}/system/common/time-service.cpp
  ${adaptor_internal_dir}/system/common/trigger-event-factory.cpp
  ${adaptor_internal_dir}/system/common/update-status-logger.cpp
  ${adaptor_internal_dir}/system/common/widget-application-impl.cpp
  ${adaptor_internal_dir}/system/common/logging.cpp
  ${adaptor_internal_dir}/system/generic/color-controller-impl.cpp
  ${adaptor_internal_dir}/system/generic/system-settings.cpp
  ${adaptor_internal_dir}/system/windows/callback-manager-win.cpp
  ${adaptor_internal_dir}/system/windows/file-closer-win.cpp
  ${adaptor_internal_dir}/system/windows/timer-impl-win.cpp
  ${adaptor_internal_dir}/system/windows/trigger-event.cpp
  ${adaptor_internal_dir}/system/windows/widget-application-impl-win.cpp
  ${adaptor_internal_dir}/system/windows/widget-controller-win.cpp
  
  #text
  ${adaptor_internal_dir}/text/text-abstraction/bidirectional-support-impl.cpp
  ${adaptor_internal_dir}/text/text-abstraction/font-client-helper.cpp
  ${adaptor_internal_dir}/text/text-abstraction/font-client-impl.cpp
  ${adaptor_internal_dir}/text/text-abstraction/font-client-plugin-impl.cpp
  ${adaptor_internal_dir}/text/text-abstraction/segmentation-impl.cpp
  ${adaptor_internal_dir}/text/text-abstraction/shaping-impl.cpp

  #trace
  ${adaptor_internal_dir}/trace/common/trace-factory.cpp
  ${adaptor_internal_dir}/trace/common/trace-manager-impl.cpp
  
  #video
  ${adaptor_internal_dir}/video/common/video-player-impl.cpp
  
  #window-system
  ${adaptor_internal_dir}/window-system/common/display-connection.cpp
  ${adaptor_internal_dir}/window-system/common/indicator-buffer.cpp
  ${adaptor_internal_dir}/window-system/common/native-render-surface-factory.cpp
  ${adaptor_internal_dir}/window-system/common/orientation-impl.cpp
  ${adaptor_internal_dir}/window-system/common/window-impl.cpp
  ${adaptor_internal_dir}/window-system/windows/display-connection-factory-win.cpp
  ${adaptor_internal_dir}/window-system/windows/display-connection-impl-win.cpp
  ${adaptor_internal_dir}/window-system/windows/event-handler-ecore-win.cpp
  ${adaptor_internal_dir}/window-system/windows/pixmap-render-surface-ecore-win.cpp
  ${adaptor_internal_dir}/window-system/windows/render-surface-factory-ecore-win.cpp
  ${adaptor_internal_dir}/window-system/windows/window-base-ecore-win.cpp
  ${adaptor_internal_dir}/window-system/windows/window-factory-ecore-win.cpp
  ${adaptor_internal_dir}/window-system/windows/window-render-surface-ecore-win.cpp
)

SET( adaptor_public_api_dir dali/public-api )

SET( SOURCES ${SOURCES}
  ${adaptor_public_api_dir}/adaptor-framework/application.cpp
  ${adaptor_public_api_dir}/adaptor-framework/key.cpp
  ${adaptor_public_api_dir}/adaptor-framework/window.cpp
  ${adaptor_public_api_dir}/adaptor-framework/timer.cpp
  ${adaptor_public_api_dir}/adaptor-framework/tts-player.cpp
  ${adaptor_public_api_dir}/adaptor-framework/timer.cpp
  ${adaptor_public_api_dir}/adaptor-framework/widget.cpp
  ${adaptor_public_api_dir}/adaptor-framework/timer.cpp
  ${adaptor_public_api_dir}/adaptor-framework/widget-application.cpp
  ${adaptor_public_api_dir}/adaptor-framework/timer.cpp
  ${adaptor_public_api_dir}/adaptor-framework/widget-impl.cpp
  ${adaptor_public_api_dir}/adaptor-framework/native-image-source.cpp
  ${adaptor_public_api_dir}/dali-adaptor-version.cpp
)

SET( SOURCES ${SOURCES}
  third-party/glyphy/glyphy-arcs.cc
  third-party/glyphy/glyphy-blob-impl.cc
  third-party/glyphy/glyphy-extents.cc
  third-party/glyphy/glyphy-outline.cc
  third-party/glyphy/glyphy-sdf.cc
  third-party/glyphy/vector-font-cache.cpp
  
  third-party/libunibreak/linebreak.c
  third-party/libunibreak/linebreakdata.c
  third-party/libunibreak/linebreakdef.c
  third-party/libunibreak/wordbreak.c
  
  third-party/resampler/resampler.cpp
)

ADD_DEFINITIONS(
-D "DALI_ADAPTOR_COMPILATION"
-D "_NSIG=65"
-D "FT_FACE_FLAG_COLOR=16384"
-D "NON_POWER_OF_TWO_TEXTURES"
-D "HAVE_STRUCT_TIMESPEC"
-D "CURL_STATICLIB=0"
-D "LC_MESSAGES=0"
/DBUILDING_DALI_ADAPTOR
/FI"${dali_SOURCE_DIR}/dali-adaptor/dali-windows-backend/ExInclude/PreprocessorDefinitions.h"
/FI"${dali_SOURCE_DIR}/dali-adaptor/dali-windows-backend/ExInclude/AdaptorPreDefine.h"
/NODEFAULTLIB:"libcmt.lib"
/vmg
/MP
)

link_directories(
${dali_SOURCE_DIR}/dali-adaptor/dali-windows-backend/ExLib
${dali_SOURCE_DIR}/bin
)

#head file path  
INCLUDE_DIRECTORIES(  
./
${dali_SOURCE_DIR}/dali-core
${dali_SOURCE_DIR}/dali-adaptor/dali-windows-backend/ExInclude
${dali_SOURCE_DIR}/dali-adaptor/dali-windows-backend/ExInclude/OpenGL/Include
)

set(LIBRARY_OUTPUT_PATH ${dali_SOURCE_DIR}/bin)

#set(BUILD_SHARED_LIBS "On")

#SET_TARGET_PROPERTIES (dali-core123_static PROPERTIES LINKER_LANGUAGE C++)

ADD_LIBRARY(dali-adaptor SHARED ${SOURCES})

target_link_libraries(dali-adaptor dali-core.lib)
target_link_libraries(dali-adaptor libjpeg.lib)
target_link_libraries(dali-adaptor libcurl_a.lib)
target_link_libraries(dali-adaptor turbojpeg.lib)
target_link_libraries(dali-adaptor getopt.lib)
target_link_libraries(dali-adaptor giflib.lib)
target_link_libraries(dali-adaptor libEGL.lib)
target_link_libraries(dali-adaptor libexif.lib)
target_link_libraries(dali-adaptor libGLESv2.lib)
target_link_libraries(dali-adaptor pthread.lib)
target_link_libraries(dali-adaptor libpng.lib)
target_link_libraries(dali-adaptor zlibstat.lib)
target_link_libraries(dali-adaptor freetype.lib)
target_link_libraries(dali-adaptor FontConfig.lib)
target_link_libraries(dali-adaptor libexpat.lib)
target_link_libraries(dali-adaptor fribidi.lib)
target_link_libraries(dali-adaptor harfbuzz-1.4.7.lib)
target_link_libraries(dali-adaptor dlfcn.lib)
target_link_libraries(dali-adaptor Win32File.lib)
target_link_libraries(dali-adaptor WindowsPlatform.lib)

endif()