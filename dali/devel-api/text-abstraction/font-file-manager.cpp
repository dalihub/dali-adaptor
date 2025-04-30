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

// CLASS HEADER
#include <dali/devel-api/text-abstraction/font-file-manager.h>

// INTERNAL INCLUDES
#include <dali/internal/text/text-abstraction/font-file-manager-impl.h>

namespace Dali
{
namespace TextAbstraction
{
FontFileManager::FontFileManager()
{
}

FontFileManager::~FontFileManager()
{
}

FontFileManager::FontFileManager(Internal::FontFileManager* impl)
: BaseHandle(impl)
{
}

FontFileManager FontFileManager::Get()
{
  return Internal::FontFileManager::Get();
}

FontFileManager::FontFileManager(const FontFileManager& handle) = default;

FontFileManager& FontFileManager::operator=(const FontFileManager& handle) = default;

FontFileManager::FontFileManager(FontFileManager&& handle) noexcept = default;

FontFileManager& FontFileManager::operator=(FontFileManager&& handle) noexcept = default;

bool FontFileManager::FindFontFile(const FontPath& fontPath) const
{
  return GetImplementation(*this).FindFontFile(fontPath);
}

bool FontFileManager::FindFontFile(const FontPath& fontPath, Dali::Any& fontFilePtr, std::streampos& fileSize) const
{
  return GetImplementation(*this).FindFontFile(fontPath, fontFilePtr, fileSize);
}

void FontFileManager::CacheFontFile(const FontPath& fontPath, Dali::Vector<uint8_t>&& fontFileBuffer, const std::streampos& fileSize) const
{
  GetImplementation(*this).CacheFontFile(fontPath, std::move(fontFileBuffer), fileSize);
}

void FontFileManager::ClearCache()
{
  GetImplementation(*this).ClearCache();
}

} // namespace TextAbstraction

} // namespace Dali
