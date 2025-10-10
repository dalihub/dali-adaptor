#ifndef DALI_INTERNAL_UNIFIED_TRIGGER_EVENT_MANAGER_IMPL_H
#define DALI_INTERNAL_UNIFIED_TRIGGER_EVENT_MANAGER_IMPL_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/signals/callback.h>
#include <memory>
#include <unordered_set>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/trigger-event-interface.h>
#include <dali/internal/system/common/file-descriptor-monitor.h>
#include <dali/internal/system/common/unified-trigger-event-manager.h>

namespace Dali
{
namespace Internal::Adaptor
{
class TriggerEvent;

/**
 * The Controller for trigger events. It has one global static fd, and invoke
 * after collect each EventTrigger's trigger called.
 */
class UnifiedTriggerEventManager : public BaseObject
{
public:
  /**
   * @brief Get pointer of instance from singletone.
   * Could return nullptr if singletone service uninstalled.
   */
  static Dali::UnifiedTriggerEventManager Get();

public:
  /**
   * @brief Generate new trigger events that unified tirgger event manager could control.
   *
   * @param[in] callback Callback when event triggerd
   * @param[in] options Additional option flags for trigger
   * @return New generated event trigger pointer.
   */
  TriggerEvent* GenerateTriggerEvent(CallbackBase* callback, TriggerEventInterface::Options options);

  /**
   * @brief Discard trigger events. This will be deleted at the end of Triggered() API.
   *
   * @param[in] triggerEvent Discarded trigger event
   */
  void DiscardTriggerEvent(TriggerEvent* triggerEvent);

public:
  /**
   * Triggers the event.
   *
   * This can be called from various threads in order to wake up another thread.
   * @param[in] triggerEvent Trigger events that want to trigger.
   */
  void Trigger(TriggerEvent* triggerEvent);

private:
  UnifiedTriggerEventManager();
  ~UnifiedTriggerEventManager();

private:
  /**
   * @brief Called when our event file descriptor has been written to.
   * @param[in] eventBitMask bit mask of events that occured on the file descriptor
   * @param[in] fileDescriptor The file descriptor
   */
  void Triggered(FileDescriptorMonitor::EventType eventBitMask, int fileDescriptor);

private:
  std::unique_ptr<FileDescriptorMonitor> mFileDescriptorMonitor;

  std::unordered_set<uint32_t>      mValidEventsId; ///< Keep Id only
  std::unordered_set<TriggerEvent*> mDiscardedEvents;

  // Worker thread accessable data,
  int                               mFileDescriptor;
  Dali::Mutex                       mTriggerMutex; // Mutex for trigger
  std::unordered_set<TriggerEvent*> mTriggeredEvents;
  bool                              mFileDescriptorWritten;
};
} // namespace Internal::Adaptor

inline Internal::Adaptor::UnifiedTriggerEventManager& GetImplementation(Dali::UnifiedTriggerEventManager& obj)
{
  DALI_ASSERT_ALWAYS(obj && "UnifiedTriggerEventManager is empty");

  Dali::BaseObject& handle = obj.GetBaseObject();

  return static_cast<Internal::Adaptor::UnifiedTriggerEventManager&>(handle);
}

inline const Internal::Adaptor::UnifiedTriggerEventManager& GetImplementation(const Dali::UnifiedTriggerEventManager& obj)
{
  DALI_ASSERT_ALWAYS(obj && "UnifiedTriggerEventManager is empty");

  const Dali::BaseObject& handle = obj.GetBaseObject();

  return static_cast<const Internal::Adaptor::UnifiedTriggerEventManager&>(handle);
}
} // namespace Dali

#endif // DALI_INTERNAL_UNIFIED_TRIGGER_EVENT_MANAGER_IMPL_H
