#ifndef __DALI_INTERNAL_TEXT_ABSTRACTION_SHAPING_IMPL_H__
#define __DALI_INTERNAL_TEXT_ABSTRACTION_SHAPING_IMPL_H__

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
#include <public-api/shaping.h>
#include <dali/public-api/object/base-object.h>



namespace Dali
{

namespace Internal
{

namespace TextAbstraction
{

/**
 * Implementation of the Shaping
 */

class Shaping :  public Dali::BaseObject
{
public:

  /**
   * Constructor
   */
  Shaping();

  /**
   * Destructor
   */
  ~Shaping();

  /**
   * @copydoc Dali::Shaping::Get()
   */
  static Dali::TextAbstraction::Shaping Get();

private:

  // Undefined copy constructor.
  Shaping( const Shaping& );

  // Undefined assignment constructor.
  Shaping& operator=( Shaping& );

  void*   mPlugin;  ///< TODO replace this with shaping plugin

}; // class Shaping


} // namespace TextAbstraction

} // namespace Internal

inline static Internal::TextAbstraction::Shaping& GetImplementation(Dali::TextAbstraction::Shaping& shaping)
{
  DALI_ASSERT_ALWAYS( shaping && "shaping handle is empty" );
  BaseObject& handle = shaping.GetBaseObject();
  return static_cast<Internal::TextAbstraction::Shaping&>(handle);
}

inline static const  Internal::TextAbstraction::Shaping& GetImplementation(const Dali::TextAbstraction::Shaping& shaping)
{
  DALI_ASSERT_ALWAYS( shaping && "shaping handle is empty" );
  const BaseObject& handle = shaping.GetBaseObject();
  return static_cast<const Internal::TextAbstraction::Shaping&>(handle);
}

} // namespace Dali

#endif // __DALI_INTERNAL_TEXT_ABSTRACTION_SHAPING_IMPL_H__
