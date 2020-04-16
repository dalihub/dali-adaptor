CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
CMAKE_POLICY(SET CMP0012 NEW) # Prevent dereferencing of OFF/ON as variables

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_C_STANDARD 99)
PROJECT(${name} CXX)
SET(PKG_NAME ${name})

SET(name "dali-adaptor")

SET(GCC_COMPILER_VERSION_REQUIRED "6")
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  IF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS GCC_COMPILER_VERSION_REQUIRED)
    MESSAGE(FATAL_ERROR "The GCC required compiler version is " ${GCC_COMPILER_VERSION_REQUIRED})
  ENDIF()
ENDIF()

OPTION(ENABLE_PKG_CONFIGURE  "Use pkgconfig" ON)
OPTION(ENABLE_LINK_TEST      "Enable the link test" ON)

# Include additional macros
INCLUDE( common.cmake )

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  MESSAGE( STATUS "CMAKE_BUILD_TYPE: " Debug )
  SET( ENABLE_DEBUG "ON" CACHE STRING "" )
  SET( ENABLE_EXPORTALL "ON" CACHE STRING "" )
ELSE()
  MESSAGE( STATUS "CMAKE_BUILD_TYPE: " Release )
ENDIF()

# from root/build/tizen, get back to root.
# set the variable only if it's not set yet. it allows to access build script
# from top-level directory
IF( NOT ROOT_SRC_DIR )
  SET(ROOT_SRC_DIR ${CMAKE_SOURCE_DIR}/../..)
ENDIF()
# Make sure the path is absolute
GET_FILENAME_COMPONENT(ROOT_SRC_DIR ${ROOT_SRC_DIR} ABSOLUTE)

# API VERSION (Not DALi release version)
SET(${name}_VERSION_MAJOR 0)
SET(${name}_VERSION_MINOR 0)
SET(${name}_VERSION_PATCH 0)
SET(${name}_VERSION ${${name}_VERSION_MAJOR}.${${name}_VERSION_MINOR}.${${name}_VERSION_PATCH})

SET(DALI_ADAPTOR_VERSION ${${name}_VERSION} )

IF( WIN32 ) # WIN32 includes x64 as well according to the cmake doc.
  FIND_PACKAGE( dali-windows-dependencies REQUIRED)
  FIND_PACKAGE( dali-core REQUIRED)
ENDIF()

SET( VCPKG_INCLUDE_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")

IF( UNIX )
  ADD_DEFINITIONS(-DPLATFORM_TIZEN)
  ADD_DEFINITIONS(-DPIC -DSTDC_HEADERS)

  IF("${ARCH}" STREQUAL "arm")
    ADD_DEFINITIONS("-DTARGET")
  ENDIF("${ARCH}" STREQUAL "arm")
ELSEIF( WIN32 ) # WIN32 includes x64 as well according to the cmake doc.
  IF(NOT DEFINED STATIC)
    ADD_DEFINITIONS( "-DBUILDING_DALI_ADAPTOR" )
  ENDIF()

  ADD_DEFINITIONS( "-DDALI_ADAPTOR_COMPILATION" )
  ADD_DEFINITIONS( "-D_NSIG=65" )
ENDIF()

# Deployment folder should come from spec file or command line:
SET( PREFIX ${CMAKE_INSTALL_PREFIX})
SET( EXEC_PREFIX ${CMAKE_INSTALL_PREFIX})

# Set up the include dir
SET( INCLUDE_DIR $ENV{includedir} )
IF( NOT INCLUDE_DIR )
  SET( INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR} )
ENDIF()
IF( NOT INCLUDE_DIR )
  SET( INCLUDE_DIR ${PREFIX}/include )
ENDIF()

# Set up the lib dir
SET( LIB_DIR $ENV{libdir} )
IF( NOT LIB_DIR )
  SET( LIB_DIR ${CMAKE_INSTALL_LIBDIR} )
ENDIF()
IF( NOT LIB_DIR )
  SET( LIB_DIR ${PREFIX}/lib )
ENDIF()

# Set up the bin dir
SET( BIN_DIR $ENV{bindir} )
IF( NOT BIN_DIR )
  SET( BIN_DIR ${CMAKE_INSTALL_BINDIR} )
ENDIF()
IF( NOT BIN_DIR )
  SET( BIN_DIR ${PREFIX}/bin )
ENDIF()

IF( UNIX )
  INCLUDE( deps-check.cmake )
