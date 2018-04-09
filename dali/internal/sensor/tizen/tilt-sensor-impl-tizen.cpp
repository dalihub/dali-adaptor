/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/internal/sensor/tizen/tilt-sensor-impl-tizen.h>
#include <dali/internal/sensor/common/tilt-sensor-factory.h>

// EXTERNAL INCLUDES
#include <cmath>
#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/singleton-service-impl.h>

namespace // unnamed namespace
{
const char* const SIGNAL_TILTED = "tilted";

const float MAX_ORIENTATION_ROLL_VALUE = 90.f;
const float MAX_ORIENTATION_PITCH_VALUE = 180.f;
const float MAX_ACCELEROMETER_VALUE = 9.8f;

// Type Registration
Dali::BaseHandle Create()
{
  return Dali::Internal::Adaptor::TiltSensorFactory::Get();
}

Dali::TypeRegistration typeRegistration( typeid(Dali::TiltSensor), typeid(Dali::BaseHandle), Create );

Dali::SignalConnectorType signalConnector1( typeRegistration, SIGNAL_TILTED, Dali::Internal::Adaptor::TiltSensor::DoConnectSignal );

} // unnamed namespace

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

#ifdef SENSOR_ENABLED
static void sensor_changed_cb (sensor_h sensor, sensor_event_s *event, void *user_data)
{
  TiltSensorTizen* tiltSensor = reinterpret_cast< TiltSensorTizen* >( user_data );

  if(tiltSensor)
  {
    tiltSensor->Update(event);
  }

  return;
}

static std::string get_sensor_error_string(int errorValue)
{
  std::string ret;

  switch(errorValue)
  {
    case SENSOR_ERROR_IO_ERROR:
      ret = "SENSOR_ERROR_IO_ERROR";
      break;
    case SENSOR_ERROR_INVALID_PARAMETER:
      ret = "SENSOR_ERROR_INVALID_PARAMETER";
      break;
    case SENSOR_ERROR_NOT_SUPPORTED:
      ret = "SENSOR_ERROR_NOT_SUPPORTED";
      break;
    case SENSOR_ERROR_PERMISSION_DENIED:
      ret = "SENSOR_ERROR_PERMISSION_DENIED";
      break;
    case SENSOR_ERROR_OUT_OF_MEMORY:
      ret = "SENSOR_ERROR_OUT_OF_MEMORY";
      break;
    case SENSOR_ERROR_NOT_NEED_CALIBRATION:
      ret = "SENSOR_ERROR_NOT_NEED_CALIBRATION";
      break;
    case SENSOR_ERROR_OPERATION_FAILED:
      ret = "SENSOR_ERROR_OPERATION_FAILED";
      break;
  }

  return ret;
}
#endif

TiltSensorTizen* TiltSensorTizen::New()
{
  return new TiltSensorTizen();
}

TiltSensorTizen::~TiltSensorTizen()
{
  Disconnect();
}

bool TiltSensorTizen::Connect()
{
#ifdef SENSOR_ENABLED
  if(mState != DISCONNECTED)
  {
    Stop();
    Disconnect();
  }

  const int interval = 1000/mFrequencyHertz;

  int ret = 0;
  bool isSupported = false;

  // try to use Orientation sensor at first for less power consumption.
  ret = sensor_is_supported(SENSOR_ORIENTATION, &isSupported);

  if(ret < 0)
  {
    DALI_LOG_ERROR("sensor_is_supported() failed : %s\n", get_sensor_error_string(ret).c_str());
    return false;
  }

  if(isSupported == true)
  {
    mSensorType = SENSOR_ORIENTATION;
  }
  else
  {
    DALI_LOG_ERROR("sensor does not support SENSOR_ORIENTATION\n");

    ret = sensor_is_supported(SENSOR_ACCELEROMETER, &isSupported);

    if(ret < 0)
    {
      DALI_LOG_ERROR("sensor_is_supported() failed : %s\n", get_sensor_error_string(ret).c_str());
      return false;
    }

    if(isSupported == false)
    {
      DALI_LOG_ERROR("sensor does not support both SENSOR_ORIENTATION and SENSOR_ACCELEROMETER\n");
      return false;
    }

    mSensorType = SENSOR_ACCELEROMETER;
  }

  ret = sensor_get_default_sensor(mSensorType, &mSensor); /* mSensor should not be deleted */

  if(ret < 0)
  {
    DALI_LOG_ERROR("sensor_get_default_sensor() failed : %s\n", get_sensor_error_string(ret).c_str());
    return false;
  }

  sensor_create_listener(mSensor, &mSensorListener);
  sensor_listener_set_event_cb(mSensorListener, interval, sensor_changed_cb, this);
  sensor_listener_set_interval(mSensorListener, interval);

  sensor_listener_set_option(mSensorListener, SENSOR_OPTION_DEFAULT /* Not receive data when LCD is off and in power save mode */);

  mState = CONNECTED;

  return true;
#endif

  return false;
}

void TiltSensorTizen::Disconnect()
{
  if(mSensorListener)
  {
    if(mState == STARTED)
    {
      Stop();
    }

    if(mState == STOPPED || mState == CONNECTED)
    {
#ifdef SENSOR_ENABLED
      sensor_listener_unset_event_cb(mSensorListener);
      sensor_listener_stop(mSensorListener);
      sensor_destroy_listener(mSensorListener);
#endif
      mSensor = NULL;
      mSensorListener = NULL;
      mState = DISCONNECTED;
    }
  }
}

