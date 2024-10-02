#ifndef DALI_WEB_ENGINE_USER_MEDIA_PERMISSION_REQUEST_H
#define DALI_WEB_ENGINE_USER_MEDIA_PERMISSION_REQUEST_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

namespace Dali
{
/**
 * @brief A class WebEngineUserMediaPermissionRequest for handling user's media permission of web engine.
 */
class WebEngineUserMediaPermissionRequest
{
public:
  /**
   * @brief Constructor.
   */
  WebEngineUserMediaPermissionRequest() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineUserMediaPermissionRequest() = default;

  /**
   * @brief Request to set user media permission of web engine.
   *
   * @param[in] allowed if true, allow to set, false, not allow to set.
   */
  virtual void Set(bool allowed) const = 0;

  /**
   * @brief Suspend user media permission request process of web engine.
   * @return true if the suspension was successful, false otherwise.
   */
  virtual bool Suspend() const = 0;
};
} // namespace Dali

#endif // DALI_WEB_ENGINE_USER_MEDIA_PERMISSION_REQUEST_H
