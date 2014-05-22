/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include "resource-loader-debug.h"

#if defined(DEBUG_ENABLED)

#include <iostream>
#include <sstream>
#include <string>
#include <dali/integration-api/glyph-set.h>

namespace Dali
{
namespace SlpPlatform
{
using namespace Dali::Integration;

/**
 * Filter for resource loader debug. Change levels here to turn on debugging
 */
Debug::Filter* gLoaderFilter = Debug::Filter::New(Debug::Concise, false, "LOG_RESOURCE_LOADER");


std::string DebugRequestList(TextResourceType::CharacterList& chars)
{
  std::string textString;

  for(std::size_t i=0, length=chars.size(); i<length; i++)
  {
    std::ostringstream oss;
    const TextResourceType::GlyphPosition& glyph = chars[i];
    oss << static_cast<char>(glyph.character) << "(" << glyph.xPosition << "," << glyph.yPosition
        << " " << (glyph.quality?"Hg":"Lw") << " " << (glyph.loaded?"LD":"UN") << ") ";
    textString.append(oss.str());
  }
  return textString;
}

std::string DebugResourceList(LoadedResource& partialResource)
{
  GlyphSetPointer glyphSet = static_cast<GlyphSet*>(partialResource.resource.Get());
  const GlyphSet::CharacterList& characterList = glyphSet->GetCharacterList();
  std::string textString;

  for(std::size_t i=0, length=characterList.size(); i<length; i++)
  {
    std::ostringstream oss;
    const GlyphMetrics& gm = characterList[i].second;
    oss << static_cast<char>(gm.code) << "(" << gm.xPosition << "," << gm.yPosition
        << " " << (gm.quality?"H":"L") << ") ";
    textString.append(oss.str());
  }
  return textString;
}

} //SlpPlatform
} //Dali

#endif
