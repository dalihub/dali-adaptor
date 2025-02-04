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

// EXTERNAL INCLUDES
#include <dali/devel-api/addons/addon-base.h>
#include <dali/integration-api/debug.h>
#include <string>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/gl-window.h>
#include <dali/devel-api/common/addon-binder.h>
#include <dali/internal/window-system/gl-window/gl-window-impl.h>
#include <dali/public-api/adaptor-framework/window.h>

using namespace Dali;
using GlWindowImpl = Internal::Adaptor::GlWindow;

namespace
{
const char* const DALI_ADAPTOR_GL_WINDOW_ADDON_NAME("AdaptorGlWindowAddOn");

GlWindow
GlWindowNew(PositionSize positionSize, const std::string& name, const std::string& className, bool isTransparent)
{
  GlWindow                     newWindow;
  Internal::Adaptor::GlWindow* window = Internal::Adaptor::GlWindow::New(positionSize, name, className, isTransparent);
  newWindow                           = GlWindow(window);

  const bool isAdaptorAvailable = Dali::Adaptor::IsAvailable();
  if(isAdaptorAvailable)
  {
    Dali::Adaptor&        adaptor = Internal::Adaptor::Adaptor::Get();
    Dali::WindowContainer windows = adaptor.GetWindows();
    if(!windows.empty())
    {
      window->SetChild(windows[0]);
    }
  }
  return newWindow;
}

void GlWindowSetGraphicsConfig(GlWindowImpl& glWindowImpl, bool depth, bool stencil, int msaa, GlWindow::GlesVersion version)
{
  glWindowImpl.SetGraphicsConfig(depth, stencil, msaa, version);
}

void GlWindowRaise(GlWindowImpl& glWindowImpl)
{
  glWindowImpl.Raise();
}

void GlWindowLower(GlWindowImpl& glWindowImpl)
{
  glWindowImpl.Lower();
}

void GlWindowActivate(GlWindowImpl& glWindowImpl)
{
  glWindowImpl.Activate();
}

void GlWindowShow(GlWindowImpl& glWindowImpl)
{
  glWindowImpl.Show();
}

void GlWindowHide(GlWindowImpl& glWindowImpl)
{
  glWindowImpl.Hide();
}

unsigned int GlWindowGetSupportedAuxiliaryHintCount(const GlWindowImpl& glWindowImpl)
{
  return glWindowImpl.GetSupportedAuxiliaryHintCount();
}

std::string GlWindowGetSupportedAuxiliaryHint(const GlWindowImpl& glWindowImpl, unsigned int index)
{
  return glWindowImpl.GetSupportedAuxiliaryHint(index);
}

unsigned int GlWindowAddAuxiliaryHint(GlWindowImpl& glWindowImpl, const std::string& hint, const std::string& value)
{
  return glWindowImpl.AddAuxiliaryHint(hint, value);
}

bool GlWindowRemoveAuxiliaryHint(GlWindowImpl& glWindowImpl, unsigned int id)
{
  return glWindowImpl.RemoveAuxiliaryHint(id);
}

bool GlWindowSetAuxiliaryHintValue(GlWindowImpl& glWindowImpl, unsigned int id, const std::string& value)
{
  return glWindowImpl.SetAuxiliaryHintValue(id, value);
}

std::string GlWindowGetAuxiliaryHintValue(const GlWindowImpl& glWindowImpl, unsigned int id)
{
  return glWindowImpl.GetAuxiliaryHintValue(id);
}

unsigned int GlWindowGetAuxiliaryHintId(const GlWindowImpl& glWindowImpl, const std::string& hint)
{
  return glWindowImpl.GetAuxiliaryHintId(hint);
}

void GlWindowSetInputRegion(GlWindowImpl& glWindowImpl, const Rect<int>& inputRegion)
{
  glWindowImpl.SetInputRegion(inputRegion);
}

void GlWindowSetOpaqueState(GlWindowImpl& glWindowImpl, bool opaque)
{
  glWindowImpl.SetOpaqueState(opaque);
}

bool GlWindowIsOpaqueState(const GlWindowImpl& glWindowImpl)
{
  return glWindowImpl.IsOpaqueState();
}

void GlWindowSetPositionSize(GlWindowImpl& glWindowImpl, PositionSize positionSize)
{
  glWindowImpl.SetPositionSize(positionSize);
}

PositionSize GlWindowGetPositionSize(const GlWindowImpl& glWindowImpl)
{
  return glWindowImpl.GetPositionSize();
}

WindowOrientation GlWindowGetCurrentOrientation(const GlWindowImpl& glWindowImpl)
{
  return glWindowImpl.GetCurrentOrientation();
}

void GlWindowSetAvailableOrientations(GlWindowImpl& glWindowImpl, const Dali::Vector<WindowOrientation>& orientations)
{
  glWindowImpl.SetAvailableOrientations(orientations);
}

void GlWindowSetPreferredOrientation(GlWindowImpl& glWindowImpl, WindowOrientation orientation)
{
  glWindowImpl.SetPreferredOrientation(orientation);
}

void GlWindowRegisterGlCallbacks(GlWindowImpl& glWindowImpl, CallbackBase* initCallback, CallbackBase* renderFrameCallback, CallbackBase* terminateCallback)
{
  glWindowImpl.RegisterGlCallbacks(initCallback, renderFrameCallback, terminateCallback);
}

void GlWindowRenderOnce(GlWindowImpl& glWindowImpl)
{
  glWindowImpl.RenderOnce();
}

void GlWindowSetRenderingMode(GlWindowImpl& glWindowImpl, GlWindow::RenderingMode mode)
{
  glWindowImpl.SetRenderingMode(mode);
}

GlWindow::RenderingMode GlWindowGetRenderingMode(const GlWindowImpl& glWindowImpl)
{
  return glWindowImpl.GetRenderingMode();
}

GlWindow::FocusChangeSignalType& GlWindowFocusChangeSignal(GlWindowImpl& glWindowImpl)
{
  return glWindowImpl.FocusChangeSignal();
}

GlWindow::ResizeSignalType& GlWindowResizeSignal(GlWindowImpl& glWindowImpl)
{
  return glWindowImpl.ResizeSignal();
}

GlWindow::KeyEventSignalType& GlWindowKeyEventSignal(GlWindowImpl& glWindowImpl)
{
  return glWindowImpl.KeyEventSignal();
}

GlWindow::TouchEventSignalType& GlWindowTouchedSignal(GlWindowImpl& glWindowImpl)
{
  return glWindowImpl.TouchedSignal();
}

GlWindow::VisibilityChangedSignalType& GlWindowVisibilityChangedSignal(GlWindowImpl& glWindowImpl)
{
  return glWindowImpl.VisibilityChangedSignal();
}

} // unnamed namespace

