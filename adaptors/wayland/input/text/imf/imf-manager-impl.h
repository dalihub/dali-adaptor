#ifndef __DALI_INTERNAL_IMF_MANAGER_WL_H
#define __DALI_INTERNAL_IMF_MANAGER_WL_H

/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/events/key-event-integ.h>

// INTERNAL INCLUDES
#include <imf-manager.h>
#include "../text-input-manager.h"

namespace Dali
{

class RenderSurface;

namespace Internal
{

namespace Adaptor
{

/**
 * @brief ImfManager
 *
 * Handles text input editing with the virtual keyboard.
 * The Tizen 3 Wayland text interface is still in development so some
 * features are not available to test like text prediction.
 * When this available we may need to add / test wl_text_input_commit_state
 *
 * To debug low level communication to the Wayland Compositor (Enlightenment)  use environment variable
 * export WAYLAND_DEBUG=1
 *
 */
class ImfManager : public Dali::BaseObject, public ConnectionTracker
{
public:
  typedef Dali::ImfManager::ImfManagerSignalType ImfManagerSignalType;
  typedef Dali::ImfManager::ImfEventSignalType ImfEventSignalType;
  typedef Dali::ImfManager::StatusSignalType ImfStatusSignalType;
  typedef Dali::ImfManager::VoidSignalType ImfVoidSignalType;

public:

  /**
   * @brief Check whether the ImfManager is available.
   * @return true if available, false otherwise
   */
  static bool IsAvailable();

  /**
   * @brief Get the IMF manager instance
   * It creates the instance if it has not already been created.
   * Internally, a check should be made using IsAvailable() before this is called as we do not want
   * to create an instance if not needed by applications.
   * @see IsAvailable()
   * @return handle to ImfManager
   */
  static Dali::ImfManager Get();

  /**
   * @brief Constructor
   */
  ImfManager();

  /**
   * Connect Callbacks required for IMF.
   * If you don't connect imf callbacks, you can't get the key events.
   * The events are PreeditChanged, Commit and DeleteSurrounding.
   */
  void ConnectCallbacks();

  /**
   * @brief Disconnect Callbacks attached to imf context.
   */
  void DisconnectCallbacks();

  /**
   * @copydoc Dali::ImfManager::Activate()
   */
  void Activate();

  /**
   * @copydoc Dali::ImfManager::Deactivate()
   */
  void Deactivate();

  /**
   * @copydoc Dali::ImfManager::Reset()
   */
  void Reset();

  /**
   * @copydoc Dali::ImfManager::GetContext()
   */
  void* GetContext();

  /**
   * @copydoc Dali::ImfManager::RestoreAfterFocusLost()
   */
  bool RestoreAfterFocusLost() const;

  /**
   * @copydoc Dali::ImfManager::SetRestoreAfterFocusLost()
   */
  void SetRestoreAfterFocusLost( bool toggle );


  // Cursor related
  /**
   * @copydoc Dali::ImfManager::NotifyCursorPosition()
   */
  void NotifyCursorPosition();

  /**
   * @copydoc Dali::ImfManager::SetCursorPosition()
   */
  void SetCursorPosition( unsigned int cursorPosition );

  /**
   * @copydoc Dali::ImfManager::GetCursorPosition()
   */
  unsigned int GetCursorPosition() const;

  /**
   * @copydoc Dali::ImfManager::SetSurroundingText()
   */
  void SetSurroundingText( const std::string& text );

  /**
   * @copydoc Dali::ImfManager::GetSurroundingText()
   */
  const std::string& GetSurroundingText() const;

  /**
  * @copydoc Dali::ImfManager::NotifyTextInputMultiLine()
  */
  void NotifyTextInputMultiLine( bool multiLine );

  /**
  * @copydoc Dali::ImfManager::GetTextDirection()
  */
  Dali::ImfManager::TextDirection GetTextDirection();

  /**
  * @copydoc Dali::ImfManager::GetInputMethodArea()
  */
  Dali::Rect<int> GetInputMethodArea();

  /**
  * @copydoc Dali::ImfManager::ApplyOptions()
  */
  void ApplyOptions( const InputMethodOptions& options );

  /**
   * @copydoc Dali::ImfManager::SetInputPanelUserData()
   */
  void SetInputPanelUserData( const std::string& data );

