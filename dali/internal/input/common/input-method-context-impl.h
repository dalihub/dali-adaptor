#ifndef DALI_INTERNAL_INPUT_COMMON_INPUT_METHOD_CONTEXT_IMPL_H
#define DALI_INTERNAL_INPUT_COMMON_INPUT_METHOD_CONTEXT_IMPL_H

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
#include <functional>
#include <memory>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/object/base-object.h>
#include <dali/integration-api/events/key-event-integ.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/input-method-context.h>

namespace Dali
{

class RenderSurface;

namespace Internal
{

namespace Adaptor
{
class InputMethodContext;
typedef IntrusivePtr< InputMethodContext > InputMethodContextPtr;
using ImfContext = void;

class InputMethodContext : public Dali::BaseObject
{

public:

  using ActivatedSignalType = Dali::InputMethodContext::ActivatedSignalType;
  using KeyboardEventSignalType = Dali::InputMethodContext::KeyboardEventSignalType;
  using StatusSignalType = Dali::InputMethodContext::StatusSignalType;
  using VoidSignalType = Dali::InputMethodContext::VoidSignalType;
  using KeyboardTypeSignalType = Dali::InputMethodContext::KeyboardTypeSignalType;
  using LanguageChangedSignalType = Dali::InputMethodContext::LanguageChangedSignalType;
  using KeyboardResizedSignalType = Dali::InputMethodContext::KeyboardResizedSignalType;
  using ContentReceivedSignalType = Dali::InputMethodContext::ContentReceivedSignalType;

public:

  /**
   * Create a new input method context instance.
   */
  static InputMethodContextPtr New( Dali::Actor actor );

  /**
   * Initialize the object.
   */
  virtual void Initialize() {}

  /**
   * Connect Callbacks required for InputMethodContext.
   * If you don't connect InputMethodContext callbacks, you can't get the key events.
   * The events are PreeditChanged, Commit, DeleteSurrounding and PrivateCommand.
   */
  virtual void ConnectCallbacks() {}

  /**
   * Disconnect Callbacks attached to input method context.
   */
  virtual void DisconnectCallbacks() {}

  /**
   * @copydoc Dali::InputMethodContext::Finalize()
   */
  virtual void Finalize() {}

  /**
   * @copydoc Dali::InputMethodContext::Activate()
   */
  virtual void Activate() {}

  /**
   * @copydoc Dali::InputMethodContext::Deactivate()
   */
  virtual void Deactivate() {}

  /**
   * @copydoc Dali::InputMethodContext::Reset()
   */
  virtual void Reset() {}

  /**
   * @copydoc Dali::InputMethodContext::GetContext()
   */
  virtual ImfContext* GetContext() { return nullptr; }

  /**
   * @copydoc Dali::InputMethodContext::RestoreAfterFocusLost()
   */
  virtual bool RestoreAfterFocusLost() const { return false; }

  /**
   * @copydoc Dali::InputMethodContext::SetRestoreAfterFocusLost()
   */
  virtual void SetRestoreAfterFocusLost( bool toggle ) {}

  /**
   * @copydoc Dali::InputMethodContext::PreEditChanged()
   */
  virtual void PreEditChanged( void *data, ImfContext* imfContext, void *eventInfo ) {}

  /**
   * @copydoc Dali::InputMethodContext::CommitReceived()
   */
  virtual void CommitReceived( void *data, ImfContext* imfContext, void *eventInfo ) {}

  /**
   * @copydoc Dali::InputMethodContext::RetrieveSurrounding()
   */
  /*Eina_Bool*/
  virtual bool RetrieveSurrounding( void *data, ImfContext* imfContext, char** text, int* cursorPosition ) { return false; }

  /**
   * @copydoc Dali::InputMethodContext::DeleteSurrounding()
   */
  virtual void DeleteSurrounding( void *data, ImfContext* imfContext, void *eventInfo ) {}

  /**
   * @copydoc Dali::InputMethodContext::SendPrivateCommand()
   */
  virtual void SendPrivateCommand( void* data, ImfContext* imfContext, void* eventInfo ) {}

