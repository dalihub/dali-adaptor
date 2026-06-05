#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_OUTLINE_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_OUTLINE_H

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
 */

#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-rasterizer.h>

#if DALI_ENABLE_COLR_V1_RENDERER
#include <ft2build.h>
#include FT_FREETYPE_H
#include <thorvg.h>
#endif

namespace Dali::TextAbstraction::Internal
{

#if DALI_ENABLE_COLR_V1_RENDERER

/**
 * @brief Converts a FreeType outline to a ThorVG shape path.
 *
 * The outline must already be loaded in the coordinate space expected by the
 * caller. This helper only transfers outline contours into the shape path.
 *
 * @param[in] outline The FreeType outline.
 * @param[in,out] shape The ThorVG shape receiving the path.
 * @return @e true if all contours are converted.
 */
bool FtOutlineToTvgShape(FT_Outline* outline, tvg::Shape* shape);

#endif // DALI_ENABLE_COLR_V1_RENDERER

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_OUTLINE_H
