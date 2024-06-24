/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

#include "dali/public-api/adaptor-framework/window.h"
#include "dali/public-api/events/wheel-event.h"
#include <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>

// CLASS HEADER
#include <dali/internal/window-system/macos/window-base-mac.h>

// EXTERNAL_HEADERS
#include <dali/public-api/object/any.h>
#include <dali/integration-api/debug.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-system.h>

#include <cmath>

using Dali::Internal::Adaptor::WindowBaseCocoa;

// Angle is default selecting CGL as its backend and because
// of that we are using NSOpenGLView. Ideally we should use
// Metal as the backend. When this happends, we must change
// the parent class to MTKView.
@interface CocoaView : NSOpenGLView
- (CocoaView *) initWithFrame:(NSRect) rect withImpl:(WindowBaseCocoa::Impl *) impl;
- (BOOL) isFlipped;
- (BOOL) wantsUpdateLayer;
- (BOOL) acceptsFirstResponder;
- (void) mouseDown:(NSEvent *) event;
- (void) mouseUp:(NSEvent *) event;
- (void) mouseDragged:(NSEvent *) event;
- (void) keyDown:(NSEvent *) event;
- (void) keyUp:(NSEvent *) event;
- (void) drawRect:(NSRect) dirtyRect;
- (void) prepareOpenGL;
@end

@interface WindowDelegate : NSObject <NSWindowDelegate>
- (WindowDelegate *) init:(WindowBaseCocoa::Impl *) impl;
- (void) windowDidBecomeKey:(NSNotification *) notification;
- (void) windowDidResignKey:(NSNotification *) notification;
- (void) windowWillClose:(NSNotification *) notification;
@end

namespace Dali::Internal::Adaptor
{

namespace
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowBaseLogFilter = Debug::Filter::New( Debug::NoLogging, false, "LOG_WINDOW_BASE" );
#endif

// Converts a y coordinate from top to bottom coordinate
CGFloat BottomYCoordinate(CGFloat topYCoordinate, CGFloat windowHeight) noexcept
{
  const auto screen = [NSScreen.mainScreen frame];
  return screen.size.height - windowHeight - topYCoordinate;
}

NSRect PositionSizeToRect(const PositionSize &positionSize, bool flipped = false) noexcept
{
  // positionSize assumes top-left coordinate system
  // Cocoa assumes bottom-left coordinate system
  // If NSView isFlipped method returns YES, then it uses top-left coordinate system
  const auto windowHeight = static_cast<CGFloat>(positionSize.height);
  const auto yGiven = static_cast<CGFloat>(positionSize.y);

  CGFloat yWindow;
  if (flipped)
  {
    yWindow = yGiven;
  }
  else
  {
    yWindow = BottomYCoordinate(yGiven, windowHeight);
  }

  return
  {
    .origin =
    {
      .x = static_cast<CGFloat>(positionSize.x),
      .y = yWindow
    },
    .size =
    {
      .width = static_cast<CGFloat>(positionSize.width),
      .height = windowHeight,
    },
  };
}

} // unnamed namespace

struct WindowBaseCocoa::Impl final
{
  NSWindow *mWindow;
  NSWindowController *mWinController;
  WindowBaseCocoa *mThis;

  Impl(const Impl &rhs) = delete;
  Impl &operator<(const Impl &rhs) = delete;
  Impl(const Impl &&rhs) = delete;
  Impl &operator<(const Impl &&rhs) = delete;

  Impl(
    WindowBaseCocoa *pThis,
    PositionSize positionSize,
    Any surface,
    bool isTransparent
  );

  ~Impl();

  void OnFocus(bool focus)
  {
    if(Dali::Adaptor::IsAvailable())
    {
      mThis->mFocusChangedSignal.Emit(focus);
    }
  }

  // Handle mouse events
  void OnMouse(NSEvent *event, PointState::Type state);
  void OnMouseWheel(NSEvent *event);
  void OnKey(NSEvent *event, Integration::KeyEvent::State keyState);
  void OnWindowDamaged(const NSRect &rect);