  /**
   * @copydoc Dali::ImfManager::GetInputPanelUserData()
   */
  void GetInputPanelUserData( std::string& data );

  /**
   * @copydoc Dali::ImfManager::GetInputPanelState()
   */
  Dali::ImfManager::State GetInputPanelState();

  /**
   * @copydoc Dali::ImfManager::SetReturnKeyState()
   */
  void SetReturnKeyState( bool visible );

  /**
   * @copydoc Dali::ImfManager::AutoEnableInputPanel()
   */
  void AutoEnableInputPanel( bool enabled );

  /**
   * @copydoc Dali::ImfManager::ShowInputPanel()
   */
  void ShowInputPanel();

  /**
   * @copydoc Dali::ImfManager::HideInputPanel()
   */
  void HideInputPanel();

public:  // Signals

  /**
   * @copydoc Dali::ImfManager::ActivatedSignal()
   */
  ImfManagerSignalType& ActivatedSignal() { return mActivatedSignal; }

  /**
   * @copydoc Dali::ImfManager::EventReceivedSignal()
   */
  ImfEventSignalType& EventReceivedSignal() { return mEventSignal; }

  /**
   * @copydoc Dali::ImfManager::StatusChangedSignal()
   */
  ImfStatusSignalType& StatusChangedSignal() { return mKeyboardStatusSignal; }

  /**
   * @copydoc Dali::ImfManager::ResizedSignal()
   */
  ImfVoidSignalType& ResizedSignal() { return mKeyboardResizeSignal; }

  /**
   * @copydoc Dali::ImfManager::LanguageChangedSignal()
   */
  ImfVoidSignalType& LanguageChangedSignal() { return mKeyboardLanguageChangedSignal; }


  /**
   * @brief Called when an IMF Pre-Edit change event is received.
   * We are still predicting what the user is typing.  The latest string is what the IMF module thinks
   * the user wants to type.
   *
   * @param[in] serial event serial
   * @param[in] text pre-edit string
   * @param[in] commit commit string
   */
  void PreEditStringChange( unsigned int serial, const std::string text, const std::string commit );

  /**
   * @brief Called when an IMF Pre-Edit cursor event is received.
   * @param[in] cursor cursor position
   */
  void PreEditCursorChange( int cursor );

  /**
   * @brief called when IMF tell us to commit the text
   * @param[in] serial event serial
   * @param[in] commit text to commit
   */
  void CommitString( unsigned int serial, const std::string commit );

  /**
   * @brief called when deleting surround text
   * @param[in] index character index to start deleting from
   * @param[in] length number of characters to delete
   */
  void DeleteSurroundingText( int index, unsigned int length );

protected:

  /**
   * @brief Destructor.
   */
  virtual ~ImfManager();


private:

  ImfManagerSignalType      mActivatedSignal;
  ImfEventSignalType        mEventSignal;
  ImfStatusSignalType       mKeyboardStatusSignal;
  ImfVoidSignalType         mKeyboardResizeSignal;
  ImfVoidSignalType         mKeyboardLanguageChangedSignal;

  // Undefined
  ImfManager( const ImfManager& );
  ImfManager& operator=( ImfManager& );

private:

  TextInputManager& mTextInputManager;
  std::string mSurroundingText;
  int mPreEditCursorPosition;
  int mEditCursorPosition;
  bool mRestoreAfterFocusLost:1;  ///< Whether the keyboard needs to be restored (activated ) after focus regained.

public:

inline static Internal::Adaptor::ImfManager& GetImplementation(Dali::ImfManager& imfManager)
{
  DALI_ASSERT_ALWAYS( imfManager && "ImfManager handle is empty" );

  BaseObject& handle = imfManager.GetBaseObject();

  return static_cast<Internal::Adaptor::ImfManager&>(handle);
}

inline static const  Internal::Adaptor::ImfManager& GetImplementation(const Dali::ImfManager& imfManager)
{
  DALI_ASSERT_ALWAYS( imfManager && "ImfManager handle is empty" );

  const BaseObject& handle = imfManager.GetBaseObject();

  return static_cast<const Internal::Adaptor::ImfManager&>(handle);
}

};


} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_IMF_MANAGER_WL_H
