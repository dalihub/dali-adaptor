#ifndef DALI_WIDGET_ENGINE_PLUGIN_H
#define DALI_WIDGET_ENGINE_PLUGIN_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/events/touch-event.h>
#include <dali/public-api/events/wheel-event.h>
#include <functional>
#include <string>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/native-image.h>

namespace DALI_NAMESPACE
{
/**
 * @brief Platform-neutral interface for one widget instance.
 *
 * Provides widget lifecycle, input, metadata and frame notifications independently
 * of the UI framework. Implementations own the platform service and buffer resources.
 */
class WidgetEngineInstancePlugin
{
public:
  enum class EventType
  {
    ADDED,
    CREATION_ABORTED,
    CONTENT_UPDATED,
    UPDATE_PERIOD_CHANGED,
    FAULTED
  };

  using EventCallback        = std::function<void(EventType)>;
  using FrameUpdatedCallback = std::function<void(NativeImagePtr)>;

  virtual ~WidgetEngineInstancePlugin() = default;

  virtual const std::string& GetWidgetId() const     = 0;
  virtual const std::string& GetInstanceId() const   = 0;
  virtual const std::string& GetContentInfo() const  = 0;
  virtual const std::string& GetTitle() const        = 0;
  virtual std::string        GetPreviewImagePath()   = 0;
  virtual int                GetProcessId() const    = 0;
  virtual float              GetUpdatePeriod() const = 0;

  virtual bool IsFaulted() const = 0;

  virtual bool Pause()                      = 0;
  virtual bool Resume()                     = 0;
  virtual bool CancelTouchEvent()           = 0;
  virtual void ActivateFaultedWidget()      = 0;
  virtual void Resize(int width, int height) = 0;
  virtual void SetVisible(bool visible)     = 0;

  virtual bool FeedTouchEvent(const TouchEvent& event)                = 0;
  virtual bool FeedKeyEvent(const KeyEvent& event)                    = 0;
  virtual bool FeedWheelEvent(const WheelEvent& event)                = 0;
  virtual void SetEventCallback(EventCallback callback)               = 0;
  virtual void SetFrameUpdatedCallback(FrameUpdatedCallback callback) = 0;
  virtual void ResetCallbacks()                                       = 0;
};

/**
 * @brief Platform-neutral interface used by DALi adaptor to manage widget instances.
 */
class WidgetEnginePlugin
{
public:
  virtual ~WidgetEnginePlugin() = default;

  virtual bool Initialize(const std::string& applicationId) = 0;

  virtual WidgetEngineInstancePlugin* CreateWidget(const std::string& widgetId,
                                                   const std::string& contentInfo,
                                                   int                width,
                                                   int                height,
                                                   float              updatePeriod) = 0;

  virtual void DestroyWidget(WidgetEngineInstancePlugin* instance) = 0;

  /**
   * @brief Whether the plugin has completed all instance and resource cleanup.
   * The host retains the plugin until both its handles are released and this
   * method returns true.
   */
  virtual bool IsDestroyReady() const
  {
    return true;
  }

  /**
   * @brief Sets the event-thread notification for completion of deferred cleanup.
   * The host must not destroy the plugin from inside this callback.
   * Asynchronous implementations must notify when IsDestroyReady becomes true.
   */
  virtual void SetDestroyReadyCallback(std::function<void()> callback)
  {
  }

  /**
   * @brief Stops provider activity, timers and application callbacks.
   * Called on the event thread by the adaptor's OnStop notification, before rendering stops.
   * Retains buffers owned by graphics without waiting for GPU completion.
   * Must be idempotent and must not emit application callbacks.
   */
  virtual void Stop()
  {
  }

  /**
   * @brief Disconnects callback owners and platform connections at adaptor teardown.
   * Called on the event thread by the adaptor's OnDestroy notification without
   * waiting for GPU completion. Must be idempotent because handles can outlive the adaptor.
   */
  virtual void DetachFromAdaptor()
  {
  }
};

} // namespace DALI_NAMESPACE

#endif // DALI_WIDGET_ENGINE_PLUGIN_H
