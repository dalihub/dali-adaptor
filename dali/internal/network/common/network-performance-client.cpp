/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/internal/network/common/network-performance-client.h>

// EXTERNAL INCLUDES
#include <stdio.h>
#include <iostream>
#include <string>

// INTERNAL INCLUDES
#include <dali/internal/network/common/automation.h>
#include <dali/internal/network/common/network-performance-protocol.h>
#include <dali/internal/network/common/socket-interface.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const float MICROSECONDS_TO_SECOND = 1e-6;
const char  UNKNOWN_CMD[]          = "Command or parameter invalid, type help for list of commands\n";

/**
 * helper class to store data along with the automation callback.
 */
class AutomationCallback : public CallbackBase
{
public:
  /**
   * instead of using templates, or having different callback classes for each callback
   * we use a command id that decides which static function to call on the Automation class.
   */
  enum CommandId
  {
    UNKNOWN_COMMAND,
    SET_PROPERTY,
    CUSTOM_COMMAND,
    DUMP_SCENE
  };

  AutomationCallback(unsigned int clientId, ClientSendDataInterface& sendDataInterface)
  : CallbackBase(reinterpret_cast<void*>(this),
                 NULL, // we get the dispatcher to call function directly
                 reinterpret_cast<CallbackBase::Dispatcher>(&AutomationCallback::Dispatcher)),
    mSendDataInterface(sendDataInterface),
    mCommandId(UNKNOWN_COMMAND),
    mClientId(clientId)
  {
  }

  void AssignSetPropertyCommand(std::string setPropertyCommand)
  {
    mCommandId     = SET_PROPERTY;
    mCommandString = setPropertyCommand;
  }

  void AssignDumpSceneCommand()
  {
    mCommandId = DUMP_SCENE;
  }

  void AssignCustomCommand(std::string&& customCommand)
  {
    mCommandId     = CUSTOM_COMMAND;
    mCommandString = std::move(customCommand);
  }

  void RunCallback()
  {
    switch(mCommandId)
    {
      case SET_PROPERTY:
      {
        Automation::SetProperty(mCommandString);
        break;
      }
      case DUMP_SCENE:
      {
        Automation::DumpScene(mClientId, &mSendDataInterface);
        break;
      }
      case CUSTOM_COMMAND:
      {
        Automation::SetCustomCommand(mCommandString);
        break;
      }
      default:
      {
        DALI_ASSERT_DEBUG(0 && "Unknown command");
        break;
      }
    }
  }
  static void Dispatcher(CallbackBase& base)
  {
    AutomationCallback& automationCallback(static_cast<AutomationCallback&>(base));
    automationCallback.RunCallback();
  }

private:
  std::string              mCommandString;     ///< command string for property or custom command
  ClientSendDataInterface& mSendDataInterface; ///< Abstract client send data interface
  CommandId                mCommandId;         ///< command id
  const unsigned int       mClientId;          ///< client id
};

/**
 * @brief Helper to ensure the AutomationCallback method we want is called in the main thread
 */
template<typename T>
void TriggerOnMainThread(unsigned int clientId, ClientSendDataInterface& sendDataInterface, T&& lambda)
{
  // this needs to be run on the main thread, use the trigger event....
  AutomationCallback* callback = new AutomationCallback(clientId, sendDataInterface);
  lambda(callback);

  // create a trigger event that automatically deletes itself after the callback has run in the main thread
  TriggerEventInterface* interface = TriggerEventFactory::CreateTriggerEvent(callback, TriggerEventInterface::DELETE_AFTER_TRIGGER);

  // asynchronous call, the call back will be run sometime later on the main thread
  interface->Trigger();
}

} // unnamed namespace

NetworkPerformanceClient::NetworkPerformanceClient(pthread_t*               thread,
                                                   SocketInterface*         socket,
                                                   unsigned int             clientId,
                                                   ClientSendDataInterface& sendDataInterface,
                                                   SocketFactoryInterface&  socketFactory)
: mThread(thread),
  mSocket(socket),
  mMarkerBitmask(PerformanceMarker::FILTERING_DISABLED),
  mSendDataInterface(sendDataInterface),
  mSocketFactoryInterface(socketFactory),
  mClientId(clientId),
  mConsoleClient(false)
{
}

