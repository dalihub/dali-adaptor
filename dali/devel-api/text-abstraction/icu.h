#ifndef DALI_PLATFORM_TEXT_ABSTRACTION_ICU_H
#define DALI_PLATFORM_TEXT_ABSTRACTION_ICU_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
class ICU;

} // namespace DALI_INTERNAL

/**
 * @brief To support ICU in dali. ICU, International Components for Unicode.
 */
class DALI_ADAPTOR_API ICU : public BaseHandle
{
public:
  /**
   * @brief Create an uninitialized ICU handle.
   *
   */
  ICU();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~ICU();

  /**
   * @brief This constructor is used by ICU::New().
   *
   * @param[in] implementation A pointer to the internal icu object.
   */
  explicit DALI_INTERNAL ICU(Internal::ICU* implementation);

  /**
   * @brief Create a handle to the new ICU instance.
   *
   * @return A handle to the ICU.
   */
  static ICU New();

  /**
   * @brief Update line break information by ICU.
   * @remark Updates given line break information with ICU dictionary-based word wrap information that unibreak does not support.
   * @param[in] text A string of UTF-8 characters.
   * @param[in] numberOfCharacters The number of characters.
   * @param[in] locale The locale code. (en, ko, en_US, ko_KR.utf8, etc. ICU accepts most formats of locale code), The usual expected form is language_locale (ko_KR).
   * @param[out] breakInfo The unibreak line break information buffer.
   */
  void UpdateLineBreakInfoByLocale(const std::string& text,
                                   Length             numberOfCharacters,
                                   const char*        locale,
                                   LineBreakInfo*     breakInfo);
};

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_PLATFORM_TEXT_ABSTRACTION_ICU_H
