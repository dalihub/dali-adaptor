#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_TEXT_RENDERER_IMPL_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_TEXT_RENDERER_IMPL_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/text-renderer.h>

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

/**
 * Implementation of the TextRenderer
 */
class TextRenderer : public BaseObject
{
public:

  /**
   * Constructor
   */
  TextRenderer();

  /**
   * Destructor
   */
  ~TextRenderer();

  /**
   * @copydoc Dali::TextRenderer::Get()
   */
  static TextAbstraction::TextRenderer Get();

  /**
   * @copydoc Dali::TextRenderer::Render()
   */
  Devel::PixelBuffer Render(const TextAbstraction::TextRenderer::Parameters& parameters);

private:

  // Undefined copy constructor.
  TextRenderer(const TextRenderer&);

  // Undefined assignment constructor.
  TextRenderer& operator=(const TextRenderer&);

}; // class TextRenderer

} // namespace Internal

} // namespace TextAbstraction

inline static TextAbstraction::Internal::TextRenderer& GetImplementation(TextAbstraction::TextRenderer& textRenderer)
{
  DALI_ASSERT_ALWAYS(textRenderer && "textRenderer handle is empty");
  BaseObject& handle = textRenderer.GetBaseObject();
  return static_cast<TextAbstraction::Internal::TextRenderer&>(handle);
}

inline static const TextAbstraction::Internal::TextRenderer& GetImplementation(const TextAbstraction::TextRenderer& textRenderer)
{
  DALI_ASSERT_ALWAYS(textRenderer && "textRenderer handle is empty");
  const BaseObject& handle = textRenderer.GetBaseObject();
  return static_cast<const TextAbstraction::Internal::TextRenderer&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_TEXT_ABSTRACTION_TEXT_RENDERER_IMPL_H
