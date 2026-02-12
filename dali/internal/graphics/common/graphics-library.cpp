/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

// HEADER
#include <dali/internal/graphics/common/graphics-library.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dlfcn.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-library-open-mode.h>
#include <dali/internal/window-system/common/native-image-surface-impl.h>
#include <dali/public-api/adaptor-framework/graphics-backend.h>

namespace Dali::Internal::Adaptor::GraphicsLibrary
{
namespace
{
const char* const DALI_ADAPTOR_GRAPHICS_GLES_SO("libdali2-adaptor-gles.so");
const char* const DALI_ADAPTOR_GRAPHICS_VULKAN_SO("libdali2-adaptor-vulkan.so");

class DynamicGraphicsLibraryHandle : public Dali::Internal::Adaptor::GraphicsLibraryHandleBase
{
public:
  DynamicGraphicsLibraryHandle()
  {
    mBackend = Graphics::GetCurrentGraphicsBackend();

    DALI_LOG_DEBUG_INFO("dlopen for Graphics Backend : %s\n", mBackend == Graphics::Backend::GLES ? "GLES" : "VULKAN");

    mHandle = dlopen((mBackend == Graphics::Backend::GLES) ? DALI_ADAPTOR_GRAPHICS_GLES_SO : DALI_ADAPTOR_GRAPHICS_VULKAN_SO,
                     GetLibraryOpenMode());
    if(!mHandle)
    {
      // The shared library failed to load
      DALI_LOG_ERROR("dlopen error: %s\n", dlerror());
      DALI_ABORT("Unable to open Graphics Library\n");
    }
  }

