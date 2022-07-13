#ifndef DALI_WEB_ENGINE_POLICY_DECISION_H
#define DALI_WEB_ENGINE_POLICY_DECISION_H

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
class WebEngineFrame;

/**
 * @brief A class WebBackForwardList for back forward list of web engine.
 */
class WebEnginePolicyDecision
{
public:
  /**
   * @brief Enumeration that provides an option to policy decision types.
   */
  enum class DecisionType
  {
    USE,      ///< Use.
    DOWNLOAD, ///< Download.
    IGNORE,   ///< Ignore.
  };

  /**
   * @brief Enumeration that provides an option to policy navigation types.
   */
  enum class NavigationType
  {
    LINK_CLICKED,     ///< Link clicked.
    FORM_SUBMITTED,   ///< Form submitted.
    BACK_FORWARD,     ///< Back forward.
    RELOAD,           ///< Reload.
    FORM_RESUBMITTED, ///< Form resubmitted.
    OTHER,            ///< Other.
  };

  /**
   * @brief Constructor.
   */
  WebEnginePolicyDecision() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEnginePolicyDecision() = default;

  /**
   * @brief Returns the url that request policy decision.
   * @return The url that request policy decision
   */
  virtual std::string GetUrl() const = 0;

  /**
   * @brief Returns a cookie that web page has.
   * @return The cookie string if successfully
   */
  virtual std::string GetCookie() const = 0;

  /**
   * @brief Returns a decision type.
   * @return The decision type
   */
  virtual DecisionType GetDecisionType() const = 0;

  /**
   * @brief Returns a MIME type for response data.
   * @return The MIME type string
   */
  virtual std::string GetResponseMime() const = 0;

  /**
   * @brief Returns an HTTP status code.
   * @return The HTTP status code number
   */
  virtual int32_t GetResponseStatusCode() const = 0;

  /**
   * @brief Returns a navigation type.
   * @return The navigation type
   */
  virtual NavigationType GetNavigationType() const = 0;

  /**
   * @brief Gets frame.
   * @return The frame of policy decision
   */
  virtual WebEngineFrame& GetFrame() const = 0;

  /**
   * @brief Gets a scheme from the Policy Decision.
   * @return The scheme if succeeded, empty otherwise
   */
  virtual std::string GetScheme() const = 0;

  /**
   * @brief Accepts the action which triggers this decision.
   * @return True if successfully, false otherwise
   */
  virtual bool Use() = 0;

  /**
   * @brief Ignores the action which triggers this decision.
   * @return True if successfully, false otherwise
   */
  virtual bool Ignore() = 0;

  /**
   * @brief Suspend the operation for policy decision.
   * @return True if successfully, false otherwise
   */
  virtual bool Suspend() = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_POLICY_DECISION_H