ENDIF()

# Set up compiler flags and warnings
IF( UNIX )
  ADD_COMPILE_OPTIONS( -Wall ${DALI_CFLAGS} )# -Wextra -Wno-unused-parameter )# -Wfloat-equal )
ELSEIF( WIN32 ) # WIN32 includes x64 as well according to the cmake doc.
  ADD_COMPILE_OPTIONS( /FIdali-windows-dependencies.h ) #
  ADD_COMPILE_OPTIONS( /FIextern-definitions.h )        # Adds missing definitions.
  ADD_COMPILE_OPTIONS( /FIpreprocessor-definitions.h )  #
  ADD_COMPILE_OPTIONS( /vmg )                           # Avoids a 'reinterpret_cast' compile error while compiling signals and callbacks.
  ADD_COMPILE_OPTIONS( /wd4251 )                        # Ignores warning C4251: "'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'"
ENDIF()

ADD_DEFINITIONS( -DNON_POWER_OF_TWO_TEXTURES
                 ${DEFINITIONS} )

# from root/build/tizen, get back to root
SET(ROOT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)

# Make sure the path is absolute
GET_FILENAME_COMPONENT(ROOT_SRC_DIR ${ROOT_SRC_DIR} ABSOLUTE)

SET(SOURCE_DIR "${ROOT_SRC_DIR}/dali")
SET(ADAPTOR_ROOT ${ROOT_SRC_DIR})
SET(PACKAGE_DOXY_SRC_DIR "${ROOT_SRC_DIR}/doc")
SET(SOURCES "")

# Include list of modules
INCLUDE( module-list.cmake )

# Include profile specific setup
INCLUDE( profiles/${PROFILE_LCASE}-profile.cmake )

IF( ENABLE_PKG_CONFIGURE )
  # Configure the pkg-config file
  # Requires the following variables to be setup:
  # @PREFIX@ @EXEC_PREFIX@ @DALI_ADAPTOR_VERSION@ @LIB_DIR@ @DEV_INCLUDE_PATH@
  SET( DEV_INCLUDE_PATH ${INCLUDE_DIR} )
  SET( ADAPTOR_PKG_CFG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/dali-adaptor.pc )
  SET( ADAPTOR_INTEGRATION_PKG_CFG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/dali-adaptor-integration.pc )
  CONFIGURE_FILE( ${ADAPTOR_PKG_CFG_FILE}.in ${ADAPTOR_PKG_CFG_FILE} @ONLY )
  CONFIGURE_FILE( ${ADAPTOR_INTEGRATION_PKG_CFG_FILE}.in ${ADAPTOR_INTEGRATION_PKG_CFG_FILE} @ONLY )
ENDIF()

CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/system-cache-path.in ${ADAPTOR_ROOT}/dali/internal/adaptor/common/system-cache-path.cpp @ONLY)

SET( THIRD_PARTY_WINDOWS_PLATFORM "" )
IF( WIN32 )
  SET( THIRD_PARTY_WINDOWS_PLATFORM ${ROOT_SRC_DIR}/third-party/windows-platform )
ENDIF()

INCLUDE_DIRECTORIES(
  ${ROOT_SRC_DIR}
  ${PROFILE_INCLUDE_DIRECTORIES}
  ${THIRD_PARTY_WINDOWS_PLATFORM}
  ${VCPKG_INCLUDE_DIR}
  ${INCLUDE_DIR}
)

SET(LIBTYPE SHARED)
IF(DEFINED STATIC)
  SET(LIBTYPE STATIC)
ENDIF()

ADD_LIBRARY( ${name} ${LIBTYPE} ${SOURCES} )

IF( UNIX )
  # pthread required due to gcc issue
  FIND_LIBRARY( PTHREAD pthread )
  IF( PTHREAD AND NOT ANDROID_PROFILE )
    SET( OPTIONAL_LIBS ${OPTIONAL_LIBS} -lpthread )
  ENDIF()
ENDIF()

TARGET_LINK_LIBRARIES( ${name}
                       ${DALI_LDFLAGS}
  ${REQUIRED_LIBS}
  ${OPTIONAL_LIBS}
  ${COVERAGE}
)

SET_TARGET_PROPERTIES( ${name}
  PROPERTIES
  VERSION ${DALI_ADAPTOR_VERSION}
  SOVERSION ${${name}_VERSION_MAJOR}
  CLEAN_DIRECT_OUPUT 1
)

