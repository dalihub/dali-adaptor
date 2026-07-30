#ifndef DALI_INTERNAL_DRAG_AND_DROP_IMPL_WIN_H
#define DALI_INTERNAL_DRAG_AND_DROP_IMPL_WIN_H

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

// EXTERNAL INCLUDES
#include <windows.h>
#include <oleidl.h>
#include <map>
#include <string>
#include <vector>

// INTERNAL INCLUDES
#include <dali/internal/drag-and-drop/common/drag-and-drop-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class DragAndDropWin;

class DropTargetWin final : public IDropTarget
{
public:
  DropTargetWin(DragAndDropWin& owner, HWND window);

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* object, DWORD keys, POINTL point, DWORD* effect) override;
  HRESULT STDMETHODCALLTYPE DragOver(DWORD keys, POINTL point, DWORD* effect) override;
  HRESULT STDMETHODCALLTYPE DragLeave() override;
  HRESULT STDMETHODCALLTYPE Drop(IDataObject* object, DWORD keys, POINTL point, DWORD* effect) override;

private:
  ~DropTargetWin() = default;

private:
  DragAndDropWin& mOwner;
  HWND            mWindow{nullptr};
  LONG            mReferenceCount{1};
};

class DragAndDropWin final : public Dali::Internal::Adaptor::DragAndDrop
{
public:
  DragAndDropWin();
  ~DragAndDropWin() override;

  bool StartDragAndDrop(Dali::Actor source, Dali::Window shadowWindow, const Dali::DragAndDrop::DragData& data, SourceCallback callback) override;
  bool AddListener(Dali::Actor target, const Dali::String& mimeType, DragCallback callback) override;
  bool RemoveListener(Dali::Actor target) override;
  bool AddListener(Dali::Window target, const Dali::String& mimeType, DragCallback callback) override;
  bool RemoveListener(Dali::Window target) override;
  void SendData(void*) override;
  void ReceiveData(void*) override;
  bool CalculateDragEvent(void*) override;
  bool CalculateViewRegion(void*) override;

  HRESULT OnDragEnter(HWND window, IDataObject* object, POINTL point, DWORD* effect);
  HRESULT OnDragOver(HWND window, POINTL point, DWORD* effect);
  HRESULT OnDragLeave(HWND window);
  HRESULT OnDrop(HWND window, IDataObject* object, POINTL point, DWORD* effect);

  void UpdateShadow();
  void WindowDestroyed(HWND window);

private:
  struct ActorTarget
  {
    Dali::Actor  target;
    HWND         window{nullptr};
    std::string  mimeType;
    DragCallback callback;
  };

  struct WindowTarget
  {
    Dali::Window target;
    HWND         window{nullptr};
    std::string  mimeType;
    DragCallback callback;
  };

  struct NativeTarget
  {
    DropTargetWin* target{nullptr};
    unsigned int   references{0u};
  };

  bool RegisterWindow(HWND window);
  void ReleaseWindow(HWND window);
  void RevokeWindows();

  bool IsTextAvailable(IDataObject* object) const;
  bool ReadTextData(IDataObject* object, Dali::String& data) const;
  bool UpdateActiveTarget(HWND window, POINTL point, bool emitMove);
  bool EmitDrop(HWND window, POINTL point, const Dali::String& data);
  void EmitLeave(POINTL point);
  void EmitActorEvent(const ActorTarget& target, POINTL point, Dali::DragAndDrop::DragType type, const Dali::String& data = Dali::String());
  void EmitWindowEvent(const WindowTarget& target, POINTL point, Dali::DragAndDrop::DragType type, const Dali::String& data = Dali::String());

  ActorTarget* FindActorTarget(HWND window, POINTL point);
  WindowTarget* FindWindowTarget(HWND window);

  void PrepareShadowWindow();
  void RestoreShadowWindow();
  void CallSourceCallback(Dali::DragAndDrop::SourceEventType type);

private:
  std::vector<ActorTarget>      mActorTargets;
  std::vector<WindowTarget>     mWindowTargets;
  std::map<HWND, NativeTarget>  mNativeTargets;
  SourceCallback                mSourceCallback;
  Dali::Window                  mShadowWindow;
  Dali::Actor                   mActiveActor;
  Dali::Window                  mActiveDropWindow;
  HWND                          mActiveWindow{nullptr};
  HWND                          mShadowHandle{nullptr};
  LONG_PTR                      mShadowExtendedStyle{0};
  POINTL                        mLastPoint{};
  bool                          mOleInitialized{false};
  bool                          mDragging{false};
  bool                          mTextAvailable{false};
};

void NotifyDragAndDropWindowDestroyed(HWND window);

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_DRAG_AND_DROP_IMPL_WIN_H
