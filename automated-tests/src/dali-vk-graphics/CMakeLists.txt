SET(PKG_NAME "dali-vk-graphics")

SET(EXEC_NAME "tct-${PKG_NAME}-core")
SET(RPM_NAME "core-${PKG_NAME}-tests")
MESSAGE(STATUS "ENABLE_VULKAN: ${ENABLE_VULKAN}")

SET(CAPI_LIB "dali-vk-graphics")

SET(TC_SOURCES
    utc-Dali-VkGraphicsBuffer.cpp
)

SET(TC_SOURCE_LIST ${TC_SOURCES} CACHE STRING "List of test sources")

LIST(APPEND TC_SOURCES
  ../dali-adaptor/dali-test-suite-utils/dali-test-suite-utils.cpp
  ../dali-adaptor/dali-test-suite-utils/mesh-builder.cpp
  ../dali-adaptor/dali-test-suite-utils/test-harness.cpp
  ../dali-adaptor/dali-test-suite-utils/test-actor-utils.cpp
  ../dali-adaptor/dali-test-suite-utils/test-graphics-vk-application.cpp
  ../dali-adaptor/dali-test-suite-utils/test-vk-abstraction.cpp
  ../dali-adaptor/dali-test-suite-utils/test-graphics-sync-impl.cpp
  ../dali-adaptor/dali-test-suite-utils/test-graphics-sync-object.cpp

  ../dali-adaptor/dali-test-suite-utils/test-native-image.cpp
  ../dali-adaptor/dali-test-suite-utils/test-platform-abstraction.cpp
  ../dali-adaptor/dali-test-suite-utils/test-render-controller.cpp
  ../dali-adaptor/dali-test-suite-utils/test-render-surface.cpp
  ../dali-adaptor/dali-test-suite-utils/test-trace-call-stack.cpp
  ../dali-adaptor/dali-test-suite-utils/adaptor-test-adaptor-impl.cpp
  tct-dali-vk-graphics-core.cpp
)

PKG_CHECK_MODULES(${CAPI_LIB} REQUIRED
    dali2-core
    dali2-adaptor
    vulkan
)

ADD_COMPILE_OPTIONS(-Wno-switch -Wno-switch-enum -Wno-error=switch -Wno-error=switch-enum)
ADD_COMPILE_OPTIONS(-Wno-init-list-lifetime)
INCLUDE(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(-Wno-class-memaccess HAVE_NO_CLASS_MEMACCESS)
IF (HAVE_NO_CLASS_MEMACCESS)
  ADD_COMPILE_OPTIONS( -Wno-class-memaccess )
ENDIF()

ADD_COMPILE_OPTIONS( -O0 -ggdb --coverage -Wall -Werror )
ADD_COMPILE_OPTIONS( ${${CAPI_LIB}_CFLAGS_OTHER} )

ADD_DEFINITIONS(-DTEST_RESOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../../resources\" )

FOREACH(directory ${${CAPI_LIB}_LIBRARY_DIRS})
    SET(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -L${directory}")
ENDFOREACH(directory ${CAPI_LIB_LIBRARY_DIRS})

INCLUDE_DIRECTORIES(
  ../../../
  ../../../dali/integration-api/adaptor-framework
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
