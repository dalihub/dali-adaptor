#ifndef DALI_INTERNAL_ADAPTOR_SYSTEM_COMMON_EVENT_LOOP_H
#define DALI_INTERNAL_ADAPTOR_SYSTEM_COMMON_EVENT_LOOP_H

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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * @brief Abstract interface for platform event loop operations.
 *
 * This class abstracts the platform-specific event loop initialization,
 * shutdown, and main loop execution. Each backend provides a concrete
 * implementation (e.g., ecore_init/ecore_shutdown/ecore_main_loop_begin/
 * ecore_main_loop_quit for Ecore).
 */
class DALI_ADAPTOR_API EventLoop
{
public:
  EventLoop()          = default;
  virtual ~EventLoop() = default;

  /**
   * @brief Initializes the platform event loop system.
   * @param[in] argc The number of arguments.
   * @param[in] argv The argument list.
   */
  virtual void Initialize(int argc, char** argv) = 0;

  /**
   * @brief Shuts down the platform event loop system.
   */
  virtual void Shutdown() = 0;

  /**
   * @brief Starts the main event loop (blocks until Quit is called).
   */
  virtual void Run() = 0;

  /**
   * @brief Requests the main event loop to quit.
   */
  virtual void Quit() = 0;

  // Not copyable or movable
  EventLoop(const EventLoop&)            = delete;
  EventLoop& operator=(const EventLoop&) = delete;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_SYSTEM_COMMON_EVENT_LOOP_H
