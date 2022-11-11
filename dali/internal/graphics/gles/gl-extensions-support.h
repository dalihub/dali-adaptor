#ifndef DALI_INTERNAL_GL_EXTENSION_SUPPORT_H
#define DALI_INTERNAL_GL_EXTENSION_SUPPORT_H

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
#include <dali/public-api/common/vector-wrapper.h>
#include <cstdint>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Common extensions support checker type.
 */
using ExtensionCheckerType = uint32_t;

struct ExtensionSupportedCache
{
  bool isSupported{false};
  bool cached{false};
};

struct ExtensionSupportedCacheListInterface
{
  ExtensionSupportedCacheListInterface(const uint32_t maxCount)
  : mCachedItemCount(0u),
    mMaxCount(maxCount),
    mData{maxCount}
  {
  }

  inline bool NeedFullCheck() const
  {
    return mCachedItemCount < mMaxCount;
  }

  inline void SetSupported(ExtensionCheckerType type, bool isSupported = true)
  {
    auto& cache = mData[static_cast<uint32_t>(type)];
    if(!cache.cached)
    {
      cache.cached      = true;
      cache.isSupported = isSupported;
      ++mCachedItemCount;
    }
  }

  inline bool GetSupported(ExtensionCheckerType type) const
  {
    return mData[static_cast<uint32_t>(type)].isSupported;
  }

  inline bool GetCached(ExtensionCheckerType type) const
  {
    return mData[static_cast<uint32_t>(type)].cached;
  }

  void SetAllUncachedAsNotSupported()
  {
    if(NeedFullCheck())
    {
      for(auto&& iter : mData)
      {
        if(!iter.cached)
        {
          iter.isSupported = false;
          iter.cached      = true;
        }
      }

      // Mark all cached.
      mCachedItemCount = mMaxCount;
    }
  }

  uint32_t       mCachedItemCount;
  const uint32_t mMaxCount;

  std::vector<ExtensionSupportedCache> mData;
};

/**
 * Gl extensions support checker system.
 */
namespace GlExtensionCache
{
enum GlExtensionCheckerType
{
  BLEND_EQUATION_ADVANCED = 0,
  MULTISAMPLED_RENDER_TO_TEXTURE,
  ///< Append additional extension checker type here.
  EXTENSION_CHECKER_TYPE_MAX,
};

struct GlExtensionSupportedCacheList : public ExtensionSupportedCacheListInterface
{
  GlExtensionSupportedCacheList()
  : ExtensionSupportedCacheListInterface(static_cast<size_t>(GlExtensionCheckerType::EXTENSION_CHECKER_TYPE_MAX))
  {
  }

  /**
   * Ensure that we check all gl extension features for this system.
   */
  void EnsureGlExtensionSupportedCheck();
};
} // namespace GlExtensionCache

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif /* DALI_INTERNAL_GL_EXTENSION_SUPPORT_H */
