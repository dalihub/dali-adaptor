/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/android/window-base-android.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>

// EXTERNAL_HEADERS
#include <dali/integration-api/adaptor-framework/android/android-framework.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/events/mouse-button.h>
#include <dali/public-api/object/any.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowBaseLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_WINDOW_BASE");
#endif

WindowBaseAndroid::WindowBaseAndroid(Dali::PositionSize positionSize, Any surface, bool isTransparent)
: mWindow(nullptr),
  mOwnSurface(false),
  mIsTransparent(false), // Should only be set to true once we actually create a transparent window regardless of what isTransparent is.
  mRotationAppSet(false)
{
  Initialize(positionSize, surface, isTransparent);
}

WindowBaseAndroid::~WindowBaseAndroid() = default;

void WindowBaseAndroid::Initialize(PositionSize positionSize, Any surface, bool isTransparent)
{
  if(!surface.Empty())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Initialising using supplied Android native window\n");
    mWindow = static_cast<ANativeWindow*>(AnyCast<void*>(surface));
  }
  else
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Initialising using default Android native window\n");
    mWindow = Dali::Integration::AndroidFramework::Get().GetApplicationWindow();
  }

  DALI_ASSERT_ALWAYS(mWindow && "Failed to get Android window");
  mIsTransparent = true;
}

void WindowBaseAndroid::OnDeleteRequest()
{
  if(Dali::Adaptor::IsAvailable())
  {
    mDeleteRequestSignal.Emit();
  }
}

void WindowBaseAndroid::OnFocusIn(void* data, int type, void* event)
{
}

void WindowBaseAndroid::OnFocusOut(void* data, int type, void* event)
{
}

void WindowBaseAndroid::OnWindowDamaged(void* data, int type, void* event)
{
}

void WindowBaseAndroid::OnMouseButtonDown(void* data, int type, void* event)
{
}

void WindowBaseAndroid::OnMouseButtonUp(void* data, int type, void* event)
{
}

void WindowBaseAndroid::OnMouseButtonMove(void* data, int type, void* event)
{
}

void WindowBaseAndroid::OnMouseWheel(void* data, int type, void* event)
{
}

void WindowBaseAndroid::OnKeyDown(void* data, int type, void* event)
{
}

void WindowBaseAndroid::OnKeyUp(void* data, int type, void* event)
{
}

void WindowBaseAndroid::OnSelectionClear(void* data, int type, void* event)
{
}

void WindowBaseAndroid::OnSelectionNotify(void* data, int type, void* event)
{
}

Any WindowBaseAndroid::GetNativeWindow()
{
  return static_cast<void*>(mWindow);
}

int WindowBaseAndroid::GetNativeWindowId()
{
  return 0;
}

std::string WindowBaseAndroid::GetNativeWindowResourceId()
{
  return std::string();
}

Dali::Any WindowBaseAndroid::CreateWindow(int width, int height)
{
  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Returns the window created for us.\n");

  auto window = static_cast<void*>(mWindow);
  return window;
}

void WindowBaseAndroid::DestroyWindow()
{
  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Does nothing, the window is not owned by us.\n");
}

void WindowBaseAndroid::SetWindowRotation(int angle)
{
}

void WindowBaseAndroid::SetWindowBufferTransform(int angle)
{
}

void WindowBaseAndroid::SetWindowTransform(int angle)
{
}

void WindowBaseAndroid::ResizeWindow(PositionSize positionSize)
{
}

bool WindowBaseAndroid::IsWindowRotationSupported()
{
  return false;
}

void WindowBaseAndroid::Move(PositionSize positionSize)
{
}

void WindowBaseAndroid::Resize(PositionSize positionSize)
{
}

void WindowBaseAndroid::MoveResize(PositionSize positionSize)
{
}

void WindowBaseAndroid::SetLayout(unsigned int numCols, unsigned int numRows, unsigned int column, unsigned int row, unsigned int colSpan, unsigned int rowSpan)
{
}

void WindowBaseAndroid::SetClass(const std::string& name, const std::string& className)
{
}

void WindowBaseAndroid::Raise()
{
}

