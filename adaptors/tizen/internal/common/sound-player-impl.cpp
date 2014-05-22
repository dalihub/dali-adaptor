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
#include <internal/common/sound-player-impl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <internal/common/adaptor-impl.h>

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
  return SoundPlayer::Get();
}

Dali::TypeRegistration SOUND_PLAYER_TYPE( typeid(Dali::SoundPlayer), typeid(Dali::BaseHandle), Create );

Dali::SignalConnectorType SIGNAL_CONNECTOR_1( SOUND_PLAYER_TYPE, Dali::SoundPlayer::SIGNAL_SOUND_PLAY_FINISHED, Dali::Internal::Adaptor::SoundPlayer::DoConnectSignal );

} // unnamed namespace

Dali::SoundPlayer SoundPlayer::New()
{
  Dali::SoundPlayer player = Dali::SoundPlayer( new SoundPlayer() );
  return player;
}

Dali::SoundPlayer SoundPlayer::Get()
{
  Dali::SoundPlayer player;

  if ( Adaptor::IsAvailable() )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = Adaptor::Get().GetSingleton( typeid( Dali::SoundPlayer ) );
    if ( handle )
    {
      // If so, downcast the handle
      player = Dali::SoundPlayer( dynamic_cast< SoundPlayer* >( handle.GetObjectPtr() ) );
    }
    else
    {
      Adaptor& adaptorImpl( Adaptor::GetImplementation( Adaptor::Get() ) );
      player = Dali::SoundPlayer( New() );
      adaptorImpl.RegisterSingleton( typeid( player ), player );
    }
  }

  return player;
}

int SoundPlayer::PlaySound( const std::string fileName )
{
  return mPlugin.PlaySound( fileName );
}

void SoundPlayer::Stop( int handle )
{
  mPlugin.StopSound( handle );
}

SoundPlayer::SoundPlayFinishedSignalV2& SoundPlayer::SoundPlayFinishedSignal()
{
  return mSoundPlayFinishedSignalV2;
}

bool SoundPlayer::DoConnectSignal( BaseObject* object, ConnectionTrackerInterface* tracker, const std::string& signalName, FunctorDelegate* functor )
{
  bool connected( true );
  SoundPlayer* player = dynamic_cast<SoundPlayer*>( object );

  if( player &&
      Dali::SoundPlayer::SIGNAL_SOUND_PLAY_FINISHED == signalName )
  {
    player->SoundPlayFinishedSignal().Connect( tracker, functor );
  }
  else
  {
    // signalName does not match any signal
    connected = false;
  }

  return connected;
}

SoundPlayer::SoundPlayer()
: mPlugin( FeedbackPluginProxy::DEFAULT_OBJECT_NAME )
{
}

SoundPlayer::~SoundPlayer()
{
}

void SoundPlayer::EmitSoundPlayFinishedSignal()
{
  // Emit SoundPlayFinished signal

  if ( !mSoundPlayFinishedSignalV2.Empty() )
  {
    Dali::SoundPlayer handle( this );
    mSoundPlayFinishedSignalV2.Emit( handle );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
