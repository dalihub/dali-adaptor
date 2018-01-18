#ifndef DALI_INTERNAL_INPUT_COMMON_IMF_MANAGER_IMPL_H
#define DALI_INTERNAL_INPUT_COMMON_IMF_MANAGER_IMPL_H

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/object/base-object.h>
#include <dali/integration-api/events/key-event-integ.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/imf-manager.h>

#include <memory>

namespace Dali
{

class RenderSurface;

namespace Internal
{

namespace Adaptor
{

using ImfContext = void;

class DALI_IMPORT_API ImfManager : public Dali::BaseObject
{

public:

  using ImfManagerSignalType = Dali::ImfManager::ImfManagerSignalType;
  using ImfEventSignalType = Dali::ImfManager::ImfEventSignalType;
  using ImfStatusSignalType = Dali::ImfManager::StatusSignalType;
  using ImfVoidSignalType = Dali::ImfManager::VoidSignalType;
  using ImfKeyboardTypeSignalType = Dali::ImfManager::KeyboardTypeSignalType;
  using ImfLanguageChangedSignalType = Dali::ImfManager::LanguageChangedSignalType;
  using ImfKeyboardResizedSignalType = Dali::ImfManager::KeyboardResizedSignalType;

public:

  /**
   * Check whether the ImfManager is available.
   * @return true if available, false otherwise
   */
  static bool IsAvailable();

  /**
   * Get the IMF manager instance, it creates the instance if it has not already been created.
   * Internally, a check should be made using IsAvailable() before this is called as we do not want
   * to create an instance if not needed by applications.
   * @see IsAvailable()
   */
  static Dali::ImfManager Get();

  /**
   * Connect Callbacks required for IMF.
   * If you don't connect imf callbacks, you can't get the key events.
   * The events are PreeditChanged, Commit, DeleteSurrounding and PrivateCommand.
   */
  virtual void ConnectCallbacks();

  /**
   * Disconnect Callbacks attached to imf context.
   */
  virtual void DisconnectCallbacks();

  /**
   * @copydoc Dali::ImfManager::Activate()
   */
  virtual void Activate();

  /**
   * @copydoc Dali::ImfManager::Deactivate()
   */
  virtual void Deactivate();

  /**
   * @copydoc Dali::ImfManager::Reset()
   */
  virtual void Reset();

  /**
   * @copydoc Dali::ImfManager::GetContext()
   */
  virtual ImfContext* GetContext();

  /**
   * @copydoc Dali::ImfManager::RestoreAfterFocusLost()
   */
  virtual bool RestoreAfterFocusLost() const;

  /**
   * @copydoc Dali::ImfManager::SetRestoreAfterFocusLost()
   */
  virtual void SetRestoreAfterFocusLost( bool toggle );

  /**
   * @copydoc Dali::ImfManager::PreEditChanged()
   */
  virtual void PreEditChanged( void *data, ImfContext* imfContext, void *event_info );

  /**
   * @copydoc Dali::ImfManager::NotifyCursorPosition()
   */
  virtual void CommitReceived( void *data, ImfContext* imfContext, void *event_info );

  /**
   * @copydoc Dali::ImfManager::NotifyCursorPosition()
   */
  /*Eina_Bool*/
  virtual bool RetrieveSurrounding( void *data, ImfContext* imfContext, char** text, int* cursorPosition ) ;

  /**
   * @copydoc Dali::ImfManager::DeleteSurrounding()
   */
  virtual void DeleteSurrounding( void *data, ImfContext* imfContext, void *event_info ) ;

  /**
   * @copydoc Dali::ImfManager::SendPrivateCommand()
   */
  virtual void SendPrivateCommand( void* data, ImfContext* imfContext, void* event_info ) ;

  // Cursor related
  /**
   * @copydoc Dali::ImfManager::NotifyCursorPosition()
   */
  virtual void NotifyCursorPosition();

