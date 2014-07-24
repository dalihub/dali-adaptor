/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// CLASS HEADER
#include "server-connection.h"

// EXTERNAL INCLUDES
#include <Ecore.h>

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
#if defined(DEBUG_ENABLED)
extern Debug::Filter* gIndicatorLogFilter;
#endif


ServerConnection::ServerConnection(
  const char*                 serviceName,
  int                         serviceNumber,
  bool                        isSystem,
  ServerConnection::Observer* observer)

: mConnected(false),
  mObserver(observer)
{
  ecore_ipc_init();
  mService.name = eina_stringshare_add(serviceName);
  mService.num = serviceNumber;
  mService.isSystem = isSystem;

  DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "ServerConnection: Connecting to %s %d\n", mService.name, mService.num );

  mIpcServer = NULL;

  if( !mIpcServer )
  {
    ecore_ipc_shutdown();
  }
  else
  {
    mIpcHandlers.push_back( ecore_event_handler_add( ECORE_IPC_EVENT_SERVER_ADD,
                                                     &ServerConnection::IpcServerAdd,
                                                     this ) );

    mIpcHandlers.push_back( ecore_event_handler_add( ECORE_IPC_EVENT_SERVER_DEL,
                                                     &ServerConnection::IpcServerDel,
                                                     this ) );

    mIpcHandlers.push_back( ecore_event_handler_add( ECORE_IPC_EVENT_SERVER_DATA,
                                                     &ServerConnection::IpcServerData,
                                                     this));

    mConnected = true;
  }
}

} // Adaptor
} // Internal
} // Dali
