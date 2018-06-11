#ifndef __DALI_INTERNAL_BASE_INDICATOR_INTERFACE_H__
#define __DALI_INTERNAL_BASE_INDICATOR_INTERFACE_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/actors/actor.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

class Adaptor;

/**
 * @brief The Indicator interface.
 * Can be used to draw an indicator graphic generated from the platform as an overlay on
 * top of DALi scene contents.
 */
class IndicatorInterface
{
public:

  /**
   * @brief Type of indiciator
   */
  enum Type
  {
    INDICATOR_TYPE_UNKNOWN,
    INDICATOR_TYPE_1,
    INDICATOR_TYPE_2
  };


public:

  /**
   * @brief observer for listening to indicator events
   */
  class Observer
  {
  public:
    /**
     * @brief Notify the observer if the indicator type changes
     * @param[in] type The new indicator type
     */
    virtual void IndicatorTypeChanged( Type type ) = 0;

    /**
     * @brief Notify the observer when the upload has completed.
     * @param[in] indicator The indicator that has finished uploading.
     */
    virtual void IndicatorClosed(IndicatorInterface* indicator) = 0;

    /**
     * @brief Notify the observer when the indicator visible status is changed.
     * @param[in] isShowing Whether the indicator is visible.
     */
    virtual void IndicatorVisibilityChanged( bool isVisible ) = 0;
  };


public:

  /**
   * @brief constructor
   */
  IndicatorInterface() {}

  /**
   * @brief Virtual Destructor
   */
  virtual ~IndicatorInterface() {}

  /**
   * @brief assign the adaptor to this object
   * @param[in] adaptor
   */
  virtual void SetAdaptor(Adaptor* adaptor) = 0;

  /**
   * @brief Get the actor which contains the indicator image. Ensure that the handle is
   * released when no longer needed.
   * Changes from the indicator service will modify the image and resize the actor appropriately.
   * @return The indicator actor.
   */
  virtual Dali::Actor GetActor() = 0;

  /**
   * Opens a new connection for the required orientation.
   * @param[in] orientation The new orientation
   */
  virtual void Open( Dali::Window::WindowOrientation orientation ) = 0;

  /**
   * Close the current connection. Will respond with Observer::IndicatorClosed()
   * when done.
   * @note, IndicatorClosed() will be called synchronously if there's no update
   * in progress, or asychronously if waiting for SignalUploaded )
   */
  virtual void Close() = 0;

  /**
   * Notify the indicator flicked.
   */
  virtual void Flicked() = 0;

  /**
   * Set the opacity mode of the indicator background.
   * @param[in] mode opacity mode
   */
  virtual void SetOpacityMode( Dali::Window::IndicatorBgOpacity mode ) = 0;

  /**
   * Set whether the indicator is visible or not.
   * @param[in] visibleMode visible mode for indicator bar.
   * @param[in] forceUpdate true if want to change visible mode forcely
   */
  virtual void SetVisible( Dali::Window::IndicatorVisibleMode visibleMode, bool forceUpdate = false ) = 0;

  /**
   * Check whether the indicator is connected to the indicator service.
   * @return whether the indicator is connected or not.
   */
  virtual bool IsConnected() = 0;

  /**
   * Send message to the indicator service.
   * @param[in] messageDomain Message Reference number
   * @param[in] messageId Reference number of the message this message refers to
   * @param[in] data The data to send as part of the message
   * @param[in] size Length of the data, in bytes, to send
   * @return whether the message is sent successfully or not
   */
  virtual bool SendMessage( int messageDomain, int messageId, const void *data, int size ) = 0;
};

} // Adaptor
} // Internal
} // Dali

#endif
