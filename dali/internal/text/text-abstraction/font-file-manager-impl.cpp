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

// CLASS  HEADER
#include <dali/internal/text/text-abstraction/font-file-manager-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <unordered_map>

namespace
{
#if defined(DEBUG_ENABLED)
Dali::Integration::Log::Filter* gLogFilter = Dali::Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_FONT_FILE_MANAGER");
#endif

} // namespace

namespace Dali
{
namespace TextAbstraction
{
namespace Internal
{
namespace
{

} // namespace

// For Font Pre-load.
Dali::TextAbstraction::FontFileManager FontFileManager::gFontFileManager(nullptr);

struct FontFileManager::Plugin
{
  Plugin()
  : mFontFileCache()
  {
  }

  ~Plugin()
  {
  }

  bool FindFontFile(const FontPath& fontPath) const
  {
    return mFontFileCache.find(fontPath) != mFontFileCache.end();
  }

  bool FindFontFile(const FontPath& fontPath, Dali::Any& fontFilePtr, std::streampos& fileSize) const
  {
    auto it = mFontFileCache.find(fontPath);
    if(it != mFontFileCache.end())
    {
      fontFilePtr = AnyCast<uint8_t*>(it->second.first.Begin());
      fileSize    = it->second.second;
      return true;
    }
    return false;
  }

  void CacheFontFile(const FontPath& fontPath, Dali::Vector<uint8_t>&& fontFileBuffer, const std::streampos& fileSize)
  {
    mFontFileCache[fontPath] = std::make_pair(std::move(fontFileBuffer), fileSize);
  }

  void ClearCache()
  {
    mFontFileCache.clear();
  }

  std::unordered_map<FontPath, std::pair<Dali::Vector<uint8_t>, std::streampos>> mFontFileCache; ///< Caches font data with each font path as the key, allowing faster loading of fonts later on.
};

FontFileManager::FontFileManager()
 : mPlugin(std::make_unique<Plugin>())
{
}

FontFileManager::~FontFileManager()
{
}

TextAbstraction::FontFileManager FontFileManager::Get()
{
  TextAbstraction::FontFileManager fontFileManagerHandle;

  SingletonService service(SingletonService::Get());
  if(service)
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(TextAbstraction::FontFileManager));
    if(handle)
    {
      // If so, downcast the handle
      FontFileManager* impl = dynamic_cast<Internal::FontFileManager*>(handle.GetObjectPtr());
      fontFileManagerHandle = TextAbstraction::FontFileManager(impl);
    }
    else // create and register the object
    {
      if(gFontFileManager)
      {
        fontFileManagerHandle = gFontFileManager;
        gFontFileManager.Reset();
      }
      else
      {
        fontFileManagerHandle = TextAbstraction::FontFileManager(new FontFileManager);
      }
      service.Register(typeid(fontFileManagerHandle), fontFileManagerHandle);
    }
  }
  else
  {
    if(!gFontFileManager)
    {
      // For Font Pre-load, global font client will use this.
      gFontFileManager = Dali::TextAbstraction::FontFileManager(new FontFileManager);
    }
    return gFontFileManager;
  }
  return fontFileManagerHandle;
}

bool FontFileManager::FindFontFile(const FontPath& fontPath) const
{
  return mPlugin->FindFontFile(fontPath);
}

bool FontFileManager::FindFontFile(const FontPath& fontPath, Dali::Any& fontFilePtr, std::streampos& fileSize) const
{
  return mPlugin->FindFontFile(fontPath, fontFilePtr, fileSize);
}

void FontFileManager::CacheFontFile(const FontPath& fontPath, Dali::Vector<uint8_t>&& fontFileBuffer, const std::streampos& fileSize) const
{
  mPlugin->CacheFontFile(fontPath, std::move(fontFileBuffer), fileSize);
}

void FontFileManager::ClearCache()
{
  mPlugin->ClearCache();
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
