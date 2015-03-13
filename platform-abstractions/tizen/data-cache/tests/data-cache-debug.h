#ifndef __DALI_TIZEN_PLATFORM_DATA_CACHE_DEBUG_H__
#define __DALI_TIZEN_PLATFORM_DATA_CACHE_DEBUG_H__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

//#define DATA_CACHE_DEBUG // un-comment to enable
#ifdef DATA_CACHE_DEBUG

namespace Dali
{
namespace TizenPlatform
{
namespace DataCacheIo
{

/**
 * Threading stress test.
 * Launches a 8 of threads to read / write to the simultaneously to a single data cache.
 * Each thread reads/writes a number of entries, using random keys and random data.
 * When it has completed, a concise check of all data is made to detect any errors.
 *
 */
void ThreadedStressTest();

} // namespace DataCacheIO

} // namespace TizenPlatform

} // namespace Dali

#endif // __DALI_TIZEN_PLATFORM_DATA_CACHE_DEBUG_H__
#endif // #ifdef DATA_CACHE_DEBUG
