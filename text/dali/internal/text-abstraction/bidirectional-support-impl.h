#ifndef __DALI_INTERNAL_TEXT_ABSTRACTION_BIDIRECTIONAL_SUPPORT_IMPL_H__
#define __DALI_INTERNAL_TEXT_ABSTRACTION_BIDIRECTIONAL_SUPPORT_IMPL_H__

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
#include <dali/public-api/text-abstraction/bidirectional-support.h>

namespace Dali
{

namespace Internal
{

namespace TextAbstraction
{

/**
 * Implementation of the BidirectionalSupport
 */

class BidirectionalSupport :  public Dali::BaseObject
{
public:

  /**
   * Constructor
   */
  BidirectionalSupport();

  /**
   * Destructor
   */
  ~BidirectionalSupport();

  /**
   * @copydoc Dali::BidirectionalSupport::Get()
   */
  static Dali::TextAbstraction::BidirectionalSupport Get();

  /**
   * @copydoc Dali::BidirectionalSupport::CreateInfo()
   */
  Dali::TextAbstraction::BidiInfoIndex CreateInfo( const Dali::TextAbstraction::Character* const paragraph,
                                                   Dali::TextAbstraction::Length numberOfCharacters );

  /**
   * @copydoc Dali::BidirectionalSupport::DestroyInfo()
   */
  void DestroyInfo( Dali::TextAbstraction::BidiInfoIndex bidiInfoIndex );

  /**
   * @copydoc Dali::BidirectionalSupport::Reorder()
   */
  void Reorder( Dali::TextAbstraction::BidiInfoIndex bidiInfoIndex,
                Dali::TextAbstraction::CharacterIndex firstCharacterIndex,
                Dali::TextAbstraction::Length numberOfCharacters,
                Dali::TextAbstraction::CharacterIndex* visualToLogicalMap );

private:

  // Undefined copy constructor.
  BidirectionalSupport( const BidirectionalSupport& );

  // Undefined assignment constructor.
  BidirectionalSupport& operator=( BidirectionalSupport& );

  void* mPlugin; ///< TODO replace this with bidirectional support plugin

}; // class BidirectionalSupport


} // namespace TextAbstraction

} // namespace Internal

inline static Internal::TextAbstraction::BidirectionalSupport& GetImplementation(Dali::TextAbstraction::BidirectionalSupport& bidirectionalSupport)
{
  DALI_ASSERT_ALWAYS( bidirectionalSupport && "bidirectional support handle is empty" );
  BaseObject& handle = bidirectionalSupport.GetBaseObject();
  return static_cast<Internal::TextAbstraction::BidirectionalSupport&>(handle);
}

inline static const  Internal::TextAbstraction::BidirectionalSupport& GetImplementation(const Dali::TextAbstraction::BidirectionalSupport& bidirectionalSupport)
{
  DALI_ASSERT_ALWAYS( bidirectionalSupport && "bidirectional support handle is empty" );
  const BaseObject& handle = bidirectionalSupport.GetBaseObject();
  return static_cast<const Internal::TextAbstraction::BidirectionalSupport&>(handle);
}

} // namespace Dali

#endif // __DALI_INTERNAL_TEXT_ABSTRACTION_BIDIRECTIONAL_SUPPORT_IMPL_H__
