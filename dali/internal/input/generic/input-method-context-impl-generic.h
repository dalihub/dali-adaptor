#ifndef __DALI_INTERNAL_INPUT_METHOD_CONTEXT_IMPL_GENERIC_H
#define __DALI_INTERNAL_INPUT_METHOD_CONTEXT_IMPL_GENERIC_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/input-method-context-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * @brief Satisfies input API requirements for platforms without input method context support
 */
class InputMethodContextGeneric : public Dali::Internal::Adaptor::InputMethodContext
{
public:
  /**
   * @brief Creates a new InputMethodContext handle
   *
   * @param[in] actor The actor that uses the new InputMethodContext instance.
   * @return InputMethodContext pointer
   */
  static InputMethodContextPtr New(Dali::Actor actor);

  /**
   * Constructor
   * @param[in] actor The actor that uses the new InputMethodContext instance.
   */
  explicit InputMethodContextGeneric(Dali::Actor actor);

public:
  /**
   * @brief Initializes member data.
   */
  void Initialize() override;

  /**
   * Connect Callbacks required for InputMethodContext.
   * If you don't connect InputMethodContext callbacks, you can't get the key events.
   * The events are PreeditChanged, Commit and DeleteSurrounding.
   */
  void ConnectCallbacks() override;

  /**
   * Disconnect Callbacks attached to input method context.
   */
  void DisconnectCallbacks() override;

  /**
   * @copydoc Dali::InputMethodContext::Finalize()
   */
  void Finalize() override;

  /**
   * @copydoc Dali::InputMethodContext::Activate()
   */
  void Activate() override;

  /**
   * @copydoc Dali::InputMethodContext::Deactivate()
   */
  void Deactivate() override;

  /**
   * @copydoc Dali::InputMethodContext::Reset()
   */
  void Reset() override;

  /**
   * @copydoc Dali::InputMethodContext::GetContext()
   */
  ImfContext* GetContext() override;

  /**
   * @copydoc Dali::InputMethodContext::RestoreAfterFocusLost()
   */
  bool RestoreAfterFocusLost() const override;

  /**
   * @copydoc Dali::InputMethodContext::SetRestoreAfterFocusLost()
   */
  bool SetRestoreAfterFocusLost(bool toggle) override;

  /**
   * @copydoc Dali::InputMethodContext::PreEditChanged()
   */
  void PreEditChanged(void* data, ImfContext* imfContext, void* eventInfo) override;

  /**
   * @copydoc Dali::InputMethodContext::NotifyCursorPosition()
   */
  void CommitReceived(void* data, ImfContext* imfContext, void* eventInfo) override;

  /**
   * @copydoc Dali::InputMethodContext::NotifyCursorPosition()
   */
  bool RetrieveSurrounding(void* data, ImfContext* imfContext, char** text, int* cursorPosition) override;

  /**
   * @copydoc Dali::InputMethodContext::DeleteSurrounding()
   */
  void DeleteSurrounding(void* data, ImfContext* imfContext, void* eventInfo) override;

  /**
   * @copydoc Dali::InputMethodContext::SendPrivateCommand()
   */
  void SendPrivateCommand(void* data, ImfContext* imfContext, void* eventInfo) override;

  /**
   * @copydoc Dali::InputMethodContext::SendCommitContent()
   */
  void SendCommitContent(void* data, ImfContext* imfContext, void* eventInfo) override;

  /**
   * @copydoc Dali::InputMethodContext::SendSelectionSet()
   */
  void SendSelectionSet(void* data, ImfContext* imfContext, void* eventInfo) override;

  // Cursor related
  /**
   * @copydoc Dali::InputMethodContext::NotifyCursorPosition()
   */
  void NotifyCursorPosition() override;

  /**
   * @copydoc Dali::InputMethodContext::SetCursorPosition()
   */
  void SetCursorPosition(unsigned int cursorPosition) override;

  /**
   * @copydoc Dali::InputMethodContext::GetCursorPosition()
   */
  unsigned int GetCursorPosition() const override;

  /**
   * @copydoc Dali::InputMethodContext::SetSurroundingText()
   */
  void SetSurroundingText(const Dali::String& text) override;

  /**
   * @copydoc Dali::InputMethodContext::GetSurroundingText()
   */
  Dali::String GetSurroundingText() const override;

