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

// CLASS HEADER
#include <dali/internal/system/common/shared-file.h>

// EXTERNAL INCLUDES
#include <sys/mman.h>
#include <sys/types.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
int SharedFile::Open(const char* filename, int size, int oflag, mode_t mode)
{
  return shm_open(filename, oflag, mode);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
