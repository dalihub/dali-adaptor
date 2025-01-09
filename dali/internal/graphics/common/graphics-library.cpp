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

// HEADER
#include <dali/internal/graphics/common/graphics-library.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dlfcn.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/native-image-surface-impl.h>
#include <dali/public-api/adaptor-framework/graphics-backend.h>

namespace Dali::Internal::Adaptor::GraphicsLibrary
{
namespace
{
const char* const DALI_ADAPTOR_GRAPHICS_GLES_SO("libdali2-adaptor-gles.so");
const char* const DALI_ADAPTOR_GRAPHICS_VULKAN_SO("libdali2-adaptor-vulkan.so");

struct GraphicsLibrary
{
public:
  GraphicsLibrary()
  {
    mHandle = dlopen((Graphics::GetCurrentGraphicsBackend() == Graphics::Backend::GLES) ? DALI_ADAPTOR_GRAPHICS_GLES_SO : DALI_ADAPTOR_GRAPHICS_VULKAN_SO,
                     RTLD_NOW | RTLD_GLOBAL | RTLD_DEEPBIND);
    if(!mHandle)
    {
      // The shared library failed to load
      DALI_LOG_ERROR("dlopen error: %s\n", dlerror());
      DALI_ABORT("Unable to open Graphics Library\n");
    }
  }

  ~GraphicsLibrary()
  {
    if(mHandle)
    {
      dlclose(mHandle);
    }
  }

public:
  void* mHandle{nullptr};
};
std::unique_ptr<GraphicsLibrary> gGraphicsLibraryHandle;

template<typename FunctionSignature>
FunctionSignature GetFunction(const char* const functionName)
{
  if(!gGraphicsLibraryHandle)
  {
    gGraphicsLibraryHandle.reset(new GraphicsLibrary);
  }

  if(gGraphicsLibraryHandle)
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
void CallVoidFunction(const char* const functionName, Args... args)
{
  static FunctionSignature func = GetFunction<FunctionSignature>(functionName);
  if(func)
  {
    func(args...);
  }
}

template<typename FunctionSignature, typename ReturnValue, typename... Args>
ReturnValue CallReturnValueFunction(const char* const functionName, Args... args)
{
  static FunctionSignature func = GetFunction<FunctionSignature>(functionName);
  if(func)
  {
    return func(args...);
  }
  return {};
}
} // unnamed namespace

std::unique_ptr<GraphicsFactoryInterface> CreateGraphicsFactory(EnvironmentOptions& environmentOptions)
{
  return CallReturnValueFunction<std::unique_ptr<GraphicsFactoryInterface> (*)(EnvironmentOptions&), std::unique_ptr<GraphicsFactoryInterface>, EnvironmentOptions&>("CreateGraphicsFactory", environmentOptions);
}

std::unique_ptr<RenderSurfaceFactory> GetRenderSurfaceFactory()
{
  return CallReturnValueFunction<std::unique_ptr<RenderSurfaceFactory> (*)(), std::unique_ptr<RenderSurfaceFactory>>("GetRenderSurfaceFactory");
}

std::unique_ptr<NativeImageSourceFactory> GetNativeImageSourceFactory()
{
  return CallReturnValueFunction<std::unique_ptr<NativeImageSourceFactory> (*)(), std::unique_ptr<NativeImageSourceFactory>>("GetNativeImageSourceFactory");
}

std::unique_ptr<Graphics::SurfaceFactory> CreateSurfaceFactory(Graphics::NativeWindowInterface& nativeWindow)
{
  return CallReturnValueFunction<std::unique_ptr<Graphics::SurfaceFactory> (*)(Graphics::NativeWindowInterface&), std::unique_ptr<Graphics::SurfaceFactory>, Graphics::NativeWindowInterface&>("CreateSurfaceFactory", nativeWindow);
}

std::unique_ptr<NativeImageSurface> CreateNativeImageSurface(NativeImageSourceQueuePtr queue)
{
  return CallReturnValueFunction<std::unique_ptr<NativeImageSurface> (*)(NativeImageSourceQueuePtr), std::unique_ptr<NativeImageSurface>, NativeImageSourceQueuePtr>("CreateNativeImageSurface", queue);
}

Any CastToNativeGraphicsType(void* display)
{
  return CallReturnValueFunction<Any (*)(void*), Any, void*>("CastToNativeGraphicsType", display);
}

} // namespace Dali::Internal::Adaptor::GraphicsLibrary
