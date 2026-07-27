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
 *
 */

#include <dali/internal/network/common/socket-factory.h>
#include <dali/internal/network/windows/socket-impl-win.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
SocketInterface* SocketFactory::NewSocket(SocketInterface::Protocol protocol)
{
  return new SocketWin(protocol);
}

void SocketFactory::DestroySocket(SocketInterface* socketInterface)
{
  delete static_cast<SocketWin*>(socketInterface);
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
