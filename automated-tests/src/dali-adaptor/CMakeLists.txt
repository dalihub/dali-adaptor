SET(PKG_NAME "dali-adaptor")

SET(EXEC_NAME "tct-${PKG_NAME}-core")
SET(RPM_NAME "core-${PKG_NAME}-tests")

SET(CAPI_LIB "dali-adaptor")
SET(TC_SOURCES
    utc-Dali-Application.cpp
    utc-Dali-EncodedImageBuffer.cpp
    utc-Dali-Capture.cpp
    utc-Dali-WindowData.cpp
    utc-Dali-FileLoader.cpp
    utc-Dali-GifLoading.cpp
    utc-Dali-Gl-Window.cpp
    utc-Dali-ImageLoading.cpp
    utc-Dali-Key.cpp
    utc-Dali-NativeImageSource.cpp
    utc-Dali-PixelBuffer.cpp
    utc-Dali-TextScript.cpp
    utc-Dali-Timer.cpp
    utc-Dali-TtsPlayer.cpp
    utc-Dali-WidgetApplication.cpp
    utc-Dali-Widget.cpp
    utc-Dali-Window.cpp
    utc-Dali-WindowSystem.cpp
)

LIST(APPEND TC_SOURCES
    dali-test-suite-utils/mesh-builder.cpp
    dali-test-suite-utils/dali-test-suite-utils.cpp
    dali-test-suite-utils/test-actor-utils.cpp
    dali-test-suite-utils/test-harness.cpp
    dali-test-suite-utils/test-application.cpp
    dali-test-suite-utils/test-gesture-generator.cpp
    dali-test-suite-utils/test-gl-abstraction.cpp
    dali-test-suite-utils/test-graphics-sync-impl.cpp
    dali-test-suite-utils/test-graphics-sync-object.cpp
    dali-test-suite-utils/test-graphics-buffer.cpp
    dali-test-suite-utils/test-graphics-command-buffer.cpp
    dali-test-suite-utils/test-graphics-controller.cpp
    dali-test-suite-utils/test-graphics-framebuffer.cpp
    dali-test-suite-utils/test-graphics-pipeline.cpp
    dali-test-suite-utils/test-graphics-reflection.cpp
    dali-test-suite-utils/test-graphics-texture.cpp
    dali-test-suite-utils/test-graphics-sampler.cpp
    dali-test-suite-utils/test-graphics-shader.cpp
    dali-test-suite-utils/test-graphics-program.cpp
    dali-test-suite-utils/test-native-image.cpp
    dali-test-suite-utils/test-platform-abstraction.cpp
    dali-test-suite-utils/test-render-controller.cpp
    dali-test-suite-utils/test-trace-call-stack.cpp
    dali-test-suite-utils/adaptor-test-adaptor-impl.cpp
    dali-test-suite-utils/adaptor-environment-variable.cpp
)

PKG_CHECK_MODULES(${CAPI_LIB} REQUIRED
    dali2-core
    dali2-adaptor
    ecore
    ecore-x
)

ADD_COMPILE_OPTIONS( -O0 -ggdb --coverage -Wall -Werror )
ADD_COMPILE_OPTIONS( ${${CAPI_LIB}_CFLAGS_OTHER} )

ADD_DEFINITIONS(-DTEST_RESOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../../resources\" )

FOREACH(directory ${${CAPI_LIB}_LIBRARY_DIRS})
    SET(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -L${directory}")
ENDFOREACH(directory ${CAPI_LIB_LIBRARY_DIRS})

INCLUDE_DIRECTORIES(
    ../../../
    ${${CAPI_LIB}_INCLUDE_DIRS}
    dali-test-suite-utils
)

ADD_EXECUTABLE(${EXEC_NAME} ${EXEC_NAME}.cpp ${TC_SOURCES})
TARGET_LINK_LIBRARIES(${EXEC_NAME}
  ${${CAPI_LIB}_LIBRARIES}
  --coverage
)

INSTALL(PROGRAMS ${EXEC_NAME}
    DESTINATION ${BIN_DIR}/${EXEC_NAME}
)
