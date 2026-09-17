/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dali/internal/accessibility/bridge/dbus/dbus-client-gdbus.h>
#include <atomic>
#include <vector>

namespace
{
const char* BUS        = "org.a11y.Bus";
const char* PATH       = "/org/a11y/bus";
const char* STATUS     = "org.a11y.Status";
const char* PROPERTIES = "org.freedesktop.DBus.Properties";
const char* busAddress = nullptr;

void Pump(unsigned int milliseconds)
{
  const auto end = g_get_monotonic_time() + milliseconds * 1000;
  do
  {
    while(g_main_context_iteration(nullptr, FALSE))
    {
    }
    g_usleep(1000);
  } while(g_get_monotonic_time() < end);
}

template<typename Predicate>
void WaitFor(Predicate predicate, unsigned int milliseconds = 3000)
{
  const auto end = g_get_monotonic_time() + milliseconds * 1000;
  while(!predicate() && g_get_monotonic_time() < end)
  {
    Pump(1);
  }
  g_assert_true(predicate());
}

GDBusConnection* Connect()
{
  GError* error      = nullptr;
  auto    connection = g_dbus_connection_new_for_address_sync(busAddress,
                                                              static_cast<GDBusConnectionFlags>(G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT | G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION),
                                                              nullptr, nullptr, &error);
  g_assert_no_error(error);
  g_assert_nonnull(connection);
  g_dbus_connection_set_exit_on_close(connection, FALSE);
  return connection;
}

struct Service
{
  GDBusConnection* connection{Connect()};
  guint            registration{0};
  bool             enabled{false};
  bool             screenReader{false};
  bool             holdReplies{false};
  unsigned int     gets{0};
  unsigned int     getAlls{0};
  struct HeldReply
  {
    GDBusMethodInvocation* invocation;
    GVariant*              value;
  };
  std::vector<HeldReply> held;

  Service()
  {
    const char* xml =
      "<node><interface name='org.freedesktop.DBus.Properties'>"
      "<method name='Get'><arg type='s' direction='in'/><arg type='s' direction='in'/><arg type='v' direction='out'/></method>"
      "<method name='GetAll'><arg type='s' direction='in'/><arg type='a{sv}' direction='out'/></method>"
      "</interface></node>";
    GError* error = nullptr;
    auto    info  = g_dbus_node_info_new_for_xml(xml, &error);
    g_assert_no_error(error);
    static const GDBusInterfaceVTable vtable = {OnCall, nullptr, nullptr, {nullptr}};
    registration                             = g_dbus_connection_register_object(connection, PATH, info->interfaces[0], &vtable, this, nullptr, &error);
    g_assert_no_error(error);
    g_assert_cmpuint(registration, !=, 0);
    g_dbus_node_info_unref(info);
  }

  ~Service()
  {
    ReplyHeld();
    g_dbus_connection_unregister_object(connection, registration);
    g_dbus_connection_close_sync(connection, nullptr, nullptr);
    g_object_unref(connection);
  }

  void NameCall(const char* method, GVariant* args)
  {
    GError* error = nullptr;
    auto    reply = g_dbus_connection_call_sync(connection, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
                                                method, args, G_VARIANT_TYPE("(u)"), G_DBUS_CALL_FLAGS_NONE, 1000, nullptr, &error);
    g_assert_no_error(error);
    guint result = 0;
    g_variant_get(reply, "(u)", &result);
    g_assert_cmpuint(result, ==, 1);
    g_variant_unref(reply);
  }

  void Own()
  {
    NameCall("RequestName", g_variant_new("(su)", BUS, 0u));
  }

  void Release()
  {
    NameCall("ReleaseName", g_variant_new("(s)", BUS));
  }

