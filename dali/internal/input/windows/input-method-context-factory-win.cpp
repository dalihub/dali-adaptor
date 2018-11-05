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

#include <memory>
#include <dali/internal/input/common/input-method-context-factory.h>
#include <dali/internal/input/windows/input-method-context-impl-win.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class InputMethodContext;

namespace InputMethodContextFactory
{

// InputMethodContext Factory to be implemented by the platform
InputMethodContextPtr CreateInputMethodContext()
{
  return Dali::Internal::Adaptor::InputMethodContextWin::New();
}

}

}



}
}