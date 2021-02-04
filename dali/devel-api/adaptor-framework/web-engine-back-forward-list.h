#ifndef DALI_WEB_ENGINE_BACK_FORWARD_LIST_H
#define DALI_WEB_ENGINE_BACK_FORWARD_LIST_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/web-engine-back-forward-list-item.h>
#include <string>

namespace Dali
{
class WebEngineBackForwardListItem;

/**
 * @brief A class WebBackForwardList for back forward list of web engine.
 */
class WebEngineBackForwardList
{
public:
  /**
   * @brief Constructor.
   */
  WebEngineBackForwardList() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineBackForwardList() = default;

  /**
   * @brief Returns the current item in the @a list.
   * @return The item of back-forward list.
   */
  virtual WebEngineBackForwardListItem& GetCurrentItem() const = 0;

  /**
   * @brief Returns the item at a given @a index relative to the current item.
   * @param[in] index The index of the item
   * @return The item of back-forward list.
   */
  virtual WebEngineBackForwardListItem& GetItemAtIndex(uint32_t index) const = 0;

  /**
   * @brief Returns the length of the back-forward list including the current
   *        item.
   * @return The length of the back-forward list including the current item,
   *         otherwise @c 0 in case of an error
   */
  virtual uint32_t GetItemCount() const = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_BACK_FORWARD_LIST_H