  ~DynamicGraphicsLibraryHandle()
  {
    if(DALI_LIKELY(mGraphics))
    {
      // Release graphics before dlclose()
      mGraphics->Destroy();
      mGraphics.reset();
    }
    if(mHandle)
    {
      DALI_LOG_DEBUG_INFO("dlclose for Graphics Backend : %s\n", mBackend == Graphics::Backend::GLES ? "GLES" : "VULKAN");
      dlclose(mHandle);
    }
  }

public:
  Graphics::Backend mBackend{Graphics::Backend::DEFAULT};
  void*             mHandle{nullptr};
};
std::shared_ptr<DynamicGraphicsLibraryHandle> gGraphicsLibraryHandle;

template<typename FunctionSignature>
FunctionSignature GetFunction(const char* const functionName)
{
  if(DALI_UNLIKELY(!gGraphicsLibraryHandle))
  {
    gGraphicsLibraryHandle.reset(new DynamicGraphicsLibraryHandle);
  }

  if(DALI_LIKELY(gGraphicsLibraryHandle))
  {
    FunctionSignature func = reinterpret_cast<FunctionSignature>(dlsym(gGraphicsLibraryHandle->mHandle,
                                                                       functionName));
    if(!func)
    {
      DALI_LOG_ERROR("Cannot find %s function: %s\n", functionName, dlerror());
      DALI_ABORT("Unable to find required function in dynamically loaded library\n");
    }
    return func;
  }
  return nullptr;
}

template<typename FunctionSignature, typename... Args>
void CallVoidFunction(const char* const functionName, bool reloadFunction, Args... args)
{
  static FunctionSignature func = GetFunction<FunctionSignature>(functionName);
  if(DALI_UNLIKELY(reloadFunction))
  {
    func = GetFunction<FunctionSignature>(functionName);
  }
  else
  {
    if(DALI_LIKELY(func))
    {
      func(args...);
    }
  }
}

template<typename FunctionSignature, typename ReturnValue, typename... Args>
ReturnValue CallReturnValueFunction(const char* const functionName, bool reloadFunction, Args... args)
{
  static FunctionSignature func = GetFunction<FunctionSignature>(functionName);
  if(DALI_UNLIKELY(reloadFunction))
  {
    func = GetFunction<FunctionSignature>(functionName);
  }
  else
  {
    if(DALI_LIKELY(func))
    {
      return func(args...);
    }
  }
  return {};
}
} // unnamed namespace

Dali::Graphics::Backend GetCurrentGraphicsLibraryBackend()
{
  if(DALI_LIKELY(gGraphicsLibraryHandle))
  {
    return gGraphicsLibraryHandle->mBackend;
  }
  return Graphics::Backend::DEFAULT;
}

void ResetGraphicsLibrary(bool reload)
{
  gGraphicsLibraryHandle.reset();
  if(reload)
  {
    class DummyWindow : public Graphics::NativeWindowInterface
    {
    public:
      Any GetNativeWindow() override
      {
        return {};
      }

      int GetNativeWindowId() override
      {
        return 0;
      }
    };
    DummyWindow        dummyWindow;
    EnvironmentOptions dummyEnvironmentOptions;

    CallReturnValueFunction<GraphicsFactoryInterface* (*)(EnvironmentOptions&), GraphicsFactoryInterface*, EnvironmentOptions&>("CreateGraphicsFactory", true, dummyEnvironmentOptions);
    CallReturnValueFunction<RenderSurfaceFactory* (*)(), RenderSurfaceFactory*>("GetRenderSurfaceFactory", true);
    CallReturnValueFunction<NativeImageSourceFactory* (*)(), NativeImageSourceFactory*>("GetNativeImageSourceFactory", true);
    CallReturnValueFunction<Graphics::SurfaceFactory* (*)(Graphics::NativeWindowInterface&), Graphics::SurfaceFactory*, Graphics::NativeWindowInterface&>("CreateSurfaceFactory", true, dummyWindow);
    CallReturnValueFunction<NativeImageSurface* (*)(NativeImageSourceQueuePtr), NativeImageSurface*, NativeImageSourceQueuePtr>("CreateNativeImageSurface", true, NativeImageSourceQueuePtr());
    CallReturnValueFunction<Any (*)(void*), Any, void*>("CastToNativeGraphicsType", true, nullptr);
  }

  DALI_LOG_DEBUG_INFO("Reset graphics backend library done (reload : %d)\n", reload);
}

GraphicsLibraryHandlePtr GetGraphicsLibraryHandle()
{
  return std::static_pointer_cast<Dali::Internal::Adaptor::GraphicsLibraryHandleBase>(gGraphicsLibraryHandle);
}

std::unique_ptr<GraphicsFactoryInterface> CreateGraphicsFactory(EnvironmentOptions& environmentOptions)
{
  return std::unique_ptr<GraphicsFactoryInterface>(CallReturnValueFunction<GraphicsFactoryInterface* (*)(EnvironmentOptions&), GraphicsFactoryInterface*, EnvironmentOptions&>("CreateGraphicsFactory", false, environmentOptions));
}

std::unique_ptr<RenderSurfaceFactory> GetRenderSurfaceFactory()
{
  return std::unique_ptr<RenderSurfaceFactory>(CallReturnValueFunction<RenderSurfaceFactory* (*)(), RenderSurfaceFactory*>("GetRenderSurfaceFactory", false));
}

std::unique_ptr<NativeImageSourceFactory> GetNativeImageSourceFactory()
{
  return std::unique_ptr<NativeImageSourceFactory>(CallReturnValueFunction<NativeImageSourceFactory* (*)(), NativeImageSourceFactory*>("GetNativeImageSourceFactory", false));
}

std::unique_ptr<Graphics::SurfaceFactory> CreateSurfaceFactory(Graphics::NativeWindowInterface& nativeWindow)
{
  return std::unique_ptr<Graphics::SurfaceFactory>(CallReturnValueFunction<Graphics::SurfaceFactory* (*)(Graphics::NativeWindowInterface&), Graphics::SurfaceFactory*, Graphics::NativeWindowInterface&>("CreateSurfaceFactory", false, nativeWindow));
}

std::unique_ptr<NativeImageSurface> CreateNativeImageSurface(NativeImageSourceQueuePtr queue)
{
  return std::unique_ptr<NativeImageSurface>(CallReturnValueFunction<NativeImageSurface* (*)(NativeImageSourceQueuePtr), NativeImageSurface*, NativeImageSourceQueuePtr>("CreateNativeImageSurface", false, queue));
}

Any CastToNativeGraphicsType(void* display)
{
  return CallReturnValueFunction<Any (*)(void*), Any, void*>("CastToNativeGraphicsType", false, display);
}

} // namespace Dali::Internal::Adaptor::GraphicsLibrary