  void OnRedraw(void)
  {
    if(Dali::Adaptor::IsAvailable())
    {
      mThis->mWindowRedrawRequestSignal.Emit();
    }
  }

private:
  uint32_t GetKeyModifiers(NSEvent *event) const noexcept;
  std::string GetKeyName(NSEvent *event) const;
};

WindowBaseCocoa::Impl::Impl(
  WindowBaseCocoa *pThis,
  PositionSize positionSize,
  Any surface,
  bool isTransparent
) : mThis(pThis)
{
  constexpr NSUInteger style =
    NSWindowStyleMaskTitled
    | NSWindowStyleMaskClosable
    | NSWindowStyleMaskMiniaturizable
    | NSWindowStyleMaskResizable;

  mWindow = [[NSWindow alloc] initWithContentRect:PositionSizeToRect(positionSize)
                                        styleMask:style
                                          backing:NSBackingStoreBuffered
                                            defer:NO];

  mWindow.alphaValue = static_cast<CGFloat>(!isTransparent);
  mWinController = [[NSWindowController alloc] initWithWindow:mWindow];

  mWindow.delegate = [[WindowDelegate alloc] init:this];

  NSView *view = [[CocoaView alloc] initWithFrame:PositionSizeToRect(positionSize, true)
                                         withImpl:this];
  NSPoint origin{0, 0};
  [view setFrameOrigin:origin];

  mWindow.contentView = view;

  [mWindow makeKeyAndOrderFront:nil];
}

WindowBaseCocoa::Impl::~Impl()
{
  [mWinController close];
  [NSApp stop:nil];
}

void WindowBaseCocoa::Impl::OnMouse(NSEvent *event, PointState::Type state)
{
  if(Dali::Adaptor::IsAvailable())
  {
    Integration::Point point;
    point.SetDeviceId(event.deviceID);
    point.SetState(state);
    auto p = [event locationInWindow];
    auto [x, y] = [mWindow.contentView convertPoint:p fromView:nil];
    point.SetScreenPosition(Vector2(x, y));
    point.SetRadius(std::sqrt(x*x + y*y));
    point.SetPressure(event.pressure);

    if (x == 0.0)
    {
      point.SetAngle(Degree(0.0));
    }
    else
    {
      point.SetAngle(Radian(std::atan(y/x)));
    }

    DALI_LOG_INFO(
      gWindowBaseLogFilter,
      Debug::Verbose,
      "WindowBaseCocoa::Impl::OnMouse(%.1f, %.1f)\n",
      x,
      y
    );

    // timestamp is given in seconds, the signal expects it in milliseconds
    mThis->mTouchEventSignal.Emit(point, event.timestamp * 1000);

    mThis->mMouseFrameEventSignal.Emit();
  }
}

void WindowBaseCocoa::Impl::OnMouseWheel(NSEvent *event)
{
  if(Dali::Adaptor::IsAvailable())
  {
    auto p = [event locationInWindow];
    auto [x, y] = [mWindow.contentView convertPoint:p fromView:nil];

    const auto modifiers = GetKeyModifiers(event);
    const Vector2 vec(x, y);
    const auto timestamp = event.timestamp * 1000;

    if (event.scrollingDeltaY)
    {
      Integration::WheelEvent wheelEvent(
        Integration::WheelEvent::MOUSE_WHEEL,
        0,
        modifiers,
        vec,
        event.scrollingDeltaY < 0 ? -1 : 1,
        timestamp
      );

      mThis->mWheelEventSignal.Emit(wheelEvent);
    }

    if (event.scrollingDeltaX)
    {
      Integration::WheelEvent wheelEvent(
        Integration::WheelEvent::MOUSE_WHEEL,
        0,
        modifiers,
        vec,
        event.scrollingDeltaX < 0 ? -1 : 1,
        timestamp
      );

      mThis->mWheelEventSignal.Emit(wheelEvent);
    }
  }
}

