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

// CLASS HEADER
#include <dali/devel-api/text-abstraction/glyph-info.h>

namespace Dali
{

namespace TextAbstraction
{

GlyphInfo::GlyphInfo()
: fontId{ 0u },
  index{ 0u },
  width( 0.f ),
  height{ 0.f },
  xBearing{ 0.f },
  yBearing{ 0.f },
  advance{ 0.f },
  scaleFactor{ 0.f },
  isItalicRequired{ false },
  isBoldRequired{ false }
{
}

GlyphInfo::GlyphInfo( FontId font, GlyphIndex i )
: fontId{ font },
  index{ i },
  width( 0.f ),
  height{ 0.f },
  xBearing{ 0.f },
  yBearing{ 0.f },
  advance{ 0.f },
  scaleFactor{ 0.f },
  isItalicRequired{ false },
  isBoldRequired{ false }
{
}

} // namespace TextAbstraction

} // namespace Dali
