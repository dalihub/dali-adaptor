#ifndef __DALI_PHYSICAL_KEYBOARD_H__
#define __DALI_PHYSICAL_KEYBOARD_H__

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
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal-v2.h>

namespace Dali DALI_IMPORT_API
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class PhysicalKeyboard;
}
}

/**
 * This is a handle to a physical keyboard connected to the device.
 */
class PhysicalKeyboard : public BaseHandle
{
public:

  typedef SignalV2< void (PhysicalKeyboard) > Signal;

public:

  /**
   * Create an uninitialized PhysicalKeyboard handle; this can be initialized with GetKeyboard()
   * Calling member functions with an uninitialized Dali::Object is not allowed.
   */
  PhysicalKeyboard();

  /**
   * Virtual destructor.
   */
  virtual ~PhysicalKeyboard();

  /**
   * Gets a handle to the physical keyboard.
   * @return A handle to the physical keyboard.
   */
  static PhysicalKeyboard Get();

  /**
   * Queries whether a physical keyboard is attached or not.
   * @return true if a physical keyboard is attached, false otherwise.
   */
  bool IsAttached() const;

  // Signals

  /**
   * Emitted when the status of the physical keyboard changes.
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(PhysicalKeyboard keyboard);
   * @endcode
   * @pre The PhysicalKeyboard has been initialized.
   * @return The status changed signal.
   */
  Signal& StatusChangedSignal();

  // Not intended for application developers

  /**
   * Creates a new handle from the implementation.
   * @param[in] impl A pointer to the object.
   */
  explicit DALI_INTERNAL PhysicalKeyboard( Internal::Adaptor::PhysicalKeyboard* impl );
};

} // namespace Dali

#endif // __DALI_PHYSICAL_KEYBOARD_H__
