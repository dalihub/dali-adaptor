#ifndef DALI_INTERNAL_OFFSCREEN_APPLICATION_IMPL_H
#define DALI_INTERNAL_OFFSCREEN_APPLICATION_IMPL_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <memory>

#include <dali/integration-api/adaptor-framework/scene-holder-impl.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/signals/connection-tracker.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/offscreen-application.h>
#include <dali/devel-api/adaptor-framework/offscreen-window.h>


namespace Dali
{
class Adaptor;

namespace Internal
{

/**
 * Implementation of the OffscreenApplication class.
 */
class OffscreenApplication : public BaseObject,
                   public ConnectionTracker
{
public:

  typedef Dali::OffscreenApplication::OffscreenApplicationSignalType OffscreenApplicationSignalType;

  /**
   * @brief Create a new OffscreenApplication
   * @param[in] width The width of the default OffscreenWindow
   * @param[in] height The height of the default OffscreenWindow
   * @param[in] surface The native surface handle to create the default OffscreenWindow
   * @param[in] isTranslucent Whether the OffscreenWindow is translucent or not
   */
  static IntrusivePtr<OffscreenApplication> New( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent);

public:

  /**
   * @copydoc Dali::OffscreenApplication::Start()
   */
  void Run();

  /**
   * @copydoc Dali::OffscreenApplication::Stop()
   */
  void Stop();

  /**
   * @copydoc Dali::OffscreenApplication::GetDefaultWindow()
   */
  Dali::OffscreenWindow GetWindow();

  /**
   * @copydoc Dali::OffscreenApplication::InitSignal()
   */
  OffscreenApplicationSignalType& InitSignal()
  {
    return mInitSignal;
  }

  /**
   * @copydoc Dali::OffscreenApplication::TerminateSignal()
   */
  OffscreenApplicationSignalType& TerminateSignal()
  {
    return mTerminateSignal;
  }

public:
  /**
   * All methods here are for the internal use
   */
  enum State
  {
    READY,
    RUNNING,
    STOPPED,
  };

  /**
   * @brief This is for internal use to get notified before InitSignal() emits
   */
  OffscreenApplicationSignalType& PreInitSignal()
  {
    return mPreInitSignal;
  }

  /**
   * @brief Get OffscreenApplication's current state
   *
   * @return The state
   */
  const State GetState() const
  {
    return mState;
  }

  /*
   * @bried Get currently working Adaptor. It can be null.
   */
  Dali::Adaptor* GetAdaptor();

private:
  /**
   * Private constructor
   * @param[in] width The width of the OffscreenWindow
   * @param[in] height The height of the OffscreenApplication
   * @param[in] surface The native surface handle to create the default OffscreenWindow
   * @param[in] isTranslucent Whether the OffscreenWindow is translucent or not
   */
  OffscreenApplication( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent );


  /**
   * Destructor
   */
  virtual ~OffscreenApplication();

  // Undefined
  OffscreenApplication( const OffscreenApplication& );
  OffscreenApplication& operator=( OffscreenApplication& );

private:
  Dali::Adaptor*                           mAdaptor;
  Dali::OffscreenWindow                     mDefaultWindow;

  OffscreenApplicationSignalType              mPreInitSignal;
  OffscreenApplicationSignalType              mInitSignal;
  OffscreenApplicationSignalType              mTerminateSignal;

  State                                    mState;
};

inline OffscreenApplication& GetImplementation( Dali::OffscreenApplication& offscreenApplication )
{
  DALI_ASSERT_ALWAYS( offscreenApplication && "OffscreenApplication handle is empty" );

  BaseObject& handle = offscreenApplication.GetBaseObject();

  return static_cast<OffscreenApplication&>( handle );
}

inline const OffscreenApplication& GetImplementation( const Dali::OffscreenApplication& offscreenApplication )
{
  DALI_ASSERT_ALWAYS( offscreenApplication && "OffscreenApplication handle is empty" );

  const BaseObject& handle = offscreenApplication.GetBaseObject();

  return static_cast<const OffscreenApplication&>( handle );
}

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_OFFSCREEN_APPLICATION_IMPL_H
