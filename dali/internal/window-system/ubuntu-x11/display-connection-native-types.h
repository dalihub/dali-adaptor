#pragma once

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/public-api/object/any.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/ubuntu-x11/ecore-x-types.h>

namespace Dali::Internal::Adaptor
{
/**
 * Returns the Any cast of the given display to the native graphics type.
 * @param display The X Display to cast
 * @return The Any with the appropriate cast
 */
Any CastToNativeGraphicsType(XDisplay* display);
} // namespace Dali::Internal::Adaptor