  /**
   * @copydoc Dali::InputMethodContext::SendCommitContent()
   */
  virtual void SendCommitContent( void* data, ImfContext* imfContext, void* eventInfo ) {}

  // Cursor related
  /**
   * @copydoc Dali::InputMethodContext::NotifyCursorPosition()
   */
  virtual void NotifyCursorPosition() {}

  /**
   * @copydoc Dali::InputMethodContext::SetCursorPosition()
   */
  virtual void SetCursorPosition( unsigned int cursorPosition ) {}

  /**
   * @copydoc Dali::InputMethodContext::GetCursorPosition()
   */
  virtual unsigned int GetCursorPosition() const { return 0; }

  /**
   * @copydoc Dali::InputMethodContext::SetSurroundingText()
   */
  virtual void SetSurroundingText( const std::string& text ) {}

  /**
   * @copydoc Dali::InputMethodContext::GetSurroundingText()
   */
  virtual const std::string& GetSurroundingText() const;

  /**
  * @copydoc Dali::InputMethodContext::NotifyTextInputMultiLine()
  */
  virtual void NotifyTextInputMultiLine( bool multiLine ) {}

  /**
  * @copydoc Dali::InputMethodContext::GetTextDirection()
  */
  virtual Dali::InputMethodContext::TextDirection GetTextDirection() { return Dali::InputMethodContext::TextDirection(); }

  /**
  * @copydoc Dali::InputMethodContext::GetInputMethodArea()
  */
  virtual Dali::Rect<int> GetInputMethodArea() { return Dali::Rect<int>(); }

  /**
  * @copydoc Dali::InputMethodContext::ApplyOptions()
  */
  virtual void ApplyOptions( const InputMethodOptions& options ) {}

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelData()
   */
  virtual void SetInputPanelData( const std::string& data ) {}

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelData()
   */
  virtual void GetInputPanelData( std::string& data ) {}

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelState()
   */
  virtual Dali::InputMethodContext::State GetInputPanelState() { return Dali::InputMethodContext::State(); }

  /**
   * @copydoc Dali::InputMethodContext::SetReturnKeyState()
   */
  virtual void SetReturnKeyState( bool visible ) {}

  /**
   * @copydoc Dali::InputMethodContext::AutoEnableInputPanel()
   */
  virtual void AutoEnableInputPanel( bool enabled ) {}

  /**
   * @copydoc Dali::InputMethodContext::ShowInputPanel()
   */
  virtual void ShowInputPanel() {}

  /**
   * @copydoc Dali::InputMethodContext::HideInputPanel()
   */
  virtual void HideInputPanel() {}

  /**
   * @copydoc Dali::InputMethodContext::GetKeyboardType()
   */
  virtual Dali::InputMethodContext::KeyboardType GetKeyboardType() { return Dali::InputMethodContext::KeyboardType(); }

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelLocale()
   */
  virtual std::string GetInputPanelLocale() { return std::string(); }

  /**
   * @copydoc Dali::InputMethodContext::SetContentMIMETypes()
   */
  virtual void SetContentMIMETypes( const std::string& mimeTypes ) {}

  /**
   * @copydoc Dali::InputMethodContext::FilterEventKey()
   */
  virtual bool FilterEventKey( const Dali::KeyEvent& keyEvent ) { return false; }

  /**
   * @copydoc Dali::InputMethodContext::AllowTextPrediction()
   */
  virtual void AllowTextPrediction( bool prediction ) {}

  /**
   * @copydoc Dali::InputMethodContext::IsTextPredictionAllowed()
   */
  virtual bool IsTextPredictionAllowed() const { return false; }

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelLanguage()
   */
  virtual void SetInputPanelLanguage( Dali::InputMethodContext::InputPanelLanguage language ) {}

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelLanguage()
   */
  virtual Dali::InputMethodContext::InputPanelLanguage GetInputPanelLanguage() const { return Dali::InputMethodContext::InputPanelLanguage(); }

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelPosition()
   */
  virtual void SetInputPanelPosition( unsigned int x, unsigned int y ) {}

