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

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <mutex>
#include <unordered_map>

namespace
{
struct MemoryFileState
{
  uint8_t* buffer;
  size_t   capacity;
  bool     readable;
  bool     writable;
  bool     append;
};

using MemoryFileRegistry = std::unordered_map<const void*, MemoryFileState>;

MemoryFileRegistry& GetMemoryFileRegistry()
{
  static MemoryFileRegistry registry;
  return registry;
}

std::mutex& GetMemoryFileRegistryMutex()
{
  static std::mutex mutex;
  return mutex;
}

FILE* OpenTemporaryFile()
{
#if defined(_MSC_VER)
  FILE* stream = nullptr;
  return ::tmpfile_s(&stream) == 0 ? stream : nullptr;
#else
  return std::tmpfile();
#endif
}

bool ParseMode(const char* mode, bool& readable, bool& writable, bool& append, bool& truncate)
{
  if(mode == nullptr || mode[0] == '\0')
  {
    return false;
  }

  const bool update = std::strchr(mode, '+') != nullptr;
  readable          = mode[0] == 'r' || update;
  writable          = mode[0] == 'w' || mode[0] == 'a' || update;
  append            = mode[0] == 'a';
  truncate          = mode[0] == 'w';

  return mode[0] == 'r' || mode[0] == 'w' || mode[0] == 'a';
}

size_t GetAppendLength(const uint8_t* buffer, size_t capacity)
{
  const void* const terminator = std::memchr(buffer, '\0', capacity);
  return terminator == nullptr ? capacity : static_cast<const uint8_t*>(terminator) - buffer;
}

int SynchronizeBuffer(FILE* stream, const MemoryFileState& state)
{
  if(!state.writable)
  {
    return 0;
  }

  if(std::fflush(stream) != 0 || std::fseek(stream, 0L, SEEK_END) != 0)
  {
    return EOF;
  }

  const long streamLength = std::ftell(stream);
  if(streamLength < 0L || std::fseek(stream, 0L, SEEK_SET) != 0)
  {
    return EOF;
  }

  const bool   overflow    = static_cast<size_t>(streamLength) > state.capacity;
  const size_t bytesToCopy = std::min(static_cast<size_t>(streamLength), state.capacity);
  std::clearerr(stream);
  const size_t bytesCopied = std::fread(state.buffer, 1u, bytesToCopy, stream);
  if(bytesCopied != bytesToCopy)
  {
    return EOF;
  }

  if(bytesCopied < state.capacity)
  {
    state.buffer[bytesCopied] = '\0';
  }

  // A raw CRT FILE* cannot prevent callers from writing past the emulated
  // fmemopen capacity.  Report that condition at close instead of silently
  // accepting a truncated result.
  return overflow ? EOF : 0;
}
} // unnamed namespace

void* MemFOpen(uint8_t* buffer, size_t dataSize, const char* mode)
{
  if(buffer == nullptr || dataSize == 0u)
  {
    return nullptr;
  }

  bool readable = false;
  bool writable = false;
  bool append   = false;
  bool truncate = false;
  if(!ParseMode(mode, readable, writable, append, truncate))
  {
    return nullptr;
  }

  FILE* const stream = OpenTemporaryFile();
  if(stream == nullptr)
  {
    return nullptr;
  }

  size_t initialLength = 0u;
  if(append)
  {
    initialLength = GetAppendLength(buffer, dataSize);
  }
  else if(!truncate)
  {
    initialLength = dataSize;
  }

  if(initialLength > 0u && std::fwrite(buffer, 1u, initialLength, stream) != initialLength)
  {
    std::fclose(stream);
    return nullptr;
  }

  if(truncate)
  {
    buffer[0] = '\0';
  }

  if(initialLength > static_cast<size_t>(std::numeric_limits<long>::max()))
  {
    std::fclose(stream);
    return nullptr;
  }

  const long initialPosition = append ? static_cast<long>(initialLength) : 0L;
  if(std::fseek(stream, initialPosition, SEEK_SET) != 0)
  {
    std::fclose(stream);
    return nullptr;
  }

  try
  {
    std::lock_guard<std::mutex> lock(GetMemoryFileRegistryMutex());
    GetMemoryFileRegistry().emplace(stream, MemoryFileState{buffer, dataSize, readable, writable, append});
  }
  catch(...)
  {
    std::fclose(stream);
    return nullptr;
  }

  return stream;
}