void WindowBaseAndroid::Lower()
{
}

void WindowBaseAndroid::Activate()
{
}

void WindowBaseAndroid::Maximize(bool maximize)
{
}

bool WindowBaseAndroid::IsMaximized() const
{
  return false;
}

void WindowBaseAndroid::SetMaximumSize(Dali::Window::WindowSize size)
{
}

void WindowBaseAndroid::Minimize(bool minimize)
{
}

bool WindowBaseAndroid::IsMinimized() const
{
  return false;
}

void WindowBaseAndroid::SetMimimumSize(Dali::Window::WindowSize size)
{
}

void WindowBaseAndroid::SetAvailableAnlges(const std::vector<int>& angles)
{
}

void WindowBaseAndroid::SetPreferredAngle(int angle)
{
}

void WindowBaseAndroid::SetAcceptFocus(bool accept)
{
}

void WindowBaseAndroid::Show()
{
}

void WindowBaseAndroid::Hide()
{
}

unsigned int WindowBaseAndroid::GetSupportedAuxiliaryHintCount() const
{
  return 0;
}

std::string WindowBaseAndroid::GetSupportedAuxiliaryHint(unsigned int index) const
{
  return std::string();
}

unsigned int WindowBaseAndroid::AddAuxiliaryHint(const std::string& hint, const std::string& value)
{
  return 0;
}

bool WindowBaseAndroid::RemoveAuxiliaryHint(unsigned int id)
{
  return false;
}

bool WindowBaseAndroid::SetAuxiliaryHintValue(unsigned int id, const std::string& value)
{
  return false;
}

std::string WindowBaseAndroid::GetAuxiliaryHintValue(unsigned int id) const
{
  return std::string();
}

unsigned int WindowBaseAndroid::GetAuxiliaryHintId(const std::string& hint) const
{
  return 0;
}

void WindowBaseAndroid::SetInputRegion(const Rect<int>& inputRegion)
{
}

void WindowBaseAndroid::SetType(Dali::WindowType type)
{
}

Dali::WindowType WindowBaseAndroid::GetType() const
{
  return Dali::WindowType::NORMAL;
}

Dali::WindowOperationResult WindowBaseAndroid::SetNotificationLevel(Dali::WindowNotificationLevel level)
{
  return Dali::WindowOperationResult::NOT_SUPPORTED;
}

Dali::WindowNotificationLevel WindowBaseAndroid::GetNotificationLevel() const
{
  return Dali::WindowNotificationLevel::NONE;
}

void WindowBaseAndroid::SetOpaqueState(bool opaque)
{
}

Dali::WindowOperationResult WindowBaseAndroid::SetScreenOffMode(WindowScreenOffMode screenOffMode)
{
  return Dali::WindowOperationResult::NOT_SUPPORTED;
}

WindowScreenOffMode WindowBaseAndroid::GetScreenOffMode() const
{
  return WindowScreenOffMode::TIMEOUT;
}

Dali::WindowOperationResult WindowBaseAndroid::SetBrightness(int brightness)
{
  return Dali::WindowOperationResult::NOT_SUPPORTED;
}

int WindowBaseAndroid::GetBrightness() const
{
  return 0;
}

bool WindowBaseAndroid::GrabKey(Dali::KEY key, KeyGrab::KeyGrabMode grabMode)
{
  return false;
}

bool WindowBaseAndroid::UngrabKey(Dali::KEY key)
{
  return false;
}

bool WindowBaseAndroid::GrabKeyList(const Dali::Vector<Dali::KEY>& key, const Dali::Vector<KeyGrab::KeyGrabMode>& grabMode, Dali::Vector<bool>& result)
{
  return false;
}

bool WindowBaseAndroid::UngrabKeyList(const Dali::Vector<Dali::KEY>& key, Dali::Vector<bool>& result)
{
  return false;
}

void WindowBaseAndroid::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  AConfiguration* config = Dali::Integration::AndroidFramework::Get().GetApplicationConfiguration();

  int32_t density = AConfiguration_getDensity(config);
  if(density == ACONFIGURATION_DENSITY_ANY)
  {
    DALI_LOG_ERROR("Failed to get Android DPI, use 0 instead.");
    density = 0;
  }

  dpiHorizontal = density;
  dpiVertical   = density;
}

