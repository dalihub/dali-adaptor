#ifndef DALI_SENSOR_COMMON_TILT_SENSOR_IMPL_H
#define DALI_SENSOR_COMMON_TILT_SENSOR_IMPL_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/devel-api/adaptor-framework/tilt-sensor.h>
#include <deque>

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

  /**
   * Constructor
   */
  TiltSensor();

  /**
   * Destructor
   */
  ~TiltSensor() override;

  typedef Dali::TiltSensor::TiltedSignalType TiltedSignalType;

  /**
   * @copydoc Dali::TiltSensor::Start()
   */
  virtual bool Start();

  /**
   * @copydoc Dali::TiltSensor::Stop()
   */
  virtual void Stop();

  /**
   * @copydoc Dali::TiltSensor::IsStarted()
   */
  virtual bool IsStarted() const;

  /**
   * @copydoc Dali::TiltSensor::GetRoll()
   */
  virtual float GetRoll() const;

  /**
   * @copydoc Dali::TiltSensor::GetPitch()
   */
  virtual float GetPitch() const;

  /**
   * @copydoc Dali::TiltSensor::GetRotation()
   */
  virtual Quaternion GetRotation() const;

  /**
   * @copydoc Dali::TiltSensor::TiltedSignal()
   */
  virtual TiltedSignalType& TiltedSignal();

  /**
   * @copydoc Dali::TiltSensor::SetUpdateFrequency()
   */
  virtual void SetUpdateFrequency( float frequencyHertz );

  /**
   * @copydoc Dali::TiltSensor::GetUpdateFrequency()
   */
  virtual float GetUpdateFrequency() const;

  /**
   * @copydoc Dali::TiltSensor::SetRotationThreshold()
   */
  virtual void SetRotationThreshold(Radian rotationThreshold);

  /**
   * @copydoc Dali::TiltSensor::GetRotationThreshold()
   */
  virtual Radian GetRotationThreshold() const;

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
};

} // namespace Adaptor

} // namespace Internal

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

#endif // DALI_SENSOR_COMMON_TILT_SENSOR_IMPL_H
