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
 *
 */

// CLASS HEADER
#include <dali/internal/clipboard/common/clipboard-impl.h>

// EXTERNAL INCLUDES
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// WinUser.h defines CreateWindow as a macro (CreateWindowA/W). It collides with the
// WindowBase::CreateWindow() virtual method that is pulled in transitively by the DALi
// headers included below. Undef it here, mirroring input-method-context-impl-win.cpp.
#ifdef CreateWindow
#undef CreateWindow
#endif

#include <algorithm>
#include <climits>
#include <cstring>
#include <deque>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/clipboard/common/clipboard-factory.h>
#include <dali/internal/window-system/windows/platform-implement-win.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/public-api/object/any.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
constexpr const char* MIME_TYPE_TEXT_PLAIN_WINDOWS = "text/plain;charset=utf-8";
constexpr uint32_t    INVALID_DATA_ID               = 0u;

bool IsPlainTextMimeType(const std::string& mimeType)
{
  return mimeType == MIME_TYPE_TEXT_PLAIN_WINDOWS;
}

HWND GetApplicationWindow()
{
  if(!Adaptor::IsAvailable())
  {
    return nullptr;
  }

  Adaptor& adaptorImpl = Adaptor::GetImplementation(Adaptor::Get());
  Any      nativeWindow = adaptorImpl.GetNativeWindowHandle();
  if(nativeWindow.Empty() || !nativeWindow.IsType<WinWindowHandle>())
  {
    return nullptr;
  }

  HWND window = reinterpret_cast<HWND>(AnyCast<WinWindowHandle>(nativeWindow));
  return window && IsWindow(window) != 0 ? window : nullptr;
}

bool ConvertUtf8ToWide(const std::string& utf8, std::wstring& wide)
{
  wide.clear();
  if(utf8.empty())
  {
    return true;
  }
  if(utf8.size() > static_cast<size_t>(INT_MAX))
  {
    return false;
  }

  const int inputLength    = static_cast<int>(utf8.size());
  const int requiredLength = MultiByteToWideChar(CP_UTF8,
                                                 MB_ERR_INVALID_CHARS,
                                                 utf8.data(),
                                                 inputLength,
                                                 nullptr,
                                                 0);
  if(requiredLength <= 0)
  {
    return false;
  }

  wide.resize(static_cast<size_t>(requiredLength));
  return MultiByteToWideChar(CP_UTF8,
                             MB_ERR_INVALID_CHARS,
                             utf8.data(),
                             inputLength,
                             &wide[0],
                             requiredLength) == requiredLength;
}

bool ConvertWideToUtf8(const wchar_t* wide, size_t wideCapacity, std::string& utf8)
{
  utf8.clear();
  if(!wide)
  {
    return false;
  }

  size_t wideLength = 0u;
  while(wideLength < wideCapacity && wide[wideLength] != L'\0')
  {
    ++wideLength;
  }
  if(wideLength == wideCapacity)
  {
    return false;
  }
  if(wideLength == 0u)
  {
    return true;
  }
  if(wideLength > static_cast<size_t>(INT_MAX))
  {
    return false;
  }

  const int inputLength    = static_cast<int>(wideLength);
  const int requiredLength = WideCharToMultiByte(CP_UTF8,
                                                 WC_ERR_INVALID_CHARS,
                                                 wide,
                                                 inputLength,
                                                 nullptr,
                                                 0,
                                                 nullptr,
                                                 nullptr);
  if(requiredLength <= 0)
  {
    return false;
  }

  utf8.resize(static_cast<size_t>(requiredLength));
  return WideCharToMultiByte(CP_UTF8,
                             WC_ERR_INVALID_CHARS,
                             wide,
                             inputLength,
                             &utf8[0],
                             requiredLength,
                             nullptr,
                             nullptr) == requiredLength;
}

class ClipboardScope
{
public:
  explicit ClipboardScope(HWND ownerWindow)
  : mOwnerWindow(ownerWindow)
  {
  }

