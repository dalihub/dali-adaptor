#ifndef DALI_WEB_ENGINE_FORM_REPOST_DECISION_H
#define DALI_WEB_ENGINE_FORM_REPOST_DECISION_H

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

namespace Dali
{
/**
 * @brief A class WebEngineFormRepostDecision for form repost decision.
 */
class WebEngineFormRepostDecision
{
public:
  /**
   * @brief Constructor.
   */
  WebEngineFormRepostDecision() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineFormRepostDecision() = default;

  /**
   * @brief Reply the result about form repost decision.
   *
   * @param[in] allowed Whether allow form repost decision request or not
   */
  virtual void Reply(bool allowed) = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_FORM_REPOST_DECISION_H