  static void OnCall(GDBusConnection*, const gchar*, const gchar*, const gchar*, const gchar* method, GVariant* parameters, GDBusMethodInvocation* invocation, gpointer data)
  {
    auto&     service = *static_cast<Service*>(data);
    GVariant* reply   = nullptr;
    if(g_str_equal(method, "Get"))
    {
      ++service.gets;
      const gchar* interface = nullptr;
      const gchar* property  = nullptr;
      g_variant_get(parameters, "(&s&s)", &interface, &property);
      g_assert_cmpstr(interface, ==, STATUS);
      const bool value = g_str_equal(property, "IsEnabled") ? service.enabled : service.screenReader;
      reply            = g_variant_new("(v)", g_variant_new_boolean(value));
    }
    else
    {
      ++service.getAlls;
      const gchar* interface = nullptr;
      g_variant_get(parameters, "(&s)", &interface);
      g_assert_cmpstr(interface, ==, STATUS);
      GVariantBuilder properties;
      g_variant_builder_init(&properties, G_VARIANT_TYPE("a{sv}"));
      g_variant_builder_add(&properties, "{sv}", "IsEnabled", g_variant_new_boolean(service.enabled));
      g_variant_builder_add(&properties, "{sv}", "ScreenReaderEnabled", g_variant_new_boolean(service.screenReader));
      reply = g_variant_new("(a{sv})", &properties);
    }
    if(service.holdReplies)
    {
      service.held.push_back({G_DBUS_METHOD_INVOCATION(g_object_ref(invocation)), g_variant_ref_sink(reply)});
    }
    else
    {
      g_dbus_method_invocation_return_value(invocation, reply);
    }
  }

  void ReplyHeld()
  {
    for(const auto& reply : held)
    {
      g_dbus_method_invocation_return_value(reply.invocation, reply.value);
      g_variant_unref(reply.value);
      g_object_unref(reply.invocation);
    }
    held.clear();
  }

  void Emit(const char* interface, const char* property, bool value, bool invalidate = false)
  {
    GVariantBuilder changed;
    GVariantBuilder invalidated;
    g_variant_builder_init(&changed, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_init(&invalidated, G_VARIANT_TYPE("as"));
    if(invalidate)
      g_variant_builder_add(&invalidated, "s", property);
    else
      g_variant_builder_add(&changed, "{sv}", property, g_variant_new_boolean(value));
    GError* error = nullptr;
    g_dbus_connection_emit_signal(connection, nullptr, PATH, PROPERTIES, "PropertiesChanged",
                                  g_variant_new("(sa{sv}as)", interface, &changed, &invalidated), &error);
    g_assert_no_error(error);
    g_dbus_connection_flush_sync(connection, nullptr, nullptr);
  }
};

struct Client
{
  GDBusConnection*                            connection{Connect()};
  GDBusProxy*                                 proxy{nullptr};
  std::vector<bool>                           values;
  std::shared_ptr<DBus::GdbusPropertyMonitor> listener;

  ~Client()
  {
    if(listener)
    {
      listener->Stop();
    }
    listener.reset();
    g_clear_object(&proxy);
    g_dbus_connection_close_sync(connection, nullptr, nullptr);
    g_object_unref(connection);
  }

  void CreateProxy()
  {
    GError*    error = nullptr;
    const auto start = g_get_monotonic_time();
    proxy            = g_dbus_proxy_new_sync(connection, DBus::GDBUS_CLIENT_PROXY_FLAGS, nullptr, BUS, PATH, STATUS, nullptr, &error);
    g_assert_no_error(error);
    g_assert_nonnull(proxy);
    g_assert_cmpint(g_get_monotonic_time() - start, <, 250000);
  }

