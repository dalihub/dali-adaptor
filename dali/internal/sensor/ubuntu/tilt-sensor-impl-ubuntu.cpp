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
#include <dali/internal/sensor/ubuntu/tilt-sensor-impl-ubuntu.h>
#include <dali/internal/sensor/common/tilt-sensor-factory.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/singleton-service-impl.h>

namespace // unnamed namespace
{

const char* const SIGNAL_TILTED = "tilted";

const int NUMBER_OF_SAMPLES = 10;

const float MAX_ACCELEROMETER_XY_VALUE = 9.8f;

// Type Registration
Dali::BaseHandle GetInstance()
{
  return Dali::Internal::Adaptor::TiltSensorFactory::Get();
}

Dali::TypeRegistration typeRegistration( typeid(Dali::TiltSensor), typeid(Dali::BaseHandle), GetInstance );

Dali::SignalConnectorType signalConnector1( typeRegistration, SIGNAL_TILTED, Dali::Internal::Adaptor::TiltSensor::DoConnectSignal );

} // unnamed namespace

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

TiltSensorUbuntu* TiltSensorUbuntu::New()
{
  return new TiltSensorUbuntu();
}

TiltSensorUbuntu::~TiltSensorUbuntu()
{
  Stop();
}

bool TiltSensorUbuntu::Start()
{
  // Make sure sensor API is responding
  bool success = Update();

  if ( success )
  {
    if ( !mTimer )
    {
      mTimer = Dali::Timer::New( 1000.0f / mFrequencyHertz );
      mTimer.TickSignal().Connect( mTimerSlot, &TiltSensorUbuntu::Update );
    }

    if ( mTimer &&
         !mTimer.IsRunning() )
    {
      mTimer.Start();
    }
  }

  return success;
}

void TiltSensorUbuntu::Stop()
{
  if ( mTimer )
  {
    mTimer.Stop();
    mTimer.Reset();
  }
}

bool TiltSensorUbuntu::IsStarted() const
{
  return ( mTimer && mTimer.IsRunning() );
}

float TiltSensorUbuntu::GetRoll() const
{
  return mRoll;
}

float TiltSensorUbuntu::GetPitch() const
{
  return mPitch;
}

Quaternion TiltSensorUbuntu::GetRotation() const
{
  return mRotation;
}

TiltSensor::TiltedSignalType& TiltSensorUbuntu::TiltedSignal()
{
  return mTiltedSignal;
}

void TiltSensorUbuntu::SetUpdateFrequency( float frequencyHertz )
{
  DALI_ASSERT_ALWAYS( frequencyHertz > 0.0f && "Frequency must have a positive value" );

  if ( fabsf(mFrequencyHertz - frequencyHertz) >= GetRangedEpsilon(mFrequencyHertz, frequencyHertz) )
  {
    mFrequencyHertz = frequencyHertz;

    if ( mTimer )
    {
      mTimer.SetInterval( 1000.0f / mFrequencyHertz );
    }
  }
}

float TiltSensorUbuntu::GetUpdateFrequency() const
{
  return mFrequencyHertz;
}

void TiltSensorUbuntu::SetRotationThreshold(Radian rotationThreshold)
{
  mRotationThreshold = rotationThreshold;
}

Radian TiltSensorUbuntu::GetRotationThreshold() const
{
  return mRotationThreshold;
}

bool TiltSensorUbuntu::DoConnectSignal( BaseObject* object, ConnectionTrackerInterface* tracker, const std::string& signalName, FunctorDelegate* functor )
{
  bool connected( true );
  TiltSensor* sensor = dynamic_cast<TiltSensor*>( object );

  if( sensor && ( SIGNAL_TILTED == signalName ) )
  {
    sensor->TiltedSignal().Connect( tracker, functor );
  }
  else
  {
    // signalName does not match any signal
    connected = false;
  }

  return connected;
}

TiltSensorUbuntu::TiltSensorUbuntu()
: mFrequencyHertz( Dali::TiltSensor::DEFAULT_UPDATE_FREQUENCY ),
  mTimerSlot( this ),
  mSensorFrameworkHandle( -1 ),
  mRoll( 0.0f ),
  mPitch( 0.0f ),
  mRotation( Dali::ANGLE_0, Vector3::YAXIS ),
  mRotationThreshold( 0.0f )
{
  mRollValues.resize( NUMBER_OF_SAMPLES, 0.0f );
  mPitchValues.resize( NUMBER_OF_SAMPLES, 0.0f );
}

bool TiltSensorUbuntu::Update()
{
  float newRoll = 0.0f;
  float newPitch = 0.0f;
  Quaternion newRotation;

  Radian angle(Quaternion::AngleBetween(newRotation, mRotation));
  // If the change in value is more than the threshold then emit tilted signal.
  if( angle > mRotationThreshold )
  {
    mRoll = newRoll;
    mPitch = newPitch;
    mRotation = newRotation;

    if ( !mTiltedSignal.Empty() )
    {
      Dali::TiltSensor handle( this );
      mTiltedSignal.Emit( handle );
    }
  }

  return true;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
