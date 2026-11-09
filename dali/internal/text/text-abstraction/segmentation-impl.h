#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_SEGMENTATION_IMPL_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_SEGMENTATION_IMPL_H

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
#include <dali/devel-api/text-abstraction/segmentation.h>

namespace DALI_NAMESPACE
{
namespace TextAbstraction
{
namespace Internal
{
/**
 * Implementation of the Segmentation
 */

class Segmentation : public BaseObject
{
public:
  /**
   * @brief Creates an unreferenced implementation only for BaseHandle construction.
   * @warning Pass the result directly to a BaseHandle-derived handle constructor.
   * The constructor must acquire its first reference (the returned reference count is zero).
   * Use New() when an IntrusivePtr owner is required.
   */
  static DALI_INTERNAL Segmentation* NewForBaseHandle()
  {
    return new Segmentation();
  }

  /**
   * @brief Creates a reference-counted implementation.
   */
  static Dali::IntrusivePtr<Segmentation> New()
  {
    return Dali::IntrusivePtr<Segmentation>(new Segmentation());
  }

  /**
   * @copydoc Dali::Segmentation::Get()
   */
  static TextAbstraction::Segmentation Get();

  /**
   * @copydoc Dali::Segmentation::GetLineBreakPositions()
   */
  void GetLineBreakPositions(const Character* const text,
                             Length                 numberOfCharacters,
                             LineBreakInfo*         breakInfo);

  /**
   * @copydoc Dali::Segmentation::GetWordBreakPositions()
   */
  void GetWordBreakPositions(const Character* const text,
                             Length                 numberOfCharacters,
                             WordBreakInfo*         breakInfo);

  /**
   * @copydoc Dali::Segmentation::GetLineBreakPositionsUtf8()
   */
  void GetLineBreakPositionsUtf8(const uint8_t* const text,
                                 Length               numberOfCharacters,
                                 LineBreakInfo*       breakInfo);

  /**
   * @copydoc Dali::Segmentation::GetWordBreakPositionsUtf8()
   */
  void GetWordBreakPositionsUtf8(const uint8_t* const text,
                                 Length               numberOfCharacters,
                                 WordBreakInfo*       breakInfo);

protected:
  /**
   * Constructor
   */
  Segmentation();

  /**
   * Destructor
   */
  ~Segmentation();

private:
  /**
   * Helper for lazy initialization.
   */
  void CreatePlugin();
  // Undefined copy constructor.
  Segmentation(const Segmentation&);

  // Undefined assignment constructor.
  Segmentation& operator=(Segmentation&);

private:
  struct Plugin;
  Plugin* mPlugin;

}; // class Segmentation

} // namespace Internal

} // namespace TextAbstraction

inline static TextAbstraction::Internal::Segmentation& GetImplementation(TextAbstraction::Segmentation& segmentation)
{
  DALI_ASSERT_ALWAYS(segmentation && "segmentation handle is empty");
  BaseObject& handle = segmentation.GetBaseObject();
  return static_cast<TextAbstraction::Internal::Segmentation&>(handle);
}

inline static const TextAbstraction::Internal::Segmentation& GetImplementation(const TextAbstraction::Segmentation& segmentation)
{
  DALI_ASSERT_ALWAYS(segmentation && "segmentation handle is empty");
  const BaseObject& handle = segmentation.GetBaseObject();
  return static_cast<const TextAbstraction::Internal::Segmentation&>(handle);
}

} //namespace DALI_NAMESPACE

#endif // DALI_INTERNAL_TEXT_ABSTRACTION_SEGMENTATION_IMPL_H
