#ifndef DALI_INTERNAL_ADAPTOR_ANDROIDJNI_FRAMEWORK_H
#define DALI_INTERNAL_ADAPTOR_ANDROIDJNI_FRAMEWORK_H

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
 */

// INTERNAL INCLUDES
#include <dali/internal/adaptor/android/framework-android-interface.h>
#include <dali/internal/adaptor/common/framework.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * FrameworkAndroidJni class provides an Framework Android JNI implementation.
 */
class FrameworkAndroidJni : public Framework, public FrameworkAndroidInterface
{
public:
  /**
   * @copydoc Dali::Internal::Adaptor::Framework()
   */
  FrameworkAndroidJni(Observer& observer, TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread);

  /**
   * Destructor
   */
  ~FrameworkAndroidJni();

public:
  /**
   * @copydoc Dali::Internal::Adaptor::Framework::Run()
   */
  void Run() override;

  /**
   * @copydoc Dali::Internal::Adaptor::Framework::Quit()
   */
  void Quit() override;

  /**
   * @copydoc Dali::Internal::Adaptor::FrameworkAndroidInterface::AppStatusHandler()
   */
  bool AppStatusHandler(int type, void* data) override;

  /**
   * @copydoc Dali::Internal::Adaptor::FrameworkAndroidInterface::AddIdle()
   */
  unsigned int AddIdle(int timeout, void* data, bool (*callback)(void* data)) override;

  /**
   * @copydoc Dali::Internal::Adaptor::FrameworkAndroidInterface::RemoveIdle()
   */
  void RemoveIdle(unsigned int id) override;

private:
  // Undefined
  FrameworkAndroidJni(const FrameworkAndroidJni&);
  FrameworkAndroidJni& operator=(FrameworkAndroidJni&);

private:
  bool mInitialised;

private: // impl members
  struct Impl;
  Impl* mImpl;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_ANDROIDJNI_FRAMEWORK_H
