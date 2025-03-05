/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

// The following macro enables VMA internal definitions
// See https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/quick_start.html

#define VMA_IMPLEMENTATION
#include <dali/internal/graphics/vulkan/vulkan-memory-allocation.h>

#if defined(DEBUG_ENABLED)

Debug::Filter* gVmaLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VMA");

#endif // DEBUG_ENABLED
