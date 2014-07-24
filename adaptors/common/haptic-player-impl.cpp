/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <haptic-player-impl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <adaptor-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace // unnamed namespace
{

// Type Registration
Dali::BaseHandle Create()
{
  return HapticPlayer::Get();
}

Dali::TypeRegistration HAPTIC_PLAYER_TYPE( typeid(Dali::HapticPlayer), typeid(Dali::BaseHandle), Create );

} // unnamed namespace

Dali::HapticPlayer HapticPlayer::New()
{
  Dali::HapticPlayer player = Dali::HapticPlayer( new HapticPlayer() );
  return player;
}

Dali::HapticPlayer HapticPlayer::Get()
{
  Dali::HapticPlayer player;

  if ( Adaptor::IsAvailable() )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = Adaptor::Get().GetSingleton( typeid( Dali::HapticPlayer ) );
    if ( handle )
    {
      // If so, downcast the handle
      player = Dali::HapticPlayer( dynamic_cast< HapticPlayer* >( handle.GetObjectPtr() ) );
    }
    else
    {
      Adaptor& adaptorImpl( Adaptor::GetImplementation( Adaptor::Get() ) );
      player = Dali::HapticPlayer( New() );
      adaptorImpl.RegisterSingleton( typeid( player ), player );
    }
  }

  return player;
}

void HapticPlayer::PlayMonotone( unsigned int duration )
{
  mPlugin.PlayHapticMonotone( duration );
}

void HapticPlayer::PlayFile( const std::string& filePath )
{
  mPlugin.PlayHaptic( filePath );
}

void HapticPlayer::Stop()
{
  mPlugin.StopHaptic();
}

HapticPlayer::HapticPlayer()
: mPlugin( FeedbackPluginProxy::DEFAULT_OBJECT_NAME )
{
}

HapticPlayer::~HapticPlayer()
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
