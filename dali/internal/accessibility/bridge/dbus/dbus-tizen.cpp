/*
 * Copyright 2019  Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// CLASS HEADER
#include <dali/internal/accessibility/bridge/accessibility-common.h>
#include <dali/internal/accessibility/bridge/dbus/dbus.h>

// EXTERNAL INCLUDES
#include <iostream>
#include <mutex>
#include <sstream>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/internal/accessibility/bridge/dbus/dbus-wrapper-eldbus.h>


#define DBUS_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"

std::atomic<unsigned int>                       DBus::detail::CallId::LastId{0};
static std::function<void(const char*, size_t)> debugPrinter;
static std::mutex                               debugLock;

thread_local std::string                DBus::DBusServer::currentObjectPath;
thread_local DBusWrapper::ConnectionPtr DBus::DBusServer::currentConnection;

void DBus::setDebugPrinter(std::function<void(const char*, size_t)> printer)
{
  std::lock_guard<std::mutex> lock(debugLock);
  debugPrinter = std::move(printer);
}

void DBus::debugPrint(const char* file, size_t line, const char* format, ...)
{
  std::function<void(const char*, size_t)> debugPrintFunc;
  {
    std::lock_guard<std::mutex> lock(debugLock);
    if(!debugPrinter)
      return;
    debugPrintFunc = debugPrinter;
  }
  std::vector<char> buf(4096);
  int               offset;
  while(true)
  {
    offset = snprintf(buf.data(), buf.size(), "%s:%u: ", file, static_cast<unsigned int>(line));
    if(offset < 0)
      return;
    if(static_cast<size_t>(offset) < buf.size())
      break;
    buf.resize(offset + 1);
  }

  while(true)
  {
    va_list args;
    va_start(args, format);
    int z = vsnprintf(buf.data() + offset, buf.size() - offset, format, args);
    va_end(args);
    if(z < 0)
      return;
    bool done = static_cast<size_t>(z) + static_cast<size_t>(offset) < buf.size();
    buf.resize(static_cast<size_t>(z) + static_cast<size_t>(offset));
    if(done)
      break;
  }
  debugPrintFunc(buf.data(), buf.size());
}

DBusWrapper::ConnectionPtr DBus::getDBusConnectionByName(const std::string& name)
{
  return DBUS_W->eldbus_address_connection_get_impl(name);
}

DBusWrapper::ConnectionPtr DBus::getDBusConnectionByType(DBusWrapper::ConnectionType connectionType)
{
  return DBUS_W->eldbus_connection_get_impl(connectionType);
}

std::string DBus::getConnectionName(const DBusWrapper::ConnectionPtr& c)
{
  return DBUS_W->eldbus_connection_unique_name_get_impl(c);
}

void DBus::requestBusName(const DBusWrapper::ConnectionPtr& conn, const std::string& bus)
{
  DBUS_W->eldbus_name_request_impl(conn, bus);
}

void DBus::releaseBusName(const DBusWrapper::ConnectionPtr& conn, const std::string& bus)
{
  DBUS_W->eldbus_name_release_impl(conn, bus);
}


DBus::DBusClient::DBusClient(std::string busName, std::string pathName, std::string interfaceName, ConnectionType tp)
: DBusClient(std::move(busName), std::move(pathName), std::move(interfaceName), getDBusConnectionByType(tp))
{
}

DBus::DBusClient::DBusClient(std::string busName, std::string pathName, std::string interfaceName, const DBusWrapper::ConnectionPtr& conn)
{
  if(!conn)
    connectionState->connection = getDBusConnectionByType(ConnectionType::SESSION);
  else
    connectionState->connection = conn;

  if(!connectionState->connection)
  {
    DALI_LOG_ERROR("DBusClient connection is not ready\n");
    return;
  }

  connectionState->object = DBUS_W->eldbus_object_get_impl(connectionState->connection, busName.c_str(), pathName.c_str());
  if(connectionState->object)
  {
    connectionState->proxy = DBUS_W->eldbus_proxy_get_impl(connectionState->object, interfaceName);
    if(interfaceName != DBUS_INTERFACE_PROPERTIES)
    {
      connectionState->propertiesProxy = DBUS_W->eldbus_proxy_get_impl(connectionState->object, DBUS_INTERFACE_PROPERTIES);
    }
    else
    {
      connectionState->propertiesProxy = DBUS_W->eldbus_proxy_copy_impl(connectionState->proxy);
    }
  }
  connectionInfo                = std::make_shared<ConnectionInfo>();
  connectionInfo->busName       = std::move(busName);
  connectionInfo->pathName      = std::move(pathName);
  connectionInfo->interfaceName = std::move(interfaceName);
}

DBus::DBusServer::DBusServer(ConnectionType tp)
: DBus::DBusServer(DBus::getDBusConnectionByType(tp))
{
}

DBus::DBusServer::DBusServer(const DBusWrapper::ConnectionPtr& conn)
{
  if(!conn)
    connection = getDBusConnectionByType(ConnectionType::SESSION);
  else
    connection = conn;
}

DBus::DBusInterfaceDescription::DBusInterfaceDescription(std::string interfaceName)
: interfaceName(std::move(interfaceName))
{
}

void DBus::DBusServer::addInterface(const std::string& pathName, DBusInterfaceDescription& dscr, bool fallback)
{
  DBUS_W->add_interface_impl(fallback, pathName, connection, destructorObject->destructors, dscr.interfaceName, dscr.methods, dscr.properties, dscr.signals);
}

std::string DBus::DBusServer::getBusName() const
{
  return getConnectionName(connection);
}

bool DBus::DBusClient::getFromEinaValue(const _Eina_Value* v, void* dst)
{
  return eina_value_get(const_cast<Eina_Value*>(v), dst);
}

static std::unique_ptr<DBusWrapper> InstalledWrapper;

DBusWrapper* DBusWrapper::Installed()
{
  if(!InstalledWrapper)
  {
    InstalledWrapper.reset(new EldbusDBusWrapper);
  }
  return InstalledWrapper.get();
}

void DBusWrapper::Install(std::unique_ptr<DBusWrapper> w)
{
  InstalledWrapper = std::move(w);
}