NetworkPerformanceClient::~NetworkPerformanceClient()
{
  if(mSocket->SocketIsOpen())
  {
    mSocket->CloseSocket();
  }
  mSocketFactoryInterface.DestroySocket(mSocket);
}

unsigned int NetworkPerformanceClient::GetId() const
{
  return mClientId;
}

SocketInterface& NetworkPerformanceClient::GetSocket()
{
  return *mSocket;
}

bool NetworkPerformanceClient::WriteSocket(const void* buffer, unsigned int bufferSizeInBytes)
{
  return mSocket->Write(buffer, bufferSizeInBytes);
}

bool NetworkPerformanceClient::TransmitMarker(const PerformanceMarker& marker, const char* const description)
{
  if(!marker.IsFilterEnabled(mMarkerBitmask))
  {
    return true;
  }
  if(mConsoleClient)
  {
    // write out the time stamp
    char   *buffer;
    double usec = marker.GetTimeStamp().microseconds;
    int    size = asprintf(&buffer, "%.6f (seconds), %s\n", usec * MICROSECONDS_TO_SECOND, description);
    auto retVal = mSocket->Write(buffer, size);
    free(buffer);
    return retVal;
  }

  // todo serialize the data
  return false;
}

void NetworkPerformanceClient::ExitSelect()
{
  mSocket->ExitSelect();
}

pthread_t* NetworkPerformanceClient::GetThread()
{
  return mThread;
}

void NetworkPerformanceClient::ProcessCommand(char* buffer, unsigned int bufferSizeInBytes)
{
  // if connected via console, then strip off the carriage return, and switch to console mode
  if(buffer[bufferSizeInBytes - 1] == '\n')
  {
    buffer[bufferSizeInBytes - 1] = 0;
    mConsoleClient                = true;
  }
  unsigned int                   param(0);
  std::string                    stringParam;
  PerformanceProtocol::CommandId commandId(PerformanceProtocol::UNKNOWN_COMMAND);

  bool ok = PerformanceProtocol::GetCommandId(buffer, bufferSizeInBytes, commandId, param, stringParam);
  if(!ok)
  {
    WriteSocket(UNKNOWN_CMD, sizeof(UNKNOWN_CMD));
    return;
  }
  std::string response;

  switch(commandId)
  {
    case PerformanceProtocol::HELP_MESSAGE:
    {
      response = PerformanceProtocol::GetHelpMessage();
      break;
    }

    case PerformanceProtocol::ENABLE_TIME_MARKER_BIT_MASK:
    {
      mMarkerBitmask = static_cast<PerformanceMarker::MarkerFilter>(param);
      response       = "enable time marker ";
      break;
    }

    case PerformanceProtocol::DUMP_SCENE_GRAPH:
    {
      TriggerOnMainThread(mClientId, mSendDataInterface, [&](AutomationCallback* callback){callback->AssignDumpSceneCommand();});
      break;
    }

    case PerformanceProtocol::SET_PROPERTIES:
    {
      TriggerOnMainThread(mClientId, mSendDataInterface, [&](AutomationCallback* callback){callback->AssignSetPropertyCommand(stringParam);});
      response = "Completed";
      break;
    }

    case PerformanceProtocol::CUSTOM_COMMAND:
    {
      TriggerOnMainThread(mClientId, mSendDataInterface, [&](AutomationCallback* callback){callback->AssignCustomCommand(std::move(stringParam));});
      response = "Completed";
      break;
    }

    case PerformanceProtocol::LIST_METRICS_AVAILABLE:
    case PerformanceProtocol::ENABLE_METRIC:
    case PerformanceProtocol::DISABLE_METRIC:
    {
      response = "Metrics currently not supported";
      break;
    }
    default:
    {
      response = UNKNOWN_CMD;
      break;
    }
  }
  if(!response.empty())
  {
    // add a carriage return for console clients
    if(mConsoleClient)
    {
      response += "\n";
    }
    WriteSocket(response.c_str(), response.length());
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
