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
#include "locale-utils.h"

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

struct LocaleDirection
{
  const char * locale;
  const char * name;
  Dali::ImfManager::TextDirection direction;
};

const LocaleDirection LOCALE_DIRECTION_LOOKUP_TABLE[] =
{
  { "af", "Afrikaans",          Dali::ImfManager::LeftToRight },
  { "am", "Amharic",            Dali::ImfManager::LeftToRight },
  { "ar", "Arabic",             Dali::ImfManager::RightToLeft },
  { "as", "Assamese",           Dali::ImfManager::LeftToRight },
  { "az", "Azeri",              Dali::ImfManager::LeftToRight },
  { "be", "Belarusian",         Dali::ImfManager::LeftToRight },
  { "bg", "Bulgarian",          Dali::ImfManager::LeftToRight },
  { "bn", "Bengali",            Dali::ImfManager::LeftToRight },
  { "bo", "Tibetan",            Dali::ImfManager::LeftToRight },
  { "bs", "Bosnian",            Dali::ImfManager::LeftToRight },
  { "ca", "Catalan",            Dali::ImfManager::LeftToRight },
  { "cs", "Czech",              Dali::ImfManager::LeftToRight },
  { "cy", "Welsh",              Dali::ImfManager::LeftToRight },
  { "da", "Danish",             Dali::ImfManager::LeftToRight },
  { "de", "German",             Dali::ImfManager::LeftToRight },
  { "dv", "Divehi",             Dali::ImfManager::RightToLeft },
  { "el", "Greek",              Dali::ImfManager::LeftToRight },
  { "en", "English",            Dali::ImfManager::LeftToRight },
  { "es", "Spanish",            Dali::ImfManager::LeftToRight },
  { "et", "Estonian",           Dali::ImfManager::LeftToRight },
  { "eu", "Basque",             Dali::ImfManager::LeftToRight },
  { "fa", "Farsi",              Dali::ImfManager::RightToLeft },
  { "fi", "Finnish",            Dali::ImfManager::LeftToRight },
  { "fo", "Faroese",            Dali::ImfManager::LeftToRight },
  { "fr", "French",             Dali::ImfManager::LeftToRight },
  { "gd", "Gaelic",             Dali::ImfManager::LeftToRight },
  { "gl", "Galician",           Dali::ImfManager::LeftToRight },
  { "gn", "Guarani",            Dali::ImfManager::LeftToRight },
  { "gu", "Gujarati",           Dali::ImfManager::LeftToRight },
  { "he", "Hebrew",             Dali::ImfManager::RightToLeft },
  { "hi", "Hindi",              Dali::ImfManager::LeftToRight },
  { "hr", "Croatian",           Dali::ImfManager::LeftToRight },
  { "hu", "Hungarian",          Dali::ImfManager::LeftToRight },
  { "hy", "Armenian",           Dali::ImfManager::LeftToRight },
  { "id", "Indonesian",         Dali::ImfManager::LeftToRight },
  { "is", "Icelandic",          Dali::ImfManager::LeftToRight },
  { "it", "Italian",            Dali::ImfManager::LeftToRight },
  { "ja", "Japanese",           Dali::ImfManager::LeftToRight },
  { "ka", "Georgian",           Dali::ImfManager::LeftToRight },
  { "kk", "Kazakh",             Dali::ImfManager::RightToLeft },
  { "km", "Khmer",              Dali::ImfManager::LeftToRight },
  { "kn", "Kannada",            Dali::ImfManager::LeftToRight },
  { "ko", "Korean",             Dali::ImfManager::LeftToRight },
  { "ks", "Kashmiri",           Dali::ImfManager::RightToLeft },
  { "la", "Latin",              Dali::ImfManager::LeftToRight },
  { "lo", "Lao",                Dali::ImfManager::LeftToRight },
  { "lt", "Lithuanian",         Dali::ImfManager::LeftToRight },
  { "lv", "Latvian",            Dali::ImfManager::LeftToRight },
  { "mi", "Maori",              Dali::ImfManager::LeftToRight },
  { "mk", "FYRO Macedonia",     Dali::ImfManager::LeftToRight },
  { "ml", "Malayalam",          Dali::ImfManager::LeftToRight },
  { "mn", "Mongolian",          Dali::ImfManager::LeftToRight },
  { "mr", "Marathi",            Dali::ImfManager::LeftToRight },
  { "ms", "Malay",              Dali::ImfManager::LeftToRight },
  { "mt", "Maltese",            Dali::ImfManager::LeftToRight },
  { "my", "Burmese",            Dali::ImfManager::LeftToRight },
  { "nb", "Norwegian: Bokml",   Dali::ImfManager::LeftToRight },
  { "ne", "Nepali",             Dali::ImfManager::LeftToRight },
  { "nl", "Dutch",              Dali::ImfManager::LeftToRight },
  { "nn", "Norwegian: Nynorsk", Dali::ImfManager::LeftToRight },
  { "or", "Oriya",              Dali::ImfManager::LeftToRight },
  { "pa", "Punjabi",            Dali::ImfManager::LeftToRight },
  { "pl", "Polish",             Dali::ImfManager::LeftToRight },
  { "pt", "Portuguese",         Dali::ImfManager::LeftToRight },
  { "rm", "Raeto-Romance",      Dali::ImfManager::LeftToRight },
  { "ro", "Romanian",           Dali::ImfManager::LeftToRight },
  { "ru", "Russian",            Dali::ImfManager::LeftToRight },
  { "sa", "Sanskrit",           Dali::ImfManager::LeftToRight },
  { "sb", "Sorbian",            Dali::ImfManager::LeftToRight },
  { "sd", "Sindhi",             Dali::ImfManager::LeftToRight },
  { "si", "Sinhala",            Dali::ImfManager::LeftToRight },
  { "sk", "Slovak",             Dali::ImfManager::LeftToRight },
  { "sl", "Slovenian",          Dali::ImfManager::LeftToRight },
  { "so", "Somali",             Dali::ImfManager::LeftToRight },
  { "sq", "Albanian",           Dali::ImfManager::LeftToRight },
  { "sr", "Serbian",            Dali::ImfManager::LeftToRight },
  { "sv", "Swedish",            Dali::ImfManager::LeftToRight },
  { "sw", "Swahili",            Dali::ImfManager::LeftToRight },
  { "ta", "Tamil",              Dali::ImfManager::LeftToRight },
  { "te", "Telugu",             Dali::ImfManager::LeftToRight },
  { "tg", "Tajik",              Dali::ImfManager::RightToLeft },
  { "th", "Thai",               Dali::ImfManager::LeftToRight },
  { "tk", "Turkmen",            Dali::ImfManager::LeftToRight },
  { "tn", "Setsuana",           Dali::ImfManager::LeftToRight },
  { "tr", "Turkish",            Dali::ImfManager::LeftToRight },
  { "ts", "Tsonga",             Dali::ImfManager::LeftToRight },
  { "tt", "Tatar",              Dali::ImfManager::LeftToRight },
  { "uk", "Ukrainian",          Dali::ImfManager::LeftToRight },
  { "ur", "Urdu",               Dali::ImfManager::RightToLeft },
  { "uz", "Uzbek",              Dali::ImfManager::LeftToRight },
  { "vi", "Vietnamese",         Dali::ImfManager::LeftToRight },
  { "xh", "Xhosa",              Dali::ImfManager::LeftToRight },
  { "yi", "Yiddish",            Dali::ImfManager::RightToLeft },
  { "zh", "Chinese",            Dali::ImfManager::LeftToRight },
  { "zu", "Zulu",               Dali::ImfManager::LeftToRight },

  { NULL, NULL, Dali::ImfManager::LeftToRight }
};

} // unnamed namespace

Dali::ImfManager::TextDirection GetTextDirection( std::string locale )
{
  Dali::ImfManager::TextDirection direction( Dali::ImfManager::LeftToRight );

  if ( !locale.empty() && locale.size() > 2 )
  {
    // We're only interested in the first two characters
    locale.resize(2);

    for ( const LocaleDirection* iter = &LOCALE_DIRECTION_LOOKUP_TABLE[0]; iter->locale; ++iter )
    {
      if ( !locale.compare( iter->locale ) )
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
