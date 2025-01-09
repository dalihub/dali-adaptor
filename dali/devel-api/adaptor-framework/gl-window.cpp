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
#include <dali/devel-api/adaptor-framework/gl-window.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/window.h>

// INTERNAL INCLUDES
#include <dali/devel-api/common/addon-binder.h>
#include <dali/internal/window-system/gl-window/gl-window-impl.h>
#include <dali/public-api/adaptor-framework/graphics-backend.h>

namespace Dali
{
using GlWindowImpl = Internal::Adaptor::GlWindow;

namespace
{
const char* const DALI_ADAPTOR_GL_WINDOW_ADDON_SO("libdali2-adaptor-gl-window-addon.so");
const char* const DALI_ADAPTOR_GL_WINDOW_ADDON_NAME("AdaptorGlWindowAddOn");

struct AdaptorGlWindowAddOn : public Dali::AddOn::AddOnBinder
{
  AdaptorGlWindowAddOn()
  : Dali::AddOn::AddOnBinder(DALI_ADAPTOR_GL_WINDOW_ADDON_NAME, DALI_ADAPTOR_GL_WINDOW_ADDON_SO)
  {
  }

  ~AdaptorGlWindowAddOn() = default;

  ADDON_BIND_FUNCTION(GlWindowNew, GlWindow(PositionSize, const std::string&, const std::string&, bool));
  ADDON_BIND_FUNCTION(GlWindowSetGraphicsConfig, void(GlWindowImpl&, bool, bool, int, GlWindow::GlesVersion));
  ADDON_BIND_FUNCTION(GlWindowRaise, void(GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowLower, void(GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowActivate, void(GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowShow, void(GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowHide, void(GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowGetSupportedAuxiliaryHintCount, unsigned int(const GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowGetSupportedAuxiliaryHint, std::string(const GlWindowImpl&, unsigned int));
  ADDON_BIND_FUNCTION(GlWindowAddAuxiliaryHint, unsigned int(GlWindowImpl&, const std::string&, const std::string&));
  ADDON_BIND_FUNCTION(GlWindowRemoveAuxiliaryHint, bool(GlWindowImpl&, unsigned int));
  ADDON_BIND_FUNCTION(GlWindowSetAuxiliaryHintValue, bool(GlWindowImpl&, unsigned int, const std::string&));
  ADDON_BIND_FUNCTION(GlWindowGetAuxiliaryHintValue, std::string(const GlWindowImpl&, unsigned int));
  ADDON_BIND_FUNCTION(GlWindowGetAuxiliaryHintId, unsigned int(const GlWindowImpl&, const std::string&));
  ADDON_BIND_FUNCTION(GlWindowSetInputRegion, void(GlWindowImpl&, const Rect<int>&));
  ADDON_BIND_FUNCTION(GlWindowSetOpaqueState, void(GlWindowImpl&, bool));
  ADDON_BIND_FUNCTION(GlWindowIsOpaqueState, bool(const GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowSetPositionSize, void(GlWindowImpl&, PositionSize));
  ADDON_BIND_FUNCTION(GlWindowGetPositionSize, PositionSize(const GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowGetCurrentOrientation, WindowOrientation(const GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowSetAvailableOrientations, void(GlWindowImpl&, const Dali::Vector<WindowOrientation>&));
  ADDON_BIND_FUNCTION(GlWindowSetPreferredOrientation, void(GlWindowImpl&, WindowOrientation));
  ADDON_BIND_FUNCTION(GlWindowRegisterGlCallbacks, void(GlWindowImpl&, CallbackBase*, CallbackBase*, CallbackBase*));
  ADDON_BIND_FUNCTION(GlWindowRenderOnce, void(GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowSetRenderingMode, void(GlWindowImpl&, GlWindow::RenderingMode));
  ADDON_BIND_FUNCTION(GlWindowGetRenderingMode, GlWindow::RenderingMode(const GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowFocusChangeSignal, GlWindow::FocusChangeSignalType&(GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowResizeSignal, GlWindow::ResizeSignalType&(GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowKeyEventSignal, GlWindow::KeyEventSignalType&(GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowTouchedSignal, GlWindow::TouchEventSignalType&(GlWindowImpl&));
  ADDON_BIND_FUNCTION(GlWindowVisibilityChangedSignal, GlWindow::VisibilityChangedSignalType&(GlWindowImpl&));
};

std::unique_ptr<AdaptorGlWindowAddOn> gAdaptorGlWindowAddOn;
} // unnamed namespace

GlWindow GlWindow::New()
{
  PositionSize positionSize(0, 0, 0, 0);
  return Dali::GlWindow::New(positionSize, "", "", false);
}

GlWindow GlWindow::New(PositionSize positionSize, const std::string& name, const std::string& className, bool isTransparent)
{
  if(Graphics::GetCurrentGraphicsBackend() == Graphics::Backend::GLES)
  {
    if(!gAdaptorGlWindowAddOn)
    {
      gAdaptorGlWindowAddOn.reset(new AdaptorGlWindowAddOn);
    }
    DALI_ASSERT_ALWAYS(gAdaptorGlWindowAddOn && "Cannot load the GlWindow Addon\n");
    return gAdaptorGlWindowAddOn->GlWindowNew(positionSize, name, className, isTransparent);
  }
  DALI_ABORT("Current Graphics Backend does not support GlWindow\n");
}

GlWindow::GlWindow() = default;

GlWindow::~GlWindow() = default;

GlWindow::GlWindow(const GlWindow& handle) = default;

GlWindow& GlWindow::operator=(const GlWindow& rhs) = default;

GlWindow::GlWindow(GlWindow&& rhs) = default;

GlWindow& GlWindow::operator=(GlWindow&& rhs) = default;

void GlWindow::SetGraphicsConfig(bool depth, bool stencil, int msaa, GlesVersion version)
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowSetGraphicsConfig(impl, depth, stencil, msaa, version);
  }
}

void GlWindow::Raise()
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowRaise(impl);
  }
}

void GlWindow::Lower()
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowLower(impl);
  }
}

void GlWindow::Activate()
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowActivate(impl);
  }
}

void GlWindow::Show()
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowShow(impl);
  }
}

void GlWindow::Hide()
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowHide(impl);
  }
}

unsigned int GlWindow::GetSupportedAuxiliaryHintCount() const
{
  const GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  return gAdaptorGlWindowAddOn ? gAdaptorGlWindowAddOn->GlWindowGetSupportedAuxiliaryHintCount(impl) : 0u;
}

std::string GlWindow::GetSupportedAuxiliaryHint(unsigned int index) const
{
  const GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  return gAdaptorGlWindowAddOn ? gAdaptorGlWindowAddOn->GlWindowGetSupportedAuxiliaryHint(impl, index) : "";
}

unsigned int GlWindow::AddAuxiliaryHint(const std::string& hint, const std::string& value)
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  return gAdaptorGlWindowAddOn ? gAdaptorGlWindowAddOn->GlWindowAddAuxiliaryHint(impl, hint, value) : 0u;
}

bool GlWindow::RemoveAuxiliaryHint(unsigned int id)
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  return gAdaptorGlWindowAddOn ? gAdaptorGlWindowAddOn->GlWindowRemoveAuxiliaryHint(impl, id) : false;
}

bool GlWindow::SetAuxiliaryHintValue(unsigned int id, const std::string& value)
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  return gAdaptorGlWindowAddOn ? gAdaptorGlWindowAddOn->GlWindowSetAuxiliaryHintValue(impl, id, value) : false;
}