bool MemFIsMemoryFile(const void* fp)
{
  if(fp == nullptr)
  {
    return false;
  }

  std::lock_guard<std::mutex> lock(GetMemoryFileRegistryMutex());
  return GetMemoryFileRegistry().find(fp) != GetMemoryFileRegistry().end();
}

int MemFClose(const void* fp)
{
  std::lock_guard<std::mutex> lock(GetMemoryFileRegistryMutex());
  auto&                       registry = GetMemoryFileRegistry();
  const auto                  iterator = registry.find(fp);
  if(iterator == registry.end())
  {
    return EOF;
  }

  FILE* const stream       = static_cast<FILE*>(const_cast<void*>(fp));
  const int   syncResult   = SynchronizeBuffer(stream, iterator->second);
  const int   closeResult  = std::fclose(stream);
  registry.erase(iterator);
  return syncResult == 0 ? closeResult : syncResult;
}

size_t MemFRead(void* buffer, size_t elementSize, size_t elementCount, const void* fp)
{
  if(elementSize == 0u || elementCount == 0u)
  {
    return 0u;
  }

  std::lock_guard<std::mutex> lock(GetMemoryFileRegistryMutex());
  const auto                  iterator = GetMemoryFileRegistry().find(fp);
  if(iterator == GetMemoryFileRegistry().end() || !iterator->second.readable)
  {
    return 0u;
  }

  return std::fread(buffer, elementSize, elementCount, static_cast<FILE*>(const_cast<void*>(fp)));
}

size_t MemFWrite(const void* buffer, size_t elementSize, size_t elementCount, const void* fp)
{
  if(elementSize == 0u || elementCount == 0u)
  {
    return 0u;
  }

  std::lock_guard<std::mutex> lock(GetMemoryFileRegistryMutex());
  const auto                  iterator = GetMemoryFileRegistry().find(fp);
  if(iterator == GetMemoryFileRegistry().end() || !iterator->second.writable)
  {
    return 0u;
  }

  FILE* const             stream = static_cast<FILE*>(const_cast<void*>(fp));
  const MemoryFileState& state  = iterator->second;
  if(state.append && std::fseek(stream, 0L, SEEK_END) != 0)
  {
    return 0u;
  }

  const long position = std::ftell(stream);
  if(position < 0L || static_cast<size_t>(position) > state.capacity)
  {
    return 0u;
  }

  const size_t available       = state.capacity - static_cast<size_t>(position);
  const size_t writableElements = std::min(elementCount, available / elementSize);
  return std::fwrite(buffer, elementSize, writableElements, stream);
}

int MemFSeek(const void* fp, long offset, int origin)
{
  if(origin != SEEK_SET && origin != SEEK_CUR && origin != SEEK_END)
  {
    return -1;
  }

  std::lock_guard<std::mutex> lock(GetMemoryFileRegistryMutex());
  const auto                  iterator = GetMemoryFileRegistry().find(fp);
  if(iterator == GetMemoryFileRegistry().end())
  {
    return -1;
  }

  FILE* const stream           = static_cast<FILE*>(const_cast<void*>(fp));
  const long  originalPosition = std::ftell(stream);
  if(originalPosition < 0L || std::fseek(stream, offset, origin) != 0)
  {
    return -1;
  }

  const long newPosition = std::ftell(stream);
  if(newPosition < 0L || static_cast<size_t>(newPosition) > iterator->second.capacity)
  {
    static_cast<void>(std::fseek(stream, originalPosition, SEEK_SET));
    return -1;
  }

  return 0;
}

long MemFTell(const void* fp)
{
  std::lock_guard<std::mutex> lock(GetMemoryFileRegistryMutex());
  if(GetMemoryFileRegistry().find(fp) == GetMemoryFileRegistry().end())
  {
    return -1L;
  }

  return std::ftell(static_cast<FILE*>(const_cast<void*>(fp)));
}

int MemFEof(const void* fp)
{
  std::lock_guard<std::mutex> lock(GetMemoryFileRegistryMutex());
  if(GetMemoryFileRegistry().find(fp) == GetMemoryFileRegistry().end())
  {
    return 1;
  }

  return std::feof(static_cast<FILE*>(const_cast<void*>(fp)));
}
