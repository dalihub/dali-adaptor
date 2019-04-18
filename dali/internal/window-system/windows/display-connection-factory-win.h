#ifndef DALI_INTERNAL_WINDOWSYSTEM_WIN_DISPLAY_CONNECTION_FACTORY_WIN_H
#define DALI_INTERNAL_WINDOWSYSTEM_WIN_DISPLAY_CONNECTION_FACTORY_WIN_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

#include <dali/internal/window-system/common/display-connection-factory.h>
#include <dali/internal/window-system/common/display-utils.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class DisplayConnectionFactoryWin : public DisplayConnectionFactory
{
public:
  std::unique_ptr<Dali::Internal::Adaptor::DisplayConnection> CreateDisplayConnection() override;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali


#endif // DALI_INTERNAL_WINDOWSYSTEM_WIN_DISPLAY_CONNECTION_FACTORY_WIN_H
