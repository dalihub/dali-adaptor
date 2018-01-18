/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <dali/internal/input/common/imf-manager-impl.h>
#include <dali/internal/input/common/imf-manager-factory.h>
#include <dali/internal/system/common/locale-utils.h>
#include <dali/internal/system/common/singleton-service-impl.h>

namespace Dali
{
namespace Internal
{

namespace Adaptor
{


bool ImfManager::IsAvailable()
{
  bool available(false);

  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    available = service.GetSingleton(typeid(Dali::ImfManager));
  }

  return available;
}

Dali::ImfManager ImfManager::Get()
{
  return Dali::Internal::Adaptor::ImfManagerFactory::CreateImfManager();
}

ImfManager::~ImfManager()
{}

void ImfManager::NotifyCursorPosition()
{}

void ImfManager::ConnectCallbacks()
{}

void ImfManager::DisconnectCallbacks()
{}

void ImfManager::Activate()
{}

void ImfManager::Deactivate()
{}

void ImfManager::Reset()
{}

ImfContext *ImfManager::GetContext()
{
  return nullptr;
}

bool ImfManager::RestoreAfterFocusLost() const
{
  return false;
}

void ImfManager::SetRestoreAfterFocusLost(bool toggle)
{}

void ImfManager::PreEditChanged(void *data, ImfContext *imfContext, void *event_info)
{}

void ImfManager::CommitReceived(void *data, ImfContext *imfContext, void *event_info)
{}

bool ImfManager::RetrieveSurrounding(void *data, ImfContext *imfContext, char **text, int *cursorPosition)
{
  return false;
}

void ImfManager::DeleteSurrounding(void *data, ImfContext *imfContext, void *event_info)
{}

void ImfManager::SendPrivateCommand(void *data, ImfContext *imfContext, void *event_info)
{}

void ImfManager::SetCursorPosition(unsigned int cursorPosition)
{}

unsigned int ImfManager::GetCursorPosition() const
{
  return 0;
}

void ImfManager::SetSurroundingText(const std::string &text)
{}

const std::string &ImfManager::GetSurroundingText() const
{
  static std::string str("");
  return str;
}

void ImfManager::NotifyTextInputMultiLine(bool multiLine)
{}

Dali::ImfManager::TextDirection ImfManager::GetTextDirection()
{
  return Dali::ImfManager::TextDirection();
}

Dali::Rect<int> ImfManager::GetInputMethodArea()
{
  return Dali::Rect<int>();
}

void ImfManager::ApplyOptions(const InputMethodOptions &options)
{}

void ImfManager::SetInputPanelData(const std::string &data)
{}

void ImfManager::GetInputPanelData(std::string &data)
{}

Dali::ImfManager::State ImfManager::GetInputPanelState()
{
  return Dali::ImfManager::State();
}

void ImfManager::SetReturnKeyState(bool visible)
{}

void ImfManager::AutoEnableInputPanel(bool enabled)
{}

void ImfManager::ShowInputPanel()
{}

void ImfManager::HideInputPanel()
{}

Dali::ImfManager::KeyboardType ImfManager::GetKeyboardType()
{
  return Dali::ImfManager::KeyboardType();
}

std::string ImfManager::GetInputPanelLocale()
{
  return std::string();
}

}
}
}