  /**
   * @copydoc Dali::ImfManager::SetCursorPosition()
   */
  virtual void SetCursorPosition( unsigned int cursorPosition ) ;

  /**
   * @copydoc Dali::ImfManager::GetCursorPosition()
   */
  virtual unsigned int GetCursorPosition() const;

  /**
   * @copydoc Dali::ImfManager::SetSurroundingText()
   */
  virtual void SetSurroundingText( const std::string& text ) ;

  /**
   * @copydoc Dali::ImfManager::GetSurroundingText()
   */
  virtual const std::string& GetSurroundingText() const;

  /**
  * @copydoc Dali::ImfManager::NotifyTextInputMultiLine()
  */
  virtual void NotifyTextInputMultiLine( bool multiLine ) ;

  /**
  * @copydoc Dali::ImfManager::GetTextDirection()
  */
  virtual Dali::ImfManager::TextDirection GetTextDirection() ;

  /**
  * @copydoc Dali::ImfManager::GetInputMethodArea()
  */
  virtual Dali::Rect<int> GetInputMethodArea() ;

  /**
  * @copydoc Dali::ImfManager::ApplyOptions()
  */
  virtual void ApplyOptions( const InputMethodOptions& options ) ;

  /**
   * @copydoc Dali::ImfManager::SetInputPanelData()
   */
  virtual void SetInputPanelData( const std::string& data ) ;

  /**
   * @copydoc Dali::ImfManager::GetInputPanelData()
   */
  virtual void GetInputPanelData( std::string& data ) ;

  /**
   * @copydoc Dali::ImfManager::GetInputPanelState()
   */
  virtual Dali::ImfManager::State GetInputPanelState() ;

  /**
   * @copydoc Dali::ImfManager::SetReturnKeyState()
   */
  virtual void SetReturnKeyState( bool visible ) ;

  /**
   * @copydoc Dali::ImfManager::AutoEnableInputPanel()
   */
  virtual void AutoEnableInputPanel( bool enabled ) ;

  /**
   * @copydoc Dali::ImfManager::ShowInputPanel()
   */
  virtual void ShowInputPanel() ;

  /**
   * @copydoc Dali::ImfManager::HideInputPanel()
   */
  virtual void HideInputPanel() ;

  /**
   * @copydoc Dali::ImfManager::GetKeyboardType()
   */
  virtual Dali::ImfManager::KeyboardType GetKeyboardType() ;

  /**
   * @copydoc Dali::ImfManager::GetInputPanelLocale()
   */
  virtual std::string GetInputPanelLocale() ;

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
  ImfKeyboardResizedSignalType& ResizedSignal()  { return mKeyboardResizeSignal; }

  /**
   * @copydoc Dali::ImfManager::LanguageChangedSignal()
   */
  ImfLanguageChangedSignalType& LanguageChangedSignal() { return mKeyboardLanguageChangedSignal; }

  /**
   * @copydoc Dali::ImfManager::KeyboardTypeChangedSignal()
   */
  ImfKeyboardTypeSignalType& KeyboardTypeChangedSignal() { return mKeyboardTypeChangedSignal; }

protected:

  /**
   * Constructor
   */
  ImfManager() = default;

  /**
   * Destructor
   */
  ~ImfManager() override;

private:

  ImfManager( const ImfManager& ) = delete;
  ImfManager& operator=( ImfManager& )  = delete;

protected:

  ImfManagerSignalType          mActivatedSignal;
  ImfEventSignalType            mEventSignal;
  ImfStatusSignalType           mKeyboardStatusSignal;
  ImfKeyboardResizedSignalType  mKeyboardResizeSignal;
  ImfLanguageChangedSignalType  mKeyboardLanguageChangedSignal;
  ImfKeyboardTypeSignalType     mKeyboardTypeChangedSignal;

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

#endif // DALI_INTERNAL_INPUT_COMMON_IMF_MANAGER_IMPL_H