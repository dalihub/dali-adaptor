#ifndef DALI_WEB_ENGINE_BACK_FORWARD_LIST_ITEM_H
#define DALI_WEB_ENGINE_BACK_FORWARD_LIST_ITEM_H

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
 * @brief A class WebBackForwardListItem for back forward list item of web engine.
 */
class WebEngineBackForwardListItem
{
public:
  /**
   * @brief Constructor.
   */
  WebEngineBackForwardListItem() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineBackForwardListItem() = default;

  /**
   * @brief Returns the URL of the item.
   * @details The returned URL may differ from the original URL (For example,
   *          if the page is redirected).
   * @return The URL of the item, otherwise "" in case of an error
   */
  virtual std::string GetUrl() const = 0;

  /**
   * @brief Returns the title of the item.
   * @return The title of the item, otherwise "" in case of an error
   */
  virtual std::string GetTitle() const = 0;

  /**
   * @brief Returns the original URL of the item.
   * @return The original URL of the item, otherwise "" in case of an error
   */
  virtual std::string GetOriginalUrl() const = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_BACK_FORWARD_LIST_ITEM_H
