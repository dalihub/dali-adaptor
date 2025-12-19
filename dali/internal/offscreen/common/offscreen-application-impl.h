#ifndef DALI_INTERNAL_OFFSCREEN_COMMON_OFFSCREEN_APPLICATION_H
#define DALI_INTERNAL_OFFSCREEN_COMMON_OFFSCREEN_APPLICATION_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/offscreen-application.h>
#include <dali/internal/system/common/environment-options.h>

namespace Dali
{
class Adaptor;

namespace Internal
{
namespace Adaptor
{
class FrameworkFactory;

/**
 * Implementation of the OffscreenApplication class.
 */
class OffscreenApplication : public BaseObject
{
public:
  /**
   * @brief Create a new OffscreenApplication
   */
  static IntrusivePtr<OffscreenApplication> New(Dali::OffscreenApplication::FrameworkBackend framework, Dali::OffscreenApplication::RenderMode renderMode);

public:
  /**
   * @brief Destructor
   */
  virtual ~OffscreenApplication();

  /**
   * @copydoc Dali::OffscreenApplication::Start()
   */
  void Start();

  /**
   * @copydoc Dali::OffscreenApplication::Terminate()
   */
  void Terminate();

  /**
   * @copydoc Dali::OffscreenApplication::GetWindow()
   */
  Dali::OffscreenWindow GetWindow();

  /**
   * @copydoc Dali::OffscreenApplication::RenderOnce()
   */
  void RenderOnce();

private:
  /**
   * @brief Private constructor
   */
  OffscreenApplication(Dali::OffscreenApplication::FrameworkBackend framework, Dali::OffscreenApplication::RenderMode renderMode);

  /**
   * @brief Creates the default offscreen window
   */
  void CreateWindow();

  /**
   * @brief Creates the adaptor.
   * It should be called after a default window created.
   */
  void CreateAdaptor();

  // Undefined
  OffscreenApplication(const OffscreenApplication&)            = delete;
  OffscreenApplication& operator=(OffscreenApplication&)       = delete;
  OffscreenApplication& operator=(const OffscreenApplication&) = delete;
  OffscreenApplication& operator=(OffscreenApplication&&)      = delete;

private:
  Dali::OffscreenWindow mDefaultWindow;

  std::unique_ptr<Dali::Adaptor>      mAdaptor{};
  std::unique_ptr<EnvironmentOptions> mEnvironmentOptions{};
  std::unique_ptr<FrameworkFactory>   mFrameworkFactory{};

  Dali::OffscreenApplication::FrameworkBackend mFrameworkBackend;
  Dali::OffscreenApplication::RenderMode       mRenderMode;

  bool mIsAdaptorStarted{false};
  bool mIsAdaptorStoped{false};
};

inline OffscreenApplication& GetImplementation(Dali::OffscreenApplication& offscreenApplication)
{
  DALI_ASSERT_ALWAYS(offscreenApplication && "OffscreenApplication handle is empty");

  BaseObject& handle = offscreenApplication.GetBaseObject();

  return static_cast<OffscreenApplication&>(handle);
}

inline const OffscreenApplication& GetImplementation(const Dali::OffscreenApplication& offscreenApplication)
{
  DALI_ASSERT_ALWAYS(offscreenApplication && "OffscreenApplication handle is empty");

  const BaseObject& handle = offscreenApplication.GetBaseObject();

  return static_cast<const OffscreenApplication&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_OFFSCREEN_COMMON_OFFSCREEN_APPLICATION_H
