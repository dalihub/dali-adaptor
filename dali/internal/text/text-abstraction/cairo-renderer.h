#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_CAIRO_RENDERER_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_CAIRO_RENDERER_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/text-renderer.h>

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{
/**
 * @brief Cairo implementation of the Dali::TextAbstraction::TextRenderer interface.
 *
 * @see Dali::TextAbstraction::TextRenderer.
 */
Devel::PixelBuffer RenderTextCairo( const TextAbstraction::TextRenderer::Parameters& parameters );

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_INTERNAL_TEXT_ABSTRACTION_CAIRO_RENDERER_H

