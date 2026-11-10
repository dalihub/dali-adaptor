cmake_minimum_required(VERSION 3.10)
project(dali-accessibility-navigation-tests LANGUAGES CXX)

# Tests the production sorting helper without linking a DALi runtime or DBus.
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_search_module(DALI_CORE QUIET dali3-core dali2-core)
endif()
find_path(DALI_CORE_INCLUDE_DIR dali/public-api/math/rect.h
  HINTS ${DALI_CORE_INCLUDE_DIRS} "$ENV{DESKTOP_PREFIX}/include"
  PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../../../../dali-core"
  PATH_SUFFIXES dali3 dali2)
if(NOT DALI_CORE_INCLUDE_DIR)
  message(FATAL_ERROR "Set DALI_CORE_INCLUDE_DIR to a dali-core source or include directory")
endif()

set(TC_SOURCES test-accessibility-navigation.cpp)
set(TC_SOURCE_LIST ${TC_SOURCES} CACHE STRING "List of test sources" FORCE)
add_executable(test-accessibility-navigation
  ${TC_SOURCES}
  ../dali-adaptor/dali-test-suite-utils/test-harness.cpp)
set_target_properties(test-accessibility-navigation PROPERTIES
  OUTPUT_NAME tct-accessibility-navigation-core)
find_package(Threads REQUIRED)
target_link_libraries(test-accessibility-navigation PRIVATE Threads::Threads)
target_compile_features(test-accessibility-navigation PRIVATE cxx_std_17)
target_include_directories(test-accessibility-navigation PRIVATE
  ../../.. ../common ../dali-adaptor/dali-test-suite-utils ${DALI_CORE_INCLUDE_DIR})
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(test-accessibility-navigation PRIVATE -Wall -Wextra -Werror)
endif()
option(ENABLE_SANITIZERS "Run with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(test-accessibility-navigation PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_libraries(test-accessibility-navigation PRIVATE -fsanitize=address,undefined)
endif()

enable_testing()
foreach(test ChangingBounds HorizontalSnapshot ZeroArea InvalidBounds LineGrouping Ties FreshSnapshot LargeFinite Empty GetterFailure)
  add_test(NAME navigation-${test} COMMAND test-accessibility-navigation ${test})
  set_tests_properties(navigation-${test} PROPERTIES TIMEOUT 10)
endforeach()
