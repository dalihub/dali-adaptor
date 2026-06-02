/* Copyright 2026  Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_DBUS_WRAPPER_GDBUS_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_DBUS_WRAPPER_GDBUS_H

#include <dali/internal/accessibility/bridge/accessibility-common.h>
#include <dali/internal/accessibility/bridge/dbus/dbus.h>

// EXTERNAL INCLUDES
#include <gio/gio.h>
#include <glib.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>


#define DBUS_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"


struct GdbusDBusWrapper : public DBusWrapper
{
  constexpr static int GDBUS_CALL_TIMEOUT = 1000;

  GdbusDBusWrapper()
  {
  }

  ~GdbusDBusWrapper()
  {
  }

#define DEFINE_GS(name, g_dbus_name, unref_call)                                                   \
  static g_dbus_name* get(const std::shared_ptr<name>& a)                                          \
  {                                                                                                \
    return static_cast<name##Impl*>(a.get())->Value;                                               \
  }                                                                                                \
  static g_dbus_name* release(const std::shared_ptr<name>& a)                                      \
  {                                                                                                \
    auto z                                   = static_cast<name##Impl*>(a.get())->Value;           \
    static_cast<name##Impl*>(a.get())->Value = nullptr;                                            \
    return z;                                                                                      \
  }                                                                                                \
  template<typename... ARGS>                                                                       \
  static std::shared_ptr<name> create(const g_dbus_name* v, ARGS&&... args)                        \
  {                                                                                                \
    return std::make_shared<name##Impl>(const_cast<g_dbus_name*>(v), std::forward<ARGS>(args)...); \
  }

#define DEFINE_TYPE(name, g_dbus_name, unref_call)           \
  struct name##Impl : public name                            \
  {                                                          \
    g_dbus_name* Value       = nullptr;                      \
    bool         EraseOnExit = false;                        \
    name##Impl(g_dbus_name* Value, bool EraseOnExit = false) \
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
  DEFINE_GS(name, g_dbus_name, )

  struct MessageIterImpl : public MessageIter
  {
    GVariant* Value = nullptr;
    GVariantIter* Iter = nullptr;
    GVariantBuilder* Builder = nullptr;
    bool EraseOnExit = false;

    MessageIterImpl(GVariant* value, GVariantIter* iter, bool eraseOnExit = false)
    : Value(value), Iter(iter), EraseOnExit(eraseOnExit)
    {
    }

    MessageIterImpl(GVariantBuilder* builder, bool eraseOnExit = false)
    : Builder(builder), EraseOnExit(eraseOnExit)
    {
    }

    MessageIterImpl()
    {
    }

    ~MessageIterImpl()
    {
      if (EraseOnExit)
      {
        if (Value)
        {
          g_variant_unref(Value);
        }
        if (Iter)
        {
          g_variant_iter_free(Iter);
        }
        if (Builder)
        {
          g_variant_builder_unref(Builder);
        }
      }
    }
  };

  static GVariant* get_value(const std::shared_ptr<MessageIter>& iter)
  {
    return static_cast<MessageIterImpl*>(iter.get())->Value;
  }

  static GVariantIter* get_iter(const std::shared_ptr<MessageIter>& iter)
  {
    return static_cast<MessageIterImpl*>(iter.get())->Iter;
  }

  static std::shared_ptr<MessageIterImpl> create(const GVariant* value, const GVariantIter* iter, bool eraseOnExit = false)
  {
    return std::make_shared<MessageIterImpl>(const_cast<GVariant*>(value), const_cast<GVariantIter*>(iter), eraseOnExit);
  }

  static GVariantBuilder* get_builder(const std::shared_ptr<MessageIter>& iter)
  {
    return static_cast<MessageIterImpl*>(iter.get())->Builder;
  }

  static std::shared_ptr<MessageIterImpl> create(const GVariantBuilder* builder, bool eraseOnExit = false)
  {
    return std::make_shared<MessageIterImpl>(const_cast<GVariantBuilder*>(builder), eraseOnExit);
  }

  static std::shared_ptr<MessageIterImpl> create_empty()
  {
    return std::make_shared<MessageIterImpl>();
  }

  struct ObjectImpl : public Object
  {
    GDBusConnection *Conn = nullptr;
    std::string Bus;
    std::string Path;
    bool EraseOnExit = false;
    std::unordered_map<std::string, GDBusProxy*> Proxies;

    ObjectImpl(GDBusConnection *conn, const std::string &bus, const std::string &path, bool eraseOnExit = false)
    : Conn(conn), Bus(bus), Path(path), EraseOnExit(eraseOnExit)
    {
    }

    ~ObjectImpl()
    {
      if (!Proxies.empty())
      {
        for(auto& pair: Proxies)
        {
          g_object_unref(pair.second);
        }
      }
      if (EraseOnExit && Conn)
      {
        g_object_unref(Conn);
      }
    }

  };

  static std::shared_ptr<ObjectImpl> create_obj(GDBusConnection *conn, const std::string &bus, const std::string &path, bool eraseOnExit = false)
  {
    return std::make_shared<ObjectImpl>(const_cast<GDBusConnection*>(conn), bus, path, eraseOnExit);
  }

  static GDBusConnection* get_conn(const std::shared_ptr<Object>& obj)
  {
    return static_cast<ObjectImpl*>(obj.get())->Conn;
  }

  static std::string get_bus(const std::shared_ptr<Object>& obj)
  {
    return static_cast<ObjectImpl*>(obj.get())->Bus;
  }

  static std::string get_path(const std::shared_ptr<Object>& obj)
  {
    return static_cast<ObjectImpl*>(obj.get())->Path;
  }

  static GDBusProxy* find_proxy(const std::shared_ptr<Object>& obj, const std::string& interface)
  {
    auto& proxies = static_cast<ObjectImpl*>(obj.get())->Proxies;
    auto it = proxies.find(interface);
    if (it != proxies.end())
    {
      return it->second;
    }
    return nullptr;
  }

  static void add_proxy(const std::shared_ptr<Object>& obj, const std::string& interface, GDBusProxy* proxy)
  {
    auto& proxies = static_cast<ObjectImpl*>(obj.get())->Proxies;
    if (proxy)
    {
      proxies[interface] = static_cast<GDBusProxy*>(g_object_ref(proxy));
    }
  }

  DEFINE_TYPE(Connection, GDBusConnection, g_object_unref(Value))
  DEFINE_TYPE(Message, GDBusMessage, g_object_unref(Value))
  DEFINE_TYPE(Proxy, GDBusProxy, g_object_unref(Value))
  DEFINE_TYPE(Pending, GCancellable, g_object_unref(Value))

#undef DEFINE_TYPE

  ConnectionPtr dbus_address_connection_get_impl(const std::string& addr) override
  {
    GError *error = nullptr;
    auto p = g_dbus_connection_new_for_address_sync(addr.c_str(),
                                                    (GDBusConnectionFlags)(G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT | G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION),
                                                    nullptr,
                                                    nullptr,
                                                    &error);

    if (error)
    {
      DBUS_DEBUG("dbus_address_connection_get_impl failed: %s", error->message);
      g_error_free(error);
      return nullptr;
    }

    return create(p, true);
  }

#define dbus_message_iter_arguments_append_impl_basic(type, sig)                            \
  void dbus_message_iter_arguments_append_impl(const MessageIterPtr& it, type src) override \
  {                                                                                         \
    g_variant_builder_add(get_builder(it), #sig, src);                                      \
  }                                                                                         \
  bool dbus_message_iter_get_and_next_impl(const MessageIterPtr& it, type& dst) override    \
  {                                                                                         \
    auto iter = get_iter(it);                                                               \
    if (!iter) return false;                                                                \
    return g_variant_iter_next(iter, #sig, &dst);                                           \
  }

  // clang-format off
  dbus_message_iter_arguments_append_impl_basic(uint8_t, y)
  dbus_message_iter_arguments_append_impl_basic(uint16_t, q)
  dbus_message_iter_arguments_append_impl_basic(uint32_t, u)
  dbus_message_iter_arguments_append_impl_basic(uint64_t, t)
  dbus_message_iter_arguments_append_impl_basic(int16_t, n)
  dbus_message_iter_arguments_append_impl_basic(int32_t, i)
  dbus_message_iter_arguments_append_impl_basic(int64_t, x)
  dbus_message_iter_arguments_append_impl_basic(double, d)
  // clang-format on

#undef dbus_message_iter_arguments_append_impl_basic

  void dbus_message_iter_arguments_append_impl(const MessageIterPtr& it, bool src) override
  {
    g_variant_builder_add(get_builder(it), "b", src);
  }

  bool dbus_message_iter_get_and_next_impl(const MessageIterPtr& it, bool& dst) override
  {
    gboolean q;
    auto iter = get_iter(it);
    if (!iter) return false;
    auto     z = g_variant_iter_next(iter, "b", &q);
    dst        = q != 0;
    return z;
  }

  void dbus_message_iter_arguments_append_impl(const MessageIterPtr& it, const std::string& src) override
  {
    g_variant_builder_add(get_builder(it), "s", src.c_str());
  }

  bool dbus_message_iter_get_and_next_impl(const MessageIterPtr& it, std::string& dst) override
  {
    auto iter = get_iter(it);
    if (!iter) return false;
    auto value = g_variant_iter_next_value(iter);
    if (!value) return false;
    const gchar* q;
    if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING) || g_variant_is_of_type(value, G_VARIANT_TYPE_OBJECT_PATH))
    {
      q = g_variant_get_string(value, nullptr);
      dst = q;
      g_variant_unref(value);
      return true;
    }

    return false;
  }

  void dbus_message_iter_arguments_append_impl(const MessageIterPtr& it, const ObjectPath& src) override
  {
    g_variant_builder_add(get_builder(it), "o", src.value.c_str());
  }

  void dbus_message_iter_arguments_append_impl(const MessageIterPtr& it, const std::string& v1, const std::string& v2) override
  {
    g_variant_builder_add(get_builder(it), "{ss}", v1.c_str(), v2.c_str());
  }

  bool dbus_message_iter_get_and_next_impl(const MessageIterPtr& it, ObjectPath& dst) override
  {
    const gchar* q;
    auto iter = get_iter(it);
    if (!iter) return false;
    if (g_variant_iter_next(iter, "o", &q))
    {
      dst.value = q;
      return true;
    }

    return false;
  }

  MessageIterPtr dbus_message_iter_container_new_impl(const MessageIterPtr& it, int type, const std::string& sig) override
  {
    auto builder = get_builder(it);
    const GVariantType* g_type = nullptr;

    switch (type)
    {
      case 'v':
        g_type = G_VARIANT_TYPE_VARIANT;
        break;

      case 'r':
        if (!sig.empty())
        {
          GVariantType* tuple_gtype = g_variant_type_new(sig.c_str());
          g_variant_builder_open(builder, tuple_gtype);
          g_variant_type_free(tuple_gtype);
          return create(builder, false);
        }
        else
        {
          g_type = G_VARIANT_TYPE_TUPLE;
        }
        break;

      case 'e':
        return it;

      case 'a':
        if (!sig.empty())
        {
          std::string array_type = "a" + sig;
          GVariantType* array_gtype = g_variant_type_new(array_type.c_str());
          g_variant_builder_open(builder, array_gtype);
          g_variant_type_free(array_gtype);
          return create(builder, false);
        }
        else
        {
          g_type = G_VARIANT_TYPE_ARRAY;
        }
        break;

      default:
        return nullptr;
    }

    if (g_type)
    {
      g_variant_builder_open(builder, g_type);
      return create(builder, false);
    }

    return nullptr;
  }

  void dbus_message_iter_container_close_impl(const MessageIterPtr& it)
  {
    g_variant_builder_close(get_builder(it));
  }

  MessageIterPtr dbus_message_iter_get_and_next_by_type_impl(const MessageIterPtr& it, int type) override
  {
    auto iter = get_iter(it);
    if (!iter) return {};
    auto value = g_variant_iter_next_value(iter);
    if (!value) return {};

    return create(value, g_variant_iter_new(value), true);
  }

  MessageIterPtr dbus_message_iter_get_impl(const MessagePtr& msg, bool write) override
  {
    if (write)
    {
      return create(g_variant_builder_new(G_VARIANT_TYPE_TUPLE), true);
    }
    else
    {
      auto body = g_dbus_message_get_body(get(msg));
      if (!body) return create_empty();
      return create(g_variant_ref(body), g_variant_iter_new(body), true);
    }
  }

  void dbus_message_iter_pack_end_impl(const MessageIterPtr& it, const MessagePtr& msg) override
  {
    g_dbus_message_set_body(get(msg), g_variant_builder_end(get_builder(it)));
  }

  MessagePtr dbus_proxy_method_call_new_impl(const ProxyPtr& proxy, const std::string& funcName) override
  {
    auto gproxy = get(proxy);
    auto message = g_dbus_message_new_method_call(g_dbus_proxy_get_name(gproxy),
                                                  g_dbus_proxy_get_object_path(gproxy),
                                                  g_dbus_proxy_get_interface_name(gproxy),
                                                  funcName.c_str());
    return create(message, true);
  }

  MessagePtr dbus_proxy_send_and_block_impl(const ProxyPtr& proxy, const MessagePtr& msg) override
  {
    GError *error = nullptr;
    auto message = release(msg);
    auto reply = g_dbus_connection_send_message_with_reply_sync(g_dbus_proxy_get_connection(get(proxy)),
                                                                message, G_DBUS_SEND_MESSAGE_FLAGS_NONE,
                                                                GDBUS_CALL_TIMEOUT, nullptr, nullptr, &error);
    g_object_unref(message);

    if (error)
    {
      DBUS_DEBUG("dbus_proxy_send_and_block_impl failed: %s", error->message);
      g_error_free(error);
      return nullptr;
    }
    return create(reply, true);
  }

  bool dbus_message_error_get_impl(const MessagePtr& msg, std::string& name, std::string& text) override
  {
    auto message = get(msg);

    if (g_dbus_message_get_message_type(message) == G_DBUS_MESSAGE_TYPE_ERROR)
    {
      auto error_detail = g_dbus_message_get_body(message);
      if (error_detail)
      {
        if (g_variant_is_of_type(error_detail, G_VARIANT_TYPE_STRING) ||
            g_variant_is_of_type(error_detail, G_VARIANT_TYPE_OBJECT_PATH) ||
            g_variant_is_of_type(error_detail, G_VARIANT_TYPE_VARIANT))
        {
          name = g_dbus_message_get_error_name(message);
          text = g_variant_get_string(error_detail, nullptr);
        }
        else
        {
          name = "";
          text = g_variant_print(error_detail, false);
        }
        return true;
      }
    }
    return false;
  }

  std::string dbus_message_signature_get_impl(const MessagePtr& msg) override
  {
    return g_dbus_message_get_signature(get(msg));
  }

  static void call_async_cb(GObject* source, GAsyncResult *result, gpointer data)
  {
    auto d = static_cast<SendCallback*>(data);
    GError *error = nullptr;
    auto msg = g_dbus_connection_send_message_with_reply_finish(G_DBUS_CONNECTION (source),
                                                                result,
                                                                &error);

    if (error)
    {
      DBUS_DEBUG("g_dbus_connection_send_message_with_reply_finish failed: %s", error->message);
      g_error_free(error);
      delete d;
      return;
    }

    (*d)(create(msg, true));
    delete d;
  }

  PendingPtr dbus_proxy_send_impl(const ProxyPtr& proxy, const MessagePtr& msg, const SendCallback& callback) override
  {
    auto cb = new SendCallback{callback};
    auto message = release(msg);

    g_dbus_connection_send_message_with_reply(g_dbus_proxy_get_connection(get(proxy)),
                                              message, G_DBUS_SEND_MESSAGE_FLAGS_NONE,
                                              GDBUS_CALL_TIMEOUT, nullptr, nullptr,
                                              call_async_cb, cb);
    g_object_unref(message);

    return create(g_cancellable_new(), true);
  }

  std::string dbus_proxy_interface_get_impl(const ProxyPtr& proxy) override
  {
    return g_dbus_proxy_get_interface_name(get(proxy));
  }

  static void on_listener_callback(GDBusConnection *bus,
                                  const char      *sender_name,
                                  const char      *object_path,
                                  const char      *interface_name,
                                  const char      *signal_name,
                                  GVariant        *parameters,
                                  gpointer         data)
  {
    auto msg = g_dbus_message_new_signal(object_path, interface_name, signal_name);
    if (sender_name) g_dbus_message_set_sender(msg, sender_name);
    if (parameters) g_dbus_message_set_body(msg, parameters);

    auto p = static_cast<std::function<void(const GDBusMessage* msg)>*>(data);
    (*p)(msg);
  }

  static void on_listener_callback_destroy(gpointer data)
  {
    auto p = static_cast<std::function<void(const GDBusMessage* msg)>*>(data);
    delete p;
  }

  void dbus_proxy_signal_handler_add_impl(const ProxyPtr& proxy, const std::string& member, const std::function<void(const MessagePtr&)>& cb) override
  {
    auto gproxy = get(proxy);
    auto tmp = new std::function<void(const GDBusMessage* msg)>
    {
      [cb](const GDBusMessage* msg)
      {
        cb(create(msg, true));
      }
    };
    g_dbus_connection_signal_subscribe(g_dbus_proxy_get_connection(gproxy),
                                       g_dbus_proxy_get_name(gproxy),
                                       g_dbus_proxy_get_interface_name(gproxy),
                                       member.c_str(),
                                       g_dbus_proxy_get_object_path(gproxy),
                                       nullptr, G_DBUS_SIGNAL_FLAGS_NONE,
                                       on_listener_callback,
                                       tmp,
                                       on_listener_callback_destroy);
  }

  std::string dbus_message_iter_signature_get_impl(const MessageIterPtr& iter) override
  {
    return g_variant_get_type_string(get_value(iter));
  }

  MessagePtr dbus_message_method_return_new_impl(const MessagePtr& msg) override
  {
    return create(g_dbus_message_new_method_reply(get(msg)), true);
  }

  MessagePtr dbus_message_error_new_impl(const MessagePtr& msg, const std::string& err, const std::string& txt) override
  {
    return create(g_dbus_message_new_method_error(get(msg), err.c_str(), "%s", txt.c_str()), true);
  }

  PendingPtr dbus_connection_send_impl(const ConnectionPtr& conn, const MessagePtr& msg) override
  {
    auto message = release(msg);

    g_dbus_connection_send_message(get(conn), message, G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, nullptr);
    g_object_unref(message);

    return create(g_cancellable_new(), true);
  }

  MessagePtr dbus_message_signal_new_impl(const std::string& path, const std::string& iface, const std::string& name) override
  {
    return create(g_dbus_message_new_signal(path.c_str(), iface.c_str(), name.c_str()), true);
  }

  MessagePtr dbus_message_ref_impl(const MessagePtr& msg) override
  {
    return create(static_cast<GDBusMessage*>(g_object_ref(get(msg))), true);
  }

  ConnectionPtr dbus_connection_get_impl(ConnectionType type) override
  {
    GBusType busType = (type == ConnectionType::SYSTEM) ? G_BUS_TYPE_SYSTEM : G_BUS_TYPE_SESSION;

    GError *error = nullptr;
    auto p = g_bus_get_sync(busType, nullptr, &error);

    if (error)
    {
      DBUS_DEBUG("dbus_connection_get_impl failed: %s", error->message);
      g_error_free(error);
      return nullptr;
    }

    return create(p, true);
  }

  std::string dbus_connection_unique_name_get_impl(const ConnectionPtr& conn) override
  {
    return g_dbus_connection_get_unique_name(get(conn));
  }

  ObjectPtr dbus_object_get_impl(const ConnectionPtr& conn, const std::string& bus, const std::string& path) override
  {
    return create_obj(get(conn), bus.c_str(), path.c_str(), false);
  }

  ProxyPtr dbus_proxy_get_impl(const ObjectPtr& obj, const std::string& interface) override
  {
    auto exist = find_proxy(obj, interface);

    if (exist) {
      return create(static_cast<GDBusProxy*>(g_object_ref(exist)), true);
    }

    auto flags = (GDBusProxyFlags)(G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS | G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES);
    if (!get_bus(obj).compare(A11yDbusName)) flags = G_DBUS_PROXY_FLAGS_NONE;

    GError *error = nullptr;
    auto proxy = g_dbus_proxy_new_sync(get_conn(obj), flags,
                                       nullptr, get_bus(obj).c_str(), get_path(obj).c_str(),
                                       interface.c_str(), nullptr, &error);

    if (error)
    {
      DBUS_DEBUG("dbus_proxy_get_impl failed: %s", error->message);
      g_error_free(error);
      return nullptr;
    }

    add_proxy(obj, interface, proxy);

    return create(proxy, true);
  }

  ProxyPtr dbus_proxy_copy_impl(const ProxyPtr& ptr) override
  {
    return create(static_cast<GDBusProxy*>(g_object_ref(get(ptr))), true);
  }

  void dbus_name_request_impl(const ConnectionPtr& conn, const std::string& bus) override
  {
    GError *error = nullptr;
    auto result = g_dbus_connection_call_sync (get(conn), "org.freedesktop.DBus", "/org/freedesktop/DBus",
                                      "org.freedesktop.DBus", "RequestName",
                                      g_variant_new ("(su)", bus.c_str(), G_BUS_NAME_OWNER_FLAGS_DO_NOT_QUEUE), G_VARIANT_TYPE ("(u)"),
                                      G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (error)
    {
      DBUS_DEBUG("dbus_name_request_impl failed: %s", error->message);
      g_error_free(error);
      return;
    }
    if (result) g_variant_unref(result);
  }

  void dbus_name_release_impl(const ConnectionPtr& conn, const std::string& bus) override
  {
    GError *error = nullptr;
    auto result = g_dbus_connection_call_sync (get(conn), "org.freedesktop.DBus", "/org/freedesktop/DBus",
                                      "org.freedesktop.DBus", "ReleaseName",
                                      g_variant_new ("(s)", bus.c_str()), G_VARIANT_TYPE ("(u)"),
                                      G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (error)
    {
      DBUS_DEBUG("dbus_name_release_impl failed: %s", error->message);
      g_error_free(error);
      return;
    }
    if (result) g_variant_unref(result);
  }

  struct Implementation
  {
    GDBusConnection *connection;
    std::map<std::string, guint> reg_ids;
    std::string object_path;

    std::map<std::string, std::unordered_map<std::string, DBus::DBusInterfaceDescription::MethodInfo>> interface_methods;
    std::map<std::string, std::unordered_map<std::string, DBus::DBusInterfaceDescription::PropertyInfo>> interface_properties;
    std::map<std::string, GDBusInterfaceInfo*> interface_infos;
    bool is_subtree_registered = false;

    ~Implementation() {
      for (auto &pair : interface_infos)
      {
        g_dbus_interface_info_unref(pair.second);
      }
    }
  };

  struct GlobalEntries
  {
    static GlobalEntries& Get()
    {
      static GlobalEntries instance;
      return instance;
    }

    Implementation* Find(const std::string &path)
    {
      Implementation*             impl = nullptr;
      auto                        it = globalEntries.find(path);
      if(it != globalEntries.end())
      {
        impl = it->second;
      }
      return impl;
    }

    void Add(const std::string &path, Implementation* impl)
    {
      globalEntries[path] = impl;
    }

    void Erase(const std::string &path)
    {
      globalEntries.erase(path);
    }

  private:
    std::map<std::string, Implementation*>globalEntries;
  };

  static GDBusInterfaceInfo **
  on_subtree_introspect(GDBusConnection *conn, const gchar *sender, const gchar *path, const gchar *node, gpointer user_data)
  {
    Implementation *impl = static_cast<Implementation*>(user_data);

    if (impl->interface_infos.empty()) return nullptr;

    GDBusInterfaceInfo **ret = g_new0(GDBusInterfaceInfo*, impl->interface_infos.size() + 1);
    int i = 0;
    for (auto const& [name, info] : impl->interface_infos) {
      ret[i++] = g_dbus_interface_info_ref(info);
    }
    return ret;
  }

  static const GDBusInterfaceVTable *
  on_subtree_dispatch(GDBusConnection *conn, const gchar *sender, const gchar *path,
                      const gchar *interface_name, const gchar *node,
                      gpointer *out_user_data, gpointer user_data)
  {
    Implementation *impl = static_cast<Implementation*>(user_data);

    if (impl->interface_infos.find(interface_name) != impl->interface_infos.end()) {
      *out_user_data = impl;
      return &interface_vtable;
    }

    return nullptr;
  }

  static constexpr GDBusSubtreeVTable subtree_vtable = {
    nullptr,
    on_subtree_introspect,
    on_subtree_dispatch,
    { 0 }
  };

  static void on_method_call(GDBusConnection     *connection,
                            const gchar           *sender,
                            const gchar           *object_path,
                            const gchar           *interface_name,
                            const gchar           *method_name,
                            GVariant              *parameters,
                            GDBusMethodInvocation *invocation,
                            gpointer               user_data)
  {
    Implementation *impl = static_cast<Implementation*>(user_data);

    if (impl->interface_methods.find(interface_name) == impl->interface_methods.end())
    {
      g_dbus_method_invocation_return_dbus_error(invocation,
                                                "org.freedesktop.DBus.Error.Failed",
                                                "Unknown interface");
      return;
    }
    auto &methods_map = impl->interface_methods[interface_name];
    auto it = methods_map.find(method_name);
    if (it == methods_map.end())
    {
      g_dbus_method_invocation_return_dbus_error(invocation,
                                                "org.freedesktop.DBus.Error.Failed",
                                                "Unknown method");
      return;
    }
    auto msg = g_dbus_method_invocation_get_message(invocation);

    DBus::DBusServer::CurrentObjectSetter currentObjectSetter(create(connection, false), object_path);
    auto reply_wrapper = it->second.callback(create(msg, false));

    if (!reply_wrapper)
    {
      g_dbus_method_invocation_return_value(invocation, nullptr);
      return;
    }

    auto reply = get(reply_wrapper);

    if (g_dbus_message_get_message_type(reply) == G_DBUS_MESSAGE_TYPE_ERROR)
    {
      g_dbus_method_invocation_return_dbus_error(invocation,
                                                 g_dbus_message_get_error_name(reply),
                                                 g_variant_print(g_dbus_message_get_body(reply), false));
      return;
    }

    g_dbus_method_invocation_return_value(invocation, g_dbus_message_get_body(reply));
  }

  static GVariant *on_get_property(GDBusConnection       *connection,
                                   const gchar           *sender,
                                   const gchar           *object_path,
                                   const gchar           *interface_name,
                                   const gchar           *property_name,
                                   GError               **error,
                                   gpointer               user_data)
  {
    Implementation *impl = static_cast<Implementation*>(user_data);

    if (impl->interface_properties.find(interface_name) == impl->interface_properties.end())
    {
      g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Unknown interface");
      return nullptr;
    }
    auto &props_map = impl->interface_properties[interface_name];
    auto it = props_map.find(property_name);
    if (it == props_map.end() || !it->second.getCallback)
    {
      g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Unknown getter");
      return nullptr;
    }

    auto iter = create(g_variant_builder_new(G_VARIANT_TYPE_TUPLE), true); 
    DBus::DBusServer::CurrentObjectSetter currentObjectSetter(create(connection, false), object_path);
    auto reply = it->second.getCallback(nullptr, iter);

    if (!reply.empty()) {
      g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "%s", reply.c_str());
      return nullptr;
    }

    auto tuple = g_variant_builder_end(get_builder(iter));
    auto value = g_variant_get_child_value(tuple, 0);
    g_variant_unref(tuple);

    return value;
  }

  static gboolean on_set_property(GDBusConnection       *connection,
                                  const gchar           *sender,
                                  const gchar           *object_path,
                                  const gchar           *interface_name,
                                  const gchar           *property_name,
                                  GVariant              *value,
                                  GError               **error,
                                  gpointer               user_data)
  {
    Implementation *impl = static_cast<Implementation*>(user_data);

    if (impl->interface_properties.find(interface_name) == impl->interface_properties.end())
    {
      g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Unknown interface");
      return FALSE;
    }
    auto &props_map = impl->interface_properties[interface_name];
    auto it = props_map.find(property_name);

    if (it == props_map.end() || !it->second.setCallback)
    {
      g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Unknown setter");
      return FALSE;
    }

    DBus::DBusServer::CurrentObjectSetter currentObjectSetter(create(connection, false), object_path);
    auto reply = it->second.setCallback(nullptr, create(static_cast<GVariant*>(g_object_ref(value)), g_variant_iter_new(value), true));

    if (!reply.empty()) {
        g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "%s", reply.c_str());
        return FALSE;
    }

    return TRUE;
  }

  static constexpr GDBusInterfaceVTable interface_vtable = {
    on_method_call,
    on_get_property,
    on_set_property,
    { 0 }
  };

  static GDBusArgInfo **build_args(const std::vector<std::pair<std::string, std::string>> &args) {
    if (args.empty()) return nullptr;

    GDBusArgInfo **ret = g_new0(GDBusArgInfo*, args.size() + 1);
    for (size_t i = 0; i < args.size(); i++) {
        ret[i] = g_new0(GDBusArgInfo, 1);
        ret[i]->ref_count = 1;
        ret[i]->name = g_strdup(args[i].second.c_str());
        ret[i]->signature = g_strdup(args[i].first.c_str());
    }
    return ret;
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
    auto impl = GlobalEntries::Get().Find(pathName);

    DBUS_DEBUG("interface %s path %s on bus %s", interfaceName.c_str(), pathName.c_str(), DBus::getConnectionName(connection).c_str());

    if (!impl)
    {
      impl = new Implementation();
      impl->connection = get(connection);
      impl->object_path = pathName;
      GlobalEntries::Get().Add(pathName, impl);
    }

    GDBusInterfaceInfo *info = g_new0(GDBusInterfaceInfo, 1);
    info->ref_count = 1;
    info->name = g_strdup(interfaceName.c_str());

    if (!dscrMethods.empty())
    {
      info->methods = g_new0(GDBusMethodInfo*, dscrMethods.size() + 1);

      size_t i = 0;
      for (auto &m : dscrMethods)
      {
        std::string method_name = m.memberName;
        DBUS_DEBUG("adding method %s", method_name.c_str());
        for (auto& r : m.in)
        {
          DBUS_DEBUG("in %s '%s'", r.first.c_str(), r.second.c_str());
        }
        for (auto& r : m.out)
        {
          DBUS_DEBUG("out %s '%s'", r.first.c_str(), r.second.c_str());
        }
        auto &stored_method = impl->interface_methods[interfaceName][method_name] = std::move(m);

        GDBusMethodInfo *g_method = g_new0(GDBusMethodInfo, 1);
        g_method->ref_count = 1;
        g_method->name = g_strdup(stored_method.memberName.c_str());

        g_method->in_args = build_args(stored_method.in);
        g_method->out_args = build_args(stored_method.out);

        info->methods[i++] = g_method;
      }
    }

    if (!dscrProperties.empty())
    {
      info->properties = g_new0(GDBusPropertyInfo*, dscrProperties.size() + 1);

      size_t i = 0;
      for (auto &m : dscrProperties)
      {
        std::string property_name = m.memberName;
        auto &stored_property = impl->interface_properties[interfaceName][property_name] = std::move(m);

        GDBusPropertyInfo *g_property = g_new0(GDBusPropertyInfo, 1);
        g_property->ref_count = 1;
        g_property->name = g_strdup(stored_property.memberName.c_str());
        g_property->signature = g_strdup(stored_property.typeSignature.c_str());
        g_property->flags = G_DBUS_PROPERTY_INFO_FLAGS_NONE;
        if (stored_property.getCallback) g_property->flags = (GDBusPropertyInfoFlags)(g_property->flags | G_DBUS_PROPERTY_INFO_FLAGS_READABLE);
        if (stored_property.setCallback) g_property->flags = (GDBusPropertyInfoFlags)(g_property->flags | G_DBUS_PROPERTY_INFO_FLAGS_WRITABLE);

        info->properties[i++] = g_property;
      }
    }

    impl->interface_infos[interfaceName] = info;

    GError *error = nullptr;
    auto id = 0;

    if (fallback)
    {
      if (!impl->is_subtree_registered)
      {
        id = g_dbus_connection_register_subtree(impl->connection,
                                                !pathName.compare("/") ? AtspiPath : pathName.c_str(),
                                                &subtree_vtable,
                                                G_DBUS_SUBTREE_FLAGS_DISPATCH_TO_UNENUMERATED_NODES,
                                                impl,
                                                nullptr,
                                                &error);
        impl->is_subtree_registered = true;
        impl->reg_ids["subtree"] = id;
      }
    }
    else
    {
      id = g_dbus_connection_register_object(impl->connection,
                                            pathName.c_str(),
                                            info,
                                            &interface_vtable,
                                            impl,
                                            NULL,
                                            &error);
      impl->reg_ids[interfaceName] = id;
    }

    if (error)
    {
      DBUS_DEBUG("add_interface_impl failed: %s", error->message);
      g_error_free(error);
      return;
    }

    destructors.push_back([impl, interfaceName, fallback]()
    {
      impl->interface_methods.erase(interfaceName);
      impl->interface_infos.erase(interfaceName);
      impl->interface_properties.erase(interfaceName);

      if (fallback) {
        if (impl->interface_infos.empty() && impl->is_subtree_registered) {
          guint subtree_id = impl->reg_ids["subtree"];
          g_dbus_connection_unregister_subtree(impl->connection, subtree_id);
          impl->is_subtree_registered = false;
          impl->reg_ids.erase("subtree");
        }
      }
      else {
        auto it = impl->reg_ids.find(interfaceName);
        if (it != impl->reg_ids.end()) {
          g_dbus_connection_unregister_object(impl->connection, it->second);
          impl->reg_ids.erase(it);
        }
      }

      if (impl->interface_infos.empty()) {
        GlobalEntries::Get().Erase(impl->object_path);
        delete impl;
      }
    });

    dscrMethods.clear();
    dscrProperties.clear();
    dscrSignals.clear();
  }

  struct PropertyListenerContext {
    std::string interface_name;
    std::string property_name;
    std::function<void(const void*)> callback;

    ~PropertyListenerContext() = default;
  };

  static void on_proxy_properties_changed(GDBusProxy *proxy,
                                          GVariant   *changed_properties,
                                          const gchar *const *invalidated_properties,
                                          gpointer    user_data)
  {
    PropertyListenerContext *ctx = static_cast<PropertyListenerContext*>(user_data);

    const gchar *ifc = g_dbus_proxy_get_interface_name(proxy);
    if (!ifc || ctx->interface_name != ifc) return;

    auto value = g_variant_lookup_value(changed_properties, 
                                        ctx->property_name.c_str(),
                                        nullptr);

    if (value)
    {
      ctx->callback(value);
      g_variant_unref(value);
    }
  }

  static void on_listener_data_destroy(gpointer data, GClosure *closure)
  {
    PropertyListenerContext *ctx = static_cast<PropertyListenerContext*>(data);
    delete ctx;
  }

  void add_property_changed_event_listener_impl(const ProxyPtr& proxy,
                                                const std::string& interface,
                                                const std::string& name,
                                                std::function<void(const void*)> cb) override
  {
    DBUS_DEBUG("add_property_changed_event_listener_impl: %s %s", interface.c_str(), name.c_str());
    auto g_proxy = get(proxy);
    if (!g_proxy) return;

    PropertyListenerContext *ctx = new PropertyListenerContext{interface, name, cb};

    g_signal_connect_data(
      g_proxy,
      "g-properties-changed",
      G_CALLBACK(on_proxy_properties_changed),
      ctx,
      on_listener_data_destroy,
      G_CONNECT_AFTER
    );
  }

  bool get_from_value_impl(const void* v, void* dst) override
  {
    auto variant = static_cast<GVariant*>(const_cast<void*>(v));

    if (!variant) return false;

    if (g_variant_type_equal(g_variant_get_type(variant), G_VARIANT_TYPE_BOOLEAN))
    {
      gboolean val = g_variant_get_boolean(variant);
      *static_cast<bool*>(dst) = val;
      return true;
    }

    return false;
  }

};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_DBUS_WRAPPER_GDBUS_H
