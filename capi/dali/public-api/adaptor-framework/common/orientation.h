#ifndef __DALI_ORIENTATION_H__
#define __DALI_ORIENTATION_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/**
 * @addtogroup CAPI_DALI_ADAPTOR_MODULE
 * @{
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
   * @brief Virtual Destructor
   */
  virtual ~Orientation();

  /**
   * @copydoc Dali::BaseHandle::operator=
   */
  using BaseHandle::operator=;


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

/**
 * @}
 */
#endif // __DALI_ORIENTATION_H__
