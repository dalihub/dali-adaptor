#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_PLUGIN_PIXEL_BUFFER_CACHE_ITEM_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_PLUGIN_PIXEL_BUFFER_CACHE_ITEM_H

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
 */

namespace Dali::TextAbstraction::Internal
{
/**
 * @brief Type used for indices addressing the vector with pixel buffers.
 */
typedef uint32_t PixelBufferId;

/**
 * @brief Caches pixel buffers.
 */
struct PixelBufferCacheItem
{
  Devel::PixelBuffer pixelBuffer; ///< The pixel buffer loaded from the url.
  std::string        url;         ///< The url.
  PixelBufferId      id;          ///< The id of this item. Should be bigger than 0.
};

} // namespace Dali::TextAbstraction::Internal

#endif //DALI_INTERNAL_TEXT_ABSTRACTION_PLUGIN_PIXEL_BUFFER_CACHE_ITEM_H
