/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// CLASS HEADER
#include <dali/internal/drag-and-drop/windows/drag-and-drop-impl-win.h>

// EXTERNAL INCLUDES
#include <algorithm>
#include <climits>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/drag-and-drop/common/drag-and-drop-factory.h>
#include <dali/internal/window-system/windows/platform-implement-win.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
constexpr const char* TEXT_MIME_TYPE = "text/plain";
constexpr const char* ALL_MIME_TYPES = "*/*";
constexpr const wchar_t* SHADOW_WINDOW_PROPERTY = L"DALI_DRAG_AND_DROP_SHADOW";
constexpr int SHADOW_OFFSET = 8;

DragAndDropWin* gDragAndDropWin = nullptr;

CLIPFORMAT GetTextMimeFormat()
{
  static const CLIPFORMAT format = static_cast<CLIPFORMAT>(RegisterClipboardFormatW(L"text/plain"));
  return format;
}

FORMATETC MakeFormat(CLIPFORMAT format)
{
  FORMATETC result{};
  result.cfFormat = format;
  result.dwAspect = DVASPECT_CONTENT;
  result.lindex   = -1;
  result.tymed    = TYMED_HGLOBAL;
  return result;
}

bool IsSupportedMimeType(const std::string& mimeType)
{
  return mimeType == TEXT_MIME_TYPE || mimeType == ALL_MIME_TYPES;
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
  const int requiredLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), inputLength, nullptr, 0);
  if(requiredLength <= 0)
  {
    return false;
  }

  wide.resize(static_cast<size_t>(requiredLength));
  return MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), inputLength, &wide[0], requiredLength) == requiredLength;
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
  const int requiredLength = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide, inputLength, nullptr, 0, nullptr, nullptr);
  if(requiredLength <= 0)
  {
    return false;
  }

  utf8.resize(static_cast<size_t>(requiredLength));
  return WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide, inputLength, &utf8[0], requiredLength, nullptr, nullptr) == requiredLength;
}

