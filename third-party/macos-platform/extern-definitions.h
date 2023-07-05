/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <memory>
#include <type_traits>

// Specialiation of std::unique_ptr for Foundation Objects
template<typename T>
using CFRef = std::unique_ptr<
  std::remove_pointer_t<T>,
  std::add_pointer_t<decltype(CFRelease)>
>;

template<typename T>
inline CFRef<T> MakeRef(T p)
{
  return CFRef<T>(p, CFRelease);
}
