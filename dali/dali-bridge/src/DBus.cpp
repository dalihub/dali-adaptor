/*
 * Copyright 2017  Samsung Electronics Co., Ltd
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

#include "DBus.hpp"
#include <sstream>
//#include "Atspi.hpp"
#include <iostream>
#include <mutex>

#define DBUS_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"

#undef EINA_FALSE
#undef EINA_TRUE
#define EINA_TRUE static_cast< Eina_Bool >( 1 )
#define EINA_FALSE static_cast< Eina_Bool >( 0 )

//#define DBUS_DEBUG(...) do { DBus::debugPrint(__FILE__, __LINE__, __VA_ARGS__); } while (0)

std::atomic< unsigned int > DBus::detail::CallId::LastId{0};
static std::function< void( const char*, size_t ) > debugPrinter;
static std::function< void( DBus::DBusAction ) > notificationCallback;
static std::mutex debugLock, notificationLock;

DBus::detail::CallOnDestructionList& DBus::detail::CallOnDestructionList::operator=( DBus::detail::CallOnDestructionList&& d )
{
  for( auto& q : functions )
  {
    q();
  }
  functions = std::move( d.functions );
  d.functions.clear();
  return *this;
}

void DBus::detail::CallOnDestructionList::add( const std::function< void() >& c )
{
  functions.push_back( c );
}

void DBus::setDBusActionNotifier( std::function< void( DBus::DBusAction ) > callback )
{
  std::lock_guard< std::mutex > lock( notificationLock );
  notificationCallback = std::move( callback );
}

void DBus::detail::emitNotification( const char* bus, const char* path, const char* interface, const char* member, DBus::DBusActionType type )
{
  std::lock_guard< std::mutex > lock( notificationLock );
  if( notificationCallback )
  {
    notificationCallback( DBusAction{type, bus, path, interface, member} );
  }
}

void DBus::setDebugPrinter( std::function< void( const char*, size_t ) > printer )
{
  std::lock_guard< std::mutex > lock( debugLock );
  debugPrinter = std::move( printer );
}

void DBus::debugPrint( const char* file, size_t line, const char* format, ... )
{
  std::function< void( const char*, size_t ) > debugPrintFunc;
  {
    std::lock_guard< std::mutex > lock( debugLock );
    if( !debugPrinter )
      return;
    debugPrintFunc = debugPrinter;
  }
  std::vector< char > buf( 4096 );
  int offset;
  while( true )
  {
    offset = snprintf( buf.data(), buf.size(), "%s:%u: ", file, static_cast< unsigned int >( line ) );
    if( offset < 0 )
      return;
    if( static_cast< size_t >( offset ) < buf.size() )
      break;
    buf.resize( offset + 1 );
  }

  while( true )
  {
    va_list args;
    va_start( args, format );
    int z = vsnprintf( buf.data() + offset, buf.size() - offset, format, args );
    va_end( args );
    if( z < 0 )
      return;
    bool done = static_cast< size_t >( z ) + static_cast< size_t >( offset ) < buf.size();
    buf.resize( static_cast< size_t >( z ) + static_cast< size_t >( offset ) );
    if( done )
      break;
  }
  debugPrintFunc( buf.data(), buf.size() );
}

std::shared_ptr< DBus::EldbusConnection > DBus::getDBusConnectionByName( const std::string& name )
{
  eldbus_init();
  auto z = getDBusConnectionByType( ConnectionType::SYSTEM );
  auto connection = eldbus_address_connection_get( name.c_str() );
  auto ptr = std::make_shared< EldbusConnection >( connection );
  eldbus_shutdown();
  return ptr;
}

std::shared_ptr< DBus::EldbusConnection > DBus::getDBusConnectionByType( ConnectionType connectionType )
{
  Eldbus_Connection_Type eldbusType = ELDBUS_CONNECTION_TYPE_SYSTEM;

  switch( connectionType )
  {
    case ConnectionType::SYSTEM:
    {
      eldbusType = ELDBUS_CONNECTION_TYPE_SYSTEM;
      break;
    }
    case ConnectionType::SESSION:
    {
      eldbusType = ELDBUS_CONNECTION_TYPE_SESSION;
      break;
    }
  }

  eldbus_init();
  auto connection = eldbus_connection_get( eldbusType );
  auto ptr = std::make_shared< EldbusConnection >( connection );
  eldbus_shutdown();
  return ptr;
}

DBus::DBusClient::DBusClient( std::string busName, std::string pathName, std::string interfaceName, ConnectionType tp ) : DBusClient( std::move( busName ), std::move( pathName ), std::move( interfaceName ), getDBusConnectionByType( tp ) )
{
}

struct caller_eldbus_object_unref
{
  void operator()( Eldbus_Object* p ) const
  {
    eldbus_object_unref( p );
  }
};

DBus::DBusClient::DBusClient( std::string busName, std::string pathName, std::string interfaceName, const std::shared_ptr< DBus::EldbusConnection >& conn )
{
  if( !conn )
    connectionState.connection = getDBusConnectionByType( ConnectionType::SESSION );
  else
    connectionState.connection = conn;

  std::ostringstream o;
  o << "bus = " << busName << " path = " << pathName << " connection = " << eldbus_connection_unique_name_get( connectionState.connection->get() );
  info = o.str();

  auto c = connectionState.connection;
  connectionState.object = {
      eldbus_object_get( connectionState.connection->get(), busName.c_str(), pathName.c_str() ),
      [c]( Eldbus_Object* p ) {
        eldbus_object_unref( p );
      }};
  if( connectionState.object )
  {
    auto obj = connectionState.object;
    connectionState.proxy = {
        eldbus_proxy_get( connectionState.object.get(), interfaceName.c_str() ),
        [obj]( Eldbus_Proxy* p ) {
          eldbus_proxy_unref( p );
        }};
    if( interfaceName != DBUS_INTERFACE_PROPERTIES )
    {
      auto obj = connectionState.object;
      connectionState.propertiesProxy = {
          eldbus_proxy_get( connectionState.object.get(), DBUS_INTERFACE_PROPERTIES ),
          [obj]( Eldbus_Proxy* p ) {
            eldbus_proxy_unref( p );
          }};
    }
    else
    {
      connectionState.propertiesProxy = connectionState.proxy;
    }
  }
  connectionInfo = std::make_shared< ConnectionInfo >();
  connectionInfo->busName = std::move( busName );
  connectionInfo->pathName = std::move( pathName );
  connectionInfo->interfaceName = std::move( interfaceName );
}

DBus::DBusServer::DBusServer( ConnectionType tp ) : DBus::DBusServer( DBus::getDBusConnectionByType( tp ) )
{
}

DBus::DBusServer::DBusServer( const std::shared_ptr< DBus::EldbusConnection >& conn )
{
  if( !conn )
    connection = getDBusConnectionByType( ConnectionType::SESSION );
  else
    connection = conn;
}

DBus::DBusInterfaceDescription::DBusInterfaceDescription( std::string interfaceName ) : interfaceName( std::move( interfaceName ) )
{
}

struct Implementation
{
  Eldbus_Service_Interface_Desc dsc;
  std::vector< Eldbus_Method > methods;
  std::vector< Eldbus_Signal > signals;
  std::vector< Eldbus_Property > properties;
  DBus::detail::StringStorage strings;

  std::unordered_map< std::string, DBus::DBusInterfaceDescription::MethodInfo > methodsMap;
  std::unordered_map< std::string, DBus::DBusInterfaceDescription::PropertyInfo > propertiesMap;
  std::unordered_map< unsigned int, DBus::DBusInterfaceDescription::SignalInfo > signalsMap;

  std::shared_ptr< DBus::EldbusConnection > connection;
};

static std::unordered_map< const Eldbus_Service_Interface*, std::unique_ptr< Implementation > > globalEntries;
static std::mutex globalEntriesMutex;
static thread_local const char* currentObjectPath = "";
static thread_local std::shared_ptr< DBus::EldbusConnection > currentConnection;

class CurrentObjectSetter
{
public:
  CurrentObjectSetter( std::shared_ptr< DBus::EldbusConnection > con, const Eldbus_Message* m )
  {
    currentObjectPath = eldbus_message_path_get( m );
    currentConnection = std::move( con );
  }
  ~CurrentObjectSetter()
  {
    currentObjectPath = "";
    currentConnection = {};
  }
  CurrentObjectSetter( const CurrentObjectSetter& ) = delete;
  CurrentObjectSetter( CurrentObjectSetter&& ) = delete;
  void operator=( const CurrentObjectSetter& ) = delete;
  void operator=( CurrentObjectSetter&& ) = delete;
};

std::string DBus::DBusServer::getCurrentObjectPath()
{
  return currentObjectPath;
}

std::shared_ptr< DBus::EldbusConnection > DBus::DBusServer::getCurrentConnection()
{
  return currentConnection;
}

static Eina_Bool property_get_callback( const Eldbus_Service_Interface* iface, const char* propertyName, Eldbus_Message_Iter* iter,
                                        const Eldbus_Message* message, Eldbus_Message** error )
{
  Implementation* impl = nullptr;
  {
    std::lock_guard< std::mutex > lock( globalEntriesMutex );
    auto it = globalEntries.find( iface );
    if( it != globalEntries.end() )
      impl = it->second.get();
  }
  if( !impl )
    return EINA_FALSE;

  auto it = impl->propertiesMap.find( propertyName );
  if( it == impl->propertiesMap.end() || !it->second.getCallback )
    return EINA_FALSE;

  CurrentObjectSetter currentObjectSetter( impl->connection, message );
  auto reply = it->second.getCallback( message, iter );
  if( !reply )
  {
    if( error )
      *error = eldbus_message_error_new( message, "org.freedesktop.DBus.Error.Failed", reply.getError().message.c_str() );
    return EINA_FALSE;
  }

  return EINA_TRUE;
}

static Eldbus_Message* property_set_callback( const Eldbus_Service_Interface* iface, const char* propertyName, Eldbus_Message_Iter* iter,
                                              const Eldbus_Message* message )
{
  Implementation* impl = nullptr;
  {
    std::lock_guard< std::mutex > lock( globalEntriesMutex );
    auto it = globalEntries.find( iface );
    if( it != globalEntries.end() )
      impl = it->second.get();
  }
  if( !impl )
  {
    auto ret = eldbus_message_error_new( message, "org.freedesktop.DBus.Error.Failed", "Unknown interface" );
    return ret;
  }
  auto it = impl->propertiesMap.find( propertyName );
  if( it == impl->propertiesMap.end() || !it->second.setCallback )
  {
    auto ret = eldbus_message_error_new( message, "org.freedesktop.DBus.Error.Failed", "Unknown setter" );
    return ret;
  }
  CurrentObjectSetter currentObjectSetter( impl->connection, message );
  auto reply = it->second.setCallback( message, iter );

  Eldbus_Message* ret = nullptr;
  if( !reply )
  {
    ret = eldbus_message_error_new( message, "org.freedesktop.DBus.Error.Failed", reply.getError().message.c_str() );
  }
  else
  {
    ret = eldbus_message_method_return_new( message );
  }
  return ret;
}

static Eldbus_Message* method_callback( const Eldbus_Service_Interface* iface, const Eldbus_Message* message )
{
  Implementation* impl = nullptr;
  {
    std::lock_guard< std::mutex > lock( globalEntriesMutex );
    auto it = globalEntries.find( iface );
    if( it != globalEntries.end() )
      impl = it->second.get();
  }
  if( !impl )
  {
    auto ret = eldbus_message_error_new( message, "org.freedesktop.DBus.Error.Failed", "Unknown interface" );
    return ret;
  }
  std::string memberName = eldbus_message_member_get( message );
  auto it = impl->methodsMap.find( memberName );
  if( it == impl->methodsMap.end() )
  {
    auto ret = eldbus_message_error_new( message, "org.freedesktop.DBus.Error.Failed", "Unknown method" );
    return ret;
  }
  CurrentObjectSetter currentObjectSetter( impl->connection, message );
  auto reply = it->second.callback( message );
  return reply;
}

static void addInterfaceImpl( bool fallback, const std::string& pathName,
                              const std::shared_ptr< DBus::EldbusConnection >& connection,
                              const std::string& interfaceName,
                              std::unordered_map< unsigned int, std::pair< const Eldbus_Service_Interface*, unsigned int > >& signalData,
                              DBus::detail::StringStorage& strings,
                              std::vector< DBus::DBusInterfaceDescription::MethodInfo >& dscrMethods,
                              std::vector< DBus::DBusInterfaceDescription::PropertyInfo >& dscrProperties,
                              std::vector< DBus::DBusInterfaceDescription::SignalInfo >& dscrSignals,
                              DBus::detail::CallOnDestructionList& destructors )
{
  std::vector< Eldbus_Method > methods;
  std::vector< Eldbus_Signal > signals;
  std::vector< Eldbus_Property > properties;
  std::unordered_map< std::string, DBus::DBusInterfaceDescription::MethodInfo > methodsMap;
  std::unordered_map< std::string, DBus::DBusInterfaceDescription::PropertyInfo > propertiesMap;
  std::unordered_map< unsigned int, DBus::DBusInterfaceDescription::SignalInfo > signalsMap;

  DBUS_DEBUG( "interface %s path %s on bus %s", interfaceName.c_str(), pathName.c_str(), DBus::getConnectionName( connection ).c_str() );
  for( auto& ee : dscrMethods )
  {
    auto key = ee.memberName;
    DBUS_DEBUG( "adding method %s", ee.memberName.c_str() );
    for( auto& r : ee.in )
    {
      if( !r.name )
        break;
      DBUS_DEBUG( "in %s '%s'", r.name, r.signature );
    }
    for( auto& r : ee.out )
    {
      if( !r.name )
        break;
      DBUS_DEBUG( "out %s '%s'", r.name, r.signature );
    }
    auto& e = ( methodsMap[key] = std::move( ee ) );
    methods.push_back( {} );
    auto& m = methods.back();
    m.member = e.memberName.c_str();
    m.in = e.in.data();
    m.out = e.out.data();
    m.cb = method_callback;
    m.flags = 0;
  }
  for( auto& ee : dscrProperties )
  {
    auto key = ee.memberName;
    DBUS_DEBUG( "adding property %s", ee.memberName.c_str() );
    auto& e = ( propertiesMap[key] = std::move( ee ) );
    properties.push_back( {} );
    auto& m = properties.back();
    m.name = e.memberName.c_str();
    m.type = e.typeSignature.c_str();
    m.get_func = e.getCallback ? property_get_callback : nullptr;
    m.set_func = e.setCallback ? property_set_callback : nullptr;
    m.flags = 0;
  }
  unsigned int signalIndex = 0;
  std::vector< unsigned int > signalIds;
  for( auto& ee : dscrSignals )
  {
    DBUS_DEBUG( "adding signal %s", ee.memberName.c_str() );
    auto& e = ( signalsMap[ee.id.id] = std::move( ee ) );
    signals.push_back( {} );
    auto& m = signals.back();
    m.name = e.memberName.c_str();
    m.args = e.args.data();
    m.flags = 0;
    signalData[e.id.id].second = signalIndex++;
    signalIds.push_back( e.id.id );
  }
  dscrMethods.clear();
  dscrProperties.clear();
  dscrSignals.clear();

  methods.push_back( {nullptr, nullptr, nullptr, nullptr, 0} );
  signals.push_back( {nullptr, nullptr, 0} );
  properties.push_back( {nullptr, nullptr, nullptr, nullptr, 0} );

  auto impl = std::unique_ptr< Implementation >( new Implementation{
      {interfaceName.c_str(),
       methods.data(),
       signals.data(),
       properties.data(),
       nullptr,
       nullptr},
      std::move( methods ),
      std::move( signals ),
      std::move( properties ),
      std::move( strings ),
      std::move( methodsMap ),
      std::move( propertiesMap ),
      std::move( signalsMap ),
      connection} );

  {
    std::lock_guard< std::mutex > lock( globalEntriesMutex );
    auto v = fallback ? eldbus_service_interface_fallback_register( connection->get(), pathName.c_str(), &impl->dsc ) : eldbus_service_interface_register( connection->get(), pathName.c_str(), &impl->dsc );
    assert( v );
    globalEntries[v] = std::move( impl );
    DBUS_DEBUG( "registering interface %p (%d)", v, fallback ? 1 : 0 );
    destructors.add( [=]() {
      eldbus_service_interface_unregister( v );
      std::lock_guard< std::mutex > lock( globalEntriesMutex );
      globalEntries.erase( v );
      DBUS_DEBUG( "unregistering interface %p (%d)", v, fallback ? 1 : 0 );
    } );
    for( auto id : signalIds )
    {
      signalData[id].first = v;
    }
  }
}

std::shared_ptr< DBus::EldbusConnection > DBus::DBusServer::getConnection()
{
  return connection;
}

void DBus::DBusServer::addInterface( const std::string& pathName, DBusInterfaceDescription& dscr, bool fallback )
{
  addInterfaceImpl( fallback, pathName, connection, dscr.interfaceName, signalData, dscr.strings, dscr.methods, dscr.properties, dscr.signals, destructors );
}

std::string DBus::DBusServer::getBusName() const
{
  return getConnectionName( connection );
}

std::string DBus::getConnectionName( const std::shared_ptr< DBus::EldbusConnection >& c )
{
  return eldbus_connection_unique_name_get( c->get() );
}
