#ifndef __SDL_RENDER_CONTROLLER_H__
#define __SDL_RENDER_CONTROLLER_H__

/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of Dali

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
*/

// INTERNAL INCLUDES
#include <dali/integration-api/render-controller.h>

namespace Dali
{

/**
 * This class is a stubb RenderController to pass to core in the Emscripten/browser environment.
 *
 */
class DALI_IMPORT_API SdlRenderController : public Dali::Integration::RenderController
{
public:
  SdlRenderController()
  {
    Initialize();
  }

  ~SdlRenderController()
  {
  }

  /**
   * @copydoc Dali::Integration::RenderController::RequestUpdate()
   */
  virtual void RequestUpdate()
  {
    mRequestUpdateCalled = true;
  }

  /**
   * @copydoc Dali::Integration::RenderController::RequestProcessEventsOnIdle()
   */
  virtual void RequestProcessEventsOnIdle()
  {
  }

  /**
   * @copydoc Dali::Integration::RenderController::RequestNotificationEventOnIdle()
   */
  virtual void RequestNotificationEventOnIdle()
  {
    mRequestNotificationEventOnIdleCalled = true;
  }

  void Initialize()
  {
    mRequestUpdateCalled = false;
    mRequestNotificationEventOnIdleCalled = false;
  }

private:
  bool mRequestUpdateCalled;
  bool mRequestNotificationEventOnIdleCalled;
};

} // Dali

#endif
