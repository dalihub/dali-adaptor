#ifndef DALI_INTERNAL_INPUT_COMMON_AUTOFILL_FACTORY_H
#define DALI_INTERNAL_INPUT_COMMON_AUTOFILL_FACTORY_H

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

// INTERNAL INCLUDES
#include <dali/internal/input/common/autofill-group-impl.h>
#include <dali/internal/input/common/autofill-item-impl.h>
#include <dali/internal/input/common/autofill-manager-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

class AutofillGroup;
class AutofillItem;
class AutofillManager;

namespace AutofillFactory
{
// Factory function creating new AutofillGroup, AutofillItem, and AutofillManager
// Symbol exists but may be overriden during linking

  Dali::AutofillGroup CreateAutofillGroup( const std::string& groupId );

  Dali::AutofillItem CreateAutofillItem( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool sensitiveData );

  Dali::AutofillManager CreateAutofillManager();

}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_INPUT_COMMON_AUTOFILL_FACTORY_H
