#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_SHAPING_IMPL_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_SHAPING_IMPL_H

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
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/shaping.h>
#include <dali/public-api/common/dali-vector.h>

namespace DALI_NAMESPACE
{
namespace TextAbstraction
{
namespace Internal
{
/**
 * Implementation of the Shaping
 */
class Shaping : public BaseObject
{
public:
  /**
   * @brief Creates an unreferenced implementation only for BaseHandle construction.
   * @warning Pass the result directly to a BaseHandle-derived handle constructor.
   * The constructor must acquire its first reference (the returned reference count is zero).
   * Use New() when an IntrusivePtr owner is required.
   */
  static DALI_INTERNAL Shaping* NewForBaseHandle()
  {
    return new Shaping();
  }

  /**
   * @brief Creates a reference-counted implementation.
   */
  static Dali::IntrusivePtr<Shaping> New()
  {
    return Dali::IntrusivePtr<Shaping>(new Shaping());
  }

  /**
   * @copydoc Dali::Shaping::Get()
   */
  static TextAbstraction::Shaping Get();

  /**
   * @copydoc Dali::Shaping::Shape()
   */
  Length Shape(TextAbstraction::FontClient& fontClient,
               const Character* const       text,
               Length                       numberOfCharacters,
               FontId                       fontId,
               Script                       script);

  /**
   * @copydoc Dali::Shaping::GetGlyphs()
   */
  void GetGlyphs(GlyphInfo*      glyphInfo,
                 CharacterIndex* glyphToCharacterMap);

protected:
  /**
   * Constructor
   */
  Shaping();

  /**
   * Destructor
   */
  ~Shaping();

private:
  /**
   * Helper for lazy initialization.
   */
  void CreatePlugin();
  // Undefined copy constructor.
  Shaping(const Shaping&);

  // Undefined assignment constructor.
  Shaping& operator=(const Shaping&);

  struct Plugin;
  Plugin* mPlugin;

}; // class Shaping

} // namespace Internal

} // namespace TextAbstraction

inline static TextAbstraction::Internal::Shaping& GetImplementation(TextAbstraction::Shaping& shaping)
{
  DALI_ASSERT_ALWAYS(shaping && "shaping handle is empty");
  BaseObject& handle = shaping.GetBaseObject();
  return static_cast<TextAbstraction::Internal::Shaping&>(handle);
}

inline static const TextAbstraction::Internal::Shaping& GetImplementation(const TextAbstraction::Shaping& shaping)
{
  DALI_ASSERT_ALWAYS(shaping && "shaping handle is empty");
  const BaseObject& handle = shaping.GetBaseObject();
  return static_cast<const TextAbstraction::Internal::Shaping&>(handle);
}

} //namespace DALI_NAMESPACE

#endif // DALI_INTERNAL_TEXT_ABSTRACTION_SHAPING_IMPL_H