  void Listen(const char* property = "IsEnabled")
  {
    if(!listener)
    {
      listener = DBus::GdbusPropertyMonitor::New(proxy);
    }
    listener->Add(property, [this](const void* value)
    {
      values.push_back(g_variant_get_boolean(static_cast<GVariant*>(const_cast<void*>(value))));
    });
  }
};

void NoActivationDuringConstruction()
{
  Client                    client;
  std::atomic<unsigned int> activations{0};
  const auto                filter = g_dbus_connection_add_filter(client.connection,
                                                                  [](GDBusConnection*, GDBusMessage* message, gboolean incoming, gpointer data) -> GDBusMessage*
                 {
    if(!incoming && g_strcmp0(g_dbus_message_get_member(message), "StartServiceByName") == 0)
      ++*static_cast<std::atomic<unsigned int>*>(data);
    return message;
  }, &activations, nullptr);
  client.CreateProxy();
  g_dbus_connection_flush_sync(client.connection, nullptr, nullptr);
  Pump(20);
  g_assert_cmpuint(activations.load(), ==, 0);
  g_dbus_connection_remove_filter(client.connection, filter);
}

void NoPropertyReadDuringConstruction()
{
  Service service;
  service.holdReplies = true;
  service.Own();
  Client client;
  client.CreateProxy();
  Pump(20);
  g_assert_cmpuint(service.gets, ==, 0);
  g_assert_cmpuint(service.getAlls, ==, 0);
}

void LateOwner()
{
  Client client;
  client.CreateProxy();
  client.Listen();
  Pump(20);
  g_assert_true(client.values.empty());
  Service service;
  service.enabled = true;
  service.Own();
  WaitFor([&]
  { return !client.values.empty(); });
  g_assert_true(client.values.back());
}

void PropertySignals()
{
  Service service;
  service.Own();
  Client client;
  client.CreateProxy();
  client.Listen();
  WaitFor([&]
  { return client.values.size() == 1; });
  g_assert_false(client.values.back());
  service.Emit(STATUS, "IsEnabled", true);
  WaitFor([&]
  { return client.values.size() == 2; });
  g_assert_true(client.values.back());
  service.Emit("org.example.Unrelated", "IsEnabled", false);
  service.Emit(STATUS, "ScreenReaderEnabled", false);
  Pump(20);
  g_assert_cmpuint(client.values.size(), ==, 2);
  service.Emit(STATUS, "IsEnabled", false);
  WaitFor([&]
  { return client.values.size() == 3; });
  g_assert_false(client.values.back());
}

void OwnerReplacement()
{
  Service oldService;
  oldService.enabled = true;
  oldService.Own();
  Client client;
  client.CreateProxy();
  client.Listen();
  WaitFor([&]
  { return client.values.size() == 1; });
  g_assert_true(client.values.back());
  oldService.Release();
  Service newService;
  newService.Own();
  WaitFor([&]
  { return client.values.size() == 2; });
  g_assert_false(client.values.back());
  oldService.Emit(STATUS, "IsEnabled", true);
  Pump(20);
  g_assert_cmpuint(client.values.size(), ==, 2);
}

void TimeoutRecovery()
{
  Service service;
  service.enabled     = true;
  service.holdReplies = true;
  service.Own();
  Client client;
  client.CreateProxy();
  client.Listen();
  WaitFor([&]
  { return service.getAlls == 1; });
  Pump(1100);
  g_assert_true(client.values.empty());
  service.holdReplies = false;
  WaitFor([&]
  { return !client.values.empty(); });
  g_assert_cmpuint(service.getAlls, >=, 2);
  g_assert_true(client.values.back());
}

void SignalSupersedesPendingRead()
{
  Service service;
  service.holdReplies = true;
  service.Own();
  Client client;
  client.CreateProxy();
  client.Listen();
  WaitFor([&]
  { return service.getAlls == 1; });
  service.Emit(STATUS, "IsEnabled", true);
  WaitFor([&]
  { return client.values.size() == 1; });
  g_assert_true(client.values.back());
  service.ReplyHeld();
  Pump(50);
  g_assert_cmpuint(client.values.size(), ==, 1);
}

void DestroyWithPendingRead()
{
  Service service;
  service.holdReplies = true;
  service.Own();
  Client client;
  client.CreateProxy();
  client.Listen();
  WaitFor([&]
  { return service.getAlls == 1; });
  std::weak_ptr<DBus::GdbusPropertyMonitor> weak = client.listener;
  client.listener.reset();
  g_assert_true(weak.expired());
  service.ReplyHeld();
  service.Emit(STATUS, "IsEnabled", true);
  Pump(50);
  g_assert_true(client.values.empty());
}

void InvalidatedProperty()
{
  Service service;
  service.Own();
  Client client;
  client.CreateProxy();
  client.Listen();
  WaitFor([&]
  { return client.values.size() == 1; });
  service.enabled = true;
  service.Emit(STATUS, "IsEnabled", false, true);
  WaitFor([&]
  { return client.values.size() == 2; });
  g_assert_true(client.values.back());
  g_assert_cmpuint(service.getAlls, ==, 2);
}

void ScreenReaderProperty()
{
  Service service;
  service.screenReader = true;
  service.Own();
  Client client;
  client.CreateProxy();
  client.Listen("ScreenReaderEnabled");
  WaitFor([&]
  { return client.values.size() == 1; });
  g_assert_true(client.values.back());
  service.Emit(STATUS, "ScreenReaderEnabled", false);
  WaitFor([&]
  { return client.values.size() == 2; });
  g_assert_false(client.values.back());
}
void SharedInitialRead()
{
  Service service;
  service.enabled = true;
  service.Own();
  Client client;
  client.CreateProxy();
  client.Listen();
  std::vector<bool> screenReader;
  client.listener->Add("ScreenReaderEnabled", [&](const void* value)
  {
    screenReader.push_back(g_variant_get_boolean(static_cast<GVariant*>(const_cast<void*>(value))));
  });
  WaitFor([&]
  { return client.values.size() == 1 && screenReader.size() == 1; });
  Pump(20);
  g_assert_cmpuint(service.getAlls, ==, 1);
  g_assert_cmpuint(service.gets, ==, 0);
  g_assert_true(client.values.back());
  g_assert_false(screenReader.back());

  service.Release();
  Service replacement;
  replacement.screenReader = true;
  replacement.Own();
  WaitFor([&]
  { return client.values.size() == 2 && screenReader.size() == 2; });
  Pump(20);
  g_assert_cmpuint(replacement.getAlls, ==, 1);
  g_assert_cmpuint(replacement.gets, ==, 0);
  g_assert_false(client.values.back());
  g_assert_true(screenReader.back());
  client.listener.reset();
}

void SharedActivation()
{
  Client client;
  client.CreateProxy();
  std::atomic<unsigned int> activations{0};
  const auto                filter = g_dbus_connection_add_filter(client.connection,
                                                                  [](GDBusConnection*, GDBusMessage* message, gboolean incoming, gpointer data) -> GDBusMessage*
                 {
    if(!incoming && g_strcmp0(g_dbus_message_get_member(message), "StartServiceByName") == 0)
      ++*static_cast<std::atomic<unsigned int>*>(data);
    return message;
  }, &activations, nullptr);
  client.Listen();
  client.Listen("ScreenReaderEnabled");
  WaitFor([&]
  { return activations.load() > 0; });
  Pump(20);
  g_assert_cmpuint(activations.load(), ==, 1);
  g_dbus_connection_remove_filter(client.connection, filter);
}

void PartialSignalDuringRead()
{
  Service service;
  service.screenReader = true;
  service.holdReplies  = true;
  service.Own();
  Client client;
  client.CreateProxy();
  client.Listen();
  std::vector<bool> screenReader;
  client.listener->Add("ScreenReaderEnabled", [&](const void* value)
  {
    screenReader.push_back(g_variant_get_boolean(static_cast<GVariant*>(const_cast<void*>(value))));
  });
  WaitFor([&]
  { return service.getAlls == 1; });
  service.Emit(STATUS, "IsEnabled", true);
  WaitFor([&]
  { return client.values.size() == 1; });
  service.ReplyHeld();
  WaitFor([&]
  { return screenReader.size() == 1; });
  Pump(20);
  g_assert_cmpuint(client.values.size(), ==, 1);
  g_assert_true(client.values.back());
  g_assert_true(screenReader.back());
  client.listener.reset();
}

void ActivationRetry()
{
  Client client;
  client.CreateProxy();
  auto       activations = std::make_shared<std::atomic<unsigned int>>(0);
  const auto filter      = g_dbus_connection_add_filter(client.connection,
                                                        [](GDBusConnection*, GDBusMessage* message, gboolean incoming, gpointer data) -> GDBusMessage*
       {
    if(!incoming && g_strcmp0(g_dbus_message_get_member(message), "StartServiceByName") == 0)
      ++**static_cast<std::shared_ptr<std::atomic<unsigned int>>*>(data);
    return message;
  }, new std::shared_ptr<std::atomic<unsigned int>>(activations),
                                                        [](gpointer data)
       {
    delete static_cast<std::shared_ptr<std::atomic<unsigned int>>*>(data);
  });
  client.Listen();
  client.Listen("ScreenReaderEnabled");
  WaitFor([&]
  { return activations->load() >= 2; });
  g_assert_true(client.values.empty());
  Service service;
  service.enabled      = true;
  service.screenReader = true;
  service.Own();
  WaitFor([&]
  { return client.values.size() == 2; });
  g_assert_cmpuint(service.getAlls, ==, 1);
  g_assert_true(client.values[0]);
  g_assert_true(client.values[1]);
  g_dbus_connection_remove_filter(client.connection, filter);
}

void StopDuringInitialRead()
{
  Service service;
  service.Own();
  auto client = std::make_unique<Client>();
  client->CreateProxy();
  client->listener                                    = DBus::GdbusPropertyMonitor::New(client->proxy);
  unsigned int                              callbacks = 0;
  std::weak_ptr<DBus::GdbusPropertyMonitor> weak      = client->listener;
  client->listener->Add("IsEnabled", [&](const void*)
  {
    ++callbacks;
    client.reset();
  });
  client->listener->Add("IsEnabled", [&](const void*)
  { ++callbacks; });
  client->listener->Add("ScreenReaderEnabled", [&](const void*)
  { ++callbacks; });
  WaitFor([&]
  { return !client; });
  Pump(20);
  g_assert_cmpuint(callbacks, ==, 1);
  g_assert_true(weak.expired());
}

void StopDuringSignal()
{
  Service service;
  service.Own();
  auto client = std::make_unique<Client>();
  client->CreateProxy();
  client->listener                                    = DBus::GdbusPropertyMonitor::New(client->proxy);
  unsigned int                              callbacks = 0;
  std::weak_ptr<DBus::GdbusPropertyMonitor> weak      = client->listener;
  client->listener->Add("IsEnabled", [&](const void* value)
  {
    ++callbacks;
    if(g_variant_get_boolean(static_cast<GVariant*>(const_cast<void*>(value))))
      client.reset();
  });
  client->listener->Add("IsEnabled", [&](const void*)
  { ++callbacks; });
  WaitFor([&]
  { return callbacks == 2; });
  service.Emit(STATUS, "IsEnabled", true);
  WaitFor([&]
  { return !client; });
  Pump(20);
  g_assert_cmpuint(callbacks, ==, 3);
  g_assert_true(weak.expired());
}
} // namespace

int main(int argc, char** argv)
{
  g_test_init(&argc, &argv, nullptr);
  auto bus = g_test_dbus_new(G_TEST_DBUS_NONE);
  g_test_dbus_up(bus);
  busAddress = g_test_dbus_get_bus_address(bus);
  g_test_add_func("/gdbus/proxy/no-activation", NoActivationDuringConstruction);
  g_test_add_func("/gdbus/proxy/no-property-read", NoPropertyReadDuringConstruction);
  g_test_add_func("/gdbus/properties/late-owner", LateOwner);
  g_test_add_func("/gdbus/properties/signals", PropertySignals);
  g_test_add_func("/gdbus/properties/owner-replacement", OwnerReplacement);
  g_test_add_func("/gdbus/properties/timeout-recovery", TimeoutRecovery);
  g_test_add_func("/gdbus/properties/stale-reply", SignalSupersedesPendingRead);
  g_test_add_func("/gdbus/properties/destroy-pending", DestroyWithPendingRead);
  g_test_add_func("/gdbus/properties/invalidation", InvalidatedProperty);
  g_test_add_func("/gdbus/properties/screen-reader", ScreenReaderProperty);
  g_test_add_func("/gdbus/properties/shared-initial-read", SharedInitialRead);
  g_test_add_func("/gdbus/properties/shared-activation", SharedActivation);
  g_test_add_func("/gdbus/properties/partial-signal-during-read", PartialSignalDuringRead);
  g_test_add_func("/gdbus/properties/activation-retry", ActivationRetry);
  g_test_add_func("/gdbus/properties/stop-during-initial-read", StopDuringInitialRead);
  g_test_add_func("/gdbus/properties/stop-during-signal", StopDuringSignal);
  const auto result = g_test_run();
  Pump(20);
  g_test_dbus_down(bus);
  g_object_unref(bus);
  return result;
}
