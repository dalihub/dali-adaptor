#ifndef _DALI_INTERNAL_ANDROID_FILE_WRAPPER_H__
#define _DALI_INTERNAL_ANDROID_FILE_WRAPPER_H__
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

// EXTERNAL INCLUDES
#include <cstdio>

// INTERNAL INCLUDES
#include <dali/public-api/common/dali-vector.h>

namespace Dali
{

namespace Internal
{

namespace Android
{

FILE* OpenFile(const char* path, const char* mode);

long GetFileSize(const std::string& filename, int& error);

int ReadFile(const std::string& filename, std::string& output);

int ReadFile(const std::string& filename, long& size, Dali::Vector<char>& memblock);

}

}

}
#endif /* _DALI_INTERNAL_ANDROID_FILE_WRAPPER_H__ */
