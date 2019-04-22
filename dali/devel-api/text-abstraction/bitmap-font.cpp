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

// FILE HEADER
#include <dali/devel-api/text-abstraction/bitmap-font.h>

namespace Dali
{

namespace TextAbstraction
{

BitmapGlyph::BitmapGlyph()
: url{},
  utf32{ 0u },
  ascender{ 0.f },
  descender{ 0.f }
{}

BitmapGlyph::BitmapGlyph( const std::string& url, GlyphIndex utf32, float ascender, float descender )
: url{ url },
  utf32{ utf32 },
  ascender{ ascender },
  descender{ descender }
{}

BitmapGlyph::~BitmapGlyph()
{}

BitmapFont::BitmapFont()
: glyphs{},
  name{},
  ascender{ 0.f },
  descender{ 0.f },
  underlinePosition{ 0.f },
  underlineThickness{ 1.f },
  isColorFont{ false }
{}

BitmapFont::~BitmapFont()
{}

} // namespace TextAbstraction

} // namespace Dali

