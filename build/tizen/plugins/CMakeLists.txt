MESSAGE(STATUS ${ROOT_SRC_DIR}/plugins )
SET( plugin_src_dir ${ROOT_SRC_DIR}/plugins )

INCLUDE( ${plugin_src_dir}/file.list )

IF( ENABLE_FEEDBACK AND (NOT UBUNTU_PROFILE) )
  PKG_CHECK_MODULES(FEEDBACK feedback)
  PKG_CHECK_MODULES(MMFSOUND mm-sound)

  SET(PLUGIN_TARGET dali2-feedback-plugin )

  SET( PLUGIN_SOURCES ${feedback_plugin_src_files})

  ADD_LIBRARY( ${PLUGIN_TARGET} SHARED ${PLUGIN_SOURCES} )

  TARGET_LINK_LIBRARIES( ${PLUGIN_TARGET} PUBLIC
    ${DLOG_LDFLAGS}
    ${MMFSOUND_LDFLAGS})

  TARGET_COMPILE_DEFINITIONS( ${PLUGIN_TARGET} PUBLIC
    -DDALI_SOUND_DIR=\"${dalisounddir}\")

  TARGET_COMPILE_OPTIONS( ${PLUGIN_TARGET} PUBLIC
    ${DLOG_CFLAGS}
    ${DALICORE_CFLAGS}
    ${MMFSOUND_CFLAGS}
    ${FEEDBACK_CFLAGS}
    ${DALI_PROFILE_CFLAGS}
    ${DALI_ADAPTOR_CFLAGS}
    -I../../../
    -Wall
  )
  # Install plugin resources
  #SET( dalisounddir ${dataReadOnlyDir}/plugins/sounds/ )
  SET( dalisounddir ${CMAKE_INSTALL_PREFIX}/../plugins/sounds/ )
    #EXIT(${dalisounddir})
  INSTALL( FILES ${feedback_plugin_dali_sound_files} DESTINATION ${dalisounddir} )
  INSTALL( TARGETS ${PLUGIN_TARGET} DESTINATION ${LIB_DIR} )
ENDIF()

# File Download curl plugin
# Windows consumes libcurl through its vcpkg CMake package.  Other profiles
# retain their existing pkg-config discovery path.
IF(WINDOWS_PROFILE)
  FIND_PACKAGE(CURL QUIET)
ELSE()
  PKG_CHECK_MODULES(CURL libcurl)
ENDIF()

IF( CURL_FOUND)
  IF(CURL_VERSION_STRING)
    SET(CURL_VERSION_DISPLAY ${CURL_VERSION_STRING})
  ELSE()
    SET(CURL_VERSION_DISPLAY ${CURL_VERSION})
  ENDIF()
  MESSAGE( STATUS "CURL found : ${CURL_VERSION_DISPLAY} dali2-file-download-plugin-curl will be built" )
  SET( FILE_DOWNLOAD_PLUGIN_TARGET dali2-file-download-plugin-curl )

  SET( FILE_DOWNLOAD_PLUGIN_SOURCES ${file_download_plugin_curl_src_files})

  ADD_LIBRARY( ${FILE_DOWNLOAD_PLUGIN_TARGET} ${LIBTYPE} ${FILE_DOWNLOAD_PLUGIN_SOURCES} )

  IF(WINDOWS_PROFILE)
    # Keep the runtime-loaded plugin beside the adaptor DLL in build trees as
    # well as after installation. Multi-config generators append Debug/Release
    # to this directory for both targets.
    SET_TARGET_PROPERTIES(${FILE_DOWNLOAD_PLUGIN_TARGET} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
  ENDIF()

  SET(DALI_FILE_DOWNLOAD_PLUGIN_CFLAGS
    ${DALICORE_CFLAGS}
    ${CURL_CFLAGS}
    ${DALI_ADAPTOR_CFLAGS}
    ${COVERAGE}
  )

  SET(DALI_FILE_DOWNLOAD_PLUGIN_LDFLAGS
    ${DALICORE_LDFLAGS}
    ${COVERAGE}
    dali2-adaptor
  )

  IF(WINDOWS_PROFILE)
    IF(TARGET CURL::libcurl)
      LIST(APPEND DALI_FILE_DOWNLOAD_PLUGIN_LDFLAGS CURL::libcurl)
    ELSE()
      # FindCURL only gained the imported target in newer CMake versions;
      # retain compatibility with the project's minimum CMake version.
      LIST(APPEND DALI_FILE_DOWNLOAD_PLUGIN_LDFLAGS ${CURL_LIBRARIES})
    ENDIF()
  ELSEIF(APPLE)
    FIND_PACKAGE(curl REQUIRED)
    SET(DALI_FILE_DOWNLOAD_PLUGIN_LDFLAGS
      ${DALI_FILE_DOWNLOAD_PLUGIN_LDFLAGS}
      CURL::libcurl
      "-framework SystemConfiguration"
      "-framework CoreFoundation"
      "-framework CoreServices"
      -lssl
      -lcrypto
      -lz
      -ldl
      -rdynamic
    )
  ELSE()
    LIST(APPEND DALI_FILE_DOWNLOAD_PLUGIN_CFLAGS -Wall)
    SET(DALI_FILE_DOWNLOAD_PLUGIN_LDFLAGS
      ${DALI_FILE_DOWNLOAD_PLUGIN_LDFLAGS}
      ${CURL_LDFLAGS}
      -ldl
      -rdynamic
    )
  ENDIF()

  TARGET_LINK_LIBRARIES( ${FILE_DOWNLOAD_PLUGIN_TARGET} PRIVATE
    ${DALI_FILE_DOWNLOAD_PLUGIN_LDFLAGS}
  )

  TARGET_COMPILE_OPTIONS( ${FILE_DOWNLOAD_PLUGIN_TARGET} PRIVATE
    ${DALI_FILE_DOWNLOAD_PLUGIN_CFLAGS}
  )

  INCLUDE_DIRECTORIES(
    ${CURL_INCLUDE_DIRS}
    ${ROOT_SRC_DIR}
    ${THIRD_PARTY_WINDOWS_PLATFORM}
    ${THIRD_PARTY_MACOS_PLATFORM}
  )

  INSTALL(TARGETS ${FILE_DOWNLOAD_PLUGIN_TARGET}
    LIBRARY DESTINATION ${LIB_DIR}
    ARCHIVE DESTINATION ${LIB_DIR}
    RUNTIME DESTINATION ${BIN_DIR}
  )

  IF(ENABLE_LINK_TEST)
    # Linker Test for curl
    SET(LINKER_TEST_PLUGIN_CURL_NAME ${DALI_ADAPTOR_PREFIX}linker-test-curl)
    ADD_EXECUTABLE(${LINKER_TEST_PLUGIN_CURL_NAME} linker-test-file-download-plugin.cpp)
    IF(WINDOWS_PROFILE)
      SET_TARGET_PROPERTIES(${LINKER_TEST_PLUGIN_CURL_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
      )
    ENDIF()
    TARGET_COMPILE_OPTIONS(${LINKER_TEST_PLUGIN_CURL_NAME} PRIVATE -I${ROOT_SRC_DIR} ${DALI_CFLAGS} ${DALI_FILE_DOWNLOAD_PLUGIN_CFLAGS} ${COVERAGE})
    TARGET_LINK_LIBRARIES(${LINKER_TEST_PLUGIN_CURL_NAME} ${name} ${COVERAGE} ${DALI_CFLAGS} ${DALI_FILE_DOWNLOAD_PLUGIN_CFLAGS} ${FILE_DOWNLOAD_PLUGIN_TARGET})
  ENDIF()
ELSE()
  MESSAGE( STATUS "CURL not found, dali2-file-download-plugin-curl will not be built" )
ENDIF()

# File Download download-provider plugin (Tizen only)
IF( NOT APPLE AND NOT WINDOWS_PROFILE )
  PKG_CHECK_MODULES(DOWNLOAD_PROVIDER capi-web-url-download)

  IF( DOWNLOAD_PROVIDER_FOUND )
    MESSAGE( STATUS "capi-web-url-download found : ${DOWNLOAD_PROVIDER_VERSION} dali2-file-download-plugin-download-api will be built" )
    SET( FILE_DOWNLOAD_API_PLUGIN_TARGET dali2-file-download-plugin-download-api )

    SET( FILE_DOWNLOAD_API_PLUGIN_SOURCES ${file_download_plugin_download_api_src_files} )

    ADD_LIBRARY( ${FILE_DOWNLOAD_API_PLUGIN_TARGET} ${LIBTYPE} ${FILE_DOWNLOAD_API_PLUGIN_SOURCES} )

    SET( DALI_FILE_DOWNLOAD_API_PLUGIN_CFLAGS
      ${DALICORE_CFLAGS}
      ${DOWNLOAD_PROVIDER_CFLAGS}
      ${DALI_ADAPTOR_CFLAGS}
      ${COVERAGE}
      -Wall
    )

    SET( DALI_FILE_DOWNLOAD_API_PLUGIN_LDFLAGS
      ${DALICORE_LDFLAGS}
      ${COVERAGE}
      dali2-adaptor
      ${DOWNLOAD_PROVIDER_LDFLAGS}
      -ldl
      -rdynamic
    )

    TARGET_LINK_LIBRARIES( ${FILE_DOWNLOAD_API_PLUGIN_TARGET} PRIVATE
      ${DALI_FILE_DOWNLOAD_API_PLUGIN_LDFLAGS}
    )

    TARGET_COMPILE_OPTIONS( ${FILE_DOWNLOAD_API_PLUGIN_TARGET} PRIVATE
      ${DALI_FILE_DOWNLOAD_API_PLUGIN_CFLAGS}
    )

    INCLUDE_DIRECTORIES(
      ${DOWNLOAD_PROVIDER_INCLUDE_DIRS}
      ${ROOT_SRC_DIR}
    )

    INSTALL( TARGETS ${FILE_DOWNLOAD_API_PLUGIN_TARGET} DESTINATION ${LIB_DIR} )
  ELSE()
    MESSAGE( STATUS "capi-web-url-download not found, dali2-file-download-plugin-download-api will not be built" )
  ENDIF()
ENDIF()
