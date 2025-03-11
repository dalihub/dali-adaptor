#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_ICU_IMPL_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_ICU_IMPL_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/icu-plugin.h>
#include <dali/devel-api/text-abstraction/icu.h>

namespace Dali
{
namespace TextAbstraction
{
namespace Internal
{
/**
 * Implementation of the ICU
 */
class ICU : public BaseObject
{
public:
  /**
   * Constructor
   */
  ICU();

  /**
   * Destructor
   */
  ~ICU();

  /**
   * @brief Dali::TextAbstraction::ICU::UpdateLineBreakInfoByLocale()
   */
  void UpdateLineBreakInfoByLocale(const std::string& text,
                                   Length             numberOfCharacters,
                                   const char*        locale,
                                   LineBreakInfo*     breakInfo);

private:
  /**
   * @brief Initializes member data and dynamically load icu-plugin.so of dali-extension.
   */
  void Initialize();

private:
  ICUPlugin* mPlugin; ///< icu plugin handle
  void*      mHandle; ///< Handle for the loaded library

  using CreateICUPluginFunction  = ICUPlugin* (*)();
  using DestroyICUPluginFunction = void (*)(ICUPlugin* plugin);

  CreateICUPluginFunction  mCreateICUPluginPtr;
  DestroyICUPluginFunction mDestroyICUPluginPtr;

  bool mInitialized : 1; ///< Whether Initialize() has been called, prevents dlopen from being attempted again.

private:
  ICU(const ICU&);
  ICU& operator=(const ICU&);

}; // class ICU

} // namespace Internal

} // namespace TextAbstraction

inline static TextAbstraction::Internal::ICU& GetImplementation(TextAbstraction::ICU& icu)
{
  DALI_ASSERT_ALWAYS(icu && "icu handle is empty");
  BaseObject& handle = icu.GetBaseObject();
  return static_cast<TextAbstraction::Internal::ICU&>(handle);
}

inline static const TextAbstraction::Internal::ICU& GetImplementation(const TextAbstraction::ICU& icu)
{
  DALI_ASSERT_ALWAYS(icu && "icu handle is empty");
  const BaseObject& handle = icu.GetBaseObject();
  return static_cast<const TextAbstraction::Internal::ICU&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_TEXT_ABSTRACTION_ICU_IMPL_H
