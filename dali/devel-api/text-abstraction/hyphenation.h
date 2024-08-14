#ifndef DALI_PLATFORM_TEXT_ABSTRACTION_HYPHENATION_H
#define DALI_PLATFORM_TEXT_ABSTRACTION_HYPHENATION_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>
#include <dali/public-api/dali-adaptor-common.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-handle.h>

namespace Dali
{
namespace TextAbstraction
{
namespace Internal DALI_INTERNAL
{
class Hyphenation;

} // namespace DALI_INTERNAL

/**
 * @brief Hyphenation provides an interface to retrieve possible hyphenation of the text.
 *
 * This module get the hyphen positions for a word.
 * To get the hyphen positions it needs to load the dictionary for the word language.
 * also it need the word to be send using the correct encoding which can be known using
 *
 * @code
 * Hyphenation hyphenation = Hyphenation::Get();
 *
 * Character* text = "Hyphenation";
 *
 * // get a vector of booleans that indicates possible hyphen locations.
 * Vector<bool> *hyphens = hyphenation.GetWordHyphens(text, 11, "en_US");
 *
 * @endcode
 */
class DALI_ADAPTOR_API Hyphenation : public BaseHandle
{
public:
  /**
   * @brief Create an uninitialized Hyphenation handle.
   *
   */
  Hyphenation();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~Hyphenation();

  /**
   * @brief This constructor is used by Hyphenation::Get().
   *
   * @param[in] implementation A pointer to the internal hyphenation object.
   */
  explicit DALI_INTERNAL Hyphenation(Internal::Hyphenation* implementation);

  /**
   * @brief Retrieve a handle to the Hyphenation instance.
   *
   * @return A handle to the Hyphenation.
   * @remarks A reference to the singleton instance of Hyphenation.
   */
  static Hyphenation Get();

  /**
   * @brief Create a handle to the new Hyphenation instance.
   *
   * @return A handle to the Hyphenation.
   * @remarks All functions of this are not thread-safe,
   * so create new handles for each worker thread to utilize them.
   */
  static Hyphenation New();

  /**
   *
   * Gets the encoding of the dictionary for the given language
   *
   * @param[in] lang language to get encoding for (en_US).
   *
   * @return The encoding of the language (UTF-32).
   */
  const char* GetDictionaryEncoding(const char* lang);

  /**
   * Gets a vector booleans that indicates possible hyphen locations.
   *
   * @param[in] word the word to get possible hyphens for.
   * @param[in] wordSize the word size in bytes.
   * @param[in] lang the language for the word
   *
   * @return vector of boolean, true if possible to hyphenate at this character position.
   */
  Vector<bool> GetWordHyphens(const char* word,
                              Length      wordSize,
                              const char* lang);
};

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_PLATFORM_TEXT_ABSTRACTION_HYPHENATION_H