class FormatEnumerator final : public IEnumFORMATETC
{
public:
  explicit FormatEnumerator(std::vector<FORMATETC> formats)
  : mFormats(std::move(formats))
  {
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override
  {
    if(!object)
    {
      return E_POINTER;
    }

    *object = nullptr;
    if(iid == IID_IUnknown || iid == IID_IEnumFORMATETC)
    {
      *object = static_cast<IEnumFORMATETC*>(this);
      AddRef();
      return S_OK;
    }
    return E_NOINTERFACE;
  }

  ULONG STDMETHODCALLTYPE AddRef() override
  {
    return static_cast<ULONG>(InterlockedIncrement(&mReferenceCount));
  }

  ULONG STDMETHODCALLTYPE Release() override
  {
    const ULONG referenceCount = static_cast<ULONG>(InterlockedDecrement(&mReferenceCount));
    if(referenceCount == 0u)
    {
      delete this;
    }
    return referenceCount;
  }

  HRESULT STDMETHODCALLTYPE Next(ULONG count, FORMATETC* formats, ULONG* fetched) override
  {
    if(!formats || (count != 1u && !fetched))
    {
      return E_POINTER;
    }

    ULONG copied = 0u;
    while(copied < count && mIndex < mFormats.size())
    {
      formats[copied]     = mFormats[mIndex];
      formats[copied].ptd = nullptr;
      ++copied;
      ++mIndex;
    }

    if(fetched)
    {
      *fetched = copied;
    }
    return copied == count ? S_OK : S_FALSE;
  }

  HRESULT STDMETHODCALLTYPE Skip(ULONG count) override
  {
    const size_t remaining = mFormats.size() - mIndex;
    const size_t skipped   = std::min(static_cast<size_t>(count), remaining);
    mIndex += skipped;
    return skipped == count ? S_OK : S_FALSE;
  }

  HRESULT STDMETHODCALLTYPE Reset() override
  {
    mIndex = 0u;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE Clone(IEnumFORMATETC** enumerator) override
  {
    if(!enumerator)
    {
      return E_POINTER;
    }

    auto* clone  = new FormatEnumerator(mFormats);
    clone->mIndex = mIndex;
    *enumerator   = clone;
    return S_OK;
  }

private:
  ~FormatEnumerator() = default;

private:
  LONG                    mReferenceCount{1};
  std::vector<FORMATETC>  mFormats;
  size_t                  mIndex{0u};
};

class DataObject final : public IDataObject
{
public:
  explicit DataObject(const Dali::DragAndDrop::DragData& data)
  {
    for(uint32_t index = 0u; index < data.GetDataCount(); ++index)
    {
      if(data.GetMimeType(index) == TEXT_MIME_TYPE)
      {
        mText = data.GetData(index).CStr();
        mHasText = true;
        break;
      }
    }
  }

  bool HasText() const
  {
    return mHasText;
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override
  {
    if(!object)
    {
      return E_POINTER;
    }

    *object = nullptr;
    if(iid == IID_IUnknown || iid == IID_IDataObject)
    {
      *object = static_cast<IDataObject*>(this);
      AddRef();
      return S_OK;
    }
    return E_NOINTERFACE;
  }

  ULONG STDMETHODCALLTYPE AddRef() override
  {
    return static_cast<ULONG>(InterlockedIncrement(&mReferenceCount));
  }

  ULONG STDMETHODCALLTYPE Release() override
  {
    const ULONG referenceCount = static_cast<ULONG>(InterlockedDecrement(&mReferenceCount));
    if(referenceCount == 0u)
    {
      delete this;
    }
    return referenceCount;
  }

  HRESULT STDMETHODCALLTYPE GetData(FORMATETC* format, STGMEDIUM* medium) override
  {
    if(!format || !medium)
    {
      return E_POINTER;
    }

    const HRESULT queryResult = QueryGetData(format);
    if(FAILED(queryResult))
    {
      return queryResult;
    }

    medium->tymed          = TYMED_HGLOBAL;
    medium->hGlobal        = nullptr;
    medium->pUnkForRelease = nullptr;

    if(format->cfFormat == GetTextMimeFormat())
    {
      const SIZE_T byteCount = mText.size() + 1u;
      HGLOBAL      memory    = GlobalAlloc(GMEM_MOVEABLE, byteCount);
      if(!memory)
      {
        return STG_E_MEDIUMFULL;
      }

      void* destination = GlobalLock(memory);
      if(!destination)
      {
        GlobalFree(memory);
        return STG_E_MEDIUMFULL;
      }

      std::memcpy(destination, mText.c_str(), byteCount);
      GlobalUnlock(memory);
      medium->hGlobal = memory;
      return S_OK;
    }

    std::wstring wide;
    if(!ConvertUtf8ToWide(mText, wide))
    {
      return DV_E_FORMATETC;
    }

    const SIZE_T byteCount = (wide.size() + 1u) * sizeof(wchar_t);
    HGLOBAL      memory    = GlobalAlloc(GMEM_MOVEABLE, byteCount);
    if(!memory)
    {
      return STG_E_MEDIUMFULL;
    }

    void* destination = GlobalLock(memory);
    if(!destination)
    {
      GlobalFree(memory);
      return STG_E_MEDIUMFULL;
    }
    std::memcpy(destination, wide.c_str(), byteCount);
    GlobalUnlock(memory);

    medium->hGlobal = memory;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC*, STGMEDIUM*) override
  {
    return DATA_E_FORMATETC;
  }

  HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC* format) override
  {
    if(!format)
    {
      return E_POINTER;
    }
    if((format->tymed & TYMED_HGLOBAL) == 0u)
    {
      return DV_E_TYMED;
    }
    if(format->dwAspect != DVASPECT_CONTENT)
    {
      return DV_E_DVASPECT;
    }
    if(format->lindex != -1)
    {
      return DV_E_LINDEX;
    }
    if(format->ptd != nullptr)
    {
      return DV_E_DVTARGETDEVICE;
    }
    if(!mHasText || (format->cfFormat != CF_UNICODETEXT && format->cfFormat != GetTextMimeFormat()))
    {
      return DV_E_FORMATETC;
    }
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC*, FORMATETC* output) override
  {
    if(!output)
    {
      return E_POINTER;
    }
    output->ptd = nullptr;
    return DATA_S_SAMEFORMATETC;
  }

  HRESULT STDMETHODCALLTYPE SetData(FORMATETC*, STGMEDIUM*, BOOL) override
  {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD direction, IEnumFORMATETC** enumerator) override
  {
    if(!enumerator)
    {
      return E_POINTER;
    }
    *enumerator = nullptr;

    if(direction != DATADIR_GET)
    {
      return E_NOTIMPL;
    }

    std::vector<FORMATETC> formats;
    if(mHasText)
    {
      formats.push_back(MakeFormat(CF_UNICODETEXT));
      formats.push_back(MakeFormat(GetTextMimeFormat()));
    }
    *enumerator = new FormatEnumerator(std::move(formats));
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC*, DWORD, IAdviseSink*, DWORD*) override
  {
    return OLE_E_ADVISENOTSUPPORTED;
  }

  HRESULT STDMETHODCALLTYPE DUnadvise(DWORD) override
  {
    return OLE_E_ADVISENOTSUPPORTED;
  }

  HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA**) override
  {
    return OLE_E_ADVISENOTSUPPORTED;
  }

private:
  ~DataObject() = default;

private:
  LONG        mReferenceCount{1};
  std::string mText;
  bool        mHasText{false};
};

class DropSource final : public IDropSource
{
public:
  explicit DropSource(DragAndDropWin& owner)
  : mOwner(owner)
  {
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override
  {
    if(!object)
    {
      return E_POINTER;
    }

    *object = nullptr;
    if(iid == IID_IUnknown || iid == IID_IDropSource)
    {
      *object = static_cast<IDropSource*>(this);
      AddRef();
      return S_OK;
    }
    return E_NOINTERFACE;
  }

  ULONG STDMETHODCALLTYPE AddRef() override
  {
    return static_cast<ULONG>(InterlockedIncrement(&mReferenceCount));
  }

  ULONG STDMETHODCALLTYPE Release() override
  {
    const ULONG referenceCount = static_cast<ULONG>(InterlockedDecrement(&mReferenceCount));
    if(referenceCount == 0u)
    {
      delete this;
    }
    return referenceCount;
  }

  HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL escapePressed, DWORD keyState) override
  {
    mOwner.UpdateShadow();
    if(escapePressed)
    {
      return DRAGDROP_S_CANCEL;
    }
    return (keyState & MK_LBUTTON) != 0u ? S_OK : DRAGDROP_S_DROP;
  }

  HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD) override
  {
    return DRAGDROP_S_USEDEFAULTCURSORS;
  }

private:
  ~DropSource() = default;

private:
  LONG            mReferenceCount{1};
  DragAndDropWin& mOwner;
};

HWND GetWindowHandle(const Dali::Window& window)
{
  if(!window)
  {
    return nullptr;
  }
  return reinterpret_cast<HWND>(AnyCast<WinWindowHandle>(window.GetNativeHandle()));
}

Vector2 GetWindowPosition(HWND window, POINTL point)
{
  POINT clientPoint{point.x, point.y};
  ScreenToClient(window, &clientPoint);
  return Vector2(static_cast<float>(clientPoint.x), static_cast<float>(clientPoint.y));
}

bool GetActorPosition(HWND window, const Dali::Actor& actor, POINTL point, Vector2& localPosition)
{
  const Vector2 windowPosition = GetWindowPosition(window, point);
  return actor.ScreenToLocal(localPosition.x, localPosition.y, windowPosition.x, windowPosition.y);
}

bool Contains(HWND window, const Dali::Actor& actor, POINTL point)
{
  Vector2 localPosition;
  if(!GetActorPosition(window, actor, point, localPosition))
  {
    return false;
  }

  const Vector2 size = actor.GetProperty<Vector2>(Dali::Actor::Property::SIZE);
  return localPosition.x >= 0.0f &&
         localPosition.y >= 0.0f &&
         localPosition.x < size.width &&
         localPosition.y < size.height;
}

} // unnamed namespace

DropTargetWin::DropTargetWin(DragAndDropWin& owner, HWND window)
: mOwner(owner),
  mWindow(window)
{
}

