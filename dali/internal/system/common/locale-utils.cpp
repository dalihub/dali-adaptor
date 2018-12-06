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
  const char * locale;
  const char * name;
  Locale::Direction direction;
};

const LocaleDirectionInfo LOCALE_DIRECTION_LOOKUP_TABLE[] =
{
  { "af", "Afrikaans",          Locale::LeftToRight },
  { "am", "Amharic",            Locale::LeftToRight },
  { "ar", "Arabic",             Locale::RightToLeft },
  { "as", "Assamese",           Locale::LeftToRight },
  { "az", "Azerbaijani",        Locale::LeftToRight },
  { "be", "Belarusian",         Locale::LeftToRight },
  { "bg", "Bulgarian",          Locale::LeftToRight },
  { "bn", "Bengali",            Locale::LeftToRight },
  { "bo", "Tibetan",            Locale::LeftToRight },
  { "bs", "Bosnian",            Locale::LeftToRight },
  { "ca", "Catalan",            Locale::LeftToRight },
  { "ck", "Iraq",               Locale::RightToLeft },
  { "cs", "Czech",              Locale::LeftToRight },
  { "cy", "Welsh",              Locale::LeftToRight },
  { "da", "Danish",             Locale::LeftToRight },
  { "de", "German",             Locale::LeftToRight },
  { "dv", "Divehi",             Locale::RightToLeft },
  { "el", "Greek",              Locale::LeftToRight },
  { "en", "English",            Locale::LeftToRight },
  { "es", "Spanish",            Locale::LeftToRight },
  { "et", "Estonian",           Locale::LeftToRight },
  { "eu", "Basque",             Locale::LeftToRight },
  { "fa", "Farsi",              Locale::RightToLeft },
  { "fi", "Finnish",            Locale::LeftToRight },
  { "fo", "Faroese",            Locale::LeftToRight },
  { "fr", "French",             Locale::LeftToRight },
  { "gd", "Gaelic",             Locale::LeftToRight },
  { "gl", "Galician",           Locale::LeftToRight },
  { "gn", "Guarani",            Locale::LeftToRight },
  { "gu", "Gujarati",           Locale::LeftToRight },
  { "ha", "Hausa",              Locale::LeftToRight },
  { "he", "Hebrew",             Locale::RightToLeft },
  { "hi", "Hindi",              Locale::LeftToRight },
  { "hr", "Croatian",           Locale::LeftToRight },
  { "hu", "Hungarian",          Locale::LeftToRight },
  { "hy", "Armenian",           Locale::LeftToRight },
  { "id", "Indonesian",         Locale::LeftToRight },
  { "is", "Icelandic",          Locale::LeftToRight },
  { "it", "Italian",            Locale::LeftToRight },
  { "ja", "Japanese",           Locale::LeftToRight },
  { "ka", "Georgian",           Locale::LeftToRight },
  { "kk", "Kazakh",             Locale::LeftToRight },
  { "km", "Khmer",              Locale::LeftToRight },
  { "kn", "Kannada",            Locale::LeftToRight },
  { "ko", "Korean",             Locale::LeftToRight },
  { "ks", "Kashmiri",           Locale::LeftToRight },
  { "la", "Latin",              Locale::LeftToRight },
  { "lo", "Lao",                Locale::LeftToRight },
  { "lt", "Lithuanian",         Locale::LeftToRight },
  { "lv", "Latvian",            Locale::LeftToRight },
  { "mi", "Maori",              Locale::LeftToRight },
  { "mk", "FYRO Macedonia",     Locale::LeftToRight },
  { "ml", "Malayalam",          Locale::LeftToRight },
  { "mn", "Mongolian",          Locale::LeftToRight },
  { "mr", "Marathi",            Locale::LeftToRight },
  { "ms", "Malay",              Locale::LeftToRight },
  { "mt", "Maltese",            Locale::LeftToRight },
  { "my", "Burmese",            Locale::LeftToRight },
  { "nb", "Norwegian: Bokml",   Locale::LeftToRight },
  { "ne", "Nepali",             Locale::LeftToRight },
  { "nl", "Dutch",              Locale::LeftToRight },
  { "nn", "Norwegian: Nynorsk", Locale::LeftToRight },
  { "or", "Oriya",              Locale::LeftToRight },
  { "pa", "Punjabi",            Locale::LeftToRight },
  { "pl", "Polish",             Locale::LeftToRight },
  { "pt", "Portuguese",         Locale::LeftToRight },
  { "rm", "Raeto-Romance",      Locale::LeftToRight },
  { "ro", "Romanian",           Locale::LeftToRight },
  { "ru", "Russian",            Locale::LeftToRight },
  { "sa", "Sanskrit",           Locale::LeftToRight },
  { "sb", "Sorbian",            Locale::LeftToRight },
  { "sd", "Sindhi",             Locale::LeftToRight },
  { "si", "Sinhala",            Locale::LeftToRight },
  { "sk", "Slovak",             Locale::LeftToRight },
  { "sl", "Slovenian",          Locale::LeftToRight },
  { "so", "Somali",             Locale::LeftToRight },
  { "sq", "Albanian",           Locale::LeftToRight },
  { "sr", "Serbian",            Locale::LeftToRight },
  { "sv", "Swedish",            Locale::LeftToRight },
  { "sw", "Swahili",            Locale::LeftToRight },
  { "ta", "Tamil",              Locale::LeftToRight },
  { "te", "Telugu",             Locale::LeftToRight },
  { "tg", "Tajik",              Locale::LeftToRight },
  { "th", "Thai",               Locale::LeftToRight },
  { "tk", "Turkmen",            Locale::LeftToRight },
  { "tn", "Setsuana",           Locale::LeftToRight },
  { "tr", "Turkish",            Locale::LeftToRight },
  { "ts", "Tsonga",             Locale::LeftToRight },
  { "tt", "Tatar",              Locale::LeftToRight },
  { "uk", "Ukrainian",          Locale::LeftToRight },
  { "ur", "Urdu",               Locale::RightToLeft },
  { "uz", "Uzbek",              Locale::LeftToRight },
  { "vi", "Vietnamese",         Locale::LeftToRight },
  { "xh", "Xhosa",              Locale::LeftToRight },
  { "yi", "Yiddish",            Locale::RightToLeft },
  { "zh", "Chinese",            Locale::LeftToRight },
  { "zu", "Zulu",               Locale::LeftToRight },

  { NULL, NULL, Locale::LeftToRight }
};

} // unnamed namespace

Locale::Direction GetDirection( const std::string& locale )
{
  Locale::Direction direction( Locale::LeftToRight );

  if ( !locale.empty() && locale.size() > 2 )
  {
    for ( const LocaleDirectionInfo* iter = &LOCALE_DIRECTION_LOOKUP_TABLE[0]; iter->locale; ++iter )
    {
      if ( !locale.compare( 0, 2, iter->locale ) )
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
