/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/text-abstraction/emoji-helper.h>

namespace Dali
{
namespace TextAbstraction
{
DALI_ADAPTOR_API bool IsSymbolScript(const TextAbstraction::Script& script)
{
  return (TextAbstraction::SYMBOLS1 == script ||
          TextAbstraction::SYMBOLS2 == script ||
          TextAbstraction::SYMBOLS3 == script ||
          TextAbstraction::SYMBOLS4 == script ||
          TextAbstraction::SYMBOLS5 == script ||
          TextAbstraction::SYMBOLS_NSLCL == script);
}

DALI_ADAPTOR_API bool IsEmojiScript(const TextAbstraction::Script& script)
{
  return (TextAbstraction::EMOJI == script);
}

DALI_ADAPTOR_API bool IsEmojiTextScript(const TextAbstraction::Script& script)
{
  return (TextAbstraction::EMOJI_TEXT == script);
}

DALI_ADAPTOR_API bool IsEmojiColorScript(const TextAbstraction::Script& script)
{
  return (TextAbstraction::EMOJI_COLOR == script);
}

DALI_ADAPTOR_API bool IsOneOfEmojiScripts(const TextAbstraction::Script& script)
{
  return (IsEmojiScript(script) || IsEmojiColorScript(script) || IsEmojiTextScript(script));
}

DALI_ADAPTOR_API bool IsSymbolOrEmojiScript(const TextAbstraction::Script& script)
{
  return (IsEmojiScript(script) || IsSymbolScript(script));
}

DALI_ADAPTOR_API bool IsSymbolOrEmojiOrTextScript(const TextAbstraction::Script& script)
{
  return (IsSymbolScript(script) || IsOneOfEmojiScripts(script));
}

DALI_ADAPTOR_API Character GetUnicodeForInvisibleGlyph()
{
  return TextAbstraction::CHAR_ZWS;
}

} // namespace TextAbstraction

} // namespace Dali