std::string GlWindow::GetAuxiliaryHintValue(unsigned int id) const
{
  const GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  return gAdaptorGlWindowAddOn ? gAdaptorGlWindowAddOn->GlWindowGetAuxiliaryHintValue(impl, id) : "";
}

unsigned int GlWindow::GetAuxiliaryHintId(const std::string& hint) const
{
  const GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  return gAdaptorGlWindowAddOn ? gAdaptorGlWindowAddOn->GlWindowGetAuxiliaryHintId(impl, hint) : 0u;
}

void GlWindow::SetInputRegion(const Rect<int>& inputRegion)
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowSetInputRegion(impl, inputRegion);
  }
}

void GlWindow::SetOpaqueState(bool opaque)
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowSetOpaqueState(impl, opaque);
  }
}

bool GlWindow::IsOpaqueState() const
{
  const GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  return gAdaptorGlWindowAddOn ? gAdaptorGlWindowAddOn->GlWindowIsOpaqueState(impl) : false;
}

void GlWindow::SetPositionSize(PositionSize positionSize)
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowSetPositionSize(impl, positionSize);
  }
}

PositionSize GlWindow::GetPositionSize() const
{
  const GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  return gAdaptorGlWindowAddOn ? gAdaptorGlWindowAddOn->GlWindowGetPositionSize(impl) : PositionSize{};
}

