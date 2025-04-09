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
#include <dali/internal/system/common/texture-upload-manager-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/graphics-api/graphics-texture-upload-helper.h> ///< for Dali::Graphics::ConvertPixelFormat
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/pixel-data-integ.h>
#include <dali/integration-api/texture-integ.h>
#include <dali/integration-api/trace.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
Dali::Devel::TextureUploadManager::ResourceId gUniqueResourceId = 0;

DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_PERFORMANCE_MARKER, false);

#if defined(DEBUG_ENABLED)
Debug::Filter* gTextureUploadManagerLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_TEXTURE_UPLOAD_MANAGER");
#endif

} // unnamed namespace

// Called by main thread

Dali::Devel::TextureUploadManager TextureUploadManager::Get()
{
  Dali::Devel::TextureUploadManager manager;
  SingletonService                  singletonService(SingletonService::Get());
  if(singletonService)
  {
    // Check whether the texture upload manager is already created
    Dali::BaseHandle handle = singletonService.GetSingleton(typeid(Dali::Devel::TextureUploadManager));
    if(handle)
    {
      // If so, downcast the handle of singleton
      manager = Dali::Devel::TextureUploadManager(dynamic_cast<Internal::Adaptor::TextureUploadManager*>(handle.GetObjectPtr()));
    }

    if(!manager)
    {
      // If not, create the texture upload manager and register it as a singleton
      Internal::Adaptor::TextureUploadManager* internalTextureUploadManager = new Internal::Adaptor::TextureUploadManager();
      manager                                                               = Dali::Devel::TextureUploadManager(internalTextureUploadManager);
      singletonService.Register(typeid(manager), manager);
    }
  }
  return manager;
}

TextureUploadManager::TextureUploadManager()
: mGraphicsController{nullptr},
  mRenderTrigger(new EventThreadCallback(MakeCallback(this, &TextureUploadManager::RequestUpdateOnce)))
{
}

TextureUploadManager::~TextureUploadManager()
{
}

Dali::Texture TextureUploadManager::GenerateTexture2D()
{
  ResourceId resourceId = GenerateUploadResourceId();

  Dali::Texture ret = Dali::Integration::NewTextureWithResourceId(Dali::TextureType::TEXTURE_2D, resourceId);

  return ret;
}

Dali::Devel::TextureUploadManager::ResourceId TextureUploadManager::GenerateUploadResourceId()
{
  auto id = ++gUniqueResourceId;

  // Jump overflow case so we can assume that resource id always valid.
  if(DALI_UNLIKELY(gUniqueResourceId == Dali::Devel::TextureUploadManager::INVALID_RESOURCE_ID))
  {
    ++gUniqueResourceId;
  }

  return id;
}

void TextureUploadManager::RequestUpdateOnce()
{
  if(Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_INFO(gTextureUploadManagerLogFilter, Debug::Concise, "UpdateOnce requested\n");
    Dali::Adaptor::Get().UpdateOnce();
  }
}

// Called by update thread

bool TextureUploadManager::ResourceUpload()
{
  DALI_ASSERT_DEBUG(mGraphicsController && "GraphicsController is not prepared!");

  // Copy queue first.
  RequestUploadQueue copiedRequestUploadQueue;

  {
    Dali::Mutex::ScopedLock lock(mRequestMutex); // Worker-Update thread mutex

    copiedRequestUploadQueue.swap(mRequestUploadQueue); // Move upload queue
  }

  // Upload.
  bool uploaded = ProcessUploadQueue(std::move(copiedRequestUploadQueue));

  return uploaded;
}

void TextureUploadManager::InitalizeGraphicsController(Dali::Graphics::Controller& graphicsController)
{
  mGraphicsController = &graphicsController;
}

bool TextureUploadManager::ProcessUploadQueue(RequestUploadQueue&& queue)
{
  bool uploaded = false;

  if(!queue.empty())
  {
    DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_WORKER_THREAD_RESOURCE_UPLOAD", [&](std::ostringstream& oss) {
      oss << "[upload request \'" << queue.size() << "\' images]";
    });
#ifdef TRACE_ENABLED
    uint32_t uploadedCount = 0u;