HRESULT STDMETHODCALLTYPE DropTargetWin::QueryInterface(REFIID iid, void** object)
{
  if(!object)
  {
    return E_POINTER;
  }

  *object = nullptr;
  if(iid == IID_IUnknown || iid == IID_IDropTarget)
  {
    *object = static_cast<IDropTarget*>(this);
    AddRef();
    return S_OK;
  }
  return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE DropTargetWin::AddRef()
{
  return static_cast<ULONG>(InterlockedIncrement(&mReferenceCount));
}

ULONG STDMETHODCALLTYPE DropTargetWin::Release()
{
  const ULONG referenceCount = static_cast<ULONG>(InterlockedDecrement(&mReferenceCount));
  if(referenceCount == 0u)
  {
    delete this;
  }
  return referenceCount;
}

HRESULT STDMETHODCALLTYPE DropTargetWin::DragEnter(IDataObject* object, DWORD, POINTL point, DWORD* effect)
{
  return mOwner.OnDragEnter(mWindow, object, point, effect);
}

HRESULT STDMETHODCALLTYPE DropTargetWin::DragOver(DWORD, POINTL point, DWORD* effect)
{
  return mOwner.OnDragOver(mWindow, point, effect);
}

HRESULT STDMETHODCALLTYPE DropTargetWin::DragLeave()
{
  return mOwner.OnDragLeave(mWindow);
}

HRESULT STDMETHODCALLTYPE DropTargetWin::Drop(IDataObject* object, DWORD, POINTL point, DWORD* effect)
{
  return mOwner.OnDrop(mWindow, object, point, effect);
}

DragAndDropWin::DragAndDropWin()
{
  const HRESULT result = OleInitialize(nullptr);
  mOleInitialized      = SUCCEEDED(result);
  if(!mOleInitialized)
  {
    DALI_LOG_ERROR("Windows drag-and-drop OLE initialization failed: 0x%08lx\n", static_cast<unsigned long>(result));
  }
  gDragAndDropWin      = this;
}

DragAndDropWin::~DragAndDropWin()
{
  RestoreShadowWindow();
  RevokeWindows();
  if(gDragAndDropWin == this)
  {
    gDragAndDropWin = nullptr;
  }
  if(mOleInitialized)
  {
    OleUninitialize();
  }
}

void DragAndDropWin::SendData(void*)
{
}

void DragAndDropWin::ReceiveData(void*)
{
}

bool DragAndDropWin::CalculateDragEvent(void*)
{
  return false;
}

bool DragAndDropWin::CalculateViewRegion(void*)
{
  return false;
}

bool DragAndDropWin::RegisterWindow(HWND window)
{
  if(!window || !mOleInitialized)
  {
    return false;
  }

  auto found = mNativeTargets.find(window);
  if(found != mNativeTargets.end())
  {
    ++found->second.references;
    return true;
  }

  auto* target = new DropTargetWin(*this, window);
  const HRESULT result = RegisterDragDrop(window, target);
  if(FAILED(result))
  {
    DALI_LOG_ERROR("RegisterDragDrop failed for HWND %p: 0x%08lx\n", window, static_cast<unsigned long>(result));
    target->Release();
    return false;
  }

  mNativeTargets.emplace(window, NativeTarget{target, 1u});
  return true;
}

void DragAndDropWin::ReleaseWindow(HWND window)
{
  auto found = mNativeTargets.find(window);
  if(found == mNativeTargets.end())
  {
    return;
  }

  if(found->second.references > 1u)
  {
    --found->second.references;
    return;
  }

  RevokeDragDrop(window);
  found->second.target->Release();
  mNativeTargets.erase(found);
}

void DragAndDropWin::RevokeWindows()
{
  for(auto& entry : mNativeTargets)
  {
    RevokeDragDrop(entry.first);
    entry.second.target->Release();
  }
  mNativeTargets.clear();
}

bool DragAndDropWin::AddListener(Dali::Actor target, const Dali::String& mimeType, DragCallback callback)
{
  const auto found = std::find_if(mActorTargets.begin(), mActorTargets.end(), [&target](const ActorTarget& item) {
    return item.target == target;
  });
  if(found != mActorTargets.end())
  {
    return false;
  }

  const Dali::Window parentWindow = Dali::Window::Get(target);
  const HWND         window       = GetWindowHandle(parentWindow);
  if(!parentWindow || !RegisterWindow(window))
  {
    return false;
  }

  mActorTargets.push_back(ActorTarget{target, window, mimeType.CStr(), callback});
  return true;
}

bool DragAndDropWin::RemoveListener(Dali::Actor target)
{
  const auto found = std::find_if(mActorTargets.begin(), mActorTargets.end(), [&target](const ActorTarget& item) {
    return item.target == target;
  });
  if(found == mActorTargets.end())
  {
    return false;
  }

  const ActorTarget removedTarget = *found;
  const bool        wasActive     = mActiveActor == target;
  mActorTargets.erase(found);
  if(wasActive)
  {
    mActiveActor.Reset();
    mActiveWindow = nullptr;
    EmitActorEvent(removedTarget, mLastPoint, Dali::DragAndDrop::DragType::LEAVE);
  }
  ReleaseWindow(removedTarget.window);
  return true;
}

bool DragAndDropWin::AddListener(Dali::Window target, const Dali::String& mimeType, DragCallback callback)
{
  const auto found = std::find_if(mWindowTargets.begin(), mWindowTargets.end(), [&target](const WindowTarget& item) {
    return item.target == target;
  });
  if(found != mWindowTargets.end())
  {
    return false;
  }

  const HWND window = GetWindowHandle(target);
  if(!RegisterWindow(window))
  {
    return false;
  }

  mWindowTargets.push_back(WindowTarget{target, window, mimeType.CStr(), callback});
  return true;
}

bool DragAndDropWin::RemoveListener(Dali::Window target)
{
  const auto found = std::find_if(mWindowTargets.begin(), mWindowTargets.end(), [&target](const WindowTarget& item) {
    return item.target == target;
  });
  if(found == mWindowTargets.end())
  {
    return false;
  }

  const WindowTarget removedTarget = *found;
  const bool         wasActive     = mActiveDropWindow == target;
  mWindowTargets.erase(found);
  if(wasActive)
  {
    mActiveDropWindow.Reset();
    mActiveWindow = nullptr;
    EmitWindowEvent(removedTarget, mLastPoint, Dali::DragAndDrop::DragType::LEAVE);
  }
  ReleaseWindow(removedTarget.window);
  return true;
}

bool DragAndDropWin::IsTextAvailable(IDataObject* object) const
{
  if(!object)
  {
    return false;
  }

  FORMATETC unicodeFormat = MakeFormat(CF_UNICODETEXT);
  if(SUCCEEDED(object->QueryGetData(&unicodeFormat)))
  {
    return true;
  }

  FORMATETC mimeFormat = MakeFormat(GetTextMimeFormat());
  return SUCCEEDED(object->QueryGetData(&mimeFormat));
}

bool DragAndDropWin::ReadTextData(IDataObject* object, Dali::String& data) const
{
  if(!object)
  {
    return false;
  }

  FORMATETC format       = MakeFormat(CF_UNICODETEXT);
  bool      unicodeText = SUCCEEDED(object->QueryGetData(&format));
  if(!unicodeText)
  {
    format = MakeFormat(GetTextMimeFormat());
  }

  STGMEDIUM medium{};
  if(FAILED(object->GetData(&format, &medium)))
  {
    return false;
  }

  bool success = false;
  if(medium.tymed == TYMED_HGLOBAL && medium.hGlobal)
  {
    const void* value = GlobalLock(medium.hGlobal);
    if(value)
    {
      const SIZE_T byteCount = GlobalSize(medium.hGlobal);
      std::string  utf8;
      if(unicodeText)
      {
        success = ConvertWideToUtf8(static_cast<const wchar_t*>(value), byteCount / sizeof(wchar_t), utf8);
      }
      else
      {
        const char* utf8Value = static_cast<const char*>(value);
        size_t      length    = 0u;
        while(length < byteCount && utf8Value[length] != '\0')
        {
          ++length;
        }
        success = length < byteCount;
        if(success)
        {
          utf8.assign(utf8Value, length);
        }
      }
      GlobalUnlock(medium.hGlobal);
      if(success)
      {
        data = utf8.c_str();
      }
    }
  }
  ReleaseStgMedium(&medium);
  return success;
}

DragAndDropWin::ActorTarget* DragAndDropWin::FindActorTarget(HWND window, POINTL point)
{
  auto found = std::find_if(mActorTargets.rbegin(), mActorTargets.rend(), [window, point](const ActorTarget& item) {
    return item.window == window && IsSupportedMimeType(item.mimeType) && Contains(window, item.target, point);
  });
  return found == mActorTargets.rend() ? nullptr : &(*found);
}

DragAndDropWin::WindowTarget* DragAndDropWin::FindWindowTarget(HWND window)
{
  const auto found = std::find_if(mWindowTargets.begin(), mWindowTargets.end(), [window](const WindowTarget& item) {
    return item.window == window && IsSupportedMimeType(item.mimeType);
  });
  return found == mWindowTargets.end() ? nullptr : &(*found);
}

void DragAndDropWin::EmitActorEvent(const ActorTarget& target, POINTL point, Dali::DragAndDrop::DragType type, const Dali::String& data)
{
  Vector2 localPosition;
  GetActorPosition(target.window, target.target, point, localPosition);
  DragEventBuilder event(type, localPosition);
  event.AddMimeType(TEXT_MIME_TYPE);
  if(!data.Empty())
  {
    event.SetData(data);
  }
  target.callback(event);
}

void DragAndDropWin::EmitWindowEvent(const WindowTarget& target, POINTL point, Dali::DragAndDrop::DragType type, const Dali::String& data)
{
  DragEventBuilder event(type, GetWindowPosition(target.window, point));
  event.AddMimeType(TEXT_MIME_TYPE);
  if(!data.Empty())
  {
    event.SetData(data);
  }
  target.callback(event);
}

void DragAndDropWin::EmitLeave(POINTL point)
{
  if(mActiveActor)
  {
    const Dali::Actor activeActor = mActiveActor;
    mActiveActor.Reset();
    const auto found = std::find_if(mActorTargets.begin(), mActorTargets.end(), [&activeActor](const ActorTarget& item) {
      return item.target == activeActor;
    });
    if(found != mActorTargets.end())
    {
      const ActorTarget target = *found;
      EmitActorEvent(target, point, Dali::DragAndDrop::DragType::LEAVE);
    }
  }

  if(mActiveDropWindow)
  {
    const Dali::Window activeWindow = mActiveDropWindow;
    mActiveDropWindow.Reset();
    const auto found = std::find_if(mWindowTargets.begin(), mWindowTargets.end(), [&activeWindow](const WindowTarget& item) {
      return item.target == activeWindow;
    });
    if(found != mWindowTargets.end())
    {
      const WindowTarget target = *found;
      EmitWindowEvent(target, point, Dali::DragAndDrop::DragType::LEAVE);
    }
  }
}

bool DragAndDropWin::UpdateActiveTarget(HWND window, POINTL point, bool emitMove)
{
  ActorTarget  actorTarget;
  WindowTarget windowTarget;
  bool         hasActorTarget  = false;
  bool         hasWindowTarget = false;

  if(ActorTarget* foundActor = FindActorTarget(window, point))
  {
    actorTarget    = *foundActor;
    hasActorTarget = true;
  }
  else if(WindowTarget* foundWindow = FindWindowTarget(window))
  {
    windowTarget    = *foundWindow;
    hasWindowTarget = true;
  }

  const bool actorChanged = hasActorTarget ? mActiveActor != actorTarget.target : static_cast<bool>(mActiveActor);
  const bool windowChanged = hasWindowTarget ? mActiveDropWindow != windowTarget.target : static_cast<bool>(mActiveDropWindow);
  if(actorChanged || windowChanged || mActiveWindow != window)
  {
    EmitLeave(point);
  }

  mActiveWindow = nullptr;
  if(hasActorTarget)
  {
    mActiveWindow = window;
    if(!mActiveActor)
    {
      mActiveActor = actorTarget.target;
      EmitActorEvent(actorTarget, point, Dali::DragAndDrop::DragType::ENTER);
    }
    else if(emitMove)
    {
      EmitActorEvent(actorTarget, point, Dali::DragAndDrop::DragType::MOVE);
    }
    return mActiveWindow == window && mActiveActor == actorTarget.target;
  }

  if(hasWindowTarget)
  {
    mActiveWindow = window;
    if(!mActiveDropWindow)
    {
      mActiveDropWindow = windowTarget.target;
      EmitWindowEvent(windowTarget, point, Dali::DragAndDrop::DragType::ENTER);
    }
    else if(emitMove)
    {
      EmitWindowEvent(windowTarget, point, Dali::DragAndDrop::DragType::MOVE);
    }
    return mActiveWindow == window && mActiveDropWindow == windowTarget.target;
  }
  return false;
}

bool DragAndDropWin::EmitDrop(HWND window, POINTL point, const Dali::String& data)
{
  ActorTarget*  actorTarget  = FindActorTarget(window, point);
  WindowTarget* windowTarget = actorTarget ? nullptr : FindWindowTarget(window);
  if(actorTarget)
  {
    const ActorTarget target = *actorTarget;
    mActiveActor.Reset();
    mActiveDropWindow.Reset();
    mActiveWindow = nullptr;
    EmitActorEvent(target, point, Dali::DragAndDrop::DragType::DROP, data);
    return true;
  }
  if(windowTarget)
  {
    const WindowTarget target = *windowTarget;
    mActiveActor.Reset();
    mActiveDropWindow.Reset();
    mActiveWindow = nullptr;
    EmitWindowEvent(target, point, Dali::DragAndDrop::DragType::DROP, data);
    return true;
  }

  EmitLeave(point);
  mActiveWindow = nullptr;
  return false;
}

HRESULT DragAndDropWin::OnDragEnter(HWND window, IDataObject* object, POINTL point, DWORD* effect)
{
  if(!effect)
  {
    return E_POINTER;
  }

  const bool copyAllowed = (*effect & DROPEFFECT_COPY) != 0u;
  mLastPoint     = point;
  mTextAvailable = IsTextAvailable(object);
  if(copyAllowed && mTextAvailable && UpdateActiveTarget(window, point, false))
  {
    *effect = DROPEFFECT_COPY;
  }
  else
  {
    EmitLeave(point);
    mActiveWindow = nullptr;
    *effect       = DROPEFFECT_NONE;
  }
  return S_OK;
}

HRESULT DragAndDropWin::OnDragOver(HWND window, POINTL point, DWORD* effect)
{
  if(!effect)
  {
    return E_POINTER;
  }

  const bool copyAllowed = (*effect & DROPEFFECT_COPY) != 0u;
  mLastPoint = point;
  if(copyAllowed && mTextAvailable && UpdateActiveTarget(window, point, true))
  {
    *effect = DROPEFFECT_COPY;
  }
  else
  {
    EmitLeave(point);
    mActiveWindow = nullptr;
    *effect       = DROPEFFECT_NONE;
  }
  return S_OK;
}

HRESULT DragAndDropWin::OnDragLeave(HWND)
{
  EmitLeave(mLastPoint);
  mActiveWindow   = nullptr;
  mTextAvailable  = false;
  return S_OK;
}

HRESULT DragAndDropWin::OnDrop(HWND window, IDataObject* object, POINTL point, DWORD* effect)
{
  if(!effect)
  {
    return E_POINTER;
  }

  const bool copyAllowed = (*effect & DROPEFFECT_COPY) != 0u;
  *effect    = DROPEFFECT_NONE;
  mLastPoint = point;

  Dali::String data;
  const bool hasTarget = copyAllowed && mTextAvailable && UpdateActiveTarget(window, point, false);
  if(hasTarget && ReadTextData(object, data) && EmitDrop(window, point, data))
  {
    *effect = DROPEFFECT_COPY;
  }
  else
  {
    EmitLeave(point);
    mActiveWindow = nullptr;
  }

  mTextAvailable = false;
  return S_OK;
}

void DragAndDropWin::PrepareShadowWindow()
{
  mShadowHandle = GetWindowHandle(mShadowWindow);
  if(!mShadowHandle)
  {
    return;
  }

  mShadowExtendedStyle = GetWindowLongPtrW(mShadowHandle, GWL_EXSTYLE);
  SetPropW(mShadowHandle, SHADOW_WINDOW_PROPERTY, reinterpret_cast<HANDLE>(mShadowHandle));
  SetWindowLongPtrW(mShadowHandle,
                    GWL_EXSTYLE,
                    mShadowExtendedStyle | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT);
}

void DragAndDropWin::RestoreShadowWindow()
{
  if(!mShadowHandle)
  {
    return;
  }

  ShowWindow(mShadowHandle, SW_HIDE);
  RemovePropW(mShadowHandle, SHADOW_WINDOW_PROPERTY);
  if(IsWindow(mShadowHandle))
  {
    SetWindowLongPtrW(mShadowHandle, GWL_EXSTYLE, mShadowExtendedStyle);
  }
  mShadowHandle        = nullptr;
  mShadowExtendedStyle = 0;
}

void DragAndDropWin::UpdateShadow()
{
  if(!mShadowHandle)
  {
    return;
  }

  POINT point{};
  if(GetCursorPos(&point))
  {
    SetWindowPos(mShadowHandle,
                 HWND_TOPMOST,
                 point.x + SHADOW_OFFSET,
                 point.y + SHADOW_OFFSET,
                 0,
                 0,
                 SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
  }
}

void DragAndDropWin::CallSourceCallback(Dali::DragAndDrop::SourceEventType type)
{
  if(mSourceCallback)
  {
    mSourceCallback(type);
  }
}

bool DragAndDropWin::StartDragAndDrop(Dali::Actor, Dali::Window shadowWindow, const Dali::DragAndDrop::DragData& data, SourceCallback callback)
{
  if(mDragging || !mOleInitialized)
  {
    return false;
  }

  auto* object = new DataObject(data);
  if(!object->HasText())
  {
    object->Release();
    return false;
  }

  mDragging       = true;
  mShadowWindow   = shadowWindow;
  mSourceCallback = callback;
  PrepareShadowWindow();
  UpdateShadow();
  CallSourceCallback(Dali::DragAndDrop::SourceEventType::START);

  auto* source = new DropSource(*this);
  DWORD effect = DROPEFFECT_NONE;
  const HRESULT result = DoDragDrop(object, source, DROPEFFECT_COPY, &effect);
  object->Release();
  source->Release();

  EmitLeave(mLastPoint);
  mActiveWindow  = nullptr;
  mTextAvailable = false;
  RestoreShadowWindow();
  if(FAILED(result))
  {
    DALI_LOG_ERROR("DoDragDrop failed: 0x%08lx\n", static_cast<unsigned long>(result));
  }
  if(result == DRAGDROP_S_DROP && effect != DROPEFFECT_NONE)
  {
    CallSourceCallback(Dali::DragAndDrop::SourceEventType::ACCEPT);
    CallSourceCallback(Dali::DragAndDrop::SourceEventType::FINISH);
  }
  else
  {
    CallSourceCallback(Dali::DragAndDrop::SourceEventType::CANCEL);
  }

  mShadowWindow.Reset();
  mSourceCallback = SourceCallback();
  mDragging       = false;
  return true;
}

void DragAndDropWin::WindowDestroyed(HWND window)
{
  if(!window)
  {
    return;
  }

  if(mShadowHandle == window)
  {
    RemovePropW(mShadowHandle, SHADOW_WINDOW_PROPERTY);
    mShadowHandle        = nullptr;
    mShadowExtendedStyle = 0;
    mShadowWindow.Reset();
  }

  if(mActiveWindow == window)
  {
    mActiveActor.Reset();
    mActiveDropWindow.Reset();
    mActiveWindow  = nullptr;
    mTextAvailable = false;
  }

  mActorTargets.erase(std::remove_if(mActorTargets.begin(), mActorTargets.end(), [window](const ActorTarget& target) {
                        return target.window == window;
                      }),
                      mActorTargets.end());
  mWindowTargets.erase(std::remove_if(mWindowTargets.begin(), mWindowTargets.end(), [window](const WindowTarget& target) {
                         return target.window == window;
                       }),
                       mWindowTargets.end());

  const auto found = mNativeTargets.find(window);
  if(found != mNativeTargets.end())
  {
    RevokeDragDrop(window);
    found->second.target->Release();
    mNativeTargets.erase(found);
  }
}

void NotifyDragAndDropWindowDestroyed(HWND window)
{
  if(gDragAndDropWin)
  {
    gDragAndDropWin->WindowDestroyed(window);
  }
}

Dali::DragAndDrop GetDragAndDrop()
{
  Dali::SingletonService singletonService(SingletonService::Get());
  if(!singletonService)
  {
    return Dali::DragAndDrop();
  }

  Dali::BaseHandle handle = singletonService.GetSingleton(typeid(Dali::DragAndDrop));
  if(handle)
  {
    return Dali::DragAndDrop(static_cast<DragAndDrop*>(handle.GetObjectPtr()));
  }

  Dali::DragAndDrop dragAndDrop(new DragAndDropWin());
  singletonService.Register(typeid(Dali::DragAndDrop), dragAndDrop);
  return dragAndDrop;
}

class DragAndDropFactoryWin final : public DragAndDropFactory
{
public:
  Dali::DragAndDrop CreateDragAndDrop() override
  {
    return GetDragAndDrop();
  }
};

std::unique_ptr<DragAndDropFactory> GetDragAndDropFactory()
{
  return std::unique_ptr<DragAndDropFactory>(new DragAndDropFactoryWin());
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
