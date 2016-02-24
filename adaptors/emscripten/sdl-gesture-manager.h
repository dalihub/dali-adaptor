#ifndef __DALI_SDL_GESTURE_MANAGER_H__
#define __DALI_SDL_GESTURE_MANAGER_H__

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
#include <dali/integration-api/gesture-manager.h>
#include <dali/public-api/common/dali-common.h>

namespace Dali
{

/**
 * Concrete implementation of the gesture manager class.
 *
 * A stubb class to give to core in the Emscripten/browser environment
 */
class DALI_IMPORT_API SdlGestureManager : public Dali::Integration::GestureManager
{

public:

  /**
   * Constructor
   */
  SdlGestureManager()
  {
    Initialize();
  }

  /**
   * Destructor
   */
  virtual ~SdlGestureManager()
  {
  }

  /**
   * @copydoc Dali::Integration::GestureManager::Register(Gesture::Type)
   */
  virtual void Register(const Integration::GestureRequest& request)
  {
    mFunctionsCalled.Register = true;
  }

  /**
   * @copydoc Dali::Integration::GestureManager::Unregister(Gesture::Type)
   */
  virtual void Unregister(const Integration::GestureRequest& request)
  {
    mFunctionsCalled.Unregister = true;
  }

  /**
   * @copydoc Dali::Integration::GestureManager::Update(Gesture::Type)
   */
  virtual void Update(const Integration::GestureRequest& request)
  {
    mFunctionsCalled.Update = true;
  }

public: // TEST FUNCTIONS

  // Enumeration of Gesture Manager methods
  enum SdlFuncEnum
  {
    RegisterType,
    UnregisterType,
    UpdateType,
  };

  /** Call this every test */
  void Initialize()
  {
    mFunctionsCalled.Reset();
  }

  bool WasCalled(SdlFuncEnum func)
  {
    switch(func)
    {
      case RegisterType:             return mFunctionsCalled.Register;
      case UnregisterType:           return mFunctionsCalled.Unregister;
      case UpdateType:               return mFunctionsCalled.Update;
    }
    return false;
  }

  void ResetCallStatistics(SdlFuncEnum func)
  {
    switch(func)
    {
      case RegisterType:             mFunctionsCalled.Register = false; break;
      case UnregisterType:           mFunctionsCalled.Unregister = false; break;
      case UpdateType:               mFunctionsCalled.Update = false; break;
    }
  }

private:

  struct SdlFunctions
  {
    SdlFunctions()
    : Register(false),
      Unregister(false),
      Update(false)
    {
    }

    void Reset()
    {
      Register = false;
      Unregister = false;
      Update = false;
    }

    bool Register;
    bool Unregister;
    bool Update;
  };

  SdlFunctions mFunctionsCalled;
};

} // Dali

#endif // __DALI_SDL_GESTURE_MANAGER_H__
