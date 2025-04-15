#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_FONT_FILE_MANAGER_IMPL_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_FONT_FILE_MANAGER_IMPL_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/font-file-manager.h>

namespace Dali
{
namespace TextAbstraction
{
namespace Internal
{
/**
 * Implementation of the FontFileManager
 */
class FontFileManager : public BaseObject
{
public:
  /**
   * Constructor
   */
  FontFileManager();

  /**
   * Destructor
   */
  ~FontFileManager();

  /**
   * @copydoc Dali::FontFileManager::Get()
   */
  static TextAbstraction::FontFileManager Get();

  /**
   * @copydoc Dali::FontFileManager::FindFontFile()
   */
  bool FindFontFile(const FontPath& fontPath) const;

  /**
   * @copydoc Dali::FontFileManager::FindFontFile()
   */
  bool FindFontFile(const FontPath& fontPath, Dali::Any& fontFilePtr, std::streampos& fileSize) const;

  /**
   * @copydoc Dali::FontFileManager::CacheFontFile()
   */
  void CacheFontFile(const FontPath& fontPath, Dali::Vector<uint8_t>&& fontFileBuffer, const std::streampos& fileSize) const;

  /**
   * @copydoc Dali::FontFileManager::ClearCache()
   */
  void ClearCache();

private:
  // Undefined copy constructor.
  FontFileManager(const FontFileManager&);

  // Undefined assignment constructor.
  FontFileManager& operator=(const FontFileManager&);

  struct Plugin;
  std::unique_ptr<Plugin> mPlugin;

  static Dali::TextAbstraction::FontFileManager gFontFileManager;

}; // class FontFileManager

} // namespace Internal

} // namespace TextAbstraction

inline static TextAbstraction::Internal::FontFileManager& GetImplementation(TextAbstraction::FontFileManager& fontFileManager)
{
  DALI_ASSERT_ALWAYS(fontFileManager && "fontFileManager handle is empty");
  BaseObject& handle = fontFileManager.GetBaseObject();
  return static_cast<TextAbstraction::Internal::FontFileManager&>(handle);
}

inline static const TextAbstraction::Internal::FontFileManager& GetImplementation(const TextAbstraction::FontFileManager& fontFileManager)
{
  DALI_ASSERT_ALWAYS(fontFileManager && "fontFileManager handle is empty");
  const BaseObject& handle = fontFileManager.GetBaseObject();
  return static_cast<const TextAbstraction::Internal::FontFileManager&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_TEXT_ABSTRACTION_FONT_FILE_MANAGER_IMPL_H
