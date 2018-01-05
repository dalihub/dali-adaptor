#ifndef __DALI_INTERNAL_INPUT_METHOD_CONTEXT_H
#define __DALI_INTERNAL_INPUT_METHOD_CONTEXT_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <Ecore_IMF.h>
#include <Ecore_X.h>

#include <dali/public-api/object/base-object.h>

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

class DALI_IMPORT_API InputMethodContext : public Dali::BaseObject
{
public:
  typedef Dali::InputMethodContext::ActivatedSignalType ActivatedSignalType;
  typedef Dali::InputMethodContext::KeyboardEventSignalType KeyboardEventSignalType;
  typedef Dali::InputMethodContext::StatusSignalType StatusSignalType;
  typedef Dali::InputMethodContext::VoidSignalType VoidSignalType;
  typedef Dali::InputMethodContext::KeyboardTypeSignalType KeyboardTypeSignalType;
  typedef Dali::InputMethodContext::KeyboardResizedSignalType KeyboardResizedSignalType;
  typedef Dali::InputMethodContext::LanguageChangedSignalType LanguageChangedSignalType;

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
  void Initialize();

public:

  /**
   * @copydoc Dali::InputMethodContext::Finalize()
   */
  void Finalize();

  /**
   * Connect Callbacks required for InputMethodContext.
   * If you don't connect InputMethodContext callbacks, you can't get the key events.
   * The events are PreeditChanged, Commit and DeleteSurrounding.
   */
  void ConnectCallbacks();

  /**
   * Disconnect Callbacks attached to InputMethodContext context.
   */
  void DisconnectCallbacks();

  /**
   * @copydoc Dali::InputMethodContext::Activate()
   */
  void Activate();

  /**
   * @copydoc Dali::InputMethodContext::Deactivate()
   */
  void Deactivate();

  /**
   * @copydoc Dali::InputMethodContext::Reset()
   */
  void Reset();

  /**
   * @copydoc Dali::InputMethodContext::GetContext()
   */
  Ecore_IMF_Context* GetContext();

  /**
   * @copydoc Dali::InputMethodContext::RestoreAfterFocusLost()
   */
  bool RestoreAfterFocusLost() const;

  /**
   * @copydoc Dali::InputMethodContext::SetRestoreAfterFocusLost()
   */
  void SetRestoreAfterFocusLost( bool toggle );

  /**
   * @copydoc Dali::InputMethodContext::PreEditChanged()
   */
  void PreEditChanged( void* data, Ecore_IMF_Context* imfContext, void* event_info );

  /**
   * @copydoc Dali::InputMethodContext::NotifyCursorPosition()
   */
  void CommitReceived( void* data, Ecore_IMF_Context* imfContext, void* event_info );

  /**
   * @copydoc Dali::InputMethodContext::NotifyCursorPosition()
   */
  Eina_Bool RetrieveSurrounding( void* data, Ecore_IMF_Context* imfContext, char** text, int* cursorPosition );

  /**
   * @copydoc Dali::InputMethodContext::DeleteSurrounding()
   */
  void DeleteSurrounding( void* data, Ecore_IMF_Context* imfContext, void* event_info );

  // Cursor related
  /**
   * @copydoc Dali::InputMethodContext::NotifyCursorPosition()
   */
  void NotifyCursorPosition();

  /**
   * @copydoc Dali::InputMethodContext::SetCursorPosition()
   */
  void SetCursorPosition( unsigned int cursorPosition );

  /**
   * @copydoc Dali::InputMethodContext::GetCursorPosition()
   */
  unsigned int GetCursorPosition() const;

  /**
   * @copydoc Dali::InputMethodContext::SetSurroundingText()
   */
  void SetSurroundingText( const std::string& text );

  /**
   * @copydoc Dali::InputMethodContext::GetSurroundingText()
   */
  const std::string& GetSurroundingText() const;

  /**
  * @copydoc Dali::InputMethodContext::NotifyTextInputMultiLine()
  */
  void NotifyTextInputMultiLine( bool multiLine );

  /**
  * @copydoc Dali::InputMethodContext::GetTextDirection()
  */
  Dali::InputMethodContext::TextDirection GetTextDirection();

  /**
  * @copydoc Dali::InputMethodContext::GetInputMethodArea()
  */
  Dali::Rect<int> GetInputMethodArea();

  /**
  * @copydoc Dali::InputMethodContext::ApplyOptions()
  */
  void ApplyOptions( const InputMethodOptions& options );

