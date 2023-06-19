#ifndef DALI_INTERNAL_TEXTURE_UPLOAD_MANAGER_IMPL_H
#define DALI_INTERNAL_TEXTURE_UPLOAD_MANAGER_IMPL_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/graphics-api/graphics-controller.h>
#include <dali/graphics-api/graphics-texture.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/object/base-object.h>
#include <utility> ///< for std::pair

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/texture-upload-manager.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * The manager for texture upload
 */
class TextureUploadManager : public Dali::BaseObject
{
public:
  using ResourceId = Dali::Devel::TextureUploadManager::ResourceId;

public: // Main thread called method
  /**
   * Singleton access
   *
   * @return The TextureUploadManager object
   */
  static Dali::Devel::TextureUploadManager Get();

  /**
   * Constructor.
   */
  TextureUploadManager();

  /**
   * Destructor.
   */
  ~TextureUploadManager() override;

  /**
   * @copydoc Dali::Devel::TextureUploadManager::GenerateTexture2D()
   */
  Dali::Texture GenerateTexture2D();

private: // Main thread called method
  /**
   * @brief Get the unique id for upload resources, called by main thread.
   *
   * @return The unique id of upload resource.
   */
  ResourceId GenerateUploadResourceId();

  /**
   * @brief Reqeust update thread once
   */
  void RequestUpdateOnce();

public: // Update thread called method
  /**
   * @copydoc Dali::Devel::TextureUploadManager::ResourceUpload()
   */
  bool ResourceUpload();

  /**
   * @brief Install graphics controller to be used when upload.
   * @note Please use this API internal side only.
   * @note This API will be removed after toolkit UTC code fixed.
   *
   * @param[in] graphicsController The graphics controller.
   */
  void InitalizeGraphicsController(Dali::Graphics::Controller& graphicsController);

private: // Update thread called method
  using UploadRequestItem  = std::pair<ResourceId, Dali::PixelData>;
  using RequestUploadQueue = std::vector<UploadRequestItem>;

  /**
   * @brief Process queue of upload.
   *
   * @param queue The requested upload queue.
   * @return True if there was at least 1 resources uploaded.
   */
  bool ProcessUploadQueue(RequestUploadQueue&& queue);

public: // Worker thread called method
  /**
   * @copydoc Dali::Devel::TextureUploadManager::RequestUpload()
   */
  void RequestUpload(ResourceId id, Dali::PixelData pixelData);

private:
  // Undefined
  TextureUploadManager(const TextureUploadManager& manager);

  // Undefined
  TextureUploadManager& operator=(const TextureUploadManager& manager);

private:
  Dali::Graphics::Controller* mGraphicsController;

  std::unique_ptr<EventThreadCallback> mRenderTrigger; ///< Trigger to update/render once for worker thread.

  Dali::Mutex        mRequestMutex; ///< For worker thread
  RequestUploadQueue mRequestUploadQueue{};
};

} // namespace Adaptor

} // namespace Internal

inline Internal::Adaptor::TextureUploadManager& GetImplementation(Dali::Devel::TextureUploadManager& obj)
{
  DALI_ASSERT_ALWAYS(obj && "TextureUploadManager is empty");

  Dali::BaseObject& handle = obj.GetBaseObject();

  return static_cast<Internal::Adaptor::TextureUploadManager&>(handle);
}

inline const Internal::Adaptor::TextureUploadManager& GetImplementation(const Dali::Devel::TextureUploadManager& obj)
{
  DALI_ASSERT_ALWAYS(obj && "TextureUploadManager is empty");

  const Dali::BaseObject& handle = obj.GetBaseObject();

  return static_cast<const Internal::Adaptor::TextureUploadManager&>(handle);
}

} // namespace Dali

#endif
