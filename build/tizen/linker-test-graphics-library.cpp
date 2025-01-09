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

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-library.h>
#include <dali/internal/window-system/common/native-image-surface-impl.h>

// Dummy Implementation for NativeWindowInterface
class NativeWindowDummyImpl : public Dali::Graphics::NativeWindowInterface
{
public:
  Dali::Any GetNativeWindow() override
  {
    return {};
  }
  int GetNativeWindowId() override
  {
    return 0;
  }
};

extern "C"
{
  extern std::unique_ptr<Dali::Internal::Adaptor::GraphicsFactoryInterface> CreateGraphicsFactory(Dali::Internal::Adaptor::EnvironmentOptions& environmentOptions);
  extern std::unique_ptr<Dali::Internal::Adaptor::RenderSurfaceFactory>     GetRenderSurfaceFactory();
  extern std::unique_ptr<Dali::Internal::Adaptor::NativeImageSourceFactory> GetNativeImageSourceFactory();
  extern std::unique_ptr<Dali::Internal::Adaptor::NativeImageSurface>       CreateNativeImageSurface(Dali::NativeImageSourceQueuePtr queue);
  extern std::unique_ptr<Dali::Graphics::SurfaceFactory>                    CreateSurfaceFactory(Dali::Graphics::NativeWindowInterface& nativeWindow);
} // extern "C"

int main(int argc, char** argv)
{
  Dali::Internal::Adaptor::EnvironmentOptions options;
  CreateGraphicsFactory(options);

  [[maybe_unused]] auto renderSurfaceFactory = GetRenderSurfaceFactory();
  [[maybe_unused]] auto imageSourceFactory   = GetNativeImageSourceFactory();
  [[maybe_unused]] auto nativeImageSurface   = CreateNativeImageSurface(Dali::NativeImageSourceQueuePtr());

  NativeWindowDummyImpl nativeWindow;
  [[maybe_unused]] auto surfaceFactory = CreateSurfaceFactory(nativeWindow);

  return 0;
}