  /**
   * @copydoc Dali::InputMethodContext::SetInputPanelData()
   */
  void SetInputPanelData( const std::string& data );

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelData()
   */
  void GetInputPanelData( std::string& data );

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelState()
   */
  Dali::InputMethodContext::State GetInputPanelState();

  /**
   * @copydoc Dali::InputMethodContext::SetReturnKeyState()
   */
  void SetReturnKeyState( bool visible );

  /**
   * @copydoc Dali::InputMethodContext::AutoEnableInputPanel()
   */
  void AutoEnableInputPanel( bool enabled );

  /**
   * @copydoc Dali::InputMethodContext::ShowInputPanel()
   */
  void ShowInputPanel();

  /**
   * @copydoc Dali::InputMethodContext::HideInputPanel()
   */
  void HideInputPanel();

  /**
   * @copydoc Dali::InputMethodContext::GetKeyboardType()
   */
  Dali::InputMethodContext::KeyboardType GetKeyboardType();

  /**
   * @copydoc Dali::InputMethodContext::GetInputPanelLocale()
   */
  std::string GetInputPanelLocale();

  /**
   * @copydoc Dali::FilterEventKey()
   */
  bool FilterEventKey( const Dali::KeyEvent& keyEvent );

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
  KeyboardResizedSignalType& ResizedSignal() { return mKeyboardResizeSignal; }

  /**
   * @copydoc Dali::InputMethodContext::LanguageChangedSignal()
   */
  LanguageChangedSignalType& LanguageChangedSignal() { return mKeyboardLanguageChangedSignal; }

  /**
   * @copydoc Dali::InputMethodContext::KeyboardTypeChangedSignal()
   */
  KeyboardTypeSignalType& KeyboardTypeChangedSignal() { return mKeyboardTypeChangedSignal; }

private:
  /**
   * Context created the first time and kept until deleted.
   * @param[in] ecoreXwin, The window is created by application.
   */
  void CreateContext( Ecore_X_Window ecoreXwin );

  /**
   * @copydoc Dali::InputMethodContext::DeleteContext()
   */
  void DeleteContext();

public:

  /**
   * Constructor
   */
  InputMethodContext() = default;


  /**
   * Destructor.
   */
  virtual ~InputMethodContext();

private:
  /**
   * Constructor
   * @param[in] ecoreXwin, The window is created by application.
   */
  InputMethodContext( Ecore_X_Window ecoreXwin );

  // Undefined copy constructor
  InputMethodContext( const InputMethodContext& inputMethodContext) = delete;

  // Undefined assignment operator
  InputMethodContext& operator=( const InputMethodContext& inputMethodContext ) = delete;

private:
  Ecore_IMF_Context* mIMFContext;
  Ecore_X_Window mEcoreXwin;
  int mIMFCursorPosition;
  std::string mSurroundingText;

  bool mRestoreAfterFocusLost:1;             ///< Whether the keyboard needs to be restored (activated ) after focus regained.
  bool mIdleCallbackConnected:1;             ///< Whether the idle callback is already connected.
  InputMethodOptions        mOptions;

  ActivatedSignalType        mActivatedSignal;
  KeyboardEventSignalType    mEventSignal;
  StatusSignalType           mKeyboardStatusSignal;
  KeyboardResizedSignalType  mKeyboardResizeSignal;
  LanguageChangedSignalType  mKeyboardLanguageChangedSignal;
  KeyboardTypeSignalType     mKeyboardTypeChangedSignal;

public:

DALI_IMPORT_API inline static Internal::Adaptor::InputMethodContext& GetImplementation(Dali::InputMethodContext& inputMethodContext)
{
  DALI_ASSERT_ALWAYS( inputMethodContext && "InputMethodContext handle is empty" );

  BaseObject& handle = inputMethodContext.GetBaseObject();

  return static_cast<Internal::Adaptor::InputMethodContext&>(handle);
}

DALI_IMPORT_API inline static const  Internal::Adaptor::InputMethodContext& GetImplementation(const Dali::InputMethodContext& inputMethodContext)
{
  DALI_ASSERT_ALWAYS( inputMethodContext && "InputMethodContext handle is empty" );

  const BaseObject& handle = inputMethodContext.GetBaseObject();

  return static_cast<const Internal::Adaptor::InputMethodContext&>(handle);
}

};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_INPUT_METHOD_CONTEXT_H
