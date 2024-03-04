/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

// CLASS  HEADER
#include <dali/internal/text/text-abstraction/hyphenation-impl.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>

#include <dali/devel-api/common/singleton-service.h>

// EXTERNAL INCLUDES
#ifdef HYPHEN_LIBRARY_AVAILABLE
#include <hyphen.h>
#else
#define HyphenDict void
#endif

#ifndef HYPHEN_DIC
#define HYPHEN_DIC "abc"
#endif

namespace
{
#if defined(DEBUG_ENABLED)
Dali::Integration::Log::Filter* gLogFilter = Dali::Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_HYPHENATION");
#endif

} // namespace

namespace Dali
{
namespace TextAbstraction
{
namespace Internal
{
const char* const  DEFAULT_LANGUAGE        = "en_US";
const unsigned int DEFAULT_LANGUAGE_LENGTH = 5u;

struct Hyphenation::HyphenDictionary
{
  HyphenDictionary(std::string lang, HyphenDict* dict)
  {
    mLanguage   = lang;
    mDictionary = dict;
  }

  ~HyphenDictionary()
  {
  }

  std::string mLanguage;
  HyphenDict* mDictionary;
};

struct Hyphenation::Plugin
{
  Plugin()
  : mHyphenDictionary()
  {
  }

  ~Plugin()
  {
#ifdef HYPHEN_LIBRARY_AVAILABLE
    for(auto&& it : mHyphenDictionary)
    {
      hnj_hyphen_free(it.mDictionary);
    }
#endif
  }

  std::string GetPathForLanguage(const char* lang)
  {
    std::string path    = HYPHEN_DIC;
    std::string filName = "/hyph_";
    filName.append(lang);
    filName.append(".dic");
    path.append(filName);

    return path; // must be ..../hyph_en_US.dic
  }

  HyphenDict* LoadDictionary(const char* language)
  {
#ifdef HYPHEN_LIBRARY_AVAILABLE
    const char* lang = (language ? language : DEFAULT_LANGUAGE);

    for(auto&& it : mHyphenDictionary)
    {
      if(!it.mLanguage.compare(lang))
      {
        return it.mDictionary;
      }
    }

    std::string path = GetPathForLanguage(lang);
    HyphenDict* dict = hnj_hyphen_load(path.c_str());
    if(!dict)
    {
      DALI_LOG_ERROR("Couldn't load hyphen dictionary:%s\n", lang);
    }
    else
    {
      mHyphenDictionary.push_back(HyphenDictionary(lang, dict));
    }

    return dict;
#else
    return nullptr;
#endif
  }

  const char* GetDictionaryEncoding(const char* lang)
  {
#ifdef HYPHEN_LIBRARY_AVAILABLE
    HyphenDict* dict = LoadDictionary(lang);
    if(!dict)
    {
      return nullptr;
    }

    return dict->cset;
#else
    return "UTF_32";
#endif
  }

  Vector<bool> GetWordHyphens(const char* word,
                              Length      wordLength,
                              const char* lang)
  {
    Vector<bool> hyphensList;

#ifdef HYPHEN_LIBRARY_AVAILABLE
    char*       hyphens = nullptr;
    char**      rep     = nullptr;
    int*        pos     = nullptr;
    int*        cut     = nullptr;
    HyphenDict* dict;

    if((!word) || (wordLength < 1))
    {
      return hyphensList;
    }

    dict = LoadDictionary(lang);
    if(!dict)
    {
      return hyphensList;
    }

    hyphens = (char*)malloc(wordLength + 5);
    if(DALI_LIKELY(hyphens))
    {
      hnj_hyphen_hyphenate2(dict, (char*)(word), wordLength, hyphens, NULL, &rep, &pos, &cut);

      hyphensList.PushBack(false);

      for(Length i = 0; i < wordLength - 1; i++)
      {
        hyphensList.PushBack((bool)(hyphens[i + 1] & 1));
      }

      free(hyphens);
    }
    else
    {
      DALI_LOG_ERROR("malloc is failed. request malloc size : %u\n", wordLength + 5);
    }
#endif

    return hyphensList;
  }

  std::vector<HyphenDictionary> mHyphenDictionary;
};

Hyphenation::Hyphenation()
: mPlugin(nullptr)
{
}

Hyphenation::~Hyphenation()
{
}

TextAbstraction::Hyphenation Hyphenation::Get()
{
  TextAbstraction::Hyphenation hyphenationHandle;

  SingletonService service(SingletonService::Get());
  if(service)
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(TextAbstraction::Hyphenation));
    if(handle)
    {
      // If so, downcast the handle
      Hyphenation* impl = dynamic_cast<Internal::Hyphenation*>(handle.GetObjectPtr());
      hyphenationHandle = TextAbstraction::Hyphenation(impl);
    }
    else // create and register the object
    {
      hyphenationHandle = TextAbstraction::Hyphenation(new Hyphenation);
      service.Register(typeid(hyphenationHandle), hyphenationHandle);
    }
  }

  return hyphenationHandle;
}

const char* Hyphenation::GetDictionaryEncoding(const char* lang)
{
  CreatePlugin();

  return mPlugin->GetDictionaryEncoding(lang);
}

Vector<bool> Hyphenation::GetWordHyphens(const char* word,
                                         Length      wordLength,
                                         const char* lang)
{
  CreatePlugin();

  return mPlugin->GetWordHyphens(word, wordLength, lang);
}

void Hyphenation::CreatePlugin()
{
  if(!mPlugin)
  {
    mPlugin = std::make_unique<Plugin>();
  }
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