  /**
   * @copydoc Dali::InputMethodContext::NotifyTextInputMultiLine()
   */
  void NotifyTextInputMultiLine(bool multiLine) override;

  /**
   * @copydoc Dali::InputMethodContext::GetTextDirection()
   */
  Dali::Integration::InputMethodContext::TextDirection GetTextDirection() override;

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelArea()
   */
  Dali::BoundsInteger GetInputPanelArea() override;

  /**
   * @copydoc Dali::InputMethodContext::ApplyOptions()
   */
  void ApplyOptions(const Dali::Integration::InputMethodOptions& options) override;

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelUserData()
   */
  bool SetInputPanelUserData(const Dali::String& data) override;

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelUserData()
   */
  Dali::String GetInputPanelUserData() const override;

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelState()
   */
  Dali::InputMethodContext::State GetInputPanelState() override;

  /**
   * @copydoc Dali::InputMethodContext::SetReturnKeyState()
   */
  bool SetReturnKeyState(bool visible) override;

  /**
   * @copydoc Dali::InputMethodContext::IsReturnKeyEnabled()
   */
  bool IsReturnKeyEnabled() const override;

  /**
   * @copydoc Dali::InputMethodContext::AutoEnableInputPanel()
   */
  bool AutoEnableInputPanel(bool enabled) override;

  /**
   * @copydoc Dali::InputMethodContext::ShowInputPanel()
   */
  bool ShowInputPanel() override;

  /**
   * @copydoc Dali::InputMethodContext::HideInputPanel()
   */
  bool HideInputPanel() override;

  /**
   * @copydoc Dali::InputMethodContext::GetKeyboardType()
   */
  Dali::InputMethodContext::KeyboardType GetKeyboardType() override;

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelLanguageLocale()
   */
  bool SetInputPanelLanguageLocale(const Dali::String& locale) override;

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelLanguageLocale()
   */
  Dali::String GetInputPanelLanguageLocale() const override;

  /**
   * @copydoc Dali::Integration::InputMethodContext::SetContentMimeTypes()
   */
  void SetContentMimeTypes(const Dali::String& mimeTypes) override;

  /**
   * @copydoc Dali::InputMethodContext::FilterEventKey()
   */
  bool FilterEventKey(const Dali::KeyEvent& keyEvent) override;

  /**
   * @copydoc Dali::InputMethodContext::AllowTextPrediction()
   */
  bool AllowTextPrediction(bool prediction) override;

  /**
   * @copydoc Dali::InputMethodContext::IsTextPredictionAllowed()
   */
  bool IsTextPredictionAllowed() const override;

  /**
   * @copydoc Dali::InputMethodContext::SetFullScreenMode()
   */
  bool SetFullScreenMode(bool fullScreen) override;

  /**
   * @copydoc Dali::InputMethodContext::IsFullScreenMode()
   */
  bool IsFullScreenMode() const override;

  /**
   * @copydoc Dali::Integration::InputMethodContext::SetInputPanelLanguage()
   */
  bool SetInputPanelLanguage(Dali::Integration::InputMethodContext::InputPanelLanguage language) override;

  /**
   * @copydoc Dali::Integration::InputMethodContext::GetInputPanelLanguage()
   */
  Dali::Integration::InputMethodContext::InputPanelLanguage GetInputPanelLanguage() const override;

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelPosition()
   */
  bool SetInputPanelPosition(unsigned int x, unsigned int y) override;

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelPositionAlign()
   */
  bool SetInputPanelPositionAlign(int x, int y, Dali::InputMethodContext::InputPanelAlign align) override;

  /**
   * @copydoc Dali::InputMethodContext::GetPreeditStyle()
   */
  void GetPreeditStyle(Dali::Integration::InputMethodContext::PreEditAttributeDataContainer& attrs) const override;

public:
  /**
   * Destructor.
   */
  virtual ~InputMethodContextGeneric();

private:
  // Undefined copy constructor
  InputMethodContextGeneric(const InputMethodContextGeneric& inputMethodContext) = delete;

  // Undefined assignment operator
  InputMethodContextGeneric& operator=(const InputMethodContextGeneric& inputMethodContext) = delete;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_INPUT_METHOD_CONTEXT_IMPL_GENERIC_H
