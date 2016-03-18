#ifndef __DALI_INTERNAL_ADAPTOR_TILT_SENSOR_H__
#define __DALI_INTERNAL_ADAPTOR_TILT_SENSOR_H__

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

// EXTERNAL INCLUDES
#ifdef CAPI_SYSTEM_SENSOR_SUPPORT
#include <sensor/sensor.h>

#define SENSOR_ENABLED
#endif

#include <deque>
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <public-api/adaptor-framework/timer.h>
#include <devel-api/adaptor-framework/tilt-sensor.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * TiltSensor provides pitch & roll values when the device is tilted.
 */
class TiltSensor : public Dali::BaseObject
{
public:

  typedef Dali::TiltSensor::TiltedSignalType TiltedSignalType;

  /**
   * Create a TiltSensor.
   * This should only be called once by the Adaptor class.
   * @return A newly allocated tilt-sensor.
   */
  static Dali::TiltSensor New();

  /**
   * @copydoc Dali::TiltSensor::Get()
   */
  static Dali::TiltSensor Get();

  /**
   * @copydoc Dali::TiltSensor::Enable()
   */
  bool Enable();

  /**
   * @copydoc Dali::TiltSensor::Disable()
   */
  void Disable();

  /**
   * @copydoc Dali::TiltSensor::IsEnabled()
   */
  bool IsEnabled() const;

  /**
   * @copydoc Dali::TiltSensor::GetRoll()
   */
  float GetRoll() const;

  /**
   * @copydoc Dali::TiltSensor::GetPitch()
   */
  float GetPitch() const;

  /**
   * @copydoc Dali::TiltSensor::GetRotation()
   */
  Quaternion GetRotation() const;

  /**
   * @copydoc Dali::TiltSensor::TiltedSignal()
   */
  TiltedSignalType& TiltedSignal();

  /**
   * @copydoc Dali::TiltSensor::SetUpdateFrequency()
   */
  void SetUpdateFrequency( float frequencyHertz );

  /**
   * @copydoc Dali::TiltSensor::GetUpdateFrequency()
   */
  float GetUpdateFrequency() const;

  /**
   * @copydoc Dali::TiltSensor::SetRotationThreshold()
   */
  void SetRotationThreshold(Radian rotationThreshold);

  /**
   * @copydoc Dali::TiltSensor::GetRotationThreshold()
   */
  Radian GetRotationThreshold() const;

  /**
   * Connects a callback function with the object's signals.
   * @param[in] object The object providing the signal.
   * @param[in] tracker Used to disconnect the signal.
   * @param[in] signalName The signal to connect to.
   * @param[in] functor A newly allocated FunctorDelegate.
   * @return True if the signal was connected.
   * @post If a signal was connected, ownership of functor was passed to CallbackBase. Otherwise the caller is responsible for deleting the unused functor.
   */
  static bool DoConnectSignal( BaseObject* object, ConnectionTrackerInterface* tracker, const std::string& signalName, FunctorDelegate* functor );

  /**
   * Update sensor data
   * @note This is called by static sensor callback function
   * @param[in] event sensor event data
   */
#ifdef SENSOR_ENABLED
  void Update(sensor_event_s *event);
#endif

private:

  enum State
  {
    DISCONNECTED,
    CONNECTED,
    STARTED,
    STOPPED
  };

  /**
   * Private constructor; see also TiltSensor::New()
   */
  TiltSensor();

  /**
   * Destructor
   */
  virtual ~TiltSensor();

  /**
   * Connect sensor device
   */
  bool Connect();
  /**
   * Disconnect sensor device
   */
  void Disconnect();

  /**
   * Start sensor operation
   */
  bool Start();
  /**
   * Stop sensor operation
   */
  void Stop();

  // Undefined
  TiltSensor(const TiltSensor&);

  // Undefined
  TiltSensor& operator=(TiltSensor&);

private:
  State mState;
  float mFrequencyHertz;

#ifdef SENSOR_ENABLED
  sensor_type_e mSensorType;
  sensor_h mSensor;
  sensor_listener_h mSensorListener;
#else
  int mSensorType;
  int* mSensor;
  int* mSensorListener;
#endif

  float mRoll;
  float mPitch;
  Quaternion mRotation;

  Radian mRotationThreshold;

  TiltedSignalType mTiltedSignal;
};

} // namespace Adaptor

} // namespace Internal

// Helpers for public-api forwarding methods

inline Internal::Adaptor::TiltSensor& GetImplementation(Dali::TiltSensor& sensor)
{
  DALI_ASSERT_ALWAYS( sensor && "TiltSensor handle is empty" );

  BaseObject& handle = sensor.GetBaseObject();

  return static_cast<Internal::Adaptor::TiltSensor&>(handle);
}

inline const Internal::Adaptor::TiltSensor& GetImplementation(const Dali::TiltSensor& sensor)
{
  DALI_ASSERT_ALWAYS( sensor && "TiltSensor handle is empty" );

  const BaseObject& handle = sensor.GetBaseObject();

  return static_cast<const Internal::Adaptor::TiltSensor&>(handle);
}

} // namespace Dali

#endif // __DALI_INTERNAL_ADAPTOR_TILT_SENSOR_H__
