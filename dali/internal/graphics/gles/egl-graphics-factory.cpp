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
#include <dali/internal/graphics/gles/egl-graphics-factory.h>
#include <dali/internal/graphics/gles/egl-graphics.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-factory.h>
#include <dali/internal/window-system/common/display-utils.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
EglGraphicsFactory::EglGraphicsFactory(EnvironmentOptions& environmentOptions)
: mEnvironmentOptions(environmentOptions)
{
}

EglGraphicsFactory::~EglGraphicsFactory()
{
  /* Deleted by Adaptor destructor */
}

Graphics::GraphicsInterface& EglGraphicsFactory::Create()
{
  Graphics::GraphicsCreateInfo info{};

  auto depthBufferRequired   = (mEnvironmentOptions.DepthBufferRequired() ? Integration::DepthBufferAvailable::TRUE : Integration::DepthBufferAvailable::FALSE);
  auto stencilBufferRequired = (mEnvironmentOptions.StencilBufferRequired() ? Integration::StencilBufferAvailable::TRUE : Integration::StencilBufferAvailable::FALSE);
  auto partialUpdateRequired = (mEnvironmentOptions.PartialUpdateRequired() ? Integration::PartialUpdateAvailable::TRUE : Integration::PartialUpdateAvailable::FALSE);

  int multiSamplingLevel = mEnvironmentOptions.GetMultiSamplingLevel();

  Graphics::GraphicsInterface* eglGraphicsInterface = new EglGraphics(mEnvironmentOptions, info, depthBufferRequired, stencilBufferRequired, partialUpdateRequired, multiSamplingLevel);
  return *eglGraphicsInterface;
}

void EglGraphicsFactory::Destroy()
{
  /* Deleted by EglGraphics */
}

/// graphics-factory.h implemements
Dali::Graphics::Backend GetCurrentGraphicsLibraryBackend()
{
  return Dali::Graphics::Backend::GLES;
}

void ResetGraphicsLibrary()
{
  /* This function defined for dynamic library case. */
}

std::unique_ptr<GraphicsFactoryInterface> CreateGraphicsFactory(EnvironmentOptions& environmentOptions)
{
  DALI_LOG_RELEASE_INFO("DALi Graphics Backend: GLES\n");
  return Utils::MakeUnique<EglGraphicsFactory>(environmentOptions);
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