  ClipboardScope(const ClipboardScope&)            = delete;
  ClipboardScope& operator=(const ClipboardScope&) = delete;
  ClipboardScope(ClipboardScope&&)                 = delete;
  ClipboardScope& operator=(ClipboardScope&&)      = delete;

  bool Open()
  {
    if(mOpened)
    {
      return true;
    }

    if(!mOwnerWindow || IsWindow(mOwnerWindow) == 0)
    {
      mLastError = ERROR_INVALID_WINDOW_HANDLE;
      return false;
    }

    SetLastError(ERROR_SUCCESS);
    mOpened    = OpenClipboard(mOwnerWindow) != 0;
    mLastError = mOpened ? ERROR_SUCCESS : GetLastError();
    return mOpened;
  }

  DWORD GetLastErrorCode() const
  {
    return mLastError;
  }

  ~ClipboardScope()
  {
    if(mOpened)
    {
      CloseClipboard();
    }
  }

private:
  HWND  mOwnerWindow{nullptr};
  bool  mOpened{false};
  DWORD mLastError{ERROR_SUCCESS};
};

void LogClipboardError(const char* operation, const std::string& mimeType, size_t byteLength, DWORD errorCode)
{
  DALI_LOG_ERROR("Windows clipboard %s failed, mime:%s, bytes:%zu, error:%lu\n",
                 operation,
                 mimeType.c_str(),
                 byteLength,
                 static_cast<unsigned long>(errorCode));
}
} // unnamed namespace

struct Clipboard::Impl
{
  explicit Impl(HWND applicationWindow)
  : mApplicationWindow(applicationWindow)
  {
  }

  struct PendingData
  {
    uint32_t    requestId{INVALID_DATA_ID};
    std::string mimeType;
    std::string content;
    bool        succeeded{false};
  };

  uint32_t GenerateDataId()
  {
    ++mDataId;
    if(DALI_UNLIKELY(mDataId == INVALID_DATA_ID))
    {
      ++mDataId;
    }
    return mDataId;
  }

  bool HasLocalType(const std::string& mimeType) const
  {
    return std::find(mMimeTypes.begin(), mMimeTypes.end(), mimeType) != mMimeTypes.end();
  }

  void InvalidateStaleLocalCache()
  {
    if(!mHasClipboardSequenceNumber)
    {
      return;
    }

    const DWORD sequenceNumber = GetClipboardSequenceNumber();
    if(sequenceNumber != 0u && sequenceNumber != mClipboardSequenceNumber)
    {
      mMimeTypes.clear();
      mDatas.clear();
      mLastType.clear();
      mClipboardSequenceNumber    = 0u;
      mHasClipboardSequenceNumber = false;
    }
  }

  void RememberClipboardSequenceNumber(DWORD sequenceNumber)
  {
    if(sequenceNumber != 0u)
    {
      mClipboardSequenceNumber    = sequenceNumber;
      mHasClipboardSequenceNumber = true;
    }
    else
    {
      mClipboardSequenceNumber    = 0u;
      mHasClipboardSequenceNumber = false;
    }
  }

  void RememberCurrentClipboardSequenceNumber()
  {
    RememberClipboardSequenceNumber(GetClipboardSequenceNumber());
  }

  void UpdateData(const std::string& mimeType, const std::string& data, bool clearBuffer)
  {
    if(clearBuffer)
    {
      mMimeTypes.clear();
      mDatas.clear();
    }

    if(!HasLocalType(mimeType))
    {
      mMimeTypes.push_back(mimeType);
    }
    mDatas[mimeType] = data;
  }

  void UpdateLocalCache(const std::string& mimeType, const std::string& data)
  {
    if(mLastType != mimeType && !mMultiSelectionTimeout)
    {
      UpdateData(mimeType, data, HasLocalType(mimeType));
    }
    else
    {
      UpdateData(mimeType, data, true);
    }
    mLastType = mimeType;
  }

  void EmitDataSelectionSignals(const char* mimeType)
  {
    const char* safeMimeType = mimeType ? mimeType : "";
    mDataOfferedSignal.Emit(Dali::String(safeMimeType));
    mDataSelectedSignal.Emit(safeMimeType);
  }

