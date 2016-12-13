cmake_minimum_required(VERSION 2.6)

if( ${EMSCRIPTEN} )
  message("EMSCRIPTEN BUILD")
  set(CMAKE_C_COMPILER "emcc")
  set(CMAKE_CXX_COMPILER "em++")
else( ${EMSCRIPTEN} )
  message( FATAL_ERROR "Native Build not supported via cmake." )
endif( ${EMSCRIPTEN} )

if(NOT DEFINED ENV{DESKTOP_PREFIX})
  message( FATAL_ERROR "DESKTOP_PREFIX is required to build adaptor against dali-core. Please make sure you have sourced your setenv script (created by dali_env)." )
endif()

project(dali-emscripten CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(SRCS
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/actor-wrapper.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/animation-wrapper.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/dali-wrapper.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/emscripten-utils.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/handle-wrapper.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/image-wrapper.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/property-buffer-wrapper.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/property-value-wrapper.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/render-task-wrapper.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/type-info-wrapper.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/base/separate-update-render/frame-time.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/base/time-service.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/common/gl/egl-image-extensions.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/common/gl/gl-extensions.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/egl-implementation-emscripten.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/main.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/sdl-application.cpp
  ${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/sdl-gl-sync-abstraction.cpp
  ${CMAKE_SOURCE_DIR}/../../platform-abstractions/emscripten/emscripten-callbacks.cpp
  ${CMAKE_SOURCE_DIR}/../../platform-abstractions/emscripten/emscripten-platform-abstraction.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/../../)
include_directories(${CMAKE_SOURCE_DIR}/../../adaptors/)
include_directories(${CMAKE_SOURCE_DIR}/../../adaptors/common)
include_directories(${CMAKE_SOURCE_DIR}/../../adaptors/tizen)
include_directories(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten)
include_directories(${CMAKE_SOURCE_DIR}/../../platform-abstractions/slp)

configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/all.html all.html COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/dali-page.html dali-page.html COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/dali-tests.css dali-tests.css COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/dali-tests.html dali-tests.html COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/geometry.js geometry.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/properties.js properties.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/signals.js signals.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/math.js math.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/animation.js animation.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/utilities.js utilities.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/shaders.js shaders.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/views.js views.js COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/qunit/qunit-1.21.0.css qunit-1.21.0.css COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/qunit/qunit-1.21.0.js qunit-1.21.0.js COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/dali-wrapper.js dali-wrapper.js COPYONLY)

set(VENDOR "samsung")
set(PACKAGE ${PROJECT_NAME})
set(DESKTOP_PREFIX $ENV{DESKTOP_PREFIX})
set(CPP_DEFINES -DDALI_GLES_VERSION=2)

set(EMSCRIPTEN_ENV_DIR ${DESKTOP_PREFIX}/share/emscripten)

set(DEBUG_FLAGS "-Wall -g -O2")
set(RELEASE_FLAGS "-Wall -g -O2")

if( ${EMSCRIPTEN} )
 set(DEBUG_FLAGS "${DEBUG_FLAGS} -std=c++11 -DBOOST_ERROR_CODE_HEADER_ONLY")
 set(RELEASE_FLAGS "${RELEASE_FLAGS} -std=c++11 -DBOOST_ERROR_CODE_HEADER_ONLY")
endif( ${EMSCRIPTEN} )


include(FindPkgConfig)

pkg_check_modules(pkgs REQUIRED
  dali-core
  # sdl from emscripten
  )

if( ${EMSCRIPTEN} )

pkg_check_modules(pkgs REQUIRED
  dali-core
  # sdl from emscripten
  )

else( ${EMSCRIPTEN} )

find_library(SDL VERSION "1.2" REQUIRED HINTS $DESKTOP_PREFIX/lib)
find_library(SDL REQUIRED)
find_library(SDL_image REQUIRED)

find_library(jpeg REQUIRED)
find_library(ft2build REQUIRED)
find_library(turbojpeg REQUIRED)

pkg_check_modules(pkgs REQUIRED
  dali-core
  # sdl from emscripten
  sdl
  egl
  )

include_directories(/usr/local/include/SDL/)
set(LIBS ${LIBS} -lSDL -lSDL_image -lEGL -lGLESv2)

endif( ${EMSCRIPTEN} )

# non pkg config
include_directories(${DESKTOP_PREFIX}/include/)

add_definitions( ${CPP_DEFINES} ) # see configure_file() for *.in to *.h style

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${RELEASE_FLAGS}")

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS} ${LIBS}) # "-lm"

# Copy dali-wrapper.js to dali-env
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/dali-wrapper.js ${EMSCRIPTEN_ENV_DIR}/dali-wrapper.js COPYONLY)

# tests
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/all.html ${EMSCRIPTEN_ENV_DIR}/all.html COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/dali-page.html ${EMSCRIPTEN_ENV_DIR}/dali-page.html COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/dali-tests.css ${EMSCRIPTEN_ENV_DIR}/dali-tests.css COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/dali-tests.html ${EMSCRIPTEN_ENV_DIR}/dali-tests.html COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/geometry.js ${EMSCRIPTEN_ENV_DIR}/geometry.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/properties.js ${EMSCRIPTEN_ENV_DIR}/properties.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/signals.js ${EMSCRIPTEN_ENV_DIR}/signals.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/math.js ${EMSCRIPTEN_ENV_DIR}/math.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/animation.js ${EMSCRIPTEN_ENV_DIR}/animation.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/utilities.js ${EMSCRIPTEN_ENV_DIR}/utilities.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/shaders.js ${EMSCRIPTEN_ENV_DIR}/shaders.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/views.js ${EMSCRIPTEN_ENV_DIR}/views.js COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/qunit/qunit-1.21.0.css ${EMSCRIPTEN_ENV_DIR}/qunit-1.21.0.css COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/../../adaptors/emscripten/wrappers/tests/qunit/qunit-1.21.0.js ${EMSCRIPTEN_ENV_DIR}/qunit-1.21.0.js COPYONLY)
