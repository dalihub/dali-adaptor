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
 *
 */

#include <dali/internal/system/common/locale-context.h>

#include <clocale>
#include <locale>
#include <sstream>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
namespace Locale
{
namespace
{
struct LocaleState
{
  std::string languageCode;
  std::string localeName;
};

LocaleState& GetState()
{
  static LocaleState state;
  return state;
}
} // unnamed namespace

void EnsureLocale()
{
  if(GetState().languageCode.empty() || GetState().localeName.empty())
  {
    const char* locale;
    locale = setlocale(LC_MESSAGES, NULL);
    if(locale)
    {
      SetLocale(locale);
    }
  }
}

const std::string& GetLanguageCode()
{
  return GetState().languageCode;
}

const std::string& GetLocaleName()
{
  return GetState().localeName;
}

void SetLocale(const std::string& locale)
{
  // To unify the tizen system locale and format, remove the string after the dot.
  std::istringstream stringStreamFull(locale);
  stringStreamFull.imbue(std::locale::classic());
  std::getline(stringStreamFull, GetState().localeName, '.');

  std::istringstream stringStream(locale);
  stringStream.imbue(std::locale::classic());
  std::getline(stringStream, GetState().languageCode, '_');
}

} // namespace Locale
} // namespace Adaptor
} // namespace Internal
} // namespace DALI_NAMESPACE
