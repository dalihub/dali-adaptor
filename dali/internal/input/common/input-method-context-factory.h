#ifndef DALI_INTERNAL_INPUT_COMMON_INPUT_METHOD_CONTEXT_FACTORY_H
#define DALI_INTERNAL_INPUT_COMMON_INPUT_METHOD_CONTEXT_FACTORY_H

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

#include <dali/internal/input/common/input-method-context-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

namespace InputMethodContextFactory
{

// Factory function creating new InputMethodContext
// Symbol exists but may be overriden during linking
InputMethodContextPtr CreateInputMethodContext( Dali::Actor actor );

}
}
}

}

#endif //DALI_INTERNAL_INPUT_COMMON_INPUT_METHOD_CONTEXT_FACTORY_H
