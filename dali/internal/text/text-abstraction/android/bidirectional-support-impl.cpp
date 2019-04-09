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
#include <dali/internal/text/text-abstraction/bidirectional-support-impl.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/singleton-service-impl.h>

// EXTERNAL INCLUDES
#include <memory.h>
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

struct BidirectionalSupport::Plugin
{
  Plugin() {}

  ~Plugin() {}

  BidiInfoIndex CreateInfo( const Character* const paragraph,
                            Length numberOfCharacters,
                            bool matchSystemLanguageDirection,
                            LayoutDirection::Type layoutDirection )
  {
    return 0;
  }

  void DestroyInfo( BidiInfoIndex bidiInfoIndex )
  {
  }

  void Reorder( BidiInfoIndex bidiInfoIndex,
                CharacterIndex firstCharacterIndex,
                Length numberOfCharacters,
                CharacterIndex* visualToLogicalMap )
  {
  }

  bool GetMirroredText( Character* text,
                        CharacterDirection* directions,
                        Length numberOfCharacters ) const
  {
    return false;
  }

  bool GetParagraphDirection( BidiInfoIndex bidiInfoIndex ) const
  {
    return false;
  }

  void GetCharactersDirection( BidiInfoIndex bidiInfoIndex,
                               CharacterDirection* directions,
                               Length numberOfCharacters )
  {
  }
};

BidirectionalSupport::BidirectionalSupport()
: mPlugin( NULL )
{
}

BidirectionalSupport::~BidirectionalSupport()
{
  delete mPlugin;
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
                                                Length numberOfCharacters,
                                                bool matchSystemLanguageDirection,
                                                Dali::LayoutDirection::Type layoutDirection )
{
  CreatePlugin();

  return mPlugin->CreateInfo( paragraph,
                              numberOfCharacters,
                              matchSystemLanguageDirection,
                              layoutDirection );
}

void BidirectionalSupport::DestroyInfo( BidiInfoIndex bidiInfoIndex )
{
  CreatePlugin();

  mPlugin->DestroyInfo( bidiInfoIndex );
}

void BidirectionalSupport::Reorder( BidiInfoIndex bidiInfoIndex,
                                    CharacterIndex firstCharacterIndex,
                                    Length numberOfCharacters,
                                    CharacterIndex* visualToLogicalMap )
{
  CreatePlugin();

  mPlugin->Reorder( bidiInfoIndex,
                    firstCharacterIndex,
                    numberOfCharacters,
                    visualToLogicalMap );
}

bool BidirectionalSupport::GetMirroredText( Character* text,
                                            CharacterDirection* directions,
                                            Length numberOfCharacters )
{
  CreatePlugin();

  return mPlugin->GetMirroredText( text, directions, numberOfCharacters );
}

bool BidirectionalSupport::GetParagraphDirection( BidiInfoIndex bidiInfoIndex ) const
{
  if( !mPlugin )
  {
    return false;
  }

  return mPlugin->GetParagraphDirection( bidiInfoIndex );
}

void BidirectionalSupport::GetCharactersDirection( BidiInfoIndex bidiInfoIndex,
                                                   CharacterDirection* directions,
                                                   Length numberOfCharacters )
{
  CreatePlugin();

  mPlugin->GetCharactersDirection( bidiInfoIndex,
                                   directions,
                                   numberOfCharacters );
}

void BidirectionalSupport::CreatePlugin()
{
  if( !mPlugin )
  {
    mPlugin = new Plugin();
  }
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
