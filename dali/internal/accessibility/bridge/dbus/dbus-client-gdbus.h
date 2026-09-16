/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
 */

#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_DBUS_CLIENT_GDBUS_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_DBUS_CLIENT_GDBUS_H

#include <gio/gio.h>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace DBus
{
// Construct a local method-call handle without activating the destination or
// waiting for its properties. Calls and signal subscriptions are explicit.
constexpr GDBusProxyFlags GDBUS_CLIENT_PROXY_FLAGS = static_cast<GDBusProxyFlags>(
  G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
  G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS |
  G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START_AT_CONSTRUCTION);

/**
 * Observes an interface's properties with one signal subscription, one name
 * watch and one asynchronous initial/recovery read, shared by all listeners.
 * Create and destroy on the connection's dispatch context, like the proxy.
 */
class GdbusPropertyMonitor : public std::enable_shared_from_this<GdbusPropertyMonitor>
{
public:
  using Callback = std::function<void(const void*)>;

  static std::shared_ptr<GdbusPropertyMonitor> New(GDBusProxy* proxy)
  {
    auto monitor       = std::shared_ptr<GdbusPropertyMonitor>(new GdbusPropertyMonitor(proxy));
    monitor->mSignalId = g_dbus_connection_signal_subscribe(monitor->mConnection,
                                                            monitor->mBus.c_str(),
                                                            "org.freedesktop.DBus.Properties",
                                                            "PropertiesChanged",
                                                            monitor->mPath.c_str(),
                                                            monitor->mInterface.c_str(),
                                                            G_DBUS_SIGNAL_FLAGS_NONE,
                                                            OnPropertiesChanged,
                                                            new WeakPtr(monitor),
                                                            DeleteWeakPtr);
    // Activation is asynchronous and starts only when properties are observed,
    // not during proxy construction. OnNameAppeared owns the initial read as
    // well as recovery after late startup or service replacement.
    monitor->WatchName();
    return monitor;
  }

  void Add(const std::string& property, Callback callback)
  {
    if(mStopped)
    {
      return;
    }
    mProperties[property].callbacks.push_back(std::move(callback));
    // Registrations made before the name-appeared callback share its GetAll.
    // A listener added later also needs its initial value.
    if(!mOwner.empty())
    {
      Read();
    }
  }

  void Stop()
  {
    if(mStopped)
    {
      return;
    }
    mStopped = true;
    CancelRead();
    if(mWatchId)
    {
      g_bus_unwatch_name(mWatchId);
      mWatchId = 0;
    }
    if(mSignalId)
    {
      g_dbus_connection_signal_unsubscribe(mConnection, mSignalId);
      mSignalId = 0;
    }
    mProperties.clear();
  }

  ~GdbusPropertyMonitor()
  {
    Stop();
    g_main_context_unref(mContext);
    g_object_unref(mConnection);
  }

private:
  using WeakPtr = std::weak_ptr<GdbusPropertyMonitor>;

  struct Property
  {
    std::vector<Callback> callbacks;
    uint64_t              revision{0};
  };

  struct ReadRequest
  {
    WeakPtr                         monitor;
    uint64_t                        revision;
    std::map<std::string, uint64_t> propertyRevisions;
  };

  explicit GdbusPropertyMonitor(GDBusProxy* proxy)
  : mConnection(G_DBUS_CONNECTION(g_object_ref(g_dbus_proxy_get_connection(proxy)))),
    mContext(g_main_context_ref_thread_default()),
    mBus(g_dbus_proxy_get_name(proxy)),
    mPath(g_dbus_proxy_get_object_path(proxy)),
    mInterface(g_dbus_proxy_get_interface_name(proxy))
  {
  }

  GdbusPropertyMonitor(const GdbusPropertyMonitor&)            = delete;
  GdbusPropertyMonitor& operator=(const GdbusPropertyMonitor&) = delete;

  static void DeleteWeakPtr(gpointer data)
  {
    delete static_cast<WeakPtr*>(data);
  }

  void WatchName()
  {
    mWatchId = g_bus_watch_name_on_connection(mConnection, mBus.c_str(),
                                              G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
                                              OnNameAppeared, OnNameVanished,
                                              new WeakPtr(weak_from_this()), DeleteWeakPtr);
  }

  void ScheduleRetry()
  {
    if(!mRetrySource && !mStopped && !g_dbus_connection_is_closed(mConnection))
    {
      mRetrySource = g_timeout_source_new(1000);
      g_source_set_callback(mRetrySource, OnRetry, new WeakPtr(weak_from_this()), DeleteWeakPtr);
      g_source_attach(mRetrySource, mContext);
    }
  }

  void CancelRead()
  {
    ++mRevision;
    if(mCancellable)
    {
      g_cancellable_cancel(mCancellable);
      g_clear_object(&mCancellable);
    }
    if(mRetrySource)
    {
      g_source_destroy(mRetrySource);
      g_source_unref(mRetrySource);
      mRetrySource = nullptr;
    }
  }

  void Read()
  {
    CancelRead();
    if(mStopped || mOwner.empty() || mProperties.empty())
    {
      return;
    }

    mCancellable = g_cancellable_new();
    auto request = new ReadRequest{weak_from_this(), mRevision, {}};
    for(const auto& property : mProperties)
    {
      request->propertyRevisions.emplace(property.first, property.second.revision);
    }
    // Address the current unique owner so a reply from a previous service
    // instance cannot overwrite a newer value or activate a replacement.
    g_dbus_connection_call(mConnection, mOwner.c_str(), mPath.c_str(),
                           "org.freedesktop.DBus.Properties", "GetAll",
                           g_variant_new("(s)", mInterface.c_str()),
                           G_VARIANT_TYPE("(a{sv})"), G_DBUS_CALL_FLAGS_NO_AUTO_START,
                           1000, mCancellable, OnRead,
                           request);
  }

  static void OnRead(GObject* source, GAsyncResult* result, gpointer data)
  {
    std::unique_ptr<ReadRequest> request(static_cast<ReadRequest*>(data));
    GError*                      error   = nullptr;
    GVariant*                    reply   = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source), result, &error);
    auto                         monitor = request->monitor.lock();
    if(monitor && request->revision == monitor->mRevision)
    {
      g_clear_object(&monitor->mCancellable);
      if(reply)
      {
        GVariant* properties = g_variant_get_child_value(reply, 0);
        for(const auto& revision : request->propertyRevisions)
        {
          if(request->revision != monitor->mRevision)
          {
            break;
          }
          const auto property = monitor->mProperties.find(revision.first);
          if(property != monitor->mProperties.end() && property->second.revision == revision.second)
          {
            if(auto value = g_variant_lookup_value(properties, revision.first.c_str(), nullptr))
            {
              monitor->Notify(revision.first, value);
              g_variant_unref(value);
            }
          }
        }
        g_variant_unref(properties);
      }
      else if(g_error_matches(error, G_IO_ERROR, G_IO_ERROR_TIMED_OUT) ||
              g_error_matches(error, G_DBUS_ERROR, G_DBUS_ERROR_NO_REPLY) ||
              g_error_matches(error, G_DBUS_ERROR, G_DBUS_ERROR_TIMEOUT) ||
              g_error_matches(error, G_DBUS_ERROR, G_DBUS_ERROR_TIMED_OUT))
      {
        // The service can own its name before it can answer requests.
        monitor->ScheduleRetry();
      }
    }
    if(reply)
    {
      g_variant_unref(reply);
    }
    g_clear_error(&error);
  }

  static gboolean OnRetry(gpointer data)
  {
    if(auto monitor = static_cast<WeakPtr*>(data)->lock())
    {
      g_source_unref(monitor->mRetrySource);
      monitor->mRetrySource = nullptr;
      if(monitor->mOwner.empty())
      {
        // Activation can initially fail because systemd has not registered
        // the bus name yet. Retry it with one watch, without an extra Get.
        g_bus_unwatch_name(monitor->mWatchId);
        monitor->WatchName();
      }
      else
      {
        monitor->Read();
      }
    }
    return G_SOURCE_REMOVE;
  }

  static void OnNameAppeared(GDBusConnection*, const gchar*, const gchar* owner, gpointer data)
  {
    auto monitor = static_cast<WeakPtr*>(data)->lock();
    if(monitor && !monitor->mStopped)
    {
      monitor->mOwner = owner;
      monitor->Read();
    }
  }

  static void OnNameVanished(GDBusConnection*, const gchar*, gpointer data)
  {
    auto monitor = static_cast<WeakPtr*>(data)->lock();
    if(monitor && !monitor->mStopped)
    {
      monitor->mOwner.clear();
      monitor->CancelRead();
      monitor->ScheduleRetry();
    }
  }

  static void OnPropertiesChanged(GDBusConnection*, const gchar* sender, const gchar*, const gchar*, const gchar*, GVariant* parameters, gpointer data)
  {
    auto monitor = static_cast<WeakPtr*>(data)->lock();
    if(!monitor || monitor->mStopped || !g_variant_is_of_type(parameters, G_VARIANT_TYPE("(sa{sv}as)")) ||
       (!monitor->mOwner.empty() && monitor->mOwner != sender))
    {
      return;
    }

    const gchar* interface   = nullptr;
    GVariant*    changed     = nullptr;
    GVariant*    invalidated = nullptr;
    g_variant_get(parameters, "(&s@a{sv}@as)", &interface, &changed, &invalidated);
    if(monitor->mInterface == interface)
    {
      GVariantIter iter;
      const gchar* name  = nullptr;
      GVariant*    value = nullptr;
      g_variant_iter_init(&iter, changed);
      while(g_variant_iter_next(&iter, "{&sv}", &name, &value))
      {
        auto property = monitor->mProperties.find(name);
        if(property != monitor->mProperties.end())
        {
          // Suppress stale GetAll data for this property only. Other
          // properties in the same in-flight reply may still be needed.
          ++property->second.revision;
          monitor->Notify(name, value);
        }
        g_variant_unref(value);
      }
      g_variant_iter_init(&iter, invalidated);
      while(g_variant_iter_next(&iter, "&s", &name))
      {
        if(monitor->mProperties.find(name) != monitor->mProperties.end())
        {
          monitor->Read();
          break;
        }
      }
    }
    g_variant_unref(changed);
    g_variant_unref(invalidated);
  }

  void Notify(const std::string& property, GVariant* value)
  {
    // A callback may add another listener. Keep this dispatch independent of
    // modifications to the listener list.
    const auto callbacks = mProperties.at(property).callbacks;
    for(const auto& callback : callbacks)
    {
      if(mStopped)
      {
        break;
      }
      callback(value);
    }
  }

  GDBusConnection*                mConnection;
  GMainContext*                   mContext;
  std::string                     mBus;
  std::string                     mPath;
  std::string                     mInterface;
  std::string                     mOwner;
  std::map<std::string, Property> mProperties;
  GCancellable*                   mCancellable{nullptr};
  GSource*                        mRetrySource{nullptr};
  guint                           mSignalId{0};
  guint                           mWatchId{0};
  uint64_t                        mRevision{0};
  bool                            mStopped{false};
};
} // namespace DBus

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_DBUS_CLIENT_GDBUS_H