  HWND GetClipboardOwnerWindow()
  {
    if(!mApplicationWindow || IsWindow(mApplicationWindow) == 0)
    {
      mApplicationWindow = GetApplicationWindow();
    }
    return mApplicationWindow;
  }

  bool WritePlainText(const std::string& mimeType, const std::string& data, DWORD& sequenceNumber)
  {
    sequenceNumber = 0u;

    std::wstring wide;
    if(!ConvertUtf8ToWide(data, wide))
    {
      DALI_LOG_ERROR("Windows clipboard UTF-8 conversion failed, mime:%s, bytes:%zu\n",
                     mimeType.c_str(),
                     data.size());
      return false;
    }

    const size_t maxCharacterCount = (std::numeric_limits<size_t>::max)() / sizeof(wchar_t) - 1u;
    if(wide.size() > maxCharacterCount)
    {
      return false;
    }

    const size_t byteCount = (wide.size() + 1u) * sizeof(wchar_t);
    HGLOBAL      memory    = GlobalAlloc(GMEM_MOVEABLE, byteCount);
    if(!memory)
    {
      LogClipboardError("GlobalAlloc", mimeType, data.size(), GetLastError());
      return false;
    }

    wchar_t* destination = static_cast<wchar_t*>(GlobalLock(memory));
    if(!destination)
    {
      LogClipboardError("GlobalLock", mimeType, data.size(), GetLastError());
      GlobalFree(memory);
      return false;
    }

    std::memcpy(destination, wide.c_str(), byteCount);
    GlobalUnlock(memory);

    ClipboardScope clipboard(GetClipboardOwnerWindow());
    if(!clipboard.Open())
    {
      LogClipboardError("OpenClipboard", mimeType, data.size(), clipboard.GetLastErrorCode());
      GlobalFree(memory);
      return false;
    }
    if(!EmptyClipboard())
    {
      LogClipboardError("EmptyClipboard", mimeType, data.size(), GetLastError());
      GlobalFree(memory);
      return false;
    }

    if(!SetClipboardData(CF_UNICODETEXT, memory))
    {
      LogClipboardError("SetClipboardData", mimeType, data.size(), GetLastError());
      GlobalFree(memory);
      return false;
    }

    sequenceNumber = GetClipboardSequenceNumber();

    // Ownership of memory transfers to the system after SetClipboardData().
    return true;
  }

  bool ReadPlainText(std::string& content)
  {
    content.clear();

    ClipboardScope clipboard(GetClipboardOwnerWindow());
    if(!clipboard.Open())
    {
      LogClipboardError("OpenClipboard", MIME_TYPE_TEXT_PLAIN_WINDOWS, 0u, clipboard.GetLastErrorCode());
      return false;
    }

    HANDLE memory = GetClipboardData(CF_UNICODETEXT);
    if(!memory)
    {
      LogClipboardError("GetClipboardData", MIME_TYPE_TEXT_PLAIN_WINDOWS, 0u, GetLastError());
      return false;
    }

    const SIZE_T byteCount = GlobalSize(memory);
    if(byteCount < sizeof(wchar_t) ||
       byteCount % sizeof(wchar_t) != 0u)
    {
      DALI_LOG_ERROR("Windows clipboard CF_UNICODETEXT has an invalid size.\n");
      return false;
    }

    const wchar_t* source = static_cast<const wchar_t*>(GlobalLock(memory));
    if(!source)
    {
      LogClipboardError("GlobalLock", MIME_TYPE_TEXT_PLAIN_WINDOWS, 0u, GetLastError());
      return false;
    }

    const size_t wideCapacity = static_cast<size_t>(byteCount / sizeof(wchar_t));
    const bool   converted    = ConvertWideToUtf8(source, wideCapacity, content);
    GlobalUnlock(memory);
    if(!converted)
    {
      DALI_LOG_ERROR("Windows clipboard UTF-16 conversion failed.\n");
      content.clear();
    }
    return converted;
  }

