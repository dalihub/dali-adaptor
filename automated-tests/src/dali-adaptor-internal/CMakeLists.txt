SET(PKG_NAME "dali-adaptor-internal")

SET(EXEC_NAME "tct-${PKG_NAME}-core")
SET(RPM_NAME "core-${PKG_NAME}-tests")

SET(CAPI_LIB "dali-adaptor-internal")

SET(TC_SOURCES
    utc-Dali-AddOns.cpp
    utc-Dali-BmpLoader.cpp
    utc-Dali-CommandLineOptions.cpp
    utc-Dali-CompressedTextures.cpp
    utc-Dali-FontClient.cpp
    utc-Dali-GifLoader.cpp
    utc-Dali-IcoLoader.cpp
    utc-Dali-ImageOperations.cpp
    utc-Dali-Internal-PixelBuffer.cpp
    utc-Dali-Lifecycle-Controller.cpp
    utc-Dali-LRUCacheContainer.cpp
    utc-Dali-TiltSensor.cpp
    utc-Dali-WbmpLoader.cpp
)


LIST(APPEND TC_SOURCES
    image-loaders.cpp
    ../dali-adaptor/dali-test-suite-utils/mesh-builder.cpp
    ../dali-adaptor/dali-test-suite-utils/dali-test-suite-utils.cpp
    ../dali-adaptor/dali-test-suite-utils/test-actor-utils.cpp
    ../dali-adaptor/dali-test-suite-utils/test-harness.cpp
    ../dali-adaptor/dali-test-suite-utils/test-application.cpp
    ../dali-adaptor/dali-test-suite-utils/test-gl-abstraction.cpp
    ../dali-adaptor/dali-test-suite-utils/test-graphics-sync-impl.cpp
    ../dali-adaptor/dali-test-suite-utils/test-graphics-sync-object.cpp
    ../dali-adaptor/dali-test-suite-utils/test-graphics-buffer.cpp
    ../dali-adaptor/dali-test-suite-utils/test-graphics-command-buffer.cpp
    ../dali-adaptor/dali-test-suite-utils/test-graphics-controller.cpp
    ../dali-adaptor/dali-test-suite-utils/test-graphics-framebuffer.cpp
    ../dali-adaptor/dali-test-suite-utils/test-graphics-texture.cpp
    ../dali-adaptor/dali-test-suite-utils/test-graphics-pipeline.cpp
    ../dali-adaptor/dali-test-suite-utils/test-graphics-reflection.cpp
    ../dali-adaptor/dali-test-suite-utils/test-graphics-sampler.cpp
    ../dali-adaptor/dali-test-suite-utils/test-graphics-shader.cpp
    ../dali-adaptor/dali-test-suite-utils/test-graphics-program.cpp
    ../dali-adaptor/dali-test-suite-utils/test-native-image.cpp
    ../dali-adaptor/dali-test-suite-utils/test-platform-abstraction.cpp
    ../dali-adaptor/dali-test-suite-utils/test-render-controller.cpp
    ../dali-adaptor/dali-test-suite-utils/test-trace-call-stack.cpp
    ../dali-adaptor/dali-test-suite-utils/adaptor-test-adaptor-impl.cpp
)

PKG_CHECK_MODULES(${CAPI_LIB} REQUIRED
    dali2-core
    dali2-adaptor
    freetype2>=9.16.3
    ecore
    ecore-x
    glesv2
)

ADD_COMPILE_OPTIONS( -O0 -ggdb --coverage -Wall -Werror )
ADD_COMPILE_OPTIONS( ${${CAPI_LIB}_CFLAGS_OTHER} )

ADD_DEFINITIONS(-DTEST_IMAGE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../../images\" )
ADD_DEFINITIONS(-DTEST_RESOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../../resources\" )
ADD_DEFINITIONS(-DADDON_LIBS_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\" )


FOREACH(directory ${${CAPI_LIB}_LIBRARY_DIRS})
    SET(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -L${directory}")
ENDFOREACH(directory ${CAPI_LIB_LIBRARY_DIRS})

# Adaptor directories are included in order of most-specific to least specific:
INCLUDE_DIRECTORIES(
    ../../../
    ../../../third-party/image-resampler
    ${${CAPI_LIB}_INCLUDE_DIRS}
    ../dali-adaptor/dali-test-suite-utils
)

ADD_EXECUTABLE(${EXEC_NAME} ${EXEC_NAME}.cpp ${TC_SOURCES})
TARGET_LINK_LIBRARIES(${EXEC_NAME}
    ${${CAPI_LIB}_LIBRARIES}
    -lpthread --coverage -rdynamic
)

INSTALL(PROGRAMS ${EXEC_NAME}
    DESTINATION ${BIN_DIR}/${EXEC_NAME}
)

# build addons
MESSAGE( STATUS "BINDIR: ${CMAKE_CURRENT_BINARY_DIR}")
SET(ADDON_NAME SampleAddOn )
SET(ADDON_SOURCES addons/test-sample-addon.cpp )
ADD_LIBRARY( ${ADDON_NAME} SHARED ${ADDON_SOURCES} )
TARGET_LINK_LIBRARIES(${ADDON_NAME}
        -lpthread -ldl --coverage -rdynamic
        )

INSTALL( TARGETS ${ADDON_NAME} DESTINATION ${BIN_DIR} )

# store AddOn list
FILE( WRITE ${CMAKE_CURRENT_BINARY_DIR}/addons.txt lib${ADDON_NAME}.so )