void WindowBaseCocoa::Impl::OnKey(NSEvent *event, Integration::KeyEvent::State keyState)
{
  if(Dali::Adaptor::IsAvailable())
  {
    const std::string empty;

    Integration::KeyEvent keyEvent(
      GetKeyName(event),
      empty,
      [event.characters UTF8String],
      event.keyCode,
      GetKeyModifiers(event),
      event.timestamp * 1000,
      keyState,
      empty,
      empty,
      Device::Class::NONE,
      Device::Subclass::NONE
    );

    DALI_LOG_INFO(
      gWindowBaseLogFilter,
      Debug::Verbose,
      "WindowBaseCocoa::Impl::OnKey(%s)\n",
      [event.characters UTF8String]
    );
    keyEvent.windowId = mThis->GetNativeWindowId();

    mThis->mKeyEventSignal.Emit(keyEvent);
  }
}

void WindowBaseCocoa::Impl::OnWindowDamaged(const NSRect &rect)
{
  if(Dali::Adaptor::IsAvailable())
  {
    const DamageArea area(
      rect.origin.x,
      rect.origin.y,
      rect.size.width,
      rect.size.height
    );

    mThis->mWindowDamagedSignal.Emit(area);
  }
}

uint32_t WindowBaseCocoa::Impl::GetKeyModifiers(NSEvent *event) const noexcept
{
  uint32_t modifiers = 0;

  if (event.modifierFlags & NSEventModifierFlagShift)
  {
    modifiers |= 1;
  }

  if (event.modifierFlags & NSEventModifierFlagControl)
  {
    modifiers |= 2;
  }

  if (event.modifierFlags & NSEventModifierFlagCommand)
  {
    modifiers |= 4;
  }

  return modifiers;
}

std::string WindowBaseCocoa::Impl::GetKeyName(NSEvent *event) const
{
  switch (event.keyCode)
  {
    case kVK_Control:     return "Control";
    case kVK_Shift:       return "Shift";
    case kVK_Delete:      return "Backspace";
    case kVK_Command:     return "Command";
    case kVK_Tab:         return "Tab";
    case kVK_Return:      return "Return";
    case kVK_Escape:      return "Escape";
    case kVK_Space:       return "Space";
    case kVK_LeftArrow:   return "Left";
    case kVK_UpArrow:     return "Up";
    case kVK_RightArrow:  return "Right";
    case kVK_DownArrow:   return "Down";
    case kVK_ANSI_0:      return "0";
    case kVK_ANSI_1:      return "1";
    case kVK_ANSI_2:      return "2";
    case kVK_ANSI_3:      return "3";
    case kVK_ANSI_4:      return "4";
    case kVK_ANSI_5:      return "5";
    case kVK_ANSI_6:      return "6";
    case kVK_ANSI_7:      return "7";
    case kVK_ANSI_8:      return "8";
    case kVK_ANSI_9:      return "9";
    default:              return [event.characters UTF8String];
  }

  return "";
}

WindowBaseCocoa::WindowBaseCocoa(PositionSize positionSize, Any surface, bool isTransparent)
  : mImpl(std::make_unique<Impl>(this, positionSize, surface, isTransparent))
{
}

WindowBaseCocoa::~WindowBaseCocoa()
{
}

Any WindowBaseCocoa::GetNativeWindow()
{
  return mImpl->mWindow;
}

int WindowBaseCocoa::GetNativeWindowId()
{
  return mImpl->mWindow.windowNumber;
}

std::string WindowBaseCocoa::GetNativeWindowResourceId()
{
  return std::string();
}

EGLNativeWindowType WindowBaseCocoa::CreateEglWindow(int width, int height)
{
  // XXX: this method is called from a secondary thread, but
  // we can only resize the window from the main thread
  //PositionSize size(0, 0, width, height);
  //Resize(size);
  return mImpl->mWindow.contentView.layer;
}

void WindowBaseCocoa::DestroyEglWindow()
{
}

void WindowBaseCocoa::SetEglWindowRotation( int angle )
{
}

void WindowBaseCocoa::SetEglWindowBufferTransform( int angle )
{
}

void WindowBaseCocoa::SetEglWindowTransform( int angle )
{
}

void WindowBaseCocoa::ResizeEglWindow( PositionSize positionSize )
{
  dispatch_async(dispatch_get_main_queue(), ^{
    Resize(positionSize);
  });
}

