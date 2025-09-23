#ifndef DALI_PLATFORM_TEXT_ABSTRACTION_FONT_FILE_MANAGER_H
#define DALI_PLATFORM_TEXT_ABSTRACTION_FONT_FILE_MANAGER_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/font-client.h>
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/any.h>
#include <dali/public-api/object/base-handle.h>

namespace Dali
{
namespace TextAbstraction
{

namespace Internal DALI_INTERNAL
{
class FontFileManager;

} //namespace Internal DALI_INTERNAL

/**
 * @brief Read font files for freetype memory face use and store them in the cache.
 */
class DALI_ADAPTOR_API FontFileManager : public BaseHandle
{
public:
  /**
   * @brief Create an uninitialized FontFileManager handle.
   *
   */
  FontFileManager();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~FontFileManager();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle.
   */
  FontFileManager(const FontFileManager& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param [in] handle  A reference to the copied handle.
   * @return A reference to this.
   */
  FontFileManager& operator=(const FontFileManager& handle);

  /**
   * @brief This move constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the moved handle.
   */
  FontFileManager(FontFileManager&& handle) noexcept;

  /**
   * @brief This move assignment operator is required for (smart) pointer semantics.
   *
   * @param [in] handle  A reference to the moved handle.
   * @return A reference to this.
   */
  FontFileManager& operator=(FontFileManager&& handle) noexcept;

  /**
   * @brief This constructor is used by FontFileManager::Get().
   *
   * @param[in] implementation A pointer to the internal fontFileManager object.
   */
  explicit DALI_INTERNAL FontFileManager(Internal::FontFileManager* implementation);

  /**
   * @brief Retrieve a handle to the FontFileManager instance.
   *
   * @return A handle to the FontFileManager.
   * @remarks A reference to the singleton instance of FontFileManager.
   */
  static FontFileManager Get();

  /**
   * @brief Checks if font file for the specified font path is cached.
   *
   * @param[in] fontPath The font path to check for cached file.
   *
   * @return @e true if the font file is cached, otherwise false.
   */
  bool FindFontFile(const FontPath& fontPath) const;

  /**
   * @brief Retrieves font file for the specified font path if it is cached.
   *
   * @param[in] fontPath The font path to retrieve the cached file for.
   * @param[out] fontFilePtr A pointer(uint8_t*) to the cached font file.
   * @param[out] fileSize The size of the cached font file.
   *
   * @return @e true if the font file is cached and retrieved successfully, otherwise false.
   */
  bool FindFontFile(const FontPath& fontPath, Dali::Any& fontFilePtr, std::streampos& fileSize) const;

  /**
   * @brief Caches font file for the specified font path.
   *
   * @param[in] fontPath The font path to cache the file for.
   * @param[in] fontFileBuffer A vector containing the font file to cache.
   * @param[in] fileSize The size of the font file to cache.
   */
  void CacheFontFile(const FontPath& fontPath, Dali::Vector<uint8_t>&& fontFileBuffer, const std::streampos& fileSize) const;

  /**
   * @brief Clear all cached font files.
   */
  void ClearCache();
};

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_PLATFORM_TEXT_ABSTRACTION_FONT_FILE_MANAGER_H