IF( INSTALL_CMAKE_MODULES )
  IF( ENABLE_DEBUG )
    SET( BIN_DIR "${BIN_DIR}/debug" )
    SET( LIB_DIR "${LIB_DIR}/debug" )
  ENDIF()

  # Install the library files.
  INSTALL( TARGETS ${name}
    EXPORT ${name}-targets
    LIBRARY DESTINATION ${LIB_DIR}
    ARCHIVE DESTINATION ${LIB_DIR}
    RUNTIME DESTINATION ${BIN_DIR}
  )

  # Install the cmake modules.
  INSTALL(
    EXPORT ${name}-targets
    NAMESPACE ${name}::
    FILE ${name}-targets.cmake
    DESTINATION share/${name}
  )

  FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${name}-config.cmake "
    include(CMakeFindDependencyMacro)
    include(\${CMAKE_CURRENT_LIST_DIR}/${name}-targets.cmake)
  ")
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${name}-config.cmake DESTINATION share/${name})

  # Install the pdb file.
  IF( ENABLE_DEBUG )
    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${name}.pdb DESTINATION ${BIN_DIR} )
  ENDIF()
ELSE()
  # Install the library so file and symlinks
  INSTALL( TARGETS ${name} DESTINATION ${LIB_DIR} )
ENDIF()

IF( ENABLE_PKG_CONFIGURE )
  # Install the pkg-config file
  INSTALL( FILES ${ADAPTOR_PKG_CFG_FILE} DESTINATION ${LIB_DIR}/pkgconfig )
  INSTALL( FILES ${ADAPTOR_INTEGRATION_PKG_CFG_FILE} DESTINATION ${LIB_DIR}/pkgconfig )
ENDIF()

# macro for installing headers by replacing prefix. (TODO, investigate
# if there is a CMAKE way of doing this automatically)
MACRO(INSTALL_HEADERS_WITH_DIRECTORY HEADER_LIST STRIP_PREFIX REPLACE_PREFIX)
  GET_FILENAME_COMPONENT( SPREFIX ${STRIP_PREFIX} ABSOLUTE )
  FOREACH(HEADER ${${HEADER_LIST}})
    STRING(REGEX MATCH "(.*)[/]" DIR ${HEADER})
    STRING(REPLACE ${SPREFIX} ${REPLACE_PREFIX} NEWDIR ${DIR})
    INSTALL(FILES ${HEADER} DESTINATION ${INCLUDE_DIR}/${NEWDIR})
  ENDFOREACH(HEADER)
ENDMACRO(INSTALL_HEADERS_WITH_DIRECTORY)


########################################################################################
# INSTALL:
# Set deployment directories
# Due to complicated adaptor deployment the install section is pretty much converted from
# the automake counterpart.
SET( tizenadaptorpublicapidir ${INCLUDE_DIR}/dali/public-api )
SET( tizenadaptordevelapidir ${INCLUDE_DIR}/dali/devel-api )
SET( tizenadaptorintegrationapidir ${INCLUDE_DIR}/dali/integration-api/adaptor-framework )
SET( tizenadaptorframeworkpublicapidir ${tizenadaptorpublicapidir}/adaptor-framework )
SET( tizenadaptorframeworkdevelapidir ${tizenadaptordevelapidir}/adaptor-framework )
SET( tizentextabstractiondevelapidir ${tizenadaptordevelapidir}/text-abstraction )
SET( tizenadaptordaliheaderdir ${INCLUDE_DIR}/dali )
SET( tizenwatchpublicapidir ${tizenadaptorpublicapidir}/watch )
SET( tizencapturepublicapidir ${tizenadaptorpublicapidir}/capture )

# Android framework headers for Android build
SET( androidadaptorintegrationapidir ${INCLUDE_DIR}/dali/integration-api/adaptor-framework/android )

# Install headers
INSTALL( FILES ${public_api_header_files} DESTINATION ${tizenadaptorpublicapidir} )
INSTALL( FILES ${adaptor_integration_api_header_files} DESTINATION ${tizenadaptorintegrationapidir} )
INSTALL( FILES ${public_api_adaptor_framework_header_files} DESTINATION ${tizenadaptorframeworkpublicapidir} )
INSTALL( FILES ${devel_api_adaptor_framework_header_files} DESTINATION ${tizenadaptorframeworkdevelapidir} )
INSTALL( FILES ${text_abstraction_header_files} DESTINATION ${tizentextabstractiondevelapidir} )

