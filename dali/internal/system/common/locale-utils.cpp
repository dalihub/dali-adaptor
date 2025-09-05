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

// CLASS HEADER
#include <dali/internal/system/common/locale-utils.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace Locale
{
namespace
{
struct LocaleDirectionInfo
{
  const char*       locale;
  const char*       name;
  Locale::Direction direction;
};

// clang-format off
const LocaleDirectionInfo LOCALE_DIRECTION_LOOKUP_TABLE[] =
{
  { "af", "Afrikaans",          Locale::LEFT_TO_RIGHT },
  { "am", "Amharic",            Locale::LEFT_TO_RIGHT },
  { "ar", "Arabic",             Locale::RIGHT_TO_LEFT },
  { "as", "Assamese",           Locale::LEFT_TO_RIGHT },
  { "az", "Azerbaijani",        Locale::LEFT_TO_RIGHT },
  { "be", "Belarusian",         Locale::LEFT_TO_RIGHT },
  { "bg", "Bulgarian",          Locale::LEFT_TO_RIGHT },
  { "bn", "Bengali",            Locale::LEFT_TO_RIGHT },
  { "bo", "Tibetan",            Locale::LEFT_TO_RIGHT },
  { "bs", "Bosnian",            Locale::LEFT_TO_RIGHT },
  { "ca", "Catalan",            Locale::LEFT_TO_RIGHT },
  { "ck", "Iraq",               Locale::RIGHT_TO_LEFT },
  { "cs", "Czech",              Locale::LEFT_TO_RIGHT },
  { "cy", "Welsh",              Locale::LEFT_TO_RIGHT },
  { "da", "Danish",             Locale::LEFT_TO_RIGHT },
  { "de", "German",             Locale::LEFT_TO_RIGHT },
  { "dv", "Divehi",             Locale::RIGHT_TO_LEFT },
  { "el", "Greek",              Locale::LEFT_TO_RIGHT },
  { "en", "English",            Locale::LEFT_TO_RIGHT },
  { "es", "Spanish",            Locale::LEFT_TO_RIGHT },
  { "et", "Estonian",           Locale::LEFT_TO_RIGHT },
  { "eu", "Basque",             Locale::LEFT_TO_RIGHT },
  { "fa", "Farsi",              Locale::RIGHT_TO_LEFT },
  { "fi", "Finnish",            Locale::LEFT_TO_RIGHT },
  { "fo", "Faroese",            Locale::LEFT_TO_RIGHT },
  { "fr", "French",             Locale::LEFT_TO_RIGHT },
  { "gd", "Gaelic",             Locale::LEFT_TO_RIGHT },
  { "gl", "Galician",           Locale::LEFT_TO_RIGHT },
  { "gn", "Guarani",            Locale::LEFT_TO_RIGHT },
  { "gu", "Gujarati",           Locale::LEFT_TO_RIGHT },
  { "ha", "Hausa",              Locale::LEFT_TO_RIGHT },
  { "he", "Hebrew",             Locale::RIGHT_TO_LEFT },
  { "hi", "Hindi",              Locale::LEFT_TO_RIGHT },
  { "hr", "Croatian",           Locale::LEFT_TO_RIGHT },
  { "hu", "Hungarian",          Locale::LEFT_TO_RIGHT },
  { "hy", "Armenian",           Locale::LEFT_TO_RIGHT },
  { "id", "Indonesian",         Locale::LEFT_TO_RIGHT },
  { "is", "Icelandic",          Locale::LEFT_TO_RIGHT },
  { "it", "Italian",            Locale::LEFT_TO_RIGHT },
  { "ja", "Japanese",           Locale::LEFT_TO_RIGHT },
  { "ka", "Georgian",           Locale::LEFT_TO_RIGHT },
  { "kk", "Kazakh",             Locale::LEFT_TO_RIGHT },
  { "km", "Khmer",              Locale::LEFT_TO_RIGHT },
  { "kn", "Kannada",            Locale::LEFT_TO_RIGHT },
  { "ko", "Korean",             Locale::LEFT_TO_RIGHT },
  { "ks", "Kashmiri",           Locale::LEFT_TO_RIGHT },
  { "la", "Latin",              Locale::LEFT_TO_RIGHT },
  { "lo", "Lao",                Locale::LEFT_TO_RIGHT },
  { "lt", "Lithuanian",         Locale::LEFT_TO_RIGHT },
  { "lv", "Latvian",            Locale::LEFT_TO_RIGHT },
  { "mi", "Maori",              Locale::LEFT_TO_RIGHT },
  { "mk", "FYRO Macedonia",     Locale::LEFT_TO_RIGHT },
  { "ml", "Malayalam",          Locale::LEFT_TO_RIGHT },
  { "mn", "Mongolian",          Locale::LEFT_TO_RIGHT },
  { "mr", "Marathi",            Locale::LEFT_TO_RIGHT },
  { "ms", "Malay",              Locale::LEFT_TO_RIGHT },
  { "mt", "Maltese",            Locale::LEFT_TO_RIGHT },
  { "my", "Burmese",            Locale::LEFT_TO_RIGHT },
  { "nb", "Norwegian: Bokml",   Locale::LEFT_TO_RIGHT },
  { "ne", "Nepali",             Locale::LEFT_TO_RIGHT },
  { "nl", "Dutch",              Locale::LEFT_TO_RIGHT },
  { "nn", "Norwegian: Nynorsk", Locale::LEFT_TO_RIGHT },
  { "or", "Oriya",              Locale::LEFT_TO_RIGHT },
  { "pa", "Punjabi",            Locale::LEFT_TO_RIGHT },
  { "pl", "Polish",             Locale::LEFT_TO_RIGHT },
  { "pt", "Portuguese",         Locale::LEFT_TO_RIGHT },
  { "rm", "Raeto-Romance",      Locale::LEFT_TO_RIGHT },
  { "ro", "Romanian",           Locale::LEFT_TO_RIGHT },
  { "ru", "Russian",            Locale::LEFT_TO_RIGHT },
  { "sa", "Sanskrit",           Locale::LEFT_TO_RIGHT },
  { "sb", "Sorbian",            Locale::LEFT_TO_RIGHT },
  { "sd", "Sindhi",             Locale::LEFT_TO_RIGHT },
  { "si", "Sinhala",            Locale::LEFT_TO_RIGHT },
  { "sk", "Slovak",             Locale::LEFT_TO_RIGHT },
  { "sl", "Slovenian",          Locale::LEFT_TO_RIGHT },
  { "so", "Somali",             Locale::LEFT_TO_RIGHT },
  { "sq", "Albanian",           Locale::LEFT_TO_RIGHT },
  { "sr", "Serbian",            Locale::LEFT_TO_RIGHT },
  { "sv", "Swedish",            Locale::LEFT_TO_RIGHT },
  { "sw", "Swahili",            Locale::LEFT_TO_RIGHT },
  { "ta", "Tamil",              Locale::LEFT_TO_RIGHT },
  { "te", "Telugu",             Locale::LEFT_TO_RIGHT },
  { "tg", "Tajik",              Locale::LEFT_TO_RIGHT },
  { "th", "Thai",               Locale::LEFT_TO_RIGHT },
  { "tk", "Turkmen",            Locale::LEFT_TO_RIGHT },
  { "tn", "Setsuana",           Locale::LEFT_TO_RIGHT },
  { "tr", "Turkish",            Locale::LEFT_TO_RIGHT },
  { "ts", "Tsonga",             Locale::LEFT_TO_RIGHT },
  { "tt", "Tatar",              Locale::LEFT_TO_RIGHT },
  { "uk", "Ukrainian",          Locale::LEFT_TO_RIGHT },
  { "ur", "Urdu",               Locale::RIGHT_TO_LEFT },
  { "uz", "Uzbek",              Locale::LEFT_TO_RIGHT },
  { "vi", "Vietnamese",         Locale::LEFT_TO_RIGHT },
  { "xh", "Xhosa",              Locale::LEFT_TO_RIGHT },
  { "yi", "Yiddish",            Locale::RIGHT_TO_LEFT },
  { "zh", "Chinese",            Locale::LEFT_TO_RIGHT },
  { "zu", "Zulu",               Locale::LEFT_TO_RIGHT },

  { NULL, NULL, Locale::LEFT_TO_RIGHT }
};
// clang-format on

} // unnamed namespace

Locale::Direction GetDirection(const std::string& locale)
{
  Locale::Direction direction(Locale::LEFT_TO_RIGHT);

  if(!locale.empty() && locale.size() > 2)
  {
    for(const LocaleDirectionInfo* iter = &LOCALE_DIRECTION_LOOKUP_TABLE[0]; iter->locale; ++iter)
    {
      if(!locale.compare(0, 2, iter->locale))
      {
        direction = iter->direction;
        break;
      }
    }
  }

  return direction;
}

} // namespace Locale

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