  bool HasType(const std::string& mimeType)
  {
    InvalidateStaleLocalCache();

    if(IsPlainTextMimeType(mimeType))
    {
      return IsClipboardFormatAvailable(CF_UNICODETEXT) != 0;
    }
    return HasLocalType(mimeType);
  }

  bool SetData(const Dali::ClipboardData& clipboardData)
  {
    const std::string mimeType = clipboardData.GetMimeType().CStr();
    const std::string data     = clipboardData.GetContent().CStr();
    if(mimeType.empty() || data.empty())
    {
      return false;
    }

    InvalidateStaleLocalCache();
    DWORD clipboardSequenceNumber = 0u;
    if(IsPlainTextMimeType(mimeType) && !WritePlainText(mimeType, data, clipboardSequenceNumber))
    {
      return false;
    }

    UpdateLocalCache(mimeType, data);
    if(clipboardSequenceNumber != 0u)
    {
      RememberClipboardSequenceNumber(clipboardSequenceNumber);
    }
    else
    {
      RememberCurrentClipboardSequenceNumber();
    }

    mDataSentSignal.Emit(mimeType.c_str(), data.c_str());
    EmitDataSelectionSignals(mimeType.c_str());
    SetMultiSelectionTimeout();
    return true;
  }

  uint32_t QueueData(const std::string& mimeType, std::string content, bool succeeded)
  {
    const uint32_t requestId = GenerateDataId();
    mDataReceiveQueue.push_back(PendingData{requestId, mimeType, std::move(content), succeeded});

    if(!mDataReceiveTimer.IsRunning())
    {
      mDataReceiveTimer.Start();
    }
    return requestId;
  }

  uint32_t GetData(const std::string& mimeType)
  {
    if(mimeType.empty())
    {
      return INVALID_DATA_ID;
    }

    InvalidateStaleLocalCache();
    if(IsPlainTextMimeType(mimeType))
    {
      if(!IsClipboardFormatAvailable(CF_UNICODETEXT))
      {
        return INVALID_DATA_ID;
      }

      std::string content;
      const bool  succeeded = ReadPlainText(content) && !content.empty();
      return QueueData(mimeType, std::move(content), succeeded);
    }

    const auto data = mDatas.find(mimeType);
    if(data == mDatas.end())
    {
      return INVALID_DATA_ID;
    }
    return QueueData(mimeType, data->second, true);
  }

  bool OnReceiveData()
  {
    while(!mDataReceiveQueue.empty())
    {
      PendingData item = std::move(mDataReceiveQueue.front());
      mDataReceiveQueue.pop_front();

      if(item.succeeded)
      {
        mDataReceivedSignal.Emit(item.requestId, item.mimeType.c_str(), item.content.c_str());
      }
      else
      {
        mDataReceivedSignal.Emit(item.requestId, "", "");
      }
    }
    return false;
  }

  void SetMultiSelectionTimeout()
  {
    mMultiSelectionTimeout = false;
    if(mMultiSelectionTimeoutTimer.IsRunning())
    {
      mMultiSelectionTimeoutTimer.Stop();
    }
    mMultiSelectionTimeoutTimer.Start();
  }

  bool OnMultiSelectionTimeout()
  {
    mMultiSelectionTimeout = true;
    return false;
  }

  HWND mApplicationWindow{nullptr};

  uint32_t mDataId{INVALID_DATA_ID};
  std::string mLastType;
  std::vector<std::string> mMimeTypes;
  std::map<std::string, std::string> mDatas;
  std::deque<PendingData> mDataReceiveQueue;

  DWORD mClipboardSequenceNumber{0u};
  bool  mHasClipboardSequenceNumber{false};

  Clipboard::DataSentSignalType     mDataSentSignal{};
  Clipboard::DataReceivedSignalType mDataReceivedSignal{};
  Clipboard::DataOfferedSignalType  mDataOfferedSignal{};
  Clipboard::DataSelectedSignalType mDataSelectedSignal{};

