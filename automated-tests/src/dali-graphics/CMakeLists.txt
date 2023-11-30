SET(PKG_NAME "dali-graphics")

SET(EXEC_NAME "tct-${PKG_NAME}-core")
SET(RPM_NAME "core-${PKG_NAME}-tests")

SET(CAPI_LIB "dali-graphics")
SET(TC_SOURCES
    utc-Dali-GlImplementation.cpp
    utc-Dali-GlesImplementation.cpp
    utc-Dali-GraphicsBuffer.cpp
    utc-Dali-GraphicsDraw.cpp
    utc-Dali-GraphicsFramebuffer.cpp
    utc-Dali-GraphicsGeometry.cpp
    utc-Dali-GraphicsNativeImage.cpp
    utc-Dali-GraphicsProgram.cpp
    utc-Dali-GraphicsSampler.cpp
    utc-Dali-GraphicsTexture.cpp
)

LIST(APPEND TC_SOURCES
  ../dali-adaptor/dali-test-suite-utils/dali-test-suite-utils.cpp
  ../dali-adaptor/dali-test-suite-utils/mesh-builder.cpp
  ../dali-adaptor/dali-test-suite-utils/test-harness.cpp
  ../dali-adaptor/dali-test-suite-utils/test-actor-utils.cpp
  ../dali-adaptor/dali-test-suite-utils/test-graphics-application.cpp
  ../dali-adaptor/dali-test-suite-utils/test-gl-abstraction.cpp

  ../dali-adaptor/dali-test-suite-utils/test-graphics-sync-impl.cpp
  ../dali-adaptor/dali-test-suite-utils/test-graphics-sync-object.cpp

  ../dali-adaptor/dali-test-suite-utils/test-native-image.cpp
  ../dali-adaptor/dali-test-suite-utils/test-platform-abstraction.cpp
  ../dali-adaptor/dali-test-suite-utils/test-render-controller.cpp
  ../dali-adaptor/dali-test-suite-utils/test-trace-call-stack.cpp
  ../dali-adaptor/dali-test-suite-utils/adaptor-test-adaptor-impl.cpp
  tct-dali-graphics-core.cpp
)

PKG_CHECK_MODULES(${CAPI_LIB} REQUIRED
    dali2-core
    dali2-adaptor
    glesv2
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
  ../dali-adaptor/dali-test-suite-utils
)

ADD_EXECUTABLE(${EXEC_NAME} ${EXEC_NAME}.cpp ${TC_SOURCES})
TARGET_LINK_LIBRARIES(${EXEC_NAME}
  ${${CAPI_LIB}_LIBRARIES}
  --coverage
)

INSTALL(PROGRAMS ${EXEC_NAME}
    DESTINATION ${BIN_DIR}/${EXEC_NAME}
)
