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

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <string>

#include "CustomFile.h"

extern void*  MemFOpen(uint8_t* buffer, size_t dataSize, const char* mode);
extern int    MemFClose(const void* fp);
extern bool   MemFIsMemoryFile(const void* fp);
extern size_t MemFRead(void* buffer, size_t elementSize, size_t elementCount, const void* fp);
extern size_t MemFWrite(const void* buffer, size_t elementSize, size_t elementCount, const void* fp);
extern int    MemFSeek(const void* fp, long offset, int origin);
extern long   MemFTell(const void* fp);
extern int    MemFEof(const void* fp);

extern void*  OriginalFOpen(const char* name, const char* mode);
extern int    OriginalFClose(const void* fp);
extern size_t OriginalFRead(void* buffer, size_t elementSize, size_t elementCount, const void* fp);
extern size_t OriginalFWrite(const void* buffer, size_t elementSize, size_t elementCount, const void* fp);
extern int    OriginalFSeek(const void* fp, long offset, int origin);
extern long   OriginalFTell(const void* fp);
extern int    OriginalFEof(const void* fp);

namespace
{
bool GetEnvironmentValue(const std::string& variableName, std::string& value)
{
#if defined(_MSC_VER)
  size_t requiredSize = 0u;
  if(::getenv_s(&requiredSize, nullptr, 0u, variableName.c_str()) != 0 || requiredSize == 0u)
  {
    return false;
  }

  value.resize(requiredSize);
  if(::getenv_s(&requiredSize, value.data(), value.size(), variableName.c_str()) != 0)
  {
    value.clear();
    return false;
  }
  value.resize(requiredSize > 0u ? requiredSize - 1u : 0u);
  return true;
#else
  const char* const variableValue = std::getenv(variableName.c_str());
  if(variableValue == nullptr)
  {
    return false;
  }

  value.assign(variableValue);
  return true;
#endif
}

bool ExpandEnvironmentPath(const char* name, std::string& realName)
{
  if(name == nullptr || name[0] != '*')
  {
    return false;
  }

  const char* const variableBegin = name + 1;
  const char* const variableEnd   = std::strchr(variableBegin, '*');
  if(variableEnd == nullptr || variableEnd == variableBegin)
  {
    return false;
  }

  const std::string variableName(variableBegin, variableEnd);
  std::string       variableValue;
  if(!GetEnvironmentValue(variableName, variableValue))
  {
    return false;
  }

  realName.assign(variableValue);
  realName.append(variableEnd + 1);
  return true;
}
} // unnamed namespace

namespace CustomFile
{
FILE* FOpen(const char* name, const char* mode)
{
  if(name == nullptr || mode == nullptr)
  {
    return nullptr;
  }

  if(name[0] == '*')
  {
    std::string realName;
    if(!ExpandEnvironmentPath(name, realName))
    {
      return nullptr;
    }
    return static_cast<FILE*>(OriginalFOpen(realName.c_str(), mode));
  }

  return static_cast<FILE*>(OriginalFOpen(name, mode));
}

int FClose(const void* fp)
{
  if(fp == nullptr)
  {
    return EOF;
  }

  return MemFIsMemoryFile(fp) ? MemFClose(fp) : OriginalFClose(fp);
}

FILE* FMemopen(void* buffer, size_t dataSize, const char* mode)
{
  return static_cast<FILE*>(MemFOpen(static_cast<uint8_t*>(buffer), dataSize, mode));
}

size_t FRead(void* buffer, size_t elementSize, size_t elementCount, const void* fp)
{
  if(fp == nullptr || (buffer == nullptr && elementSize != 0u && elementCount != 0u))
  {
    return 0u;
  }

  return MemFIsMemoryFile(fp) ? MemFRead(buffer, elementSize, elementCount, fp)
                              : OriginalFRead(buffer, elementSize, elementCount, fp);
}

void FWrite(void* buffer, size_t size, const void* fp)
{
  static_cast<void>(FWrite(buffer, 1u, size, const_cast<void*>(fp)));
}

size_t FWrite(const void* buffer, size_t elementSize, size_t elementCount, void* fp)
{
  if(fp == nullptr || (buffer == nullptr && elementSize != 0u && elementCount != 0u))
  {
    return 0u;
  }

  return MemFIsMemoryFile(fp) ? MemFWrite(buffer, elementSize, elementCount, fp)
                              : OriginalFWrite(buffer, elementSize, elementCount, fp);
}

int FSeek(const void* fp, long offset, int origin)
{
  if(fp == nullptr)
  {
    return -1;
  }

  return MemFIsMemoryFile(fp) ? MemFSeek(fp, offset, origin) : OriginalFSeek(fp, offset, origin);
}

long FTell(const void* fp)
{
  if(fp == nullptr)
  {
    return -1L;
  }

  return MemFIsMemoryFile(fp) ? MemFTell(fp) : OriginalFTell(fp);
}

int FEof(const void* fp)
{
  if(fp == nullptr)
  {
    return 1;
  }

  return MemFIsMemoryFile(fp) ? MemFEof(fp) : OriginalFEof(fp);
}
} // namespace CustomFile

extern "C"
{
size_t __cdecl fread_for_c(void* buffer, size_t elementSize, size_t elementCount, void* stream)
{
  return CustomFile::FRead(buffer, elementSize, elementCount, stream);
}

void __cdecl fwrite_for_c(void* buffer, int size, const void* stream)
{
  if(size > 0)
  {
    CustomFile::FWrite(buffer, static_cast<size_t>(size), stream);
  }
}
}
