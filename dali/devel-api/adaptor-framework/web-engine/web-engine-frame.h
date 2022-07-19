#ifndef DALI_WEB_ENGINE_FRAME_H
#define DALI_WEB_ENGINE_FRAME_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <string>

namespace Dali
{
/**
 * @brief A class WebEngineFrame for frame of web engine.
 */
class WebEngineFrame
{
public:
  /**
   * @brief Constructor.
   */
  WebEngineFrame() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineFrame() = default;

  /**
   * @brief Check whether the frame is main frame.
   * @return true if the frame is main frame, false otherwise
   */
  virtual bool IsMainFrame() const = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_FRAME_H
