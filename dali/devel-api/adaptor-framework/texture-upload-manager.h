#ifndef DALI_DEVEL_TEXTURE_UPLOAD_MANAGER_H
#define DALI_DEVEL_TEXTURE_UPLOAD_MANAGER_H

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
 */

// EXTERNAL INCLUDES
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/images/pixel-data.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/rendering/texture.h>
#include <stdint.h> ///< For uint32_t

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal DALI_INTERNAL
{
namespace Adaptor DALI_INTERNAL
{
class TextureUploadManager;
} //namespace Adaptor DALI_INTERNAL
} //namespace Internal DALI_INTERNAL

namespace Devel
{
/**
 * The manager that make we can upload resource into texture from the worker thread.
 * @code
 * (Event Thread)
 * someAsyncTask.mUploadManager = TextureUploadManager::Get();
 * someAsyncTask.mTexture = someAsyncTask.mUploadManager.GenerateTexture2D();
 * someAsyncTask.mResourceId = Integration::GetTextureResourceId(someAsyncTask.mTexture);
 * AsyncTaskManager::Get().AddTask(someAsyncTask);
 *
 * (Worker Thread)
 * SomeAsyncTask::Process()
 * {
 *   PixelData pixelData = LoadImage();
 *   ...
 *   mUploadManager.RequestUpload(mResourceId, pixelData); // Upload to Graphics::Texture paired by UploadResourceId.
 * }
 * @endcode
 *
 * @SINCE_2_2.38
 */
class DALI_ADAPTOR_API TextureUploadManager : public BaseHandle
{
public:
  using ResourceId                                = uint32_t;
  constexpr static ResourceId INVALID_RESOURCE_ID = 0u;

public: // Callbed by main thread.
  /**
   * Constructor.
   * @SINCE_2_2.38
   */
  TextureUploadManager();

  /**
   * Destructor.
   * @SINCE_2_2.38
   */
  ~TextureUploadManager();

  /**
   * @brief Gets the singleton of TextureUploadManager object.
   *
   * @SINCE_2_2.38
   * @return A handle to the TextureUploadManager
   */
  static TextureUploadManager Get();

  /**
   * @brief Generate the texture 2d that hold unique id for upload resources, called by main thread.
   *
   * @SINCE_2_2.38
   * @return The texture that hold unique id of upload resource.
   */
  Dali::Texture GenerateTexture2D();

public: // Called by update thread.
  /**
   * @brief Upload all requested resources by RequestUpload.
   *
   * @SINCE_2_2.38
   * @return True if there was at least 1 resources uploaded.
   */
  bool ResourceUpload();

public: // Can be callbed by worker thread.
  /**
   * @brief Request upload PixelData to given ResourceId.
   * @note We should not request invalid resouceId.
   * @note We should not request mutiple times into same resourceId.
   *
   * @SINCE_2_2.38
   * @param[in] resourceId The id of resource.
   * @param[in] pixelData The buffer of resource.
   */
  void RequestUpload(ResourceId resourceId, PixelData pixelData);

public:
  /// @cond internal
  /**
   * @brief Allows the creation of a TextureUploadManager handle from an internal pointer.
   *
   * @note Not intended for application developers
   * @SINCE_2_2.38
   * @param[in] impl A pointer to the object
   */
  explicit DALI_INTERNAL TextureUploadManager(Internal::Adaptor::TextureUploadManager* impl);
  /// @endcond
};
} // namespace Devel

} // namespace Dali

#endif