# Install Android framework headers for Android build
INSTALL( FILES ${adaptor_integration_api_android_header_files} DESTINATION ${androidadaptorintegrationapidir} )

# Install DOXY
SET( packagedoxydir ${INCLUDE_DIR}/dali/doc )
SET( package_doxy_dir ${ADAPTOR_ROOT}/doc )
INCLUDE( ${ADAPTOR_ROOT}/doc/file.list )
INSTALL( FILES ${package_doxy_files} DESTINATION ${packagedoxydir} )

IF( NOT UBUNTU_PROFILE AND NOT ANDROID_PROFILE AND NOT WINDOWS_PROFILE )
  INSTALL( FILES ${public_api_adaptor_tizen_header_files} DESTINATION ${tizenadaptorframeworkpublicapidir} )
  IF( NOT WAYLAND )
    INSTALL( FILES ${devel_api_adaptor_tizen_x11_header_files} DESTINATION ${tizenadaptorframeworkdevelapidir} )
  ENDIF()
ENDIF()

INSTALL( FILES ${adaptor_dali_header_file} DESTINATION ${tizenadaptordaliheaderdir} )
INSTALL( FILES ${adaptor_dali_wearable_header_file} DESTINATION ${tizenadaptorpublicapidir} )
INSTALL( FILES ${public_dali_watch_header_files} DESTINATION ${tizenwatchpublicapidir} )
INSTALL( FILES ${public_dali_capture_header_files} DESTINATION ${tizencapturepublicapidir} )

IF( ANDROID_PROFILE )
  SET( daliShaderbinCacheDir ${CMAKE_INSTALL_PREFIX}/core/shaderbin/)
  INSTALL( DIRECTORY DESTINATION ${daliShaderbinCacheDir} )
ELSEIF( UNIX )
  SET( daliShaderbinCacheDir ${CMAKE_BINARY_DIR}/${dataReadOnlyDir}/core/shaderbin/)
  INSTALL( DIRECTORY DESTINATION ${dataReadOnlyDir}/core/shaderbin )
ENDIF()

INSTALL( FILES ${ADAPTOR_ROOT}/adaptors/scripts/dalireslog.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )

########################################################################################

