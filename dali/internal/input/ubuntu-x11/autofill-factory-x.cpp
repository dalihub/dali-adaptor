/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#include <dali/internal/input/common/autofill-factory.h>
#include <dali/internal/input/ubuntu-x11/autofill-group-impl-x.h>
#include <dali/internal/input/ubuntu-x11/autofill-item-impl-x.h>
#include <dali/internal/input/ubuntu-x11/autofill-manager-impl-x.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

namespace AutofillFactory
{

// Autofill Factory to be implemented by the platform

Dali::AutofillGroup CreateAutofillGroup( const std::string& groupId )
{
  return Dali::Internal::Adaptor::AutofillGroupX::New( groupId );
}

Dali::AutofillItem CreateAutofillItem( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool sensitiveData )
{
  return Dali::Internal::Adaptor::AutofillItemX::New( id, label, hint, sensitiveData );
}

Dali::AutofillManager CreateAutofillManager()
{
  return Dali::Internal::Adaptor::AutofillManagerX::Get();
}

}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
