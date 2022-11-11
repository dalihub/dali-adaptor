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
 * Type value should be start with 0, and increase continuous.
 */
using ExtensionCheckerType = uint32_t;

/**
 * @brief Store the supported information cached or not.
 */
struct ExtensionSupportedCache
{
  ExtensionSupportedCache()
  : isSupported{false},
    cached{false}
  {
  }
  bool isSupported : 1;
  bool cached : 1;
};

/**
 * @brief Extension Supported caching system interface.
 *
 * Create ExtensionSupportedCache list and Set/Get supported value.
 * Use ExtensionCheckerType as index of container.
 *
 * It will help to check gl extension & egl extention support.
 */
struct ExtensionSupportedCacheListInterface
{
  ExtensionSupportedCacheListInterface(const uint32_t maxCount)
  : mCachedItemCount(0u),
    mMaxCount(maxCount),
    mData{maxCount}
  {
  }

  /**
   * @brief Check whether we need to check some more extension types or not.
   * @return True if some extension type to check remained. False if we checked all extension types.
   */
  inline bool NeedFullCheck() const
  {
    return mCachedItemCount < mMaxCount;
  }

  /**
   * @brief Set extension is supported or not
   * If we already cached the result before, just ignored.
   *
   * @param[in] type The index of extension type.
   * @param[in] isSupported Whether this extension supported or not. Default as true.
   */
  inline void MarkSupported(ExtensionCheckerType type, bool isSupported = true)
  {
    auto& cache = mData[static_cast<uint32_t>(type)];
    if(!cache.cached)
    {
      cache.cached      = true;
      cache.isSupported = isSupported;
      ++mCachedItemCount;
    }
  }

  /**
   * @brief Get extension is supported or not.
   *
   * @param[in] type The index of extension type.
   * @return True if we cached extension as supported. False otherwise.
   */
  inline bool IsSupported(ExtensionCheckerType type) const
  {
    return mData[static_cast<uint32_t>(type)].isSupported;
  }

  /**
   * @brief Get extension is cached or not.
   *
   * @param[in] type The index of extension type.
   * @return True if we cached extension. False otherwise.
   */
  inline bool IsCached(ExtensionCheckerType type) const
  {
    return mData[static_cast<uint32_t>(type)].cached;
  }

  /**
   * @brief Mark all uncached extension type as not supported.
   * After this API called, we can assume that every extensions are cached.
   */
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

/**
 * @brief Extension Supported caching system for gl.
 */
struct GlExtensionSupportedCacheList : public ExtensionSupportedCacheListInterface
{
  GlExtensionSupportedCacheList()
  : ExtensionSupportedCacheListInterface(static_cast<uint32_t>(GlExtensionCheckerType::EXTENSION_CHECKER_TYPE_MAX))
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