class AdaptorGlWindowAddOn : public Dali::AddOns::AddOnBase
{
public:
  void GetAddOnInfo(Dali::AddOnInfo& info) override
  {
    info.type    = Dali::AddOnType::GENERIC;
    info.name    = DALI_ADAPTOR_GL_WINDOW_ADDON_NAME;
    info.version = Dali::DALI_ADDON_VERSION(1, 0, 0);
    info.next    = nullptr;
  }

  /**
   * Dispatch table for global functions
   * @return
   */
  Dali::AddOns::DispatchTable* GetGlobalDispatchTable() override
  {
    static Dali::AddOns::DispatchTable dispatchTable{};
    if(dispatchTable.Empty())
    {
      dispatchTable["GlWindowNew"]                            = GlWindowNew;
      dispatchTable["GlWindowSetGraphicsConfig"]              = GlWindowSetGraphicsConfig;
      dispatchTable["GlWindowRaise"]                          = GlWindowRaise;
      dispatchTable["GlWindowLower"]                          = GlWindowLower;
      dispatchTable["GlWindowActivate"]                       = GlWindowActivate;
      dispatchTable["GlWindowShow"]                           = GlWindowShow;
      dispatchTable["GlWindowHide"]                           = GlWindowHide;
      dispatchTable["GlWindowGetSupportedAuxiliaryHintCount"] = GlWindowGetSupportedAuxiliaryHintCount;
      dispatchTable["GlWindowGetSupportedAuxiliaryHint"]      = GlWindowGetSupportedAuxiliaryHint;
      dispatchTable["GlWindowAddAuxiliaryHint"]               = GlWindowAddAuxiliaryHint;
      dispatchTable["GlWindowRemoveAuxiliaryHint"]            = GlWindowRemoveAuxiliaryHint;
      dispatchTable["GlWindowSetAuxiliaryHintValue"]          = GlWindowSetAuxiliaryHintValue;
      dispatchTable["GlWindowGetAuxiliaryHintValue"]          = GlWindowGetAuxiliaryHintValue;
      dispatchTable["GlWindowGetAuxiliaryHintId"]             = GlWindowGetAuxiliaryHintId;
      dispatchTable["GlWindowSetInputRegion"]                 = GlWindowSetInputRegion;
      dispatchTable["GlWindowSetOpaqueState"]                 = GlWindowSetOpaqueState;
      dispatchTable["GlWindowIsOpaqueState"]                  = GlWindowIsOpaqueState;
      dispatchTable["GlWindowSetPositionSize"]                = GlWindowSetPositionSize;
      dispatchTable["GlWindowGetPositionSize"]                = GlWindowGetPositionSize;
      dispatchTable["GlWindowGetCurrentOrientation"]          = GlWindowGetCurrentOrientation;
      dispatchTable["GlWindowSetAvailableOrientations"]       = GlWindowSetAvailableOrientations;
      dispatchTable["GlWindowSetPreferredOrientation"]        = GlWindowSetPreferredOrientation;
      dispatchTable["GlWindowRegisterGlCallbacks"]            = GlWindowRegisterGlCallbacks;
      dispatchTable["GlWindowRenderOnce"]                     = GlWindowRenderOnce;
      dispatchTable["GlWindowSetRenderingMode"]               = GlWindowSetRenderingMode;
      dispatchTable["GlWindowGetRenderingMode"]               = GlWindowGetRenderingMode;
      dispatchTable["GlWindowFocusChangeSignal"]              = GlWindowFocusChangeSignal;
      dispatchTable["GlWindowResizeSignal"]                   = GlWindowResizeSignal;
      dispatchTable["GlWindowKeyEventSignal"]                 = GlWindowKeyEventSignal;
      dispatchTable["GlWindowTouchedSignal"]                  = GlWindowTouchedSignal;
      dispatchTable["GlWindowVisibilityChangedSignal"]        = GlWindowVisibilityChangedSignal;
    }
    return &dispatchTable;
  }

  /**
   * Dispatch table for instance functions
   * @return
   */
  Dali::AddOns::DispatchTable* GetInstanceDispatchTable() override
  {
    return nullptr;
  }
};

REGISTER_ADDON_CLASS(AdaptorGlWindowAddOn);
