#ifndef DALI_SENSOR_UBUNTU_TILT_SENSOR_IMPL_UBUNTU_H
#define DALI_SENSOR_UBUNTU_TILT_SENSOR_IMPL_UBUNTU_H

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
#include <dali/public-api/object/base-object.h>
#include <deque>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/tilt-sensor.h>
#include <dali/internal/sensor/common/tilt-sensor-impl.h>
#include <dali/public-api/adaptor-framework/timer.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * TiltSensor provides pitch & roll values when the device is tilted.
 */
class TiltSensorUbuntu : public Dali::Internal::Adaptor::TiltSensor
{
public:
  typedef Dali::TiltSensor::TiltedSignalType TiltedSignalType;

  /**
   * Public constructor
   * @return New instance of TiltSensorUbuntu
   */
  static TiltSensorUbuntu* New();

  /**
   * @copydoc Dali::TiltSensor::Start()
   */
  bool Start() override;

  /**
   * @copydoc Dali::TiltSensor::Stop()
   */
  void Stop() override;

  /**
   * @copydoc Dali::TiltSensor::IsStarted()
   */
  bool IsStarted() const override;

  /**
   * @copydoc Dali::TiltSensor::GetRoll()
   */
  float GetRoll() const override;

  /**
   * @copydoc Dali::TiltSensor::GetPitch()
   */
  float GetPitch() const override;

  /**
   * @copydoc Dali::TiltSensor::GetRotation()
   */
  Quaternion GetRotation() const override;

  /**
   * @copydoc Dali::TiltSensor::TiltedSignal()
   */
  TiltedSignalType& TiltedSignal() override;

  /**
   * @copydoc Dali::TiltSensor::SetUpdateFrequency()
   */
  void SetUpdateFrequency(float frequencyHertz) override;

  /**
   * @copydoc Dali::TiltSensor::GetUpdateFrequency()
   */
  float GetUpdateFrequency() const override;

  /**
   * @copydoc Dali::TiltSensor::SetRotationThreshold()
   */
  void SetRotationThreshold(Radian rotationThreshold) override;

  /**
   * @copydoc Dali::TiltSensor::GetRotationThreshold()
   */
  Radian GetRotationThreshold() const override;

  /**
   * Connects a callback function with the object's signals.
   * @param[in] object The object providing the signal.
   * @param[in] tracker Used to disconnect the signal.
   * @param[in] signalName The signal to connect to.
   * @param[in] functor A newly allocated FunctorDelegate.
   * @return True if the signal was connected.
   * @post If a signal was connected, ownership of functor was passed to CallbackBase. Otherwise the caller is responsible for deleting the unused functor.
   */
  static bool DoConnectSignal(BaseObject* object, ConnectionTrackerInterface* tracker, const std::string& signalName, FunctorDelegate* functor);

private:
  /**
   * Private constructor; see also TiltSensor::New()
   */
  TiltSensorUbuntu();

  /**
   * Destructor
   */
  virtual ~TiltSensorUbuntu() override;

  /**
   * Timer callback to update the tilt values
   */
  bool Update();

  // Undefined
  TiltSensorUbuntu(const TiltSensorUbuntu&) = delete;

  // Undefined
  TiltSensorUbuntu& operator=(TiltSensorUbuntu&) = delete;

private:
  float                          mFrequencyHertz;
  Dali::Timer                    mTimer;
  SlotDelegate<TiltSensorUbuntu> mTimerSlot;

  int mSensorFrameworkHandle;

  float      mRoll;
  float      mPitch;
  Quaternion mRotation;

  Radian mRotationThreshold;

  std::deque<float> mRollValues;
  std::deque<float> mPitchValues;

  TiltedSignalType mTiltedSignal;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_SENSOR_UBUNTU_TILT_SENSOR_IMPL_UBUNTU_H
