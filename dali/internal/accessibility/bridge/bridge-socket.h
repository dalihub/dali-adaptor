#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_SOCKET_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_SOCKET_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/socket.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>

/**
 * @brief The BridgeSocket class contains glue code for Accessibility::Socket.
 */
class BridgeSocket : public virtual BridgeBase
{
protected:
  BridgeSocket() = default;

  /**
   * @brief Registers Socket methods as a DBus interface.
   */
  void RegisterInterfaces();

  /**
   * @brief Returns the Socket object of the currently executed DBus method call.
   *
   * @return The Socket object
   */
  Dali::Accessibility::Socket* FindSelf() const;

public:
  /**
   * @copydoc Dali::Accessibility::Socket::Embed()
   */
  DBus::ValueOrError<Dali::Accessibility::Address> Embed(Dali::Accessibility::Address plug);

  /**
   * @copydoc Dali::Accessibility::Socket::Unembed()
   */
  DBus::ValueOrError<void> Unembed(Dali::Accessibility::Address plug);

  /**
   * @copydoc Dali::Accessibility::Socket::SetOffset()
   */
  DBus::ValueOrError<void> SetOffset(std::int32_t x, std::int32_t y);
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_SOCKET_H