#endif

    DALI_LOG_INFO(gTextureUploadManagerLogFilter, Debug::Concise, "Upload request %zu images\n", queue.size());
    for(auto& requests : queue)
    {
      auto& resourceId = requests.first;
      auto& pixelData  = requests.second;

      Graphics::Texture* graphicsTexture = nullptr;

      const Dali::Pixel::Format pixelFormat = pixelData.GetPixelFormat();

      {
        // We always need to create new one
        auto createInfo = Graphics::TextureCreateInfo();
        createInfo
          .SetTextureType(Dali::Graphics::ConvertTextureType(Dali::TextureType::TEXTURE_2D))
          .SetUsageFlags(static_cast<Graphics::TextureUsageFlags>(Graphics::TextureUsageFlagBits::SAMPLE))
          .SetFormat(Dali::Graphics::ConvertPixelFormat(pixelFormat))
          .SetSize({pixelData.GetWidth(), pixelData.GetHeight()})
          .SetLayout(Graphics::TextureLayout::LINEAR)
          .SetAllocationPolicy(Graphics::TextureAllocationPolicy::UPLOAD)
          .SetData(nullptr)
          .SetDataSize(0u)
          .SetNativeImage(nullptr)
          .SetMipMapFlag(Graphics::TextureMipMapFlag::DISABLED);

        graphicsTexture = mGraphicsController->CreateTextureByResourceId(resourceId, createInfo);
      }

      if(graphicsTexture)
      {
        Graphics::TextureUpdateInfo info{};

        const uint32_t bytesPerPixel = Dali::Pixel::GetBytesPerPixel(pixelFormat);

        info.dstTexture   = graphicsTexture;
        info.dstOffset2D  = {0u, 0u};
        info.layer        = 0u;
        info.level        = 0u;
        info.srcReference = 0;
        info.srcExtent2D  = {pixelData.GetWidth(), pixelData.GetHeight()};
        info.srcOffset    = 0;
        info.srcSize      = Dali::Integration::GetPixelDataBuffer(pixelData).bufferSize;
        info.srcStride    = bytesPerPixel ? (pixelData.GetStrideBytes() / bytesPerPixel) : 0u; ///< Note : Graphics stride use pixel scale!
        info.srcFormat    = Dali::Graphics::ConvertPixelFormat(pixelFormat);

        Graphics::TextureUpdateSourceInfo updateSourceInfo{};
        updateSourceInfo.sourceType                = Graphics::TextureUpdateSourceInfo::Type::PIXEL_DATA;
        updateSourceInfo.pixelDataSource.pixelData = pixelData;

        mGraphicsController->UpdateTextures({info}, {updateSourceInfo});

        uploaded = true;
#ifdef TRACE_ENABLED
        ++uploadedCount;
#endif
      }
    }

    if(uploaded)
    {
      // Flush here
      Graphics::SubmitInfo submitInfo;
      submitInfo.cmdBuffer.clear(); // Only flush
      submitInfo.flags = 0 | Graphics::SubmitFlagBits::FLUSH;
      mGraphicsController->SubmitCommandBuffers(submitInfo);
    }
    DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_WORKER_THREAD_RESOURCE_UPLOAD", [&](std::ostringstream& oss) {
      oss << "[uploaded : \'" << uploadedCount << "\']";
    });
  }

  return uploaded;
}

// Called by worker thread

void TextureUploadManager::RequestUpload(Dali::Devel::TextureUploadManager::ResourceId resourceId, Dali::PixelData pixelData)
{
  DALI_ASSERT_ALWAYS(resourceId != Dali::Devel::TextureUploadManager::INVALID_RESOURCE_ID && "Invalid resource id generated!");
  DALI_ASSERT_ALWAYS(pixelData && "Invalid pixelData!");

  {
    Dali::Mutex::ScopedLock lock(mRequestMutex); // Worker-Update thread mutex

    mRequestUploadQueue.push_back(std::move(UploadRequestItem(resourceId, pixelData)));
  }

  // wake up the main thread
  // TODO : Is there any way to request upload once without main thread dependency?
  mRenderTrigger->Trigger();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
