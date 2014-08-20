#ifndef __DALI_ORIENTATION_H__
#define __DALI_ORIENTATION_H__

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
#include <boost/function.hpp>

#include <dali/public-api/signals/dali-signal-v2.h>
#include <dali/public-api/object/base-handle.h>

namespace Dali DALI_IMPORT_API
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class Orientation;
}
}

/**
 * @brief Orientation allows the user to determine the orientation of the device.
 *
 * A signal is emitted whenever the orientation changes.
 * Dali applications have full control over visual layout when the device is rotated
 * i.e. the application developer decides which UI controls to rotate, if any.
 */
class Orientation : public BaseHandle
{
public:

  typedef SignalV2< void (Orientation) > OrientationSignalV2; ///< Orientation changed signal type

  /**
   * @brief Create an unintialized handle.
   *
   * This can be initialized by calling Dali::Application::GetOrientation()
   */
  Orientation();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~Orientation();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param [in] handle A reference to the copied handle
   */
  Orientation(const Orientation& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param [in] rhs  A reference to the copied handle
   * @return A reference to this
   */
  Orientation& operator=(const Orientation& rhs);

  /**
   * @brief This method is defined to allow assignment of the NULL value,
   * and will throw an exception if passed any other value.
   *
   * Assigning to NULL is an alias for Reset().
   * @param [in] rhs  A NULL pointer
   * @return A reference to this handle
   */
  Orientation& operator=(BaseHandle::NullType* rhs);

  /**
   * @brief Returns the orientation of the device in degrees.
   *
   * This is one of four discrete values, in degrees clockwise: 0, 90, 180, & 270
   * For a device with a portrait form-factor:
   *   0 indicates that the device is in the "normal" portrait orientation.
   *   90 indicates that device has been rotated clockwise, into a landscape orientation.
   * @return The orientation in degrees clockwise.
   */
  int GetDegrees() const;

  /**
   * @brief Returns the orientation of the device in radians.
   *
   * This is one of four discrete values, in radians clockwise: 0, PI/2, PI, & 3xPI/2
   * For a device with a portrait form-factor:
   *   0 indicates that the device is in the "normal" portrait orientation.
   *   PI/2 indicates that device has been rotated clockwise, into a landscape orientation.
   * @return The orientation in radians clockwise.
   */
  float GetRadians() const;

  /**
   * @brief The user should connect to this signal so that they can be notified whenever
   * the orientation of the device changes.
   *
   * @return The orientation change signal.
   */
  OrientationSignalV2& ChangedSignal();

public: // Not intended for application developers
  /**
   * @brief This constructor is used by Dali::Application::GetOrientation().
   *
   * @param[in] orientation A pointer to the orientation object
   */
  explicit DALI_INTERNAL Orientation( Internal::Adaptor::Orientation* orientation );
};

} // namespace Dali

#endif // __DALI_ORIENTATION_H__
