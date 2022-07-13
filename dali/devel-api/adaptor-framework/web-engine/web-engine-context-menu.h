#ifndef DALI_WEB_ENGINE_CONTEXT_MENU_H
#define DALI_WEB_ENGINE_CONTEXT_MENU_H

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
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-context-menu-item.h>
#include <dali/public-api/math/vector2.h>
#include <memory>
#include <string>
#include <vector>

namespace Dali
{
/**
 * @brief A class WebEngineContextMenu for context menu of web engine.
 */
class WebEngineContextMenu
{
public:
  /**
   * @brief Constructor.
   */
  WebEngineContextMenu() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineContextMenu() = default;

  /**
   * @brief Count the number of the context menu item.
   *
   * @return The number of current context menu item
   */
  virtual uint32_t GetItemCount() const = 0;

  /**
   * @brief Return the nth item in a context menu.
   *
   * @param[in] index The number of the item
   *
   * @return The nth item of context menu
   */
  virtual std::unique_ptr<WebEngineContextMenuItem> GetItemAt(uint32_t index) const = 0;

  /**
   * @brief Get the list of items.
   *
   * @return item list of context menu
   */
  virtual std::vector<std::unique_ptr<WebEngineContextMenuItem>> GetItemList() const = 0;

  /**
   * @brief Remove the item from the context menu.
   *
   * @param[in] item The context menu item to be removed
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool RemoveItem(WebEngineContextMenuItem& item) = 0;

  /**
   * @brief Add the item to the context menu.
   *
   * @param[in] tag The tag of context menu item
   * @param[in] title The title of context menu item
   * @param[in] enabled If true the context menu item is enabled, false otherwise
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool AppendItemAsAction(WebEngineContextMenuItem::ItemTag tag, const std::string& title, bool enabled) = 0;

  /**
   * @brief Add the item to the context menu.
   *
   * @param[in] tag The tag of context menu item
   * @param[in] title The title of context menu item
   * @param[in] iconFile The path of icon to be set on context menu item
   * @param[in] enabled If true the context menu item is enabled, false otherwise
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool AppendItem(WebEngineContextMenuItem::ItemTag tag, const std::string& title, const std::string& iconFile, bool enabled) = 0;

  /**
   * @brief Select the item from the context menu.
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool SelectItem(WebEngineContextMenuItem& item) = 0;

  /**
   * @brief Hide the context menu.
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool Hide() = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_CONTEXT_MENU_H
