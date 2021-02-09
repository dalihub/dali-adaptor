/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include <dali/devel-api/common/singleton-service.h>
#include <dali/internal/input/common/input-method-context-factory.h>
#include <dali/internal/input/common/input-method-context-impl.h>
#include <dali/internal/system/common/locale-utils.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
InputMethodContextPtr InputMethodContext::New(Dali::Actor actor)
{
  return Dali::Internal::Adaptor::InputMethodContextFactory::CreateInputMethodContext(actor);
}

const std::string& InputMethodContext::GetSurroundingText() const
{
  static std::string str("");
  return str;
}

InputMethodContext::InputMethodContext()
: mBackupOperations(Operation::MAX_COUNT)
{
}

void InputMethodContext::ApplyBackupOperations()
{
  // Items in mBackupOperations will be changed while the iteration
  OperationList copiedList = mBackupOperations;

  for(auto& operation : copiedList)
  {
    if(operation)
    {
      operation();
    }
  }
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