bool WindowBaseCocoa::IsEglWindowRotationSupported()
{
  return false;
}

void WindowBaseCocoa::Move( PositionSize positionSize )
{
  const NSPoint p = {
    .x = static_cast<CGFloat>(positionSize.x),
    .y = static_cast<CGFloat>(positionSize.y),
  };

  [mImpl->mWindow setFrameTopLeftPoint:p];
}

void WindowBaseCocoa::Resize( PositionSize positionSize )
{
  auto r = mImpl->mWindow.frame;
  r.size.width = static_cast<CGFloat>(positionSize.width);
  r.size.height = static_cast<CGFloat>(positionSize.height);
  [mImpl->mWindow setFrame:r display:YES];

  NSSize size =
  {
    .width = r.size.width,
    .height = r.size.height,
  };

  [mImpl->mWindow.contentView setFrameSize:size];

}

void WindowBaseCocoa::MoveResize( PositionSize positionSize )
{
  [mImpl->mWindow setFrame: PositionSizeToRect(positionSize) display:YES];

  NSSize size =
  {
    .width = static_cast<CGFloat>(positionSize.width),
    .height = static_cast<CGFloat>(positionSize.height),
  };

  [mImpl->mWindow.contentView setFrameSize:size];
}

void WindowBaseCocoa::SetLayout(unsigned int numCols, unsigned int numRows, unsigned int column, unsigned int row, unsigned int colSpan, unsigned int rowSpan)
{
}

void WindowBaseCocoa::SetClass( const std::string& name, const std::string& className )
{
}

void WindowBaseCocoa::Raise()
{
  [mImpl->mWindow orderFront:nil];
}

void WindowBaseCocoa::Lower()
{
  [mImpl->mWindow orderBack:nil];
}

void WindowBaseCocoa::Activate()
{
  [mImpl->mWinController showWindow:nil];
}

void WindowBaseCocoa::Maximize(bool maximize)
{
}

bool WindowBaseCocoa::IsMaximized() const
{
  return false;
}

void WindowBaseCocoa::SetMaximumSize(Dali::Window::WindowSize size)
{
}

void WindowBaseCocoa::Minimize(bool minimize)
{
}

bool WindowBaseCocoa::IsMinimized() const
{
  return false;
}

void WindowBaseCocoa::SetMimimumSize(Dali::Window::WindowSize size)
{
}

void WindowBaseCocoa::SetAvailableAnlges( const std::vector< int >& angles )
{
}

void WindowBaseCocoa::SetPreferredAngle( int angle )
{
}

void WindowBaseCocoa::SetAcceptFocus( bool accept )
{
}

void WindowBaseCocoa::Show()
{
  [mImpl->mWinController showWindow:nil];
}

void WindowBaseCocoa::Hide()
{
  [mImpl->mWindow orderOut:nil];
}

unsigned int WindowBaseCocoa::GetSupportedAuxiliaryHintCount() const
{
  return 0;
}

std::string WindowBaseCocoa::GetSupportedAuxiliaryHint( unsigned int index ) const
{
  return std::string();
}

unsigned int WindowBaseCocoa::AddAuxiliaryHint( const std::string& hint, const std::string& value )
{
  return 0;
}

bool WindowBaseCocoa::RemoveAuxiliaryHint( unsigned int id )
{
  return false;
}

bool WindowBaseCocoa::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  return false;
}

std::string WindowBaseCocoa::GetAuxiliaryHintValue( unsigned int id ) const
{
  return std::string();
}

unsigned int WindowBaseCocoa::GetAuxiliaryHintId( const std::string& hint ) const
{
  return 0;
}

void WindowBaseCocoa::SetInputRegion( const Rect< int >& inputRegion )
{
}

void WindowBaseCocoa::SetType( Dali::WindowType type )
{
}

Dali::WindowType WindowBaseCocoa::GetType() const
{
  return Dali::WindowType::NORMAL;
}

WindowOperationResult WindowBaseCocoa::SetNotificationLevel( WindowNotificationLevel level )
{
  return WindowOperationResult::NOT_SUPPORTED;
}

