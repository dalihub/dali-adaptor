#ifndef DALI_INTERNAL_INPUT_METHOD_CONTEXT_IMPL_ECORE_WL_H
#define DALI_INTERNAL_INPUT_METHOD_CONTEXT_IMPL_ECORE_WL_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/input/linux/dali-ecore-imf.h>
#include <dali/internal/system/linux/dali-ecore.h>

#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/object/base-object.h>
#include <dali/integration-api/events/key-event-integ.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/input-method-context-impl.h>

namespace Dali
{

class RenderSurface;

namespace Internal
{

namespace Adaptor
{

class InputMethodContextEcoreWl : public Dali::Internal::Adaptor::InputMethodContext
{
public:

  /**
   * @brief Creates a new InputMethodContext handle
   *
   * @return InputMethodContext pointer
   */
  static InputMethodContextPtr New();

  /**
   * @brief Initializes member data.
   */
  void Initialize() override;

public:

  /**
   * @copydoc Dali::InputMethodContext::Finalize()
   */
  void Finalize() override;

  /**
   * Connect Callbacks required for InputMethodContext.
   * If you don't connect InputMethodContext callbacks, you can't get the key events.
   * The events are PreeditChanged, Commit, DeleteSurrounding and PrivateCommand.
   */
  void ConnectCallbacks() override;

  /**
   * Disconnect Callbacks attached to input method context.
   */
  void DisconnectCallbacks() override;

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
  void SetRestoreAfterFocusLost( bool toggle ) override;

  /**
   * @copydoc Dali::InputMethodContext::PreEditChanged()
   */
  void PreEditChanged( void *data, ImfContext *imfContext, void *eventInfo ) override;

  /**
   * @copydoc Dali::InputMethodContext::NotifyCursorPosition()
   */
  void CommitReceived( void *data, ImfContext *imfContext, void *eventInfo ) override;

  /**
   * @copydoc Dali::InputMethodContext::NotifyCursorPosition()
   */
  bool RetrieveSurrounding( void *data, ImfContext *imfContext, char** text, int* cursorPosition ) override;

  /**
   * @copydoc Dali::InputMethodContext::DeleteSurrounding()
   */
  void DeleteSurrounding( void *data, ImfContext *imfContext, void *eventInfo ) override;

 /**
  * @copydoc Dali::InputMethodContext::SendPrivateCommand()
  */
  void SendPrivateCommand( void* data, ImfContext* imfContext, void* eventInfo ) override;

  /**
   * @copydoc Dali::InputMethodContext::SendCommitContent()
   */
   void SendCommitContent( void* data, ImfContext* imfContext, void* eventInfo ) override;

  // Cursor related
  /**
   * @copydoc Dali::InputMethodContext::NotifyCursorPosition()
   */
  void NotifyCursorPosition() override;

  /**
   * @copydoc Dali::InputMethodContext::SetCursorPosition()
   */
  void SetCursorPosition( unsigned int cursorPosition ) override;

  /**
   * @copydoc Dali::InputMethodContext::GetCursorPosition()
   */
  unsigned int GetCursorPosition() const override;

  /**
   * @copydoc Dali::InputMethodContext::SetSurroundingText()
   */
  void SetSurroundingText( const std::string& text ) override;

  /**
   * @copydoc Dali::InputMethodContext::GetSurroundingText()
   */
  const std::string& GetSurroundingText() const override;

  /**
  * @copydoc Dali::InputMethodContext::NotifyTextInputMultiLine()
  */
  void NotifyTextInputMultiLine( bool multiLine ) override;

  /**
  * @copydoc Dali::InputMethodContext::GetTextDirection()
  */
  Dali::InputMethodContext::TextDirection GetTextDirection() override;

  /**
  * @copydoc Dali::InputMethodContext::GetInputMethodArea()
  */
  Dali::Rect<int> GetInputMethodArea() override;

  /**
  * @copydoc Dali::InputMethodContext::ApplyOptions()
  */
  void ApplyOptions( const InputMethodOptions& options ) override;

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelData()
   */
  void SetInputPanelData( const std::string& data ) override;

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelData()
   */
  void GetInputPanelData( std::string& data ) override;

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelState()
   */
  Dali::InputMethodContext::State GetInputPanelState() override;

  /**
   * @copydoc Dali::InputMethodContext::SetReturnKeyState()
   */
  void SetReturnKeyState( bool visible ) override;

