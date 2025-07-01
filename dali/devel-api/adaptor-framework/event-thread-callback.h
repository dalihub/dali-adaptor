#ifndef DALI_EVENT_THREAD_CALLBACK_H
#define DALI_EVENT_THREAD_CALLBACK_H

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
#include <dali/public-api/signals/callback.h>

// EXTERNAL INCLUDES
#include <cstdint> ///< for uint32_t

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @brief The EventThreadCallback class provides a mechanism for the worker thread to trigger the execution of a given callback in main event thread .
 *
 * @note The EventThreadCallback object should only be created in the main thread.
 */
class DALI_ADAPTOR_API EventThreadCallback
{
public:
  /**
   * @brief Constructor. Create an object that will call the given callback in main event thread.
   *
   * @param[in] callback The callback to call.
   */
  EventThreadCallback(CallbackBase* callback);

  /**
   * @brief Destructor.
   */
  ~EventThreadCallback();

  /**
   * @brief Trigger the calling of callback.
   *
   * The method can be used from worker threads to notify the main thread as main thread is running the event loop and thus cannot be blocked
   */
  void Trigger();

  /**
   * Get the unique id of event thread callback.
   *
   * @return Id of this event thread callback.
   */
  uint32_t GetId() const;

private:
  // undefined copy constructor.
  EventThreadCallback(const EventThreadCallback&);

  // undefined assignment operator
  EventThreadCallback& operator=(const EventThreadCallback&);

private:
  struct Impl;
  Impl* mImpl;
};

} // namespace Dali
#endif /* DALI_EVENT_THREAD_CALLBACK_H */
