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

// CLASS  HEADER
#include "bidirectional-support-impl.h"

// INTERNAL INCLUDES
#include <singleton-service-impl.h>

// EXTERNAL INCLUDES
#include <memory.h>
#include <fribidi/fribidi.h>

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

struct BidirectionalSupport::BidirectionalInfo
{
  FriBidiCharType* characterTypes;      ///< The type of each character (right, left, neutral, ...)
  FriBidiLevel*    embeddedLevels;      ///< Embedded levels.
  FriBidiParType   paragraphDirection;  ///< The paragraph's direction.
};

BidirectionalSupport::BidirectionalSupport()
: mPlugin( NULL ),
  mParagraphBidirectionalInfo(),
  mFreeIndices()
{
}

BidirectionalSupport::~BidirectionalSupport()
{
  // free all resources.
  for( Vector<BidirectionalInfo*>::Iterator it = mParagraphBidirectionalInfo.Begin(),
         endIt = mParagraphBidirectionalInfo.End();
       it != endIt;
       ++it )
  {
    BidirectionalInfo* info = *it;

    free( info->embeddedLevels );
    free( info->characterTypes );
    delete info;
  }
}

TextAbstraction::BidirectionalSupport BidirectionalSupport::Get()
{
  TextAbstraction::BidirectionalSupport bidirectionalSupportHandle;

  SingletonService service( SingletonService::Get() );
  if( service )
  {
    // Check whether the singleton is already created
    BaseHandle handle = service.GetSingleton( typeid( TextAbstraction::BidirectionalSupport ) );
    if(handle)
    {
      // If so, downcast the handle
      BidirectionalSupport* impl = dynamic_cast< Internal::BidirectionalSupport* >( handle.GetObjectPtr() );
      bidirectionalSupportHandle = TextAbstraction::BidirectionalSupport( impl );
    }
    else // create and register the object
    {
      bidirectionalSupportHandle = TextAbstraction::BidirectionalSupport( new BidirectionalSupport );
      service.Register( typeid( bidirectionalSupportHandle ), bidirectionalSupportHandle );
    }
  }

  return bidirectionalSupportHandle;
}

BidiInfoIndex BidirectionalSupport::CreateInfo( const Character* const paragraph,
                                                Length numberOfCharacters )
{
  // Reserve memory for the paragraph's bidirectional info.
  BidirectionalInfo* bidirectionalInfo = new BidirectionalInfo();

  bidirectionalInfo->characterTypes = reinterpret_cast<FriBidiCharType*>( malloc( numberOfCharacters * sizeof( FriBidiCharType ) ) );
  bidirectionalInfo->embeddedLevels = reinterpret_cast<FriBidiLevel*>( malloc( numberOfCharacters * sizeof( FriBidiLevel ) ) );

  // Retrieve the type of each character..
  fribidi_get_bidi_types( paragraph, numberOfCharacters, bidirectionalInfo->characterTypes );

  // Retrieve the paragraph's direction.
  bidirectionalInfo->paragraphDirection = fribidi_get_par_direction( paragraph, numberOfCharacters );

  // Retrieve the embedding levels.
  fribidi_get_par_embedding_levels( paragraph, numberOfCharacters, &bidirectionalInfo->paragraphDirection, bidirectionalInfo->embeddedLevels );

  // Store the bidirectional info and return the index.
  BidiInfoIndex index = 0u;
  const std::size_t numberOfItems = mFreeIndices.Count();
  if( numberOfItems != 0u )
  {
    Vector<BidiInfoIndex>::Iterator it = mFreeIndices.End() - 1u;

    index = *it;

    mFreeIndices.Remove( it );

    *( mParagraphBidirectionalInfo.Begin() + index ) = bidirectionalInfo;
  }
  else
  {
    index = static_cast<BidiInfoIndex>( numberOfItems );

    mParagraphBidirectionalInfo.PushBack( bidirectionalInfo );
  }

  return index;
}

void BidirectionalSupport::DestroyInfo( BidiInfoIndex bidiInfoIndex )
{
  if( bidiInfoIndex >= mParagraphBidirectionalInfo.Count() )
  {
    return;
  }

  // Retrieve the paragraph's bidirectional info.
  Vector<BidirectionalInfo*>::Iterator it = mParagraphBidirectionalInfo.Begin() + bidiInfoIndex;
  BidirectionalInfo* bidirectionalInfo = *it;

  if( NULL != bidirectionalInfo )
  {
    // Free resources and destroy the container.
    free( bidirectionalInfo->embeddedLevels );
    free( bidirectionalInfo->characterTypes );
    delete bidirectionalInfo;

    *it = NULL;
  }

  // Add the index to the free indices vector.
  mFreeIndices.PushBack( bidiInfoIndex );
}

void BidirectionalSupport::Reorder( BidiInfoIndex bidiInfoIndex,
                                    CharacterIndex firstCharacterIndex,
                                    Length numberOfCharacters,
                                    CharacterIndex* visualToLogicalMap )
{
  const FriBidiFlags flags = FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC;

  // Retrieve the paragraph's bidirectional info.
  const BidirectionalInfo* const bidirectionalInfo = *( mParagraphBidirectionalInfo.Begin() + bidiInfoIndex );

  // Initialize the visual to logical mapping table to the identity. Otherwise fribidi_reorder_line fails to retrieve a valid mapping table.
  for( CharacterIndex index = 0u; index < numberOfCharacters; ++index )
  {
    visualToLogicalMap[ index ] = index;
  }

  // Copy embedded levels as fribidi_reorder_line() may change them.
  const uint32_t embeddedLevelsSize = numberOfCharacters * sizeof( FriBidiLevel );
  FriBidiLevel* embeddedLevels = reinterpret_cast<FriBidiLevel*>( malloc( embeddedLevelsSize ) );
  memcpy( embeddedLevels, bidirectionalInfo->embeddedLevels + firstCharacterIndex,  embeddedLevelsSize );

  // Reorder the line.
  fribidi_reorder_line( flags,
                        bidirectionalInfo->characterTypes + firstCharacterIndex,
                        numberOfCharacters,
                        0u,
                        bidirectionalInfo->paragraphDirection,
                        embeddedLevels,
                        NULL,
                        reinterpret_cast<FriBidiStrIndex*>( visualToLogicalMap ) );

  // Free resources.
  free( embeddedLevels );
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
