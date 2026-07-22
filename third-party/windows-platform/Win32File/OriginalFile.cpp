/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
 */

#include <cstdio>

void* OriginalFOpen(const char* name, const char* mode)
{
#if defined(_MSC_VER)
  FILE* file = nullptr;
  return ::fopen_s(&file, name, mode) == 0 ? file : nullptr;
#else
  return std::fopen(name, mode);
#endif
}

int OriginalFClose(const void* fp)
{
  return std::fclose(static_cast<FILE*>(const_cast<void*>(fp)));
}

size_t OriginalFRead(void* buffer, size_t elementSize, size_t elementCount, const void* fp)
{
  return std::fread(buffer, elementSize, elementCount, static_cast<FILE*>(const_cast<void*>(fp)));
}

size_t OriginalFWrite(const void* buffer, size_t elementSize, size_t elementCount, const void* fp)
{
  return std::fwrite(buffer, elementSize, elementCount, static_cast<FILE*>(const_cast<void*>(fp)));
}

int OriginalFSeek(const void* fp, long offset, int origin)
{
  return std::fseek(static_cast<FILE*>(const_cast<void*>(fp)), offset, origin);
}

long OriginalFTell(const void* fp)
{
  return std::ftell(static_cast<FILE*>(const_cast<void*>(fp)));
}

int OriginalFEof(const void* fp)
{
  return std::feof(static_cast<FILE*>(const_cast<void*>(fp)));
}
