IF( (NOT UBUNTU_PROFILE) )
  PKG_CHECK_MODULES(FEEDBACK feedback)
  PKG_CHECK_MODULES(MMFSOUND mm-sound)

  SET(PLUGIN_TARGET dali-feedback-plugin-cxx11 )

  MESSAGE(STATUS ${ROOT_SRC_DIR}/plugins )
  SET( plugin_src_dir ${ROOT_SRC_DIR}/plugins )

  INCLUDE( ${plugin_src_dir}/file.list )

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
  INSTALL( FILES ${dali_plugin_sound_files} DESTINATION ${dalisounddir} )
  INSTALL( TARGETS ${PLUGIN_TARGET} DESTINATION ${LIB_DIR} )
ENDIF()
