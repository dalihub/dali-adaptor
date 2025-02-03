#ifndef DALI_GRAPHICS_VULKAN_MEMORY_ALLOCATION_H
#define DALI_GRAPHICS_VULKAN_MEMORY_ALLOCATION_H

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

#include <dali/integration-api/debug.h>

// This is an implementation of VMA (Vulkan Memory Allocator) included
// from the Vulkan SDK.

#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wformat"
#endif

#if defined(DEBUG_ENABLED)

#define VMA_DEBUG_MARGIN 16
#define VMA_DEBUG_DETECT_CORRUPTION 1
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_STATS_STRING_ENABLED 1

extern Debug::Filter* gVmaLogFilter;

#define VMA_DEBUG_LOG(format, ...) DALI_LOG_INFO(gVmaLogFilter, Debug::General, "[VMA] " format "\n", ##__VA_ARGS__)

#endif // DEBUG_ENABLED

// Disable VMA assert in release build temporarily as it causes crash due to memory leaks
// TODO: Enable this after memory leaks are fixed
#if !defined(DEBUG_ENABLED)
#define VMA_ASSERT(expr)
#endif // DEBUG_ENABLED

// Assert that will be called very often, like inside data structures e.g. operator[].
// Making it non-empty can make program slow, so disabled in release build
#if !defined(DEBUG_ENABLED)
#define VMA_HEAVY_ASSERT(expr)
#endif // DEBUG_ENABLED

#include <third-party/vma/vk_mem_alloc.hpp>

#if __GNUC__
#pragma GCC diagnostic pop
#endif

#endif // DALI_GRAPHICS_VULKAN_MEMORY_ALLOCATION_H