WindowNotificationLevel WindowBaseCocoa::GetNotificationLevel() const
{
  return WindowNotificationLevel::NONE;
}

void WindowBaseCocoa::SetOpaqueState( bool opaque )
{
}

WindowOperationResult WindowBaseCocoa::SetScreenOffMode(WindowScreenOffMode screenOffMode)
{
  return WindowOperationResult::NOT_SUPPORTED;
}

WindowScreenOffMode WindowBaseCocoa::GetScreenOffMode() const
{
  return WindowScreenOffMode::TIMEOUT;
}

WindowOperationResult WindowBaseCocoa::SetBrightness( int brightness )
{
  return WindowOperationResult::NOT_SUPPORTED;
}

int WindowBaseCocoa::GetBrightness() const
{
  return 0;
}

bool WindowBaseCocoa::GrabKey( Dali::KEY key, KeyGrab::KeyGrabMode grabMode )
{
  return false;
}

bool WindowBaseCocoa::UngrabKey( Dali::KEY key )
{
  return false;
}

bool WindowBaseCocoa::GrabKeyList(
  const Dali::Vector< Dali::KEY >& key,
  const Dali::Vector< KeyGrab::KeyGrabMode >& grabMode,
  Dali::Vector< bool >& result
)
{
  return false;
}

bool WindowBaseCocoa::UngrabKeyList(
  const Dali::Vector< Dali::KEY >& key,
  Dali::Vector< bool >& result
)
{
  return false;
}

void WindowBaseCocoa::GetDpi(
  unsigned int& dpiHorizontal,
  unsigned int& dpiVertical
)
{
  auto *screen = [NSScreen mainScreen];
  NSSize res = [screen.deviceDescription[NSDeviceResolution] sizeValue];
  dpiHorizontal = res.width;
  dpiVertical = res.height;
}

int WindowBaseCocoa::GetWindowRotationAngle() const
{
  return 0;
}

int WindowBaseCocoa::GetScreenRotationAngle()
{
  return 0;
}

void WindowBaseCocoa::SetWindowRotationAngle( int degree )
{
}

void WindowBaseCocoa::WindowRotationCompleted( int degree, int width, int height )
{
}

void WindowBaseCocoa::SetTransparency( bool transparent )
{
  mImpl->mWindow.alphaValue = static_cast<CGFloat>(!transparent);
}

void WindowBaseCocoa::SetParent(WindowBase* parentWinBase, bool belowParent)
{
  auto &parent = dynamic_cast<WindowBaseCocoa&>(*parentWinBase);
  [mImpl->mWindow setParentWindow:parent.mImpl->mWindow];
}

int WindowBaseCocoa::CreateFrameRenderedSyncFence()
{
  return -1;
}

int WindowBaseCocoa::CreateFramePresentedSyncFence()
{
  return -1;
}

void WindowBaseCocoa::SetPositionSizeWithAngle(PositionSize positionSize, int angle)
{
}

void WindowBaseCocoa::InitializeIme()
{
}

void WindowBaseCocoa::ImeWindowReadyToRender()
{
}

void WindowBaseCocoa::RequestMoveToServer()
{
}

void WindowBaseCocoa::RequestResizeToServer(WindowResizeDirection direction)
{
}

void WindowBaseCocoa::EnableFloatingMode(bool enable)
{
}

bool WindowBaseCocoa::IsFloatingModeEnabled() const
{
  return false;
}

void WindowBaseCocoa::IncludeInputRegion(const Rect<int>& inputRegion)
{
}

void WindowBaseCocoa::ExcludeInputRegion(const Rect<int>& inputRegion)
{
}

bool WindowBaseCocoa::PointerConstraintsLock()
{
  return false;
}

bool WindowBaseCocoa::PointerConstraintsUnlock()
{
  return false;
}

void WindowBaseCocoa::LockedPointerRegionSet(int32_t x, int32_t y, int32_t width, int32_t height)
{
}

void WindowBaseCocoa::LockedPointerCursorPositionHintSet(int32_t x, int32_t y)
{
}