bool TiltSensorTizen::Start()
{
  if( mSensorListener && ( mState == CONNECTED || mState == STOPPED ) )
  {
#ifdef SENSOR_ENABLED
    int ret = 0;
    ret = sensor_listener_start(mSensorListener);
    if(ret != SENSOR_ERROR_NONE)
    {
      DALI_LOG_ERROR("sensor_listener_start() failed : %s\n", get_sensor_error_string(ret).c_str());
      Disconnect();
      return false;
    }

    mState = STARTED;
    return true;
#endif
  }
  else
  {
    if( mState == STARTED )
    {
      DALI_LOG_ERROR("TiltSensor is already started. Current state [%d]\n", mState);
    }
    else
    {
      // mState is DISCONNECTED
      DALI_LOG_ERROR("TiltSensor is disconnected. Current state [%d]\n", mState);
    }
    return false;
  }
  return false;
}

void TiltSensorTizen::Stop()
{
#ifdef SENSOR_ENABLED
  if(mSensorListener && mState == STARTED)
  {
    sensor_listener_stop( mSensorListener );
    mState = STOPPED;
  }
#endif
}

bool TiltSensorTizen::IsStarted() const
{
  return ( mSensorListener && mState == STARTED );
}

float TiltSensorTizen::GetRoll() const
{
  return mRoll;
}

float TiltSensorTizen::GetPitch() const
{
  return mPitch;
}

Quaternion TiltSensorTizen::GetRotation() const
{
  return mRotation;
}

TiltSensor::TiltedSignalType& TiltSensorTizen::TiltedSignal()
{
  return mTiltedSignal;
}

void TiltSensorTizen::SetUpdateFrequency( float frequencyHertz )
{
  DALI_ASSERT_ALWAYS( frequencyHertz > 0.0f && "Frequency must have a positive value" );

  if ( fabsf(mFrequencyHertz - frequencyHertz) >= GetRangedEpsilon(mFrequencyHertz, frequencyHertz) )
  {
    mFrequencyHertz = frequencyHertz;

#ifdef SENSOR_ENABLED
    if(mSensorListener)
    {
      const int interval = 1000/mFrequencyHertz;
      sensor_listener_set_interval(mSensorListener, interval);
    }
#endif
  }
}

float TiltSensorTizen::GetUpdateFrequency() const
{
  return mFrequencyHertz;
}

void TiltSensorTizen::SetRotationThreshold(Radian rotationThreshold)
{
  mRotationThreshold = rotationThreshold;
}

Radian TiltSensorTizen::GetRotationThreshold() const
{
  return mRotationThreshold;
}

bool TiltSensorTizen::DoConnectSignal( BaseObject* object, ConnectionTrackerInterface* tracker, const std::string& signalName, FunctorDelegate* functor )
{
  bool connected( true );
  TiltSensor* sensor = dynamic_cast<TiltSensor*>( object );

  if( sensor && SIGNAL_TILTED == signalName )
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

TiltSensorTizen::TiltSensorTizen()
: mState(DISCONNECTED),
  mFrequencyHertz( Dali::TiltSensor::DEFAULT_UPDATE_FREQUENCY ),
  mSensor( NULL ),
  mSensorListener( NULL ),
  mRoll( 0.0f ),
  mPitch( 0.0f ),
  mRotation( Radian(0.0f), Vector3::YAXIS ),
  mRotationThreshold( 0.0f )
{
  // connect sensor
  Connect();
}

#ifdef SENSOR_ENABLED
void TiltSensorTizen::Update(sensor_event_s *event)
{
  Radian newRoll( 0.0f );
  Radian newPitch( 0.0f );
  Quaternion newRotation;

  if(mSensorType == SENSOR_ORIENTATION)
  {
    newRoll  = Clamp( float(event->values[2]  / MAX_ORIENTATION_ROLL_VALUE)  /* -90 < roll < 90 */, -1.0f/*min*/, 1.0f/*max*/ );
    newPitch = Clamp( float(event->values[1] / MAX_ORIENTATION_PITCH_VALUE) /* -180 < pitch < 180 */, -1.0f/*min*/, 1.0f/*max*/ );
  }
  else if(mSensorType == SENSOR_ACCELEROMETER)
  {
    newRoll  = Clamp( float(event->values[0] / MAX_ACCELEROMETER_VALUE), -1.0f/*min*/, 1.0f/*max*/ );
    newPitch = Clamp( float(event->values[1] / MAX_ACCELEROMETER_VALUE), -1.0f/*min*/, 1.0f/*max*/ );
  }
  else
  {
    DALI_LOG_ERROR("Invalid sensor type\n");
    return;
  }

  newRotation = Quaternion( Radian( newRoll * Math::PI * -0.5f ), Vector3::YAXIS ) *
              Quaternion( Radian( newPitch * Math::PI * -0.5f ), Vector3::XAXIS );

  Radian angle(Quaternion::AngleBetween(newRotation, mRotation));

  // If the change in value is more than the threshold then emit tilted signal.
  if( angle >= mRotationThreshold )
  {
    mRoll = newRoll;
    mPitch = newPitch;
    mRotation = newRotation;

    // emit signal
    if ( !mTiltedSignal.Empty() )
    {
      Dali::TiltSensor handle( this );
      mTiltedSignal.Emit( handle );
    }
  }
}
#endif // SENSOR_ENABLED

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
