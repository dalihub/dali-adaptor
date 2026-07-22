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

// CLASS HEADER
#include <dali/internal/thread/common/thread-settings-impl.h>

// EXTERNAL INCLUDES
#include <windows.h>
#include <processthreadsapi.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const int32_t gMainThreadId = static_cast<int32_t>(::GetCurrentThreadId());
} // unnamed namespace

namespace ThreadSettings
{
void SetThreadName(const std::string& threadName)
{
  if(threadName.empty())
  {
    return;
  }

  int characterCount = MultiByteToWideChar(CP_UTF8, 0, threadName.c_str(), -1, nullptr, 0);
  if(characterCount <= 0)
  {
    return;
  }

  std::wstring wideThreadName(static_cast<size_t>(characterCount), L'\0');
  if(MultiByteToWideChar(CP_UTF8, 0, threadName.c_str(), -1, wideThreadName.data(), characterCount) <= 0)
  {
    return;
  }

  using SetThreadDescriptionFunction = HRESULT(WINAPI*)(HANDLE, PCWSTR);
  HMODULE kernel32                    = GetModuleHandleW(L"Kernel32.dll");
  auto    setThreadDescription        = kernel32 ? reinterpret_cast<SetThreadDescriptionFunction>(GetProcAddress(kernel32, "SetThreadDescription")) : nullptr;
  if(setThreadDescription)
  {
    setThreadDescription(GetCurrentThread(), wideThreadName.c_str());
  }
}

int32_t GetThreadId()
{
  return static_cast<int32_t>(GetCurrentThreadId());
}

int32_t GetMainThreadId()
{
  return gMainThreadId;
}

} // namespace ThreadSettings
} // namespace Adaptor
} // namespace Internal
} // namespace Dali