WindowOrientation GlWindow::GetCurrentOrientation() const
{
  const GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  return gAdaptorGlWindowAddOn ? gAdaptorGlWindowAddOn->GlWindowGetCurrentOrientation(impl) : WindowOrientation::PORTRAIT;
}

void GlWindow::SetAvailableOrientations(const Dali::Vector<WindowOrientation>& orientations)
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowSetAvailableOrientations(impl, orientations);
  }
}

void GlWindow::SetPreferredOrientation(WindowOrientation orientation)
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowSetPreferredOrientation(impl, orientation);
  }
}

void GlWindow::RegisterGlCallbacks(CallbackBase* initCallback, CallbackBase* renderFrameCallback, CallbackBase* terminateCallback)
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowRegisterGlCallbacks(impl, initCallback, renderFrameCallback, terminateCallback);
  }
}

void GlWindow::RenderOnce()
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowRenderOnce(impl);
  }
}

void GlWindow::SetRenderingMode(RenderingMode mode)
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    gAdaptorGlWindowAddOn->GlWindowSetRenderingMode(impl, mode);
  }
}

GlWindow::RenderingMode GlWindow::GetRenderingMode() const
{
  const GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  return gAdaptorGlWindowAddOn ? gAdaptorGlWindowAddOn->GlWindowGetRenderingMode(impl) : RenderingMode::CONTINUOUS;
}

GlWindow::FocusChangeSignalType& GlWindow::FocusChangeSignal()
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    return gAdaptorGlWindowAddOn->GlWindowFocusChangeSignal(impl);
  }
  DALI_ABORT("Current Graphics Backend does not support GlWindow\n");
}

GlWindow::ResizeSignalType& GlWindow::ResizeSignal()
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    return gAdaptorGlWindowAddOn->GlWindowResizeSignal(impl);
  }
  DALI_ABORT("Current Graphics Backend does not support GlWindow\n");
}

GlWindow::KeyEventSignalType& GlWindow::KeyEventSignal()
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    return gAdaptorGlWindowAddOn->GlWindowKeyEventSignal(impl);
  }
  DALI_ABORT("Current Graphics Backend does not support GlWindow\n");
}

GlWindow::TouchEventSignalType& GlWindow::TouchedSignal()
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    return gAdaptorGlWindowAddOn->GlWindowTouchedSignal(impl);
  }
  DALI_ABORT("Current Graphics Backend does not support GlWindow\n");
}

GlWindow::VisibilityChangedSignalType& GlWindow::VisibilityChangedSignal()
{
  GlWindowImpl& impl = GetImplementation(*this); // Get Implementation here to catch uninitialized usage
  if(gAdaptorGlWindowAddOn)
  {
    return gAdaptorGlWindowAddOn->GlWindowVisibilityChangedSignal(impl);
  }
  DALI_ABORT("Current Graphics Backend does not support GlWindow\n");
}

GlWindow::GlWindow(Internal::Adaptor::GlWindow* window)
: BaseHandle(window)
{
}

} // namespace Dali
