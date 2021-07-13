# Find dependencies ( linux, tizen )
IF( ENABLE_PKG_CONFIGURE )
  FIND_PACKAGE(PkgConfig REQUIRED)
ENDIF()

# Special constant for ARG_ENABLE function
SET( ENABLE_VAL "#ENABLE_VAL" )
SET( HELP_ENABLES "" )
SET( CUSTOM_ARGUMENTS "" )

# Macro checks if the module is available and sets user variable
MACRO( CHECK_MODULE_AND_SET PKG_VAR PKG_LIB_REQUIREMENTS VAR_TO_SET )
  PKG_CHECK_MODULES(${PKG_VAR} "${PKG_LIB_REQUIREMENTS}")
  IF( ${PKG_VAR}_FOUND )
    SET( ${VAR_TO_SET} ON )
    SET( ${VAR_TO_SET}_ENABLED ON )
  ELSE()
    SET( ${VAR_TO_SET}_ENABLED OFF )
  ENDIF()
ENDMACRO()

# Simple ternary operator
FUNCTION( TEST_IF CONDITION OUTPUT_VAR VAL0 VAL1 )
  IF( ${CONDITION} )
    SET( ${OUTPUT_VAR} ${VAL0} PARENT_SCOPE )
  ELSE()
    SET( ${OUTPUT_VAR} {VAL1} PARENT_SCOPE )
  ENDIF()
ENDFUNCTION()

# Conditional macro immitates autoconf AM_CONDITIONAL
MACRO( CONDITIONAL VARIABLE )
  IF( ${ARGN} )
      SET( ${VARIABLE} ON )
  ENDIF()
ENDMACRO()

MACRO( OPT_STRING NAME DEFAULT_VALUE )
  SET( ${NAME} ${DEFAULT_VALUE} CACHE STRING "${ARGN}" )
ENDMACRO()

FUNCTION( ARG_ENABLE NAME INTERNAL_VAR LIST_OF_VALUES )
  SET(HELP_ENABLES "${HELP_ENABLES}\n${NAME}\t-\t${ARGN}" PARENT_SCOPE)
  SET(CUSTOM_ARGUMENTS "${CUSTOM_ARGUMENTS};${NAME}" PARENT_SCOPE)
  LIST(LENGTH LIST_OF_VALUES SIZE)
  LIST(GET LIST_OF_VALUES 0 OPT_ON )
  IF( SIZE EQUAL 2 )
    LIST(GET LIST_OF_VALUES 1 OPT_OFF )
  ENDIF()
  IF( OPT_OFF )
    SET( ${NAME} "${OPT_OFF}" CACHE STRING "${ARGN}" )
  ELSE()
    SET( ${NAME} "OFF" CACHE STRING "${ARGN}" )
  ENDIF()
  IF( ${NAME} )
    IF(OPT_ON MATCHES ENABLE_VAL)
      SET( ${INTERNAL_VAR} ${${NAME}} PARENT_SCOPE )
    ELSE()
      SET( ${INTERNAL_VAR} ${OPT_ON} PARENT_SCOPE )
    ENDIF()
  ELSE()
    IF( OPT_OFF )
      SET( ${INTERNAL_VAR} ${OPT_OFF} PARENT_SCOPE )
    ELSE()
      IF( CMAKE_MAJOR_VERSION GREATER 3 )
        UNSET( ${INTERNAL_VAR} PARENT_SCOPE )
      ELSE()
        UNSET( ${INTERNAL_VAR} )
      ENDIF()
    ENDIF()
  ENDIF()
  MESSAGE( STATUS "${NAME} = ${${NAME}}")
ENDFUNCTION()

# This macro unsets all cached argument variables, should be called
# upon exit
MACRO( CLEAN_ARG_CACHE )
  FOREACH( arg IN LISTS CUSTOM_ARGUMENTS )
    UNSET( ${arg} CACHE )
  ENDFOREACH()
ENDMACRO()

MACRO( EXIT )
  CLEAN_ARG_CACHE()
  MESSAGE(FATAL_ERROR ${ARGN})
ENDMACRO()
