# Tizen app-entity backend: pre-generated TIDL server stub.
#
# The source-package generation model follows ActionSampleAppCpp so GBS does
# not need host executables such as actionc in its target build root.
PKG_CHECK_MODULES( RPC_PORT REQUIRED rpc-port )

SET( ENTITY_DATA_STUB_DIR ${ADAPTOR_ROOT}/dali/internal/app-entity/tizen/tidl-stub )
SET( ENTITY_DATA_STUB_CC  ${ENTITY_DATA_STUB_DIR}/view-stub.cc )

# To regenerate after a Tizen.Entity.View / Tizen.Action.View change:
#   ACTIONC_ACTION_SEQ=<tizen-action>/default-actions/action.seq \
#     actionc -a Tizen.Action.View -l C++ -o implview \
#             -d <tizen-action>/default-actions
#   mv implview.h view-stub.h && mv implview.cc view-stub.cc
#   sed -i 's|#include "implview.h"|#include "view-stub.h"|' view-stub.cc
# Renaming the files/header include is the only edit made to generated output.
INCLUDE_DIRECTORIES( ${ENTITY_DATA_STUB_DIR} ${RPC_PORT_INCLUDE_DIRS} ${BUNDLE_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} )
SET( DALI_LDFLAGS ${DALI_LDFLAGS} ${RPC_PORT_LDFLAGS} ${BUNDLE_LDFLAGS} ${GLIB_LDFLAGS} )
ADD_DEFINITIONS( -DENABLE_ENTITY_DATA_TIDL )
SET_SOURCE_FILES_PROPERTIES( ${ENTITY_DATA_STUB_CC} PROPERTIES COMPILE_FLAGS "-Wno-error" )
MESSAGE( STATUS "Tizen.Action.View: using checked-in TIDL server stub" )
