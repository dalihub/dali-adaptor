/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/text/text-abstraction/text-renderer-impl.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/singleton-service-impl.h>
#include <dali/internal/text/text-abstraction/cairo-renderer.h>

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

TextRenderer::TextRenderer()
{
}

TextRenderer::~TextRenderer()
{
}

TextAbstraction::TextRenderer TextRenderer::Get()
{
  TextAbstraction::TextRenderer shapingHandle;

  SingletonService service(SingletonService::Get());
  if (service)
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(TextAbstraction::TextRenderer));
    if (handle)
    {
      // If so, downcast the handle
      TextRenderer* impl = dynamic_cast< Internal::TextRenderer* >(handle.GetObjectPtr());
      shapingHandle = TextAbstraction::TextRenderer(impl);
    }
    else // create and register the object
    {
      shapingHandle = TextAbstraction::TextRenderer(new TextRenderer);
      service.Register(typeid(shapingHandle), shapingHandle);
    }
  }

  return shapingHandle;
}

Devel::PixelBuffer TextRenderer::Render(const TextAbstraction::TextRenderer::Parameters& parameters)
{
  return RenderTextCairo(parameters);
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
