#ifndef __DALI_INTERNAL_IMF_MANAGER_H
#define __DALI_INTERNAL_IMF_MANAGER_H

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
#include <Ecore_X.h>

#include <dali/public-api/object/base-object.h>
#include <imf-manager.h>
#include <dali/integration-api/events/key-event-integ.h>

// INTERNAL INCLUDES


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class RenderSurface;

class ImfManager : public Dali::BaseObject
{
public:
  typedef Dali::ImfManager::ImfManagerSignalV2 ImfManagerSignalV2;
  typedef Dali::ImfManager::ImfEventSignalV2 ImfEventSignalV2;

public:

  /**
   * Create the IMF manager.
   */
  static Dali::ImfManager Get();

  /**
   * Constructor
   * @param[in] ecoreXwin, The window is created by application.
   */
  ImfManager( Ecore_X_Window ecoreXwin );

  /**
   * Connect Callbacks required for IMF.
   * If you don't connect imf callbacks, you can't get the key events.
   * The events are PreeditChanged, Commit and DeleteSurrounding.
   */
  void ConnectCallbacks();

  /**
   * Disconnect Callbacks attached to imf context.
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
  Ecore_IMF_Context* GetContext();

  /**
   * @copydoc Dali::ImfManager::RestoreAfterFocusLost()
   */
  bool RestoreAfterFocusLost() const;

  /**
   * @copydoc Dali::ImfManager::SetRestoreAferFocusLost()
   */
  void SetRestoreAferFocusLost( bool toggle );

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

  // Cursor related
  /**
   * @copydoc Dali::ImfManager::NotifyCursorPosition()
   */
  void NotifyCursorPosition();

  /**
   * @copydoc Dali::ImfManager::GetCursorPosition()
   */
  int GetCursorPosition();

  /**
   * @copydoc Dali::ImfManager::SetCursorPosition()
   */
  void SetCursorPosition( unsigned int cursorPosition );

  /**
   * @copydoc Dali::ImfManager::SetSurroundingText()
   */
  void SetSurroundingText( std::string text );

  /**
   * @copydoc Dali::ImfManager::GetSurroundingText()
   */
  std::string GetSurroundingText();

public:  // Signals

  /**
   * @copydoc Dali::ImfManager::ActivatedSignal()
   */
  ImfManagerSignalV2& ActivatedSignal() { return mActivatedSignalV2; }

  /**
   * @copydoc Dali::ImfManager::EventReceivedSignal()
   */
  ImfEventSignalV2& EventReceivedSignal() { return mEventSignalV2; }

protected:

  /**
   * Destructor.
   */
  virtual ~ImfManager();

private:
  /**
   * Context created the first time and kept until deleted.
   * @param[in] ecoreXwin, The window is created by application.
   */
  void CreateContext( Ecore_X_Window ecoreXwin );

  /**
   * @copydoc Dali::ImfManager::DeleteContext()
   */
  void DeleteContext();

private:
  // Undefined
  ImfManager( const ImfManager& );
  ImfManager& operator=( ImfManager& );

private:
  Ecore_IMF_Context* mIMFContext;
  int mIMFCursorPosition;
  std::string mSurroundingText;

  bool mRestoreAfterFocusLost:1;             ///< Whether the keyboard needs to be restored (activated ) after focus regained.
  bool mIdleCallbackConnected:1;             ///< Whether the idle callback is already connected.

  std::vector<Dali::Integration::KeyEvent> mKeyEvents; ///< Stores key events to be sent from idle call-back.

  ImfManagerSignalV2      mActivatedSignalV2;
  ImfEventSignalV2        mEventSignalV2;

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

#endif // __DALI_INTERNAL_IMF_MANAGER_H