# The DALI_ADAPTOR_PREFIX must be set if this CMakeLists.txt is executed
# from the top-level CMake script using ADD_SUBDIRECTORY() to avoid
# target names duplication with other DALi modules.
IF( ENABLE_COVERAGE )
  FIND_PROGRAM( LCOV_BIN "lcov" )
  IF( LCOV_BIN )

    # Define custom rules for coverage
    SET(COVERAGE_DIR .cov)
    SET(COVERAGE_OUTPUT_DIR doc/coverage)

    # lcov prior to 1.10 doesn't have -rc option; this and subsequent version don't output
    # branch coverage. Determine the lcov version, and enable branch coverage accordingly.
    EXECUTE_PROCESS( COMMAND bash -c "${LCOV_BIN} --version | cut -d' ' -f4" OUTPUT_VARIABLE LCOV_VERSION )
    STRING( REPLACE "." ";" LCOV_VLIST ${LCOV_VERSION})
    IF( NOT $<VERSION_LESS:${LCOV_VERSION},"1.10"> )
      SET(LCOV_OPTS --rc lcov_branch_coverage=1)
    ENDIF()

    ADD_CUSTOM_TARGET( ${DALI_ADAPTOR_PREFIX}rename_cov_data ./rename-cov-data )

    ADD_CUSTOM_TARGET( ${DALI_ADAPTOR_PREFIX}cov_data ${LCOV_BIN} ${LCOV_OPTS} --base-directory . --directory . -c -o dali.info
      COMMAND ${LCOV_BIN} ${LCOV_OPTS} --remove dali.info \"*/dali-env/*\" \"/usr/include/*\" -o dali.info )

    ADD_CUSTOM_TARGET( ${DALI_ADAPTOR_PREFIX}coverage genhtml ${LCOV_OPTS} -o ${COVERAGE_OUTPUT_DIR} dali.info
      DEPENDS cov_data )

    ADD_CUSTOM_TARGET( ${DALI_ADAPTOR_PREFIX}reset_coverage @${LCOV_BIN} -z --directory `pwd` )

    # Define custom rule for distclean
    ADD_CUSTOM_TARGET( ${DALI_ADAPTOR_PREFIX}distclean @echo cleaning for source distribution )
    ADD_CUSTOM_COMMAND(
          DEPENDS clean
          COMMENT "distribution clean"
          COMMAND find
          ARGS    .
          -not -name config.cmake -and \(
          -name tester.c -or
          -name Testing -or
          -name CMakeFiles -or
          -name doc -or
          -name cmake.depends -or
          -name cmake.check_depends -or
          -name CMakeCache.txt -or
          -name cmake.check_cache -or
          -name *.cmake -or
          -name Makefile -or
          -name core -or
          -name core.* -or
          -name gmon.out -or
          -name install_manifest.txt -or
          -name *.pc -or
          -name *.gcov -or
          -name *.gcno -or
          -name *.gcda -or
          -name *~ -or
          -name libdali*.so* \)
          | grep -v TC | xargs rm -rf
          TARGET  ${DALI_ADAPTOR_PREFIX}distclean
          VERBATIM
          )

  ENDIF( LCOV_BIN )
ENDIF()

IF( ENABLE_LINK_TEST )
  # Linker Test
  SET( DALI_TEST_SUITE_DIR ${ROOT_SRC_DIR}/automated-tests/src/dali/dali-test-suite-utils )
  SET( LINKER_TEST_NAME ${DALI_ADAPTOR_PREFIX}linker-test )
  SET( LINKER_TEST_SOURCES
    linker-test.cpp
  )
  ADD_EXECUTABLE( ${LINKER_TEST_NAME} ${LINKER_TEST_SOURCES} )
  MESSAGE(STATUS "libs: ${DALICORE_LDFLAGS}")
  TARGET_COMPILE_OPTIONS( ${LINKER_TEST_NAME} PRIVATE -I${ROOT_SRC_DIR} ${DALICORE_CFLAGS} )
  TARGET_LINK_LIBRARIES(${LINKER_TEST_NAME} ${name} ${DALICORE_LDFLAGS} ${VCONF_LDFLAGS} ${HARFBUZZ_LDFLAGS} )
  TARGET_INCLUDE_DIRECTORIES( ${LINKER_TEST_NAME} PRIVATE ${DALI_TEST_SUITE_DIR} )
ENDIF()

# Configuration Messages
MESSAGE( STATUS "Configuration:\n" )
MESSAGE( STATUS "Prefix:                           ${PREFIX}")
MESSAGE( STATUS "Lib Dir:                          ${LIB_DIR}" )
MESSAGE( STATUS "Include Dir:                      ${INCLUDE_DIR}" )
MESSAGE( STATUS "Coverage:                         ${ENABLE_COVERAGE}" )
MESSAGE( STATUS "Debug Build:                      ${ENABLE_DEBUG}")
MESSAGE( STATUS "Freetype bitmap support (Emoji):  ${freetype_bitmap_support_ENABLED}")
MESSAGE( STATUS "Profile:                          ${ENABLE_PROFILE}")
MESSAGE( STATUS "Data Dir (Read/Write):            ${dataReadWriteDir}")
MESSAGE( STATUS "Data Dir (Read Only):             ${dataReadOnlyDir}")
MESSAGE( STATUS "EldBus:                           ${eldbus_available_ENABLED}")
MESSAGE( STATUS "Shader Binary Cache:              ${ENABLE_SHADERBINCACHE}")
MESSAGE( STATUS "Network logging enabled:          ${ENABLE_NETWORKLOGGING}")
MESSAGE( STATUS "Font config file:                 ${fontConfigurationFile}")
MESSAGE( STATUS "Using Tizen APP FW libraries:     ${ENABLE_APPFW}")
MESSAGE( STATUS "Use pkg configure:                ${ENABLE_PKG_CONFIGURE}" )
MESSAGE( STATUS "Enable link test:                 ${ENABLE_LINK_TEST}" )
MESSAGE( STATUS "Tizen Platform Config supported   ${TIZEN_PLATFORM_CONFIG_SUPPORTED_LOGMSG}")
MESSAGE( STATUS "Compile flags:                    ${CMAKE_CXX_FLAGS}")
MESSAGE( STATUS "Compile flags:                    ${CMAKE_C_FLAGS}")
IF( enable_feedback )
  ADD_SUBDIRECTORY( plugins )
ENDIF()

CLEAN_ARG_CACHE()
