CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

PROJECT(dali-adaptor-common-tests LANGUAGES CXX)

IF(NOT WIN32)
  MESSAGE(FATAL_ERROR "The DALi adaptor common Windows tests must be built for Windows.")
ENDIF()

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

INCLUDE(GNUInstallDirs)

FIND_PACKAGE(dali2-core CONFIG REQUIRED)
FIND_PACKAGE(dali2-adaptor CONFIG REQUIRED)

SET(COMMON_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src/dali-adaptor")
SET(DALI_TEST_SOURCE_LIST_ONLY ON)
INCLUDE("${COMMON_TEST_DIR}/CMakeLists.txt")
UNSET(DALI_TEST_SOURCE_LIST_ONLY)

SET(COMMON_TC_SOURCES ${TC_SOURCE_LIST})
SET(COMMON_ALL_SOURCES ${TC_SOURCES})
LIST(REMOVE_ITEM COMMON_ALL_SOURCES dali-test-suite-utils/test-harness.cpp)
LIST(TRANSFORM COMMON_TC_SOURCES PREPEND "${COMMON_TEST_DIR}/")
LIST(TRANSFORM COMMON_ALL_SOURCES PREPEND "${COMMON_TEST_DIR}/")

SET(GENERATED_HEADER "${CMAKE_CURRENT_BINARY_DIR}/dali-adaptor-common-testcases.h")
SET(TEST_DECLARATIONS "")
SET(TEST_ENTRIES "")

FOREACH(TEST_SOURCE IN LISTS COMMON_TC_SOURCES)
  FILE(READ "${TEST_SOURCE}" TEST_CONTENT)
  STRING(REGEX MATCH "void[ \t]+[A-Za-z0-9_]+_startup\\(void\\)" STARTUP_MATCH "${TEST_CONTENT}")
  STRING(REGEX MATCH "void[ \t]+[A-Za-z0-9_]+_cleanup\\(void\\)" CLEANUP_MATCH "${TEST_CONTENT}")
  STRING(REGEX REPLACE "^void[ \t]+|\\(void\\)$" "" STARTUP_NAME "${STARTUP_MATCH}")
  STRING(REGEX REPLACE "^void[ \t]+|\\(void\\)$" "" CLEANUP_NAME "${CLEANUP_MATCH}")
  IF(STARTUP_NAME STREQUAL "")
    SET(STARTUP_NAME "nullptr")
  ELSE()
    STRING(APPEND TEST_DECLARATIONS "extern void ${STARTUP_NAME}(void);\n")
  ENDIF()
  IF(CLEANUP_NAME STREQUAL "")
    SET(CLEANUP_NAME "nullptr")
  ELSE()
    STRING(APPEND TEST_DECLARATIONS "extern void ${CLEANUP_NAME}(void);\n")
  ENDIF()

  STRING(REGEX MATCHALL "int[ \t]+Utc[A-Za-z0-9_]+\\(void\\)" TEST_MATCHES "${TEST_CONTENT}")
  FOREACH(TEST_MATCH IN LISTS TEST_MATCHES)
    STRING(REGEX REPLACE "^int[ \t]+|\\(void\\)$" "" TEST_NAME "${TEST_MATCH}")
    STRING(APPEND TEST_DECLARATIONS "extern int ${TEST_NAME}(void);\n")
    STRING(APPEND TEST_ENTRIES "  {\"${TEST_NAME}\", ${TEST_NAME}, ${STARTUP_NAME}, ${CLEANUP_NAME}},\n")
  ENDFOREACH()
ENDFOREACH()

FILE(WRITE "${GENERATED_HEADER}"
  "#pragma once\n"
  "#include <testcase.h>\n\n"
  "${TEST_DECLARATIONS}\n"
  "inline testcase gDaliAdaptorCommonTests[] = {\n"
  "${TEST_ENTRIES}"
  "  {nullptr, nullptr, nullptr, nullptr}\n"
  "};\n")

ADD_EXECUTABLE(tct-dali-adaptor-core
  src/common-test-main.cpp
  src/test-framework-factory.cpp
  ${COMMON_ALL_SOURCES}
)

TARGET_INCLUDE_DIRECTORIES(tct-dali-adaptor-core PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/../src/common"
  "${COMMON_TEST_DIR}/dali-test-suite-utils"
  "${CMAKE_CURRENT_SOURCE_DIR}/../.."
  "${CMAKE_CURRENT_SOURCE_DIR}/../../dali/integration-api/adaptor-framework"
)

TARGET_COMPILE_DEFINITIONS(tct-dali-adaptor-core PRIVATE
  TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../resources"
  __restrict__=__restrict
  NOMINMAX
  WIN32_LEAN_AND_MEAN
)

TARGET_COMPILE_OPTIONS(tct-dali-adaptor-core PRIVATE /vmg /utf-8 /W3)

TARGET_LINK_LIBRARIES(tct-dali-adaptor-core PRIVATE
  dali2-adaptor::dali2-adaptor
  dali2-core::dali2-core
)

INSTALL(TARGETS tct-dali-adaptor-core RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