  /**
   * @copydoc Dali::InputMethodContext::AutoEnableInputPanel()
   */
  void AutoEnableInputPanel( bool enabled ) override;

  /**
   * @copydoc Dali::InputMethodContext::ShowInputPanel()
   */
  void ShowInputPanel() override;

  /**
   * @copydoc Dali::InputMethodContext::HideInputPanel()
   */
  void HideInputPanel() override;

  /**
   * @copydoc Dali::InputMethodContext::GetKeyboardType()
   */
  Dali::InputMethodContext::KeyboardType GetKeyboardType() override;

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelLocale()
   */
  std::string GetInputPanelLocale() override;

  /**
   * @copydoc Dali::InputMethodContext::SetContentMIMETypes()
   */
  void SetContentMIMETypes( const std::string& mimeTypes ) override;

  /**
   * @copydoc Dali::InputMethodContext::FilterEventKey()
   */
  bool FilterEventKey( const Dali::KeyEvent& keyEvent ) override;

  /**
   * @copydoc Dali::InputMethodContext::AllowTextPrediction()
   */
  void AllowTextPrediction( bool prediction ) override;

  /**
   * @copydoc Dali::InputMethodContext::IsTextPredictionAllowed()
   */
  bool IsTextPredictionAllowed() const override;

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelLanguage()
   */
  void SetInputPanelLanguage( Dali::InputMethodContext::InputPanelLanguage language ) override;

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelLanguage()
   */
  Dali::InputMethodContext::InputPanelLanguage GetInputPanelLanguage() const override;

private:
  /**
   * Context created the first time and kept until deleted.
   */
  void CreateContext();

  /**
   * @copydoc Dali::InputMethodContext::DeleteContext()
   */
  void DeleteContext();

private:

  /**
   * @brief Process event key down, whether filter a key to isf.
   *
   * @param[in] keyEvent The event key to be handled.
   * @return Whether the event key is handled.
   */
  bool ProcessEventKeyDown( const KeyEvent& keyEvent );

  /**
   * @brief Process event key up, whether filter a key to isf.
   *
   * @param[in] keyEvent The event key to be handled.
   * @return Whether the event key is handled.
   */
  bool ProcessEventKeyUp( const KeyEvent& keyEvent );

  /**
  * Ecore_Event_Modifier enums in Ecore_Input.h do not match Ecore_IMF_Keyboard_Modifiers in Ecore_IMF.h.
  * This function converts from Ecore_Event_Modifier to Ecore_IMF_Keyboard_Modifiers enums.
  * @param[in] ecoreModifier the Ecore_Event_Modifier input.
  * @return the Ecore_IMF_Keyboard_Modifiers output.
  */
  Ecore_IMF_Keyboard_Modifiers EcoreInputModifierToEcoreIMFModifier( unsigned int ecoreModifier );

  /**
   * EcoreInputModifierToEcoreIMFLock function converts from Ecore_Event_Modifier to Ecore_IMF_Keyboard_Locks enums.
   * @param[in] modifier the Ecore_Event_Modifier input.
   * @return the Ecore_IMF_Keyboard_Locks output.
   */
  Ecore_IMF_Keyboard_Locks EcoreInputModifierToEcoreIMFLock( unsigned int modifier );

private:
  /**
   * @brief Constructor.
   */
  explicit InputMethodContextEcoreWl();

protected:
  /**
   * Destructor.
   */
  virtual ~InputMethodContextEcoreWl() override;

private:
  // Undefined copy constructor
  explicit InputMethodContextEcoreWl( const InputMethodContextEcoreWl& inputMethodContext) = delete;

  // Undefined assignment operator
  InputMethodContextEcoreWl& operator=( const InputMethodContextEcoreWl& inputMethodContext ) = delete;

private:
  Ecore_IMF_Context* mIMFContext;
  int mIMFCursorPosition;
  std::string mSurroundingText;

  bool mRestoreAfterFocusLost:1;             ///< Whether the keyboard needs to be restored (activated ) after focus regained.
  bool mIdleCallbackConnected:1;             ///< Whether the idle callback is already connected.

  std::vector<Dali::Integration::KeyEvent> mKeyEvents; ///< Stores key events to be sent from idle call-back.
  InputMethodOptions        mOptions;
};


} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_INPUT_METHOD_CONTEXT_IMPL_ECORE_WL_H
