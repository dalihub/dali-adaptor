#ifndef DALI_WEB_ENGINE_CONSOLE_MESSAGE_H
#define DALI_WEB_ENGINE_CONSOLE_MESSAGE_H

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
 * @brief A class WebEngineConsoleMessage for console message of web engine.
 */
class WebEngineConsoleMessage
{
public:
  /**
   * @brief Enumeration that provides level of log severity.
   */
  enum class SeverityLevel
  {
    EMPTY,
    LOG,
    WARNING,
    ERROR,
    DEBUG,
    INFO,
  };

  /**
   * @brief Constructor.
   */
  WebEngineConsoleMessage() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineConsoleMessage() = default;

  /**
   * @brief Return the source of the console message.
   * @return source of the console message if succeded or empty string if failed
   */
  virtual std::string GetSource() const = 0;

  /**
   * @brief Return line no of the console message.
   * @return the line number of the message if succeded or 0 if failed
   */
  virtual uint32_t GetLine() const = 0;

  /**
   * @brief Return the log severity of the console message.
   * @return SeverityLevel indicating the console message level
   */
  virtual SeverityLevel GetSeverityLevel() const = 0;

  /**
   * @brief Return the console message text.
   * @return console message text if succeded or empty string if failed
   */
  virtual std::string GetText() const = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_CONSOLE_MESSAGE_H