  /**
   * @copydoc Dali::InputMethodContext::GetPreeditStyle()
   */
  virtual Dali::InputMethodContext::PreeditStyle GetPreeditStyle() const { return Dali::InputMethodContext::PreeditStyle(); }

public:  // Signals

  /**
   * @copydoc Dali::InputMethodContext::ActivatedSignal()
   */
  ActivatedSignalType& ActivatedSignal() { return mActivatedSignal; }

  /**
   * @copydoc Dali::InputMethodContext::EventReceivedSignal()
   */
  KeyboardEventSignalType& EventReceivedSignal() { return mEventSignal; }

  /**
   * @copydoc Dali::InputMethodContext::StatusChangedSignal()
   */
  StatusSignalType& StatusChangedSignal() { return mKeyboardStatusSignal; }

  /**
   * @copydoc Dali::InputMethodContext::ResizedSignal()
   */
  KeyboardResizedSignalType& ResizedSignal()  { return mKeyboardResizeSignal; }

  /**
   * @copydoc Dali::InputMethodContext::LanguageChangedSignal()
   */
  LanguageChangedSignalType& LanguageChangedSignal() { return mKeyboardLanguageChangedSignal; }

  /**
   * @copydoc Dali::InputMethodContext::KeyboardTypeChangedSignal()
   */
  KeyboardTypeSignalType& KeyboardTypeChangedSignal() { return mKeyboardTypeChangedSignal; }

  /**
   * @copydoc Dali::InputMethodContext::ContentReceivedSignal()
   */
  ContentReceivedSignalType& ContentReceivedSignal() { return mContentReceivedSignal; }

public:

  /**
   * Constructor
   */
  InputMethodContext();

  /**
   * Destructor
   */
  ~InputMethodContext() override = default;

private:

  InputMethodContext( const InputMethodContext& ) = delete;
  InputMethodContext& operator=( InputMethodContext& )  = delete;

protected:
  /**
   * @brief Struct for providing Operation enumeration
   */
  struct Operation
  {
    enum Type
    {
      ALLOW_TEXT_PREDICTION = 0,
      AUTO_ENABLE_INPUT_PANEL,
      NOTIFY_TEXT_INPUT_MULTILINE,
      SET_CONTENT_MIME_TYPES,
      SET_INPUT_PANEL_DATA,
      SET_INPUT_PANEL_LANGUAGE,
      SET_INPUT_PANEL_POSITION,
      SET_RETURN_KEY_STATE,
      MAX_COUNT
    };
  };

  using OperationList = std::vector< std::function<void()> >;

  /**
   * @brief Apply backup operations to the InputMethodContext
   */
  void ApplyBackupOperations();

protected:

  ActivatedSignalType        mActivatedSignal;
  KeyboardEventSignalType    mEventSignal;
  StatusSignalType           mKeyboardStatusSignal;
  KeyboardResizedSignalType  mKeyboardResizeSignal;
  LanguageChangedSignalType  mKeyboardLanguageChangedSignal;
  KeyboardTypeSignalType     mKeyboardTypeChangedSignal;
  ContentReceivedSignalType  mContentReceivedSignal;
  OperationList              mBackupOperations;

public:

  inline static Internal::Adaptor::InputMethodContext& GetImplementation(Dali::InputMethodContext& inputMethodContext)
  {
    DALI_ASSERT_ALWAYS( inputMethodContext && "InputMethodContext handle is empty" );

    BaseObject& handle = inputMethodContext.GetBaseObject();

    return static_cast<Internal::Adaptor::InputMethodContext&>(handle);
  }

  inline static const Internal::Adaptor::InputMethodContext& GetImplementation(const Dali::InputMethodContext& inputMethodContext)
  {
    DALI_ASSERT_ALWAYS( inputMethodContext && "InputMethodContext handle is empty" );

    const BaseObject& handle = inputMethodContext.GetBaseObject();

    return static_cast<const Internal::Adaptor::InputMethodContext&>(handle);
  }

};


} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_INPUT_COMMON_INPUT_METHOD_CONTEXT_IMPL_H
