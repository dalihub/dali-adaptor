#ifndef DALI_ADAPTOR_ATSPI_SOCKET_H
#define DALI_ADAPTOR_ATSPI_SOCKET_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/accessibility.h>

namespace Dali::Accessibility
{
/**
 * @brief A Socket is the root of the AT-SPI tree that can be embedded as a subtree
 * in the tree belonging to another process (the Plug). The Plug initiates the Plug-Socket
 * connection by calling Embed() and terminates it by calling Unembed().
 *
 * See AT_SPI2_CORE/xml/Socket.xml for a description of this interface in XML format.
 */
class DALI_ADAPTOR_API Socket
{
public:
  /**
   * @brief Establishes the Plug-Socket connection.
   *
   * @param plug Address of the Plug (remote parent)
   * @return Address of the Socket
   */
  virtual Address Embed(Address plug);

  /**
   * @brief Terminates the Plug-Socket connection.
   *
   * @param plug Address of the Plug (remote parent)
   */
  virtual void Unembed(Address plug);

  /**
   * @brief Set the offset (position information).
   *
   * @param[in] x Horizontal offset
   * @param[in] y Vertical offset
   */
  virtual void SetOffset(std::int32_t x, std::int32_t y);
};

namespace Internal
{
template<>
struct AtspiInterfaceTypeHelper<AtspiInterface::SOCKET>
{
  using Type = Socket;
};
} // namespace Internal

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_SOCKET_H
