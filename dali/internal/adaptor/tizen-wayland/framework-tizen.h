#ifndef DALI_INTERNAL_ADAPTOR_TIZEN_FRAMEWORK_H
#define DALI_INTERNAL_ADAPTOR_TIZEN_FRAMEWORK_H

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
#include <dali/internal/adaptor/common/framework.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * FrameworkTizen class provides an Framework Tizen implementation.
 */
class FrameworkTizen : public Framework
{
public:
  /**
   * @copydoc Dali::Internal::Adaptor::Framework()
   */
  FrameworkTizen(Observer& observer, TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread);

  /**
   * Destructor
   */
  ~FrameworkTizen();

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
   * Gets bundle name which was passed in app_reset callback.
   */
  std::string GetBundleName() const;

  /**
   * Gets bundle id which was passed in app_reset callback.
   */
  std::string GetBundleId() const;

  /**
   * Sets system language.
   */
  void SetLanguage(const std::string& language);

  /**
   * Sets system region.
   */
  void SetRegion(const std::string& region);

  /**
   * Gets system language.
   */
  std::string GetLanguage() const override;

  /**
   * Gets system region.
   */
  std::string GetRegion() const override;

private:
  // Undefined
  FrameworkTizen(const FrameworkTizen&);
  FrameworkTizen& operator=(FrameworkTizen&);

private:
  /**
   * Called when the application is created.
   */
  bool Create();

  /**
   * Called app_reset callback was called with bundle.
   */
  void SetBundleName(const std::string& name);

  /**
   * Called app_reset callback was called with bundle.
   */
  void SetBundleId(const std::string& id);

private:
  bool        mInitialised;
  bool        mPaused;
  std::string mBundleName;
  std::string mBundleId;

private: // impl members
  struct Impl;
  Impl* mImpl;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_TIZEN_FRAMEWORK_H
