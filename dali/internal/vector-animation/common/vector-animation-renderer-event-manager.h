#ifndef DALI_INTERNAL_VECTOR_ANIMATION_RENDER_EVENT_MANAGER_H
#define DALI_INTERNAL_VECTOR_ANIMATION_RENDER_EVENT_MANAGER_H

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
 *
 */

// EXTERNAL INCLUDES
#include <dali/devel-api/threading/mutex.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/integration-api/processor-interface.h>
#include <set>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

class VectorAnimationRendererNative;

/**
 * @brief Singleton manager to batch vector animation event triggers.
 *
 * Manages the lifecycle of event handlers and ensures safe event delivery
 * in the main thread. Integrates with the Adaptor's Processor system for
 * reliable event processing.
 */
class VectorAnimationRendererEventManager : public Integration::Processor
{
public:
  /**
   * @brief Returns the singleton instance.
   */
  static VectorAnimationRendererEventManager& Get();

  /**
   * @brief Registers a renderer as an event handler.
   * Called in the main thread when the renderer is initialized.
   * @param[in] renderer The renderer to register.
   */
  void AddEventHandler(VectorAnimationRendererNative& renderer);

  /**
   * @brief Unregisters a renderer.
   * Called in the main thread when the renderer is finalized.
   * @param[in] renderer The renderer to unregister.
   */
  void RemoveEventHandler(VectorAnimationRendererNative& renderer);

  /**
   * @brief Called by a renderer when it has completed a frame.
   * @param[in] renderer The renderer that completed a frame.
   */
  void TriggerEvent(VectorAnimationRendererNative& renderer);

protected: // Implementation of Processor
  /**
   * @copydoc Dali::Integration::Processor::Process()
   */
  void Process(bool postProcessor) override;

  /**
   * @copydoc Dali::Integration::Processor::GetProcessorName()
   */
  std::string_view GetProcessorName() const override
  {
    return "VectorAnimationRendererEventManager";
  }

private:
  VectorAnimationRendererEventManager();
  ~VectorAnimationRendererEventManager();

  // Undefined copy constructor and assignment operator.
  VectorAnimationRendererEventManager(const VectorAnimationRendererEventManager&)            = delete;
  VectorAnimationRendererEventManager& operator=(const VectorAnimationRendererEventManager&) = delete;

  /**
   * @brief Callback called in the main thread when the trigger is fired.
   */
  void OnEventTriggered();

private:
  std::unordered_set<VectorAnimationRendererNative*>           mEventHandlers;
  std::unordered_map<VectorAnimationRendererNative*, uint32_t> mTriggeredHandlers; ///< first: trigger handler, second: trigger order. Must be locked under mMutex.
  Dali::Mutex                                                  mMutex;
  TriggerEventFactory::TriggerEventPtr                         mEventTrigger;
  uint32_t                                                     mTriggerOrderId;    ///< Used to determine the order of trigger handler.

  bool mEventTriggered;                                ///< Accessed from multiple threads. Must not be bit-field.
  bool mProcessorRegistered : 1;                       ///< Main thread only.
  bool mEventHandlerRemovedDuringEventProcessing : 1;  ///< Main thread only.
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_VECTOR_ANIMATION_RENDER_EVENT_MANAGER_H