#ifndef __DALI_INTERNAL_TEXT_ABSTRACTION_SEGMENTATION_IMPL_H__
#define __DALI_INTERNAL_TEXT_ABSTRACTION_SEGMENTATION_IMPL_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/text-abstraction/segmentation.h>

namespace Dali
{

namespace Internal
{

namespace TextAbstraction
{

/**
 * Implementation of the Segmentation
 */

class Segmentation :  public Dali::BaseObject
{
public:

  /**
   * Constructor
   */
  Segmentation();

  /**
   * Destructor
   */
  ~Segmentation();

  /**
   * @copydoc Dali::Segmentation::Get()
   */
  static Dali::TextAbstraction::Segmentation Get();

private:

  // Undefined copy constructor.
  Segmentation( const Segmentation& );

  // Undefined assignment constructor.
  Segmentation& operator=( Segmentation& );

  void* mPlugin; ///< TODO replace this with segmentation plugin

}; // class Segmentation


} // namespace TextAbstraction

} // namespace Internal

inline static Internal::TextAbstraction::Segmentation& GetImplementation(Dali::TextAbstraction::Segmentation& reordering)
{
  DALI_ASSERT_ALWAYS( reordering && "reordering handle is empty" );
  BaseObject& handle = reordering.GetBaseObject();
  return static_cast<Internal::TextAbstraction::Segmentation&>(handle);
}

inline static const  Internal::TextAbstraction::Segmentation& GetImplementation(const Dali::TextAbstraction::Segmentation& reordering)
{
  DALI_ASSERT_ALWAYS( reordering && "reordering handle is empty" );
  const BaseObject& handle = reordering.GetBaseObject();
  return static_cast<const Internal::TextAbstraction::Segmentation&>(handle);
}

} // namespace Dali

#endif // __DALI_INTERNAL_TEXT_ABSTRACTION_SEGMENTATION_IMPL_H__
