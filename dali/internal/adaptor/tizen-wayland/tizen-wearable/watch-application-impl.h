#ifndef DALI_INTERNAL_WATCH_APPLICATION_H
#define DALI_INTERNAL_WATCH_APPLICATION_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/application-impl.h>
#include <dali/public-api/watch/watch-application.h>

namespace Dali
{
class Adaptor;

namespace Internal
{
namespace Adaptor
{
class WatchApplication;
typedef IntrusivePtr<WatchApplication> WatchApplicationPtr;

enum WatchApplicationState
{
  UNINITIALIZED,
  INITIALIZED,
  PAUSED,
  RESUMED = INITIALIZED,
  TERMINATED
};

/**
 * Implementation of the WatchApplication class.
 */
class WatchApplication : public Application
{
public:
  typedef Dali::WatchApplication::WatchTimeSignal WatchTimeSignal;
  typedef Dali::WatchApplication::WatchBoolSignal WatchBoolSignal;

  /**
   * Create a new watch
   * @param[in]  argc        A pointer to the number of arguments
   * @param[in]  argv        A pointer to the argument list
   * @param[in]  stylesheet  The path to user defined theme file
   * @param[in]  windowData  The window data
   */
  static WatchApplicationPtr New(int* argc, char** argv[], const std::string& stylesheet, const WindowData& windowData);

  /**
   * Private Constructor
   * @param[in]  argc        A pointer to the number of arguments
   * @param[in]  argv        A pointer to the argument list
   * @param[in]  stylesheet  The path to user defined theme file
   * @param[in]  windowData  The window data
   */
  WatchApplication(int* argc, char** argv[], const std::string& stylesheet, const WindowData& windowData);

  /**
   * Destructor
   */
  virtual ~WatchApplication();

  /**
   * Called when the framework is initialised.
   */
  void OnInit() override;

  /**
   * Called when the framework is terminated.
   */
  void OnTerminate() override;

  /**
   * Called when the framework is paused.
   */
  void OnPause() override;

  /**
   * Called when the framework resumes from a paused state.
   */
  void OnResume() override;

  /**
   * Called every second
   */
  void OnTimeTick(WatchTime& time);

  /**
   * Called every second in ambient mode
   */
  void OnAmbientTick(WatchTime& time);

  /**
   * Called when the device enters or exits ambient mode
   */
  void OnAmbientChanged(bool ambient);

private:
  // @brief Undefined copy constructor.
  WatchApplication(const WatchApplication&);

  // @brief Undefined assignment operator.
  WatchApplication& operator=(const WatchApplication&);

public:
  // Signals
  WatchTimeSignal mTickSignal;
  WatchTimeSignal mAmbientTickSignal;
  WatchBoolSignal mAmbientChangeSignal;

private:
  WatchApplicationState mState;
};

inline WatchApplication& GetImplementation(Dali::WatchApplication& watch)
{
  DALI_ASSERT_ALWAYS(watch && "watch handle is empty");

  BaseObject& handle = watch.GetBaseObject();

  return static_cast<Internal::Adaptor::WatchApplication&>(handle);
}

inline const WatchApplication& GetImplementation(const Dali::WatchApplication& watch)
{
  DALI_ASSERT_ALWAYS(watch && "Time handle is empty");

  const BaseObject& handle = watch.GetBaseObject();

  return static_cast<const Internal::Adaptor::WatchApplication&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_WATCH_APPLICATION_H