  Dali::Timer mDataReceiveTimer{};
  Dali::Timer mMultiSelectionTimeoutTimer{};
  bool        mMultiSelectionTimeout{false};
};

Clipboard::Clipboard(Impl* impl)
: mImpl(impl)
{
  mImpl->mDataReceiveTimer = Dali::Timer::New(10u);
  mImpl->mDataReceiveTimer.TickSignal().Connect(this, &Clipboard::OnReceiveData);

  mImpl->mMultiSelectionTimeoutTimer = Dali::Timer::New(500u);
  mImpl->mMultiSelectionTimeoutTimer.TickSignal().Connect(this, &Clipboard::OnMultiSelectionTimeout);
}

Clipboard::~Clipboard()
{
  FinalizeGetDataCallbacks();

  if(mImpl->mDataReceiveTimer)
  {
    mImpl->mDataReceiveTimer.TickSignal().Disconnect(this, &Clipboard::OnReceiveData);
    if(mImpl->mDataReceiveTimer.IsRunning())
    {
      mImpl->mDataReceiveTimer.Stop();
    }
  }

  if(mImpl->mMultiSelectionTimeoutTimer)
  {
    mImpl->mMultiSelectionTimeoutTimer.TickSignal().Disconnect(this, &Clipboard::OnMultiSelectionTimeout);
    if(mImpl->mMultiSelectionTimeoutTimer.IsRunning())
    {
      mImpl->mMultiSelectionTimeoutTimer.Stop();
    }
  }

  delete mImpl;
}

class ClipboardFactoryWindows : public ClipboardFactory
{
public:
  Dali::Clipboard CreateClipboard() override
  {
    Dali::Clipboard clipboard;
    Dali::SingletonService service(SingletonService::Get());
    if(service)
    {
      Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::Clipboard));
      if(handle)
      {
        clipboard = Dali::Clipboard(dynamic_cast<Clipboard*>(handle.GetObjectPtr()));
      }
      else
      {
        clipboard = Dali::Clipboard(new Clipboard(new Clipboard::Impl(GetApplicationWindow())));
        service.Register(typeid(Dali::Clipboard), clipboard);
      }
    }
    return clipboard;
  }
};

std::unique_ptr<ClipboardFactory> GetClipboardFactory()
{
  return std::make_unique<ClipboardFactoryWindows>();
}

bool Clipboard::IsAvailable()
{
  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::Clipboard));
    if(handle)
    {
      return true;
    }
  }
  return false;
}

Clipboard::DataSentSignalType& Clipboard::DataSentSignal()
{
  return mImpl->mDataSentSignal;
}

Clipboard::DataReceivedSignalType& Clipboard::DataReceivedSignal()
{
  return mImpl->mDataReceivedSignal;
}

Clipboard::DataOfferedSignalType& Clipboard::DataOfferedSignal()
{
  return mImpl->mDataOfferedSignal;
}

Clipboard::DataSelectedSignalType& Clipboard::DataSelectedSignal()
{
  return mImpl->mDataSelectedSignal;
}

bool Clipboard::HasType(const std::string& mimeType)
{
  return mImpl->HasType(mimeType);
}

bool Clipboard::SetData(const Dali::ClipboardData& clipboardData)
{
  return mImpl->SetData(clipboardData);
}

uint32_t Clipboard::GetData(const std::string& mimeType)
{
  return mImpl->GetData(mimeType);
}

uint32_t Clipboard::GetItemCount()
{
  bool isItem = HasType(MIME_TYPE_TEXT_PLAIN) || HasType(MIME_TYPE_HTML) || HasType(MIME_TYPE_TEXT_URI);
  return isItem ? 1u : 0u;
}

void Clipboard::ShowClipboard()
{
}

void Clipboard::HideClipboard(bool)
{
}

bool Clipboard::IsVisible() const
{
  return false;
}

bool Clipboard::OnReceiveData()
{
  return mImpl->OnReceiveData();
}

bool Clipboard::OnMultiSelectionTimeout()
{
  return mImpl->OnMultiSelectionTimeout();
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