bool WindowBaseCocoa::PointerWarp(int32_t x, int32_t y)
{
  return false;
}

void WindowBaseCocoa::CursorVisibleSet(bool visible)
{
}

bool WindowBaseCocoa::KeyboardGrab(Device::Subclass::Type deviceSubclass)
{
  return false;
}

bool WindowBaseCocoa::KeyboardUnGrab()
{
  return false;
}

void WindowBaseCocoa::SetFullScreen(bool fullscreen)
{
  return;
}

bool WindowBaseCocoa::GetFullScreen()
{
  return false;
}

void WindowBaseCocoa::SetFrontBufferRendering(bool enable)
{
}

bool WindowBaseCocoa::GetFrontBufferRendering()
{
  return false;
}

void WindowBaseCocoa::SetEglWindowFrontBufferMode(bool enable)
{
}

void WindowBaseCocoa::SetModal(bool modal)
{
}

bool WindowBaseCocoa::IsModal()
{
  return false;
}

void WindowBaseCocoa::SetAlwaysOnTop(bool alwaysOnTop)
{
}

bool WindowBaseCocoa::IsAlwaysOnTop()
{
  return false;
}

} // namespace Dali::Internal::Adaptor

@implementation CocoaView
{
  WindowBaseCocoa::Impl *mImpl;
}

- (CocoaView *) initWithFrame:(NSRect) rect withImpl:(WindowBaseCocoa::Impl *) impl
{
  self = [super initWithFrame:rect];
  if (self)
  {
    mImpl = impl;
    self.wantsLayer = YES;
    self.wantsBestResolutionOpenGLSurface = NO;
  }

  return self;
}

- (BOOL) isFlipped
{
  return YES;
}

- (BOOL) wantsUpdateLayer
{
  return YES;
}

- (BOOL) acceptsFirstResponder
{
  return YES;
}

- (void) mouseDown:(NSEvent *) event
{
  mImpl->OnMouse(event, Dali::PointState::DOWN);
}

- (void) mouseUp:(NSEvent *) event
{
  mImpl->OnMouse(event, Dali::PointState::UP);
}

- (void) mouseDragged:(NSEvent *) event
{
  mImpl->OnMouse(event, Dali::PointState::MOTION);
}

- (void) keyDown:(NSEvent *) event
{
  mImpl->OnKey(event, Dali::Integration::KeyEvent::DOWN);
}

- (void) keyUp:(NSEvent *) event
{
  mImpl->OnKey(event, Dali::Integration::KeyEvent::UP);
}

- (void) drawRect:(NSRect) dirtyRect
{
  DALI_LOG_INFO(
    Dali::Internal::Adaptor::gWindowBaseLogFilter,
    Debug::Verbose,
    "-[CocoaView drawRect:(%.1f, %.1f, %.1f, %.1f)]\n",
    dirtyRect.origin.x,
    dirtyRect.origin.y,
    dirtyRect.size.width,
    dirtyRect.size.height
  );

  mImpl->OnWindowDamaged(dirtyRect);
}

- (void) prepareOpenGL
{
  auto ctx = CGLGetCurrentContext();
  DALI_ASSERT_ALWAYS(ctx);

  // Enable multithreading
  if (auto err = CGLEnable(ctx, kCGLCEMPEngine); err != kCGLNoError)
  {
    DALI_LOG_ERROR("%s - %s", __PRETTY_FUNCTION__, CGLErrorString(err));
  }
}
@end

@implementation WindowDelegate
{
  WindowBaseCocoa::Impl *mImpl;
}

- (WindowDelegate *) init:(Dali::Internal::Adaptor::WindowBaseCocoa::Impl *) impl
{
  self = [super init];
  if (self)
  {
    mImpl = impl;
  }
  return self;
}

- (void) windowDidBecomeKey:(NSNotification *) notification
{
  mImpl->OnFocus(true);
}

- (void) windowDidResignKey:(NSNotification *) notification
{
  mImpl->OnFocus(false);
}

- (void) windowWillClose:(NSNotification *) notification
{
  [NSApp stop:nil];
}
@end
