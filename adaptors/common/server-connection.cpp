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

namespace
{
// Copied from ecore_evas_extn_engine.h
// procotol version - change this as needed
const int MAJOR( 0x2011 );
}


namespace Dali
{
namespace Internal
{
namespace Adaptor
{
#if defined(DEBUG_ENABLED)
extern Debug::Filter* gIndicatorLogFilter;
#endif

ServerConnection::~ServerConnection()
{
  CloseConnection();

  if( mService.name != NULL )
  {
    eina_stringshare_del(mService.name);
  }

  for( Handlers::iterator iter = mIpcHandlers.begin(); iter != mIpcHandlers.end(); ++iter )
  {
    ecore_event_handler_del(*iter);
  }
  mIpcHandlers.clear();
}

bool ServerConnection::IsConnected()
{
  return mConnected;
}

void ServerConnection::OnDisconnect()
{
  mConnected = false;
  mIpcServer = NULL;
  ecore_ipc_shutdown();
  if( mObserver )
  {
    mObserver->ConnectionClosed();
  }
}

bool ServerConnection::SendEvent( int event, const void *data, int size )
{
  return SendEvent(event, 0, 0, data, size);
}

bool ServerConnection::SendEvent( int event, int ref, int ref_to, const void *data, int size )
{
  if( mIpcServer != NULL  && ecore_ipc_server_send(mIpcServer, MAJOR, event, ref, ref_to, 0, data, size) )
  {
    return true;
  }
  else
  {
    return false;
  }
}

Eina_Bool ServerConnection::IpcServerAdd( void *data, int /*type*/, void *event )
{
  DALI_LOG_INFO(gIndicatorLogFilter, Debug::General, "ServerConnection: IpcServerAdd\n" );

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool ServerConnection::IpcServerDel( void *data, int /*type*/, void *event )
{
  DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "ServerConnection: IpcServerDel\n" );

  Ecore_Ipc_Event_Server_Del *e = static_cast<Ecore_Ipc_Event_Server_Del *>( event );
  ServerConnection* connection = static_cast<ServerConnection*>( data );

  if( connection != NULL )
  {
    if( connection->mIpcServer == e->server)
    {
      // No longer have a server connection
      connection->OnDisconnect();
    }
  }

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool ServerConnection::IpcServerData( void *data, int /*type*/, void *event )
{
  DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "ServerConnection: IpcServerData\n" );

  Ecore_Ipc_Event_Server_Data *e = static_cast<Ecore_Ipc_Event_Server_Data *>( event );
  ServerConnection* connection = static_cast<ServerConnection*>( data );

  if( connection != NULL )
  {
    if( connection != ecore_ipc_server_data_get( e->server ) )
    {
      return ECORE_CALLBACK_PASS_ON;
    }

    if( e->major != MAJOR )
    {
      return ECORE_CALLBACK_PASS_ON;
    }

    if( connection->mObserver )
    {
      connection->mObserver->DataReceived( event );
    }
  }
  return ECORE_CALLBACK_PASS_ON;
}

void ServerConnection::CloseConnection()
{
  if( mConnected )
  {
    DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "ServerConnection: CloseConnection\n" );

    if( mIpcServer )
    {
      ecore_ipc_server_del( mIpcServer );
      mIpcServer = NULL;
    }

    ecore_ipc_shutdown();
    mConnected = false;
  }
}

} // Adaptor
} // Internal
} // Dali