int WindowBaseAndroid::GetScreenRotationAngle()
{
  return 0;
}

void WindowBaseAndroid::SetWindowRotationAngle(int degree)
{
}

void WindowBaseAndroid::WindowRotationCompleted(int degree, int width, int height)
{
}

void WindowBaseAndroid::SetTransparency(bool transparent)
{
}

void WindowBaseAndroid::SetParent(WindowBase* parentWinBase, bool belowParent)
{
}

int WindowBaseAndroid::CreateFrameRenderedSyncFence()
{
  return -1;
}

int WindowBaseAndroid::CreateFramePresentedSyncFence()
{
  return -1;
}

int WindowBaseAndroid::GetWindowRotationAngle() const
{
  return 0;
}

void WindowBaseAndroid::SetPositionSizeWithAngle(PositionSize positionSize, int angle)
{
}

void WindowBaseAndroid::InitializeIme()
{
}

void WindowBaseAndroid::ImeWindowReadyToRender()
{
}

void WindowBaseAndroid::RequestMoveToServer()
{
}

void WindowBaseAndroid::RequestResizeToServer(WindowResizeDirection direction)
{
}

void WindowBaseAndroid::EnableFloatingMode(bool enable)
{
}

bool WindowBaseAndroid::IsFloatingModeEnabled() const
{
  return false;
}

void WindowBaseAndroid::IncludeInputRegion(const Rect<int>& inputRegion)
{
}

void WindowBaseAndroid::ExcludeInputRegion(const Rect<int>& inputRegion)
{
}

bool WindowBaseAndroid::PointerConstraintsLock()
{
  return false;
}

bool WindowBaseAndroid::PointerConstraintsUnlock()
{
  return false;
}

void WindowBaseAndroid::LockedPointerRegionSet(int32_t x, int32_t y, int32_t width, int32_t height)
{
}

void WindowBaseAndroid::LockedPointerCursorPositionHintSet(int32_t x, int32_t y)
{
}

bool WindowBaseAndroid::PointerWarp(int32_t x, int32_t y)
{
  return false;
}

void WindowBaseAndroid::CursorVisibleSet(bool visible)
{
}

bool WindowBaseAndroid::KeyboardGrab(Device::Subclass::Type deviceSubclass)
{
  return false;
}

bool WindowBaseAndroid::KeyboardUnGrab()
{
  return false;
}

void WindowBaseAndroid::SetFullScreen(bool fullscreen)
{
  return;
}

bool WindowBaseAndroid::GetFullScreen()
{
  return false;
}

void WindowBaseAndroid::SetFrontBufferRendering(bool enable)
{
}

bool WindowBaseAndroid::GetFrontBufferRendering()
{
  return false;
}

void WindowBaseAndroid::SetWindowFrontBufferMode(bool enable)
{
}

void WindowBaseAndroid::SetModal(bool modal)
{
}

bool WindowBaseAndroid::IsModal()
{
  return false;
}

void WindowBaseAndroid::SetAlwaysOnTop(bool alwaysOnTop)
{
}

bool WindowBaseAndroid::IsAlwaysOnTop()
{
  return false;
}

void WindowBaseAndroid::SetBottom(bool enable)
{
}

bool WindowBaseAndroid::IsBottom()
{
  return false;
}

Any WindowBaseAndroid::GetNativeBuffer() const
{
  return 0;
}

bool WindowBaseAndroid::RelativeMotionGrab(uint32_t boundary)
{
  return false;
}

bool WindowBaseAndroid::RelativeMotionUnGrab()
{
  return false;
}

void WindowBaseAndroid::SetBackgroundBlur(int blurRadius, int cornerRadius)
{
}

int WindowBaseAndroid::GetBackgroundBlur()
{
  return 0;
}

void WindowBaseAndroid::SetBehindBlur(int blurRadius)
{
}

int WindowBaseAndroid::GetBehindBlur()
{
  return 0;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
