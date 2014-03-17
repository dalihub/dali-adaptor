#ifndef __DALI_EVAS_PLUGIN_H__
#define __DALI_EVAS_PLUGIN_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/**
 * @addtogroup CAPI_DALI_ADAPTOR_MODULE
 * @{
 */

// EXTERNAL INCLUDES
#include <boost/any.hpp>
#include <boost/function.hpp>

#include <dali/public-api/signals/dali-signal-v2.h>

// Declare this forward declaration ourselves to remove unnecessary dependencies in
// public headers.
extern "C"
{
  struct _Evas_Object;
  typedef struct _Evas_Object Evas_Object; ///< Forward declaration of Evas_Object
}


namespace Dali DALI_IMPORT_API
{
class Adaptor;

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class EvasPlugin;
}
}

/**
 * @brief Application class used by EFL applications that wish to use Dali.
 *
 * An EvasPlugin class object should be created by EFL application
 * that wishes to use Dali.  It provides a means for initialising the
 * resources required by the Dali::Core.
 *
 * The EvasPlugin class emits several signals which the user can
 * connect to.  The user should not create any Dali objects in the main
 * function and instead should connect to the Init signal of the
 * EvasPlugin and create the Dali objects in the connected callback.
 *
 * SLP and EFL applications should follow the example below:
 *
 * @code
 * void Created(EvasPlugin& evasPlugin)
 * {
 *   // Create Dali components...
 *   // Can instantiate here, if required
 * }
 *
 * void Resized(EvasPlugin& evasPlugin)
 * {
 *   // Set size properties of Dali components
 *   // Set screen layout
 * }
 *
 * int main (int argc, char **argv)
 * {
 *   elm_init(&argc, &argv);
 *
 *   Evas* e;
 *   Evas_Object* win, eo;
 *   win = elm_win_add(...);
 *   Dali::EvasPlugin evasPlugin = Dali::EvasPlugin(win);
 *   evasPlugin.InitSignal().Connect(&Created);
 *   evasPlugin.ResizeSignal().Connect(&Resized);
 *
 *   eo = evasPlugin.GetEvasImageObject();
 *   evas_object_show(eo);
 *
 *   // add eo to layout such as elm_box
 *
 *   elm_run();
 * }
 * @endcode
 *
 * If required, you can also connect class member functions to a signal:
 *
 * @code
 * MyEvasPlugin mep;
 * mep.ResumeSignal().Connect(&app, &MyEvasPlugin::Resume);
 * @endcode
 */
class EvasPlugin : public ConnectionTrackerInterface
{
public:

  typedef SignalV2< void (EvasPlugin&) > EvasPluginSignalV2; ///< Generic evas plugin signal type

public:

  /**
   * @brief This is the constructor for SLP EFL applications
   * @param[in]  parent  A parent of new evas object
   * @param[in]  isTransparent Whether the object is transparent or not
   * @param[in]  initialWidth The initial width of Dali view port
   * @param[in]  initialHeight The initial height of Dali view port
   */
  EvasPlugin(Evas_Object* parent, bool isTransparent = false, unsigned int initialWidth = 1, unsigned int initialHeight = 1);

  /**
   * @brief Virtual destructor.
   */
  virtual ~EvasPlugin();

public:

  /**
   * @brief Run Evas Plugin.
   */
  void Run();

  /**
   * @brief Pause Evas Plugin.
   */
  void Pause();

  /**
   * @brief Resume Evas Plugin.
   */
  void Resume();

  /**
   * @brief Stop Evas Plugin.
   */
  void Stop();

  /**
   * @brief This returns Evas_Object* which is created internally.
   * @return Evas_Object* evas image object which is drawen by dali.
   */
  Evas_Object* GetEvasImageObject();

  /**
   * @brief This returns Evas_Object* which is created internally.
   *
   * Application should append this access object to custom focus chain for accessibility
   *
   * e.g. elm_object_focus_custom_chain_append(layout, dali_access_object, NULL);
   *
   * @return Evas_Object* elm access object which dali image object is registered.
   */
  Evas_Object* GetElmAccessObject();

  /**
   * @brief This returns Evas_Object* which is created internally.
   *
   * If application want to handle the keyboard focus among the efl and dali view part,
   * application should set this object to efl layout instead of the evas image object from GetEvasImageObject()
   * @return Evas_Object* evas object which can handle the focus internally. It is contained the image object.
   */
  Evas_Object* GetElmFocusObject();

  /**
   * @brief This returns the internal Adaptor instance.
   *
   * @return Adaptor* adaptor instance.
   */
   Dali::Adaptor* GetAdaptor();

public:  // Signals

  /**
   * @brief Signal to notify the client when the application is ready to be initialized.
   * @return The signal
   */
  EvasPluginSignalV2& InitSignal();

  /**
   * @brief Signal to notify client when Dali has rendered at least one frame.
   *
   * The user should connect to this signal to be notified when Dali has started rendering
   * and atleast one frame has been rendered.
   * @return The signal
   */
  EvasPluginSignalV2& FirstRenderCompletedSignal();

  /**
   * @brief Signal to notify the user when the application is about to be terminated.
   *
   * @return The signal
   */
  EvasPluginSignalV2& TerminateSignal();

  /**
   * @brief Signal to notify the client when the adaptor is about to be paused.
   *
   * The user should connect to this signal if they need to perform any special
   * activities when the application is about to be paused.
   * @return The signal
   */
  EvasPluginSignalV2& PauseSignal();

  /**
   * @brief Signal to notify the client when the adpator has resumed.
   *
   * The user should connect to this signal if they need to perform any special
   * activities when the application has resumed.
   * @return The signal
   */
  EvasPluginSignalV2& ResumeSignal();

  /**
   * @brief Signal to notify the client when the evas object is resized.
   * @return The signal
   */
  EvasPluginSignalV2& ResizeSignal();

  /**
   * @brief Signal to notify the client when the evas object get the keyboard focus.
   * @return The signal
   */
  EvasPluginSignalV2& FocusedSignal();

  /**
   * @brief Signal to notify the client when the evas object lost the keyboard focus.
   * @return The signal
   */
  EvasPluginSignalV2& UnFocusedSignal();

protected:
  /**
   * @copydoc ConnectionTrackerInterface::SignalConnected
   */
  virtual void SignalConnected( SlotObserver* slotObserver, CallbackBase* callback );

  /**
   * @copydoc ConnectionTrackerInterface::SignalDisconnected
   */
  virtual void SignalDisconnected( SlotObserver* slotObserver, CallbackBase* callback );

  /**
   * @copydoc ConnectionTrackerInterface::GetConnectionCount
   */
  virtual std::size_t GetConnectionCount() const;

private:

  // Undefined
  EvasPlugin(const EvasPlugin&);
  EvasPlugin& operator=(EvasPlugin&);

private:

  Internal::Adaptor::EvasPlugin *mImpl; ///< Pointer to Implementation
  friend class Internal::Adaptor::EvasPlugin;
};

} // namespace Dali

/**
 * @}
 */
#endif // __DALI_EVAS_PLUGIN_H__
