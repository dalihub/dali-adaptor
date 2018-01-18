#ifndef DALI_INTERNAL_INPUT_COMMON_IMF_MANAGER_IMPL_ECORE_WL_H
#define DALI_INTERNAL_INPUT_COMMON_IMF_MANAGER_IMPL_ECORE_WL_H

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
#include <Ecore_IMF.h>
#include <Ecore.h>
#include <Ecore_Wayland.h>

#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/object/base-object.h>
#include <dali/integration-api/events/key-event-integ.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/imf-manager-impl.h>

namespace Dali
{

class RenderSurface;

namespace Internal
{

namespace Adaptor
{

class DALI_IMPORT_API ImfManagerEcoreWl : public Dali::Internal::Adaptor::ImfManager
{
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
  void ConnectCallbacks() override;

  /**
   * Disconnect Callbacks attached to imf context.
   */
  void DisconnectCallbacks() override;

  /**
   * @copydoc Dali::ImfManager::Activate()
   */
  void Activate() override;

  /**
   * @copydoc Dali::ImfManager::Deactivate()
   */
  void Deactivate() override;

  /**
   * @copydoc Dali::ImfManager::Reset()
   */
  void Reset() override;

  /**
   * @copydoc Dali::ImfManager::GetContext()
   */
  ImfContext* GetContext() override;

  /**
   * @copydoc Dali::ImfManager::RestoreAfterFocusLost()
   */
  bool RestoreAfterFocusLost() const override;

  /**
   * @copydoc Dali::ImfManager::SetRestoreAfterFocusLost()
   */
  void SetRestoreAfterFocusLost( bool toggle ) override;

  /**
   * @copydoc Dali::ImfManager::PreEditChanged()
   */
  void PreEditChanged( void *data, Ecore_IMF_Context *imfContext, void *event_info );

  /**
   * @copydoc Dali::ImfManager::NotifyCursorPosition()
   */
  void CommitReceived( void *data, Ecore_IMF_Context *imfContext, void *event_info );

  /**
   * @copydoc Dali::ImfManager::NotifyCursorPosition()
   */
  Eina_Bool RetrieveSurrounding( void *data, Ecore_IMF_Context *imfContext, char** text, int* cursorPosition );

  /**
   * @copydoc Dali::ImfManager::DeleteSurrounding()
   */
  void DeleteSurrounding( void *data, Ecore_IMF_Context *imfContext, void *event_info );

  /**
   * @copydoc Dali::ImfManager::SendPrivateCommand()
   */
  void SendPrivateCommand( void* data, Ecore_IMF_Context* imfContext, void* event_info );

  // Cursor related
  /**
   * @copydoc Dali::ImfManager::NotifyCursorPosition()
   */
  void NotifyCursorPosition() override;

  /**
   * @copydoc Dali::ImfManager::SetCursorPosition()
   */
  void SetCursorPosition( unsigned int cursorPosition ) override;

  /**
   * @copydoc Dali::ImfManager::GetCursorPosition()
   */
  unsigned int GetCursorPosition() const override;

  /**
   * @copydoc Dali::ImfManager::SetSurroundingText()
   */
  void SetSurroundingText( const std::string& text ) override;

  /**
   * @copydoc Dali::ImfManager::GetSurroundingText()
   */
  const std::string& GetSurroundingText() const override;

  /**
  * @copydoc Dali::ImfManager::NotifyTextInputMultiLine()
  */
  void NotifyTextInputMultiLine( bool multiLine ) override;

  /**
  * @copydoc Dali::ImfManager::GetTextDirection()
  */
  Dali::ImfManager::TextDirection GetTextDirection() override;

  /**
  * @copydoc Dali::ImfManager::GetInputMethodArea()
  */
  Dali::Rect<int> GetInputMethodArea() override;

  /**
  * @copydoc Dali::ImfManager::ApplyOptions()
  */
  void ApplyOptions( const InputMethodOptions& options ) override;

  /**
   * @copydoc Dali::ImfManager::SetInputPanelData()
   */
  void SetInputPanelData( const std::string& data ) override;

  /**
   * @copydoc Dali::ImfManager::GetInputPanelData()
   */
  void GetInputPanelData( std::string& data ) override;

  /**
   * @copydoc Dali::ImfManager::GetInputPanelState()
   */
  Dali::ImfManager::State GetInputPanelState() override;

  /**
   * @copydoc Dali::ImfManager::SetReturnKeyState()
   */
  void SetReturnKeyState( bool visible ) override;

  /**
   * @copydoc Dali::ImfManager::AutoEnableInputPanel()
   */
  void AutoEnableInputPanel( bool enabled ) override;

  /**
   * @copydoc Dali::ImfManager::ShowInputPanel()
   */
  void ShowInputPanel() override;

  /**
   * @copydoc Dali::ImfManager::HideInputPanel()
   */
  void HideInputPanel() override;

  /**
   * @copydoc Dali::ImfManager::GetKeyboardType()
   */
  Dali::ImfManager::KeyboardType GetKeyboardType() override;

  /**
   * @copydoc Dali::ImfManager::GetInputPanelLocale()
   */
  std::string GetInputPanelLocale() override;

protected:

  /**
   * Destructor.
   */
  virtual ~ImfManagerEcoreWl() override;

private:
  /**
   * Context created the first time and kept until deleted.
   * @param[in] ecoreWlwin, The window is created by application.
   */
  void CreateContext( Ecore_Wl_Window *ecoreWlwin );

  /**
   * @copydoc Dali::ImfManager::DeleteContext()
   */
  void DeleteContext();

private:
  // Undefined
  explicit ImfManagerEcoreWl( Ecore_Wl_Window *ecoreWlwin );
  explicit ImfManagerEcoreWl( const ImfManager& ) = delete;
  ImfManagerEcoreWl& operator=( ImfManager& ) = delete;

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

#endif // DALI_INTERNAL_INPUT_COMMON_IMF_MANAGER_IMPL_ECORE_WL_H
