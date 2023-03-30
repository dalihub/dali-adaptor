/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/adaptor-framework/window.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/orientation-impl.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/public-api/actors/actor.h>

namespace Dali
{
Window Window::New(PositionSize posSize, const std::string& name, bool isTransparent)
{
  return Dali::Window::New(posSize, name, "", isTransparent);
}

Window Window::New(PositionSize posSize, const std::string& name, const std::string& className, bool isTransparent)
{
  Window newWindow;

  const bool isAdaptorAvailable = Dali::Adaptor::IsAvailable();
  bool       isNewWindowAllowed = true;

  if(isAdaptorAvailable)
  {
    Dali::Adaptor& adaptor = Internal::Adaptor::Adaptor::Get();
    isNewWindowAllowed     = Internal::Adaptor::Adaptor::GetImplementation(adaptor).IsMultipleWindowSupported();
  }

  if(isNewWindowAllowed)
  {
    Internal::Adaptor::Window* window = Internal::Adaptor::Window::New(posSize, name, className, WindowType::NORMAL, isTransparent);

    Integration::SceneHolder sceneHolder = Integration::SceneHolder(window);

    if(isAdaptorAvailable)
    {
      Dali::Adaptor& adaptor = Internal::Adaptor::Adaptor::Get();
      Internal::Adaptor::Adaptor::GetImplementation(adaptor).AddWindow(sceneHolder);
    }
    newWindow = Window(window);
  }
  else
  {
    DALI_LOG_ERROR("This device can't support multiple windows.\n");
  }

  return newWindow;
}

Window::Window()
{
}

Window::~Window()
{
}

Window::Window(const Window& copy) = default;

Window& Window::operator=(const Window& rhs) = default;

Window::Window(Window&& rhs) = default;

Window& Window::operator=(Window&& rhs) = default;

Window Window::DownCast(BaseHandle handle)
{
  return Window(dynamic_cast<Dali::Internal::Adaptor::Window*>(handle.GetObjectPtr()));
}

void Window::Add(Dali::Actor actor)
{
  GetImplementation(*this).Add(actor);
}

void Window::Remove(Dali::Actor actor)
{
  GetImplementation(*this).Remove(actor);
}

void Window::SetBackgroundColor(const Vector4& color)
{
  GetImplementation(*this).SetBackgroundColor(color);
}

Vector4 Window::GetBackgroundColor() const
{
  return GetImplementation(*this).GetBackgroundColor();
}

Layer Window::GetRootLayer() const
{
  return GetImplementation(*this).GetRootLayer();
}

Layer Window::GetOverlayLayer()
{
  return GetImplementation(*this).GetOverlayLayer();
}

uint32_t Window::GetLayerCount() const
{
  return GetImplementation(*this).GetLayerCount();
}

Layer Window::GetLayer(uint32_t depth) const
{
  return GetImplementation(*this).GetLayer(depth);
}

Uint16Pair Window::GetDpi() const
{
  return GetImplementation(*this).GetDpi();
}

void Window::SetClass(std::string name, std::string klass)
{
  GetImplementation(*this).SetClass(name, klass);
}

void Window::Raise()
{
  GetImplementation(*this).Raise();
}

void Window::Lower()
{
  GetImplementation(*this).Lower();
}

void Window::Activate()
{
  GetImplementation(*this).Activate();
}

void Window::AddAvailableOrientation(WindowOrientation orientation)
{
  GetImplementation(*this).AddAvailableOrientation(orientation);
}

void Window::RemoveAvailableOrientation(WindowOrientation orientation)
{
  GetImplementation(*this).RemoveAvailableOrientation(orientation);
}

void Window::SetPreferredOrientation(WindowOrientation orientation)
{
  GetImplementation(*this).SetPreferredOrientation(orientation);
}

WindowOrientation Window::GetPreferredOrientation()
{
  return GetImplementation(*this).GetPreferredOrientation();
}

Any Window::GetNativeHandle() const
{
  return GetImplementation(*this).GetNativeHandle();
}

Window::FocusChangeSignalType& Window::FocusChangeSignal()
{
  return GetImplementation(*this).FocusChangeSignal();
}

void Window::SetAcceptFocus(bool accept)
{
  GetImplementation(*this).SetAcceptFocus(accept);
}

bool Window::IsFocusAcceptable() const
{
  return GetImplementation(*this).IsFocusAcceptable();
}

void Window::Show()
{
  GetImplementation(*this).Show();
}

void Window::Hide()
{
  GetImplementation(*this).Hide();
}

bool Window::IsVisible() const
{
  return GetImplementation(*this).IsVisible();
}

unsigned int Window::GetSupportedAuxiliaryHintCount() const
{
  return GetImplementation(*this).GetSupportedAuxiliaryHintCount();
}

std::string Window::GetSupportedAuxiliaryHint(unsigned int index) const
{
  return GetImplementation(*this).GetSupportedAuxiliaryHint(index);
}

unsigned int Window::AddAuxiliaryHint(const std::string& hint, const std::string& value)
{
  return GetImplementation(*this).AddAuxiliaryHint(hint, value);
}

bool Window::RemoveAuxiliaryHint(unsigned int id)
{
  return GetImplementation(*this).RemoveAuxiliaryHint(id);
}

bool Window::SetAuxiliaryHintValue(unsigned int id, const std::string& value)
{
  return GetImplementation(*this).SetAuxiliaryHintValue(id, value);
}

std::string Window::GetAuxiliaryHintValue(unsigned int id) const
{
  return GetImplementation(*this).GetAuxiliaryHintValue(id);
}

unsigned int Window::GetAuxiliaryHintId(const std::string& hint) const
{
  return GetImplementation(*this).GetAuxiliaryHintId(hint);
}

void Window::SetInputRegion(const Rect<int>& inputRegion)
{
  return GetImplementation(*this).SetInputRegion(inputRegion);
}

void Window::SetType(WindowType type)
{
  GetImplementation(*this).SetType(type);
}

WindowType Window::GetType() const
{
  return GetImplementation(*this).GetType();
}

WindowOperationResult Window::SetNotificationLevel(WindowNotificationLevel level)
{
  return GetImplementation(*this).SetNotificationLevel(level);
}

WindowNotificationLevel Window::GetNotificationLevel() const
{
  return GetImplementation(*this).GetNotificationLevel();
}

void Window::SetOpaqueState(bool opaque)
{
  GetImplementation(*this).SetOpaqueState(opaque);
}

bool Window::IsOpaqueState() const
{
  return GetImplementation(*this).IsOpaqueState();
}

WindowOperationResult Window::SetScreenOffMode(WindowScreenOffMode screenMode)
{
  return GetImplementation(*this).SetScreenOffMode(screenMode);
}

WindowScreenOffMode Window::GetScreenOffMode() const
{
  return GetImplementation(*this).GetScreenOffMode();
}

WindowOperationResult Window::SetBrightness(int brightness)
{
  return GetImplementation(*this).SetBrightness(brightness);
}

int Window::GetBrightness() const
{
  return GetImplementation(*this).GetBrightness();
}

Window::ResizeSignalType& Window::ResizeSignal()
{
  return GetImplementation(*this).ResizeSignal();
}

void Window::SetSize(Window::WindowSize size)
{
  GetImplementation(*this).SetSize(size);
}

Window::WindowSize Window::GetSize() const
{
  return GetImplementation(*this).GetSize();
}

void Window::SetPosition(Dali::Window::WindowPosition position)
{
  GetImplementation(*this).SetPosition(position);
}

Dali::Window::WindowPosition Window::GetPosition() const
{
  return GetImplementation(*this).GetPosition();
}

void Window::SetLayout(unsigned int numCols, unsigned int numRows, unsigned int column, unsigned int row, unsigned int colSpan, unsigned int rowSpan)
{
  return GetImplementation(*this).SetLayout(numCols, numRows, column, row, colSpan, rowSpan);
}

void Window::SetTransparency(bool transparent)
{
  GetImplementation(*this).SetTransparency(transparent);
}

Dali::RenderTaskList Window::GetRenderTaskList()
{
  return GetImplementation(*this).GetRenderTaskList();
}

Window::KeyEventSignalType& Window::KeyEventSignal()
{
  return GetImplementation(*this).KeyEventSignal();
}

Window::TouchEventSignalType& Window::TouchedSignal()
{
  return GetImplementation(*this).TouchedSignal();
}

Window::Window(Internal::Adaptor::Window* window)
: BaseHandle(window)
{
}

} // namespace Dali
