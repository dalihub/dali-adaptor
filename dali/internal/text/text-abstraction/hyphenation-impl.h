#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_HYPHENATION_IMPL_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_HYPHENATION_IMPL_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-object.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/hyphenation.h>
#include <dali/public-api/common/dali-vector.h>

namespace Dali
{
namespace TextAbstraction
{
namespace Internal
{
/**
 * Implementation of the Hyphenation
 */
class Hyphenation : public BaseObject
{
public:
  /**
   * Constructor
   */
  Hyphenation();

  /**
   * Destructor
   */
  ~Hyphenation();

  /**
   * @copydoc Dali::Hyphenation::Get()
   */
  static TextAbstraction::Hyphenation Get();

  /**
   * @copydoc Dali::Hyphenation::GetDictionaryEncoding()
   */
  const char* GetDictionaryEncoding(const char* lang);

  /**
   * @copydoc Dali::Hyphenation::GetWordHyphens()
   */
  Vector<bool> GetWordHyphens(const char* word,
                              Length      wordLength,
                              const char* lang);

private:
  /**
   * Helper for lazy initialization.
   */
  void CreatePlugin();

private:
  // Undefined copy constructor.
  Hyphenation(const Hyphenation&);

  // Undefined assignment constructor.
  Hyphenation& operator=(const Hyphenation&);

  struct HyphenDictionary;
  struct Plugin;
  std::unique_ptr<Plugin> mPlugin;

}; // class Hyphenation

} // namespace Internal

} // namespace TextAbstraction

inline static TextAbstraction::Internal::Hyphenation& GetImplementation(TextAbstraction::Hyphenation& hyphenation)
{
  DALI_ASSERT_ALWAYS(hyphenation && "hyphenation handle is empty");
  BaseObject& handle = hyphenation.GetBaseObject();
  return static_cast<TextAbstraction::Internal::Hyphenation&>(handle);
}

inline static const TextAbstraction::Internal::Hyphenation& GetImplementation(const TextAbstraction::Hyphenation& hyphenation)
{
  DALI_ASSERT_ALWAYS(hyphenation && "hyphenation handle is empty");
  const BaseObject& handle = hyphenation.GetBaseObject();
  return static_cast<const TextAbstraction::Internal::Hyphenation&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_TEXT_ABSTRACTION_HYPHENATION_IMPL_H
