#ifndef DALI_ADAPTOR_ATSPI_SOCKET_H
#define DALI_ADAPTOR_ATSPI_SOCKET_H

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
 */

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/accessible.h>

namespace Dali::Accessibility
{
/**
 * @brief A Socket is the root of the AT-SPI tree that can be embedded as a subtree
 * in the tree belonging to another process (the Plug). The Plug initiates the Plug-Socket
 * connection by calling Embed() and terminates it by calling Unembed().
 *
 * See AT_SPI2_CORE/xml/Socket.xml for a description of this interface in XML format.
 */
class DALI_ADAPTOR_API Socket : public virtual Accessible
{
public:
  /**
   * @brief Establishes the Plug-Socket connection.
   *
   * @param plug Address of the Plug (remote parent)
   * @return Address of the Socket
   */
  virtual Address Embed(Address plug) = 0;

  /**
   * @brief Terminates the Plug-Socket connection.
   *
   * @param plug Address of the Plug (remote parent)
   */
  virtual void Unembed(Address plug) = 0;

  /**
   * @brief Set the offset (position information).
   *
   * @param[in] x Horizontal offset
   * @param[in] y Vertical offset
   */
  virtual void SetOffset(std::int32_t x, std::int32_t y) = 0;

  /**
   * @brief Downcasts an Accessible to a Socket.
   *
   * @param obj The Accessible
   * @return A Socket or null
   *
   * @see Dali::Accessibility::Accessible::DownCast()
   */
  static inline Socket* DownCast(Accessible* obj);
};

namespace Internal
{
template<>
struct AtspiInterfaceTypeHelper<AtspiInterface::SOCKET>
{
  using Type = Socket;
};
} // namespace Internal

inline Socket* Socket::DownCast(Accessible* obj)
{
  return Accessible::DownCast<AtspiInterface::SOCKET>(obj);
}

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_SOCKET_H
