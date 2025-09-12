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

#include <Ecore_Input.h>
#include <Eldbus.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

#define DBUS_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"

#undef EINA_FALSE
#undef EINA_TRUE
#define EINA_TRUE static_cast<Eina_Bool>(1)
#define EINA_FALSE static_cast<Eina_Bool>(0)

//#define DBUS_DEBUG(...) do { DBus::debugPrint(__FILE__, __LINE__, __VA_ARGS__); } while (0)

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

DBusWrapper::ConnectionPtr DBus::getDBusConnectionByType(ConnectionType connectionType)
{
  return DBUS_W->eldbus_connection_get_impl(connectionType);
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

bool DBus::DBusClient::getFromEinaValue(const _Eina_Value* v, void* dst)
{
  return eina_value_get(const_cast<Eina_Value*>(v), dst);
}

static std::unique_ptr<DBusWrapper> InstalledWrapper;

struct DefaultDBusWrapper : public DBusWrapper
{
  constexpr static int ELDBUS_CALL_TIMEOUT = 1000;

  DefaultDBusWrapper()
  {
  }

  ~DefaultDBusWrapper()
  {
  }

#define DEFINE_GS(name, eldbus_name, unref_call)                                                   \
  static eldbus_name* get(const std::shared_ptr<name>& a)                                          \
  {                                                                                                \
    return static_cast<name##Impl*>(a.get())->Value;                                               \
  }                                                                                                \
  static eldbus_name* release(const std::shared_ptr<name>& a)                                      \
  {                                                                                                \
    auto z                                   = static_cast<name##Impl*>(a.get())->Value;           \
    static_cast<name##Impl*>(a.get())->Value = nullptr;                                            \
    return z;                                                                                      \
  }                                                                                                \
  template<typename... ARGS>                                                                       \
  static std::shared_ptr<name> create(const eldbus_name* v, ARGS&&... args)                        \
  {                                                                                                \
    return std::make_shared<name##Impl>(const_cast<eldbus_name*>(v), std::forward<ARGS>(args)...); \
  }

#define DEFINE_TYPE(name, eldbus_name, unref_call)           \
  struct name##Impl : public name                            \
  {                                                          \
    eldbus_name* Value       = nullptr;                      \
    bool         EraseOnExit = false;                        \
    name##Impl(eldbus_name* Value, bool EraseOnExit = false) \
    : Value(Value),                                          \
      EraseOnExit(EraseOnExit)                               \
    {                                                        \
    }                                                        \
    ~name##Impl()                                            \
    {                                                        \
      if(EraseOnExit && Value)                               \
      {                                                      \
        unref_call;                                          \
      }                                                      \
    }                                                        \
  };                                                         \
  DEFINE_GS(name, eldbus_name, unref_call)

  struct ConnectionImpl : public Connection
  {
    Eldbus_Connection* Value       = nullptr;
    bool               EraseOnExit = false;
    ConnectionImpl(Eldbus_Connection* Value, bool EraseOnExit = false)
    : Value(Value),
      EraseOnExit(EraseOnExit)
    {
      ecore_event_init();
      eldbus_init();
    }

    ~ConnectionImpl()
    {
      if(EraseOnExit && Value)
      {
        eldbus_connection_unref(Value);
      }
      eldbus_shutdown();
      ecore_event_shutdown();
    }
  };

  struct MessageIterImpl : public MessageIter
  {
    Eldbus_Message_Iter *Value = nullptr, *Parent = nullptr;
    bool                 EraseOnExit = false;
    MessageIterImpl(Eldbus_Message_Iter* Value, Eldbus_Message_Iter* Parent, bool EraseOnExit = false)
    : Value(Value),
      Parent(Parent),
      EraseOnExit(EraseOnExit)
    {
    }

    ~MessageIterImpl()
    {
      if(EraseOnExit && Value && Parent)
      {
        eldbus_message_iter_container_close(Parent, Value);
      }
    }
  };

  DEFINE_GS(Connection, Eldbus_Connection, )
  DEFINE_GS(MessageIter, Eldbus_Message_Iter, )
  DEFINE_TYPE(Message, Eldbus_Message, eldbus_message_unref(Value))
  DEFINE_TYPE(Proxy, Eldbus_Proxy, )
  DEFINE_TYPE(Object, Eldbus_Object, eldbus_object_unref(Value))
  DEFINE_TYPE(Pending, Eldbus_Pending, )
  DEFINE_TYPE(EventPropertyChanged, Eldbus_Proxy_Event_Property_Changed, )
#undef DEFINE_TYPE

  std::shared_ptr<Connection> eldbus_address_connection_get_impl(const std::string& addr) override
  {
    eldbus_init();
    auto p = eldbus_address_connection_get(addr.c_str());
    auto w = create(p, true);
    eldbus_shutdown();
    return w;
  }

#define eldbus_message_iter_arguments_append_impl_basic(type, sig)                            \
  void eldbus_message_iter_arguments_append_impl(const MessageIterPtr& it, type src) override \
  {                                                                                           \
    eldbus_message_iter_arguments_append(get(it), #sig, src);                                 \
  }                                                                                           \
  bool eldbus_message_iter_get_and_next_impl(const MessageIterPtr& it, type& dst) override    \
  {                                                                                           \
    return eldbus_message_iter_get_and_next(get(it), (#sig)[0], &dst);                        \
  }

  // clang-format off
  eldbus_message_iter_arguments_append_impl_basic(uint8_t, y)
  eldbus_message_iter_arguments_append_impl_basic(uint16_t, q)
  eldbus_message_iter_arguments_append_impl_basic(uint32_t, u)
  eldbus_message_iter_arguments_append_impl_basic(uint64_t, t)
  eldbus_message_iter_arguments_append_impl_basic(int16_t, n)
  eldbus_message_iter_arguments_append_impl_basic(int32_t, i)
  eldbus_message_iter_arguments_append_impl_basic(int64_t, x)
  eldbus_message_iter_arguments_append_impl_basic(double, d)
  // clang-format on

#undef eldbus_message_iter_arguments_append_impl_basic

                  void eldbus_message_iter_arguments_append_impl(const MessageIterPtr& it, bool src) override
  {
    eldbus_message_iter_arguments_append(get(it), "b", src ? 1 : 0);
  }

  bool eldbus_message_iter_get_and_next_impl(const MessageIterPtr& it, bool& dst) override
  {
    unsigned char q;
    auto          z = eldbus_message_iter_get_and_next(get(it), 'b', &q);
    dst             = q != 0;
    return z;
  }

  void eldbus_message_iter_arguments_append_impl(const MessageIterPtr& it, const std::string& src) override
  {
    eldbus_message_iter_arguments_append(get(it), "s", src.c_str());
  }

  bool eldbus_message_iter_get_and_next_impl(const MessageIterPtr& it, std::string& dst) override
  {
    auto        iter = get(it);
    const char* q;
    if(!eldbus_message_iter_get_and_next(iter, 's', &q))
    {
      if(!eldbus_message_iter_get_and_next(iter, 'o', &q))
      {
        return false;
      }
    }
    dst = q;
    return true;
  }

  void eldbus_message_iter_arguments_append_impl(const MessageIterPtr& it, const ObjectPath& src) override
  {
    eldbus_message_iter_arguments_append(get(it), "o", src.value.c_str());
  }

  bool eldbus_message_iter_get_and_next_impl(const MessageIterPtr& it, ObjectPath& dst) override
  {
    const char* q;
    if(!eldbus_message_iter_get_and_next(get(it), 'o', &q))
    {
      return false;
    }
    dst.value = q;
    return true;
  }

  MessageIterPtr eldbus_message_iter_container_new_impl(const MessageIterPtr& it, int type, const std::string& sig) override
  {
    auto z = eldbus_message_iter_container_new(get(it), type, !sig.empty() ? sig.c_str() : NULL);
    return create(z, get(it), true);
  }

  MessageIterPtr eldbus_message_iter_get_and_next_by_type_impl(const MessageIterPtr& it, int type) override
  {
    Eldbus_Message_Iter* entry;
    if(!eldbus_message_iter_get_and_next(get(it), type, &entry))
    {
      return {};
    }
    return create(entry, get(it), false);
  }

  MessageIterPtr eldbus_message_iter_get_impl(const MessagePtr& msg, bool) override
  {
    return create(eldbus_message_iter_get(get(msg)), nullptr, false);
  }

  MessagePtr eldbus_proxy_method_call_new_impl(const ProxyPtr& proxy, const std::string& funcName)
  {
    return create(eldbus_proxy_method_call_new(get(proxy), funcName.c_str()), true);
  }

  MessagePtr eldbus_proxy_send_and_block_impl(const ProxyPtr& proxy, const MessagePtr& msg) override
  {
    return create(eldbus_proxy_send_and_block(get(proxy), release(msg), ELDBUS_CALL_TIMEOUT), true);
  }

  bool eldbus_message_error_get_impl(const MessagePtr& msg, std::string& name, std::string& text) override
  {
    const char *errname, *errmsg;
    if(eldbus_message_error_get(get(msg), &errname, &errmsg))
    {
      name = errname;
      text = errmsg;
      return true;
    }
    return false;
  }

  std::string eldbus_message_signature_get_impl(const MessagePtr& msg) override
  {
    return eldbus_message_signature_get(get(msg));
  }

  static void callAsyncCb(void* data, const Eldbus_Message* msg, Eldbus_Pending* pending)
  {
    auto d = static_cast<SendCallback*>(data);
    (*d)(create(msg, false));
  }

  static void pendingFreeCb(void* data, const void*)
  {
    auto d = static_cast<SendCallback*>(data);
    delete d;
  }

  static void listenerCallbackFree(void* data, const void*)
  {
    auto d = static_cast<std::function<void(const Eldbus_Message* msg)>*>(data);
    delete d;
  }

  PendingPtr eldbus_proxy_send_impl(const ProxyPtr& proxy, const MessagePtr& msg, const SendCallback& callback) override
  {
    auto cb      = new SendCallback{callback};
    auto pending = eldbus_proxy_send(get(proxy), release(msg), callAsyncCb, cb, ELDBUS_CALL_TIMEOUT);
    if(pending)
    {
      eldbus_pending_free_cb_add(pending, pendingFreeCb, cb);
    }
    else
    {
      delete cb;
    }
    return create(pending, false);
  }

  std::string eldbus_proxy_interface_get_impl(const ProxyPtr& proxy) override
  {
    return eldbus_proxy_interface_get(get(proxy));
  }

  static void listenerCallback(void* data, const Eldbus_Message* msg)
  {
    auto p = static_cast<std::function<void(const Eldbus_Message* msg)>*>(data);
    (*p)(msg);
  }

  void eldbus_proxy_signal_handler_add_impl(const ProxyPtr& proxy, const std::string& member, const std::function<void(const MessagePtr&)>& cb) override
  {
    auto tmp = new std::function<void(const Eldbus_Message* msg)>{
      [cb](const Eldbus_Message* msg)
    {
      cb(create(msg, false));
    }};
    auto handler = eldbus_proxy_signal_handler_add(get(proxy), member.c_str(), listenerCallback, tmp);
    if(handler)
    {
      eldbus_proxy_free_cb_add(get(proxy), listenerCallbackFree, tmp);
    }
    else
    {
      delete tmp;
    }
  }

  std::string eldbus_message_iter_signature_get_impl(const MessageIterPtr& iter) override
  {
    return eldbus_message_iter_signature_get(get(iter));
  }

  MessagePtr eldbus_message_method_return_new_impl(const MessagePtr& msg) override
  {
    return create(eldbus_message_method_return_new(get(msg)), true);
  }

  MessagePtr eldbus_message_error_new_impl(const MessagePtr& msg, const std::string& err, const std::string& txt) override
  {
    return create(eldbus_message_error_new(get(msg), err.c_str(), txt.c_str()), true);
  }

  PendingPtr eldbus_connection_send_impl(const ConnectionPtr& conn, const MessagePtr& msg) override
  {
    return create(eldbus_connection_send(get(conn), release(msg), NULL, NULL, -1));
  }

  MessagePtr eldbus_message_signal_new_impl(const std::string& path, const std::string& iface, const std::string& name) override
  {
    return create(eldbus_message_signal_new(path.c_str(), iface.c_str(), name.c_str()), true);
  }

  MessagePtr eldbus_message_ref_impl(const MessagePtr& msg) override
  {
    return create(eldbus_message_ref(get(msg)), true);
  }

  ConnectionPtr eldbus_connection_get_impl(ConnectionType type) override
  {
    Eldbus_Connection_Type eldbusType = ELDBUS_CONNECTION_TYPE_SYSTEM;

    switch(type)
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

    auto p = eldbus_connection_get(eldbusType);
    if(!p)
    {
      DALI_LOG_ERROR("cannot get dbus connection\n");
      return NULL;
    }
    auto w = create(p, true);
    return w;
  }

  std::string eldbus_connection_unique_name_get_impl(const ConnectionPtr& conn) override
  {
    return eldbus_connection_unique_name_get(get(conn));
  }

  ObjectPtr eldbus_object_get_impl(const ConnectionPtr& conn, const std::string& bus, const std::string& path) override
  {
    return create(eldbus_object_get(get(conn), bus.c_str(), path.c_str()), true);
  }

  ProxyPtr eldbus_proxy_get_impl(const ObjectPtr& obj, const std::string& interface) override
  {
    return create(eldbus_proxy_get(get(obj), interface.c_str()), false);
  }

  ProxyPtr eldbus_proxy_copy_impl(const ProxyPtr& ptr) override
  {
    return create(get(ptr), false);
  }

  void eldbus_name_request_impl(const ConnectionPtr& conn, const std::string& bus) override
  {
    eldbus_name_request(get(conn), bus.c_str(), ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE, nullptr, nullptr);
  }

  void eldbus_name_release_impl(const ConnectionPtr& conn, const std::string& bus) override
  {
    eldbus_name_release(get(conn), bus.c_str(), nullptr, nullptr);
  }

  struct Implementation
  {
    Eldbus_Service_Interface_Desc             dsc;
    std::vector<std::vector<Eldbus_Arg_Info>> argsInfos;
    std::vector<Eldbus_Method>                methods;
    std::vector<Eldbus_Signal>                signals;
    std::vector<Eldbus_Property>              properties;

    std::unordered_map<std::string, DBusWrapper::MethodInfo>   methodsMap;
    std::unordered_map<std::string, DBusWrapper::PropertyInfo> propertiesMap;
    std::unordered_map<unsigned int, DBusWrapper::SignalInfo>  signalsMap;

    DBusWrapper::ConnectionWeakPtr connection;
  };

  struct GlobalEntries
  {
    static GlobalEntries& Get()
    {
      static GlobalEntries instance;
      return instance;
    }

    Implementation* Find(const Eldbus_Service_Interface* iface)
    {
      Implementation*             impl = nullptr;
      std::lock_guard<std::mutex> lock(globalEntriesMutex);
      auto                        it = globalEntries.find(iface);
      if(it != globalEntries.end())
      {
        impl = it->second.get();
      }
      return impl;
    }

    void Add(const Eldbus_Service_Interface* iface, std::unique_ptr<Implementation> impl)
    {
      std::lock_guard<std::mutex> lock(globalEntriesMutex);
      globalEntries[iface] = std::move(impl);
    }

    void Erase(const Eldbus_Service_Interface* iface)
    {
      std::lock_guard<std::mutex> lock(globalEntriesMutex);
      globalEntries.erase(iface);
    }

  private:
    std::unordered_map<const Eldbus_Service_Interface*, std::unique_ptr<Implementation>> globalEntries;
    std::mutex                                                                           globalEntriesMutex;
  };

#undef EINA_FALSE
#undef EINA_TRUE
#define EINA_FALSE static_cast<Eina_Bool>(0)
#define EINA_TRUE static_cast<Eina_Bool>(1)

  static Eina_Bool property_get_callback(const Eldbus_Service_Interface* iface, const char* propertyName, Eldbus_Message_Iter* iter, const Eldbus_Message* message, Eldbus_Message** error)
  {
    Implementation* impl = GlobalEntries::Get().Find(iface);
    if(!impl)
    {
      return EINA_FALSE;
    }

    auto it = impl->propertiesMap.find(propertyName);
    if(it == impl->propertiesMap.end() || !it->second.getCallback)
    {
      return EINA_FALSE;
    }

    auto connection = impl->connection.lock();
    if(!connection)
    {
      return EINA_FALSE;
    }

    DBus::DBusServer::CurrentObjectSetter currentObjectSetter(connection, eldbus_message_path_get(message));
    auto                                  reply = it->second.getCallback(create(message, false), create(iter, nullptr, false));
    if(!reply.empty())
    {
      if(error)
      {
        *error = eldbus_message_error_new(message, "org.freedesktop.DBus.Error.Failed", reply.c_str());
      }
      return EINA_FALSE;
    }

    return EINA_TRUE;
  }

  static Eldbus_Message* property_set_callback(const Eldbus_Service_Interface* iface, const char* propertyName, Eldbus_Message_Iter* iter, const Eldbus_Message* message)
  {
    Implementation* impl = GlobalEntries::Get().Find(iface);
    if(!impl)
    {
      auto ret = eldbus_message_error_new(message, "org.freedesktop.DBus.Error.Failed", "Unknown interface");
      return ret;
    }
    auto it = impl->propertiesMap.find(propertyName);
    if(it == impl->propertiesMap.end() || !it->second.setCallback)
    {
      auto ret = eldbus_message_error_new(message, "org.freedesktop.DBus.Error.Failed", "Unknown setter");
      return ret;
    }
    auto connection = impl->connection.lock();
    if(!connection)
    {
      auto ret = eldbus_message_error_new(message, "org.freedesktop.DBus.Error.Failed", "Connection lost");
      return ret;
    }

    DBus::DBusServer::CurrentObjectSetter currentObjectSetter(connection, eldbus_message_path_get(message));
    auto                                  reply = it->second.setCallback(create(message, false), create(iter, nullptr, false));

    Eldbus_Message* ret = nullptr;
    if(!reply.empty())
    {
      ret = eldbus_message_error_new(message, "org.freedesktop.DBus.Error.Failed", reply.c_str());
    }
    else
    {
      ret = eldbus_message_method_return_new(message);
    }
    return ret;
  }

  static Eldbus_Message* method_callback(const Eldbus_Service_Interface* iface, const Eldbus_Message* message)
  {
    Implementation* impl = GlobalEntries::Get().Find(iface);
    if(!impl)
    {
      auto ret = eldbus_message_error_new(message, "org.freedesktop.DBus.Error.Failed", "Unknown interface");
      return ret;
    }
    std::string memberName = eldbus_message_member_get(message);
    auto        it         = impl->methodsMap.find(memberName);
    if(it == impl->methodsMap.end())
    {
      auto ret = eldbus_message_error_new(message, "org.freedesktop.DBus.Error.Failed", "Unknown method");
      return ret;
    }
    auto connection = impl->connection.lock();
    if(!connection)
    {
      auto ret = eldbus_message_error_new(message, "org.freedesktop.DBus.Error.Failed", "Connection lost");
      return ret;
    }
    DBus::DBusServer::CurrentObjectSetter currentObjectSetter(connection, eldbus_message_path_get(message));
    auto                                  reply = it->second.callback(create(message));
    return release(reply);
  }

  void add_interface_impl(bool                                fallback,
                          const std::string&                  pathName,
                          const ConnectionPtr&                connection,
                          std::vector<std::function<void()>>& destructors,
                          const std::string&                  interfaceName,
                          std::vector<MethodInfo>&            dscrMethods,
                          std::vector<PropertyInfo>&          dscrProperties,
                          std::vector<SignalInfo>&            dscrSignals) override
  {
    std::vector<Eldbus_Method>                                                    methods;
    std::vector<Eldbus_Signal>                                                    signals;
    std::vector<Eldbus_Property>                                                  properties;
    std::vector<std::vector<Eldbus_Arg_Info>>                                     argsInfos;
    std::unordered_map<std::string, DBus::DBusInterfaceDescription::MethodInfo>   methodsMap;
    std::unordered_map<std::string, DBus::DBusInterfaceDescription::PropertyInfo> propertiesMap;
    std::unordered_map<unsigned int, DBus::DBusInterfaceDescription::SignalInfo>  signalsMap;

    DBUS_DEBUG("interface %s path %s on bus %s", interfaceName.c_str(), pathName.c_str(), DBus::getConnectionName(connection).c_str());
    auto makeArgInfo = [&](const std::vector<std::pair<std::string, std::string>>& input)
    {
      argsInfos.push_back({});
      auto& dst = argsInfos.back();
      for(auto& s : input)
      {
        auto a = Strings.add(s.first);
        auto b = Strings.add(s.second);
        dst.push_back({a, b});
      }
      dst.push_back({nullptr, nullptr});
      return dst.data();
    };
    for(auto& ee : dscrMethods)
    {
      auto key = ee.memberName;
      DBUS_DEBUG("adding method %s", ee.memberName.c_str());
      for(auto& r : ee.in)
      {
        DBUS_DEBUG("in %s '%s'", r.first.c_str(), r.second.c_str());
      }
      for(auto& r : ee.out)
      {
        DBUS_DEBUG("out %s '%s'", r.first.c_str(), r.second.c_str());
      }
      auto& e = (methodsMap[key] = std::move(ee));
      methods.push_back({});
      auto& m  = methods.back();
      m.member = e.memberName.c_str();
      m.in     = makeArgInfo(e.in);
      m.out    = makeArgInfo(e.out);
      m.cb     = method_callback;
      m.flags  = 0;
    }
    for(auto& ee : dscrProperties)
    {
      auto key = ee.memberName;
      DBUS_DEBUG("adding property %s", ee.memberName.c_str());
      auto& e = (propertiesMap[key] = std::move(ee));
      properties.push_back({});
      auto& m    = properties.back();
      m.name     = e.memberName.c_str();
      m.type     = e.typeSignature.c_str();
      m.get_func = e.getCallback ? property_get_callback : nullptr;
      m.set_func = e.setCallback ? property_set_callback : nullptr;
      m.flags    = 0;
    }
    dscrMethods.clear();
    dscrProperties.clear();
    dscrSignals.clear();

    methods.push_back({nullptr, nullptr, nullptr, nullptr, 0});
    signals.push_back({nullptr, nullptr, 0});
    properties.push_back({nullptr, nullptr, nullptr, nullptr, 0});

    auto impl = std::unique_ptr<Implementation>(new Implementation{
      {interfaceName.c_str(),
       methods.data(),
       signals.data(),
       properties.data(),
       nullptr,
       nullptr},
      std::move(argsInfos),
      std::move(methods),
      std::move(signals),
      std::move(properties),
      std::move(methodsMap),
      std::move(propertiesMap),
      std::move(signalsMap),
      connection});

    auto v = fallback ? eldbus_service_interface_fallback_register(get(connection), pathName.c_str(), &impl->dsc) : eldbus_service_interface_register(get(connection), pathName.c_str(), &impl->dsc);
    DALI_ASSERT_ALWAYS(v && "Eldbus register failed!");
    GlobalEntries::Get().Add(v, std::move(impl));
    DBUS_DEBUG("registering interface %p (%d)", v, fallback ? 1 : 0);
    destructors.push_back([=]()
    {
      DBUS_DEBUG("unregistering interface %p", v);
      GlobalEntries::Get().Erase(v);
      eldbus_service_interface_unregister(v);
    });
  }

  static void listenerEventChangedCallback(void* data, Eldbus_Proxy* proxy EINA_UNUSED, void* event)
  {
    auto p = static_cast<std::function<void(Eldbus_Proxy_Event_Property_Changed*)>*>(data);
    (*p)(static_cast<Eldbus_Proxy_Event_Property_Changed*>(event));
  }

  static void ProxyEventCallbackDelCb(void* data, const void* obj)
  {
    auto d = static_cast<std::function<void(Eldbus_Proxy_Event_Property_Changed*)>*>(data);
    delete d;
  }

  void add_property_changed_event_listener_impl(const ProxyPtr& proxy, const std::string& interface, const std::string& name, std::function<void(const Eina_Value*)> cb) override
  {
    auto callbackLambdaPtr = new std::function<void(Eldbus_Proxy_Event_Property_Changed * epc)>{
      [cb, name, interface](Eldbus_Proxy_Event_Property_Changed* ev)
    {
      const char* ifc = eldbus_proxy_interface_get(ev->proxy);
      if(ev->name && ev->name == name && ifc && interface == ifc)
      {
        cb(ev->value);
      }
    }};
    auto p = get(proxy);
    eldbus_proxy_event_callback_add(p, ELDBUS_PROXY_EVENT_PROPERTY_CHANGED, listenerEventChangedCallback, callbackLambdaPtr);
    eldbus_proxy_free_cb_add(p, ProxyEventCallbackDelCb, callbackLambdaPtr);
  }
};

DBusWrapper* DBusWrapper::Installed()
{
  if(!InstalledWrapper)
  {
    InstalledWrapper.reset(new DefaultDBusWrapper);
  }
  return InstalledWrapper.get();
}

void DBusWrapper::Install(std::unique_ptr<DBusWrapper> w)
{
  InstalledWrapper = std::move(w);
}
