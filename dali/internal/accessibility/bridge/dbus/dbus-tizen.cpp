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
#include <dali/integration-api/locale-numeric-guard.h>
#include <algorithm>
#include <locale>
#include <mutex>
#include <sstream>

// INTERNAL INCLUDES
#include <dali/internal/accessibility/bridge/dbus/dbus-wrapper-gdbus.h>
#include <dali/public-api/dali-adaptor-common.h>

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
  std::ostringstream prefixStream;
  prefixStream.imbue(std::locale::classic());
  prefixStream << file << ':' << static_cast<unsigned int>(line) << ": ";
  const std::string prefix = prefixStream.str();
  const size_t      offset = prefix.size();
  std::vector<char> buf(std::max<size_t>(4096, offset + 1));
  std::copy(prefix.begin(), prefix.end(), buf.begin());
  {
    // TODO: Replace vsnprintf when the printf-style variadic API can be changed.
    Dali::LocaleNumericGuard localeGuard;
    while(true)
    {
      va_list args;
      va_start(args, format);
      int z = vsnprintf(buf.data() + offset, buf.size() - offset, format, args);
      va_end(args);
      if(z < 0)
        return;
      const size_t formattedSize = offset + static_cast<size_t>(z);
      const bool   done          = formattedSize < buf.size();
      buf.resize(done ? formattedSize : formattedSize + 1);
      if(done)
        break;
    }
  }
  debugPrintFunc(buf.data(), buf.size());
}

DBusWrapper::ConnectionPtr DBus::getDBusConnectionByName(const std::string& name)
{
  return DBUS_W->dbus_address_connection_get_impl(name);
}

DBusWrapper::ConnectionPtr DBus::getDBusConnectionByType(DBusWrapper::ConnectionType connectionType)
{
  return DBUS_W->dbus_connection_get_impl(connectionType);
}

std::string DBus::getConnectionName(const DBusWrapper::ConnectionPtr& c)
{
  return DBUS_W->dbus_connection_unique_name_get_impl(c);
}

void DBus::requestBusName(const DBusWrapper::ConnectionPtr& conn, const std::string& bus)
{
  DBUS_W->dbus_name_request_impl(conn, bus);
}

void DBus::releaseBusName(const DBusWrapper::ConnectionPtr& conn, const std::string& bus)
{
  DBUS_W->dbus_name_release_impl(conn, bus);
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

  connectionState->object = DBUS_W->dbus_object_get_impl(connectionState->connection, busName.c_str(), pathName.c_str());
  if(connectionState->object)
  {
    connectionState->proxy = DBUS_W->dbus_proxy_get_impl(connectionState->object, interfaceName);
    if(interfaceName != DBUS_INTERFACE_PROPERTIES)
    {
      connectionState->propertiesProxy = DBUS_W->dbus_proxy_get_impl(connectionState->object, DBUS_INTERFACE_PROPERTIES);
    }
    else
    {
      connectionState->propertiesProxy = DBUS_W->dbus_proxy_copy_impl(connectionState->proxy);
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

static std::unique_ptr<DBusWrapper> InstalledWrapper;

DBusWrapper* DBusWrapper::Installed()
{
  if(!InstalledWrapper)
  {
    InstalledWrapper.reset(new GdbusDBusWrapper);
  }
  return InstalledWrapper.get();
}

void DBusWrapper::Install(std::unique_ptr<DBusWrapper> w)
{
  InstalledWrapper = std::move(w);
}
