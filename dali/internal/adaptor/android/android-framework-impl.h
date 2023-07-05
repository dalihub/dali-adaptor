#ifndef DALI_INTEGRATION_ANDROID_FRAMEWORK_IMPL_H
#define DALI_INTEGRATION_ANDROID_FRAMEWORK_IMPL_H

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

// EXTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/android/android-framework.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/android/framework-android.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/// Android application events
enum
{
  APP_WINDOW_CREATED = 0,
  APP_WINDOW_DESTROYED,
  APP_PAUSE,
  APP_RESUME,
  APP_RESET,
  APP_LANGUAGE_CHANGE,
  APP_DESTROYED,
};

/**
 * AndroidFramework implementation to set/get Android native interfaces for Android Adaptor.
 * Also passes Android application events to Android Adaptor internal framework.
 */
class AndroidFramework
{
public:
  /**
   * @brief Create a new Android framework.
   *
   * @return a reference to the Android framework
   */
  static Dali::Integration::AndroidFramework& New();

  /**
   * @brief Delete an Android framework.
   */
  static void Delete();

  /**
   * @copydoc Dali::Integration::AndroidFramework::SetNativeApplication()
   */
  void SetNativeApplication(android_app* application);

  /**
   * @copydoc Dali::Integration::AndroidFramework::GetNativeApplication()
   */
  android_app* GetNativeApplication() const;

  /**
   * @copydoc Dali::Integration::AndroidFramework::SetJVM()
   */
  void SetJVM(JavaVM* jvm);

  /**
   * @copydoc Dali::Integration::AndroidFramework::GetJVM()
   */
  JavaVM* GetJVM() const;

  /**
   * @copydoc Dali::Integration::AndroidFramework::SetApplicationAssets()
   */
  void SetApplicationAssets(AAssetManager* assets);

  /**
   * @copydoc Dali::Integration::AndroidFramework::GetApplicationAssets()
   */
  AAssetManager* GetApplicationAssets() const;

  /**
   *  copydoc Dali::Integration::AndroidFramework::SetInternalDataPath()
   */
  void SetInternalDataPath(const std::string& path);

  /**
   *  copydoc Dali::Integration::AndroidFramework::GetInternalDataPath()
   */
  std::string GetInternalDataPath() const;

  /**
   * @copydoc Dali::Integration::AndroidFramework::SetApplicationConfiguration()
   */
  void SetApplicationConfiguration(AConfiguration* configuration);

  /**
   * @copydoc Dali::Integration::AndroidFramework::GetApplicationConfiguration()
   */
  AConfiguration* GetApplicationConfiguration() const;

  /**
   * @copydoc Dali::Integration::AndroidFramework::SetApplicationWindow()
   */
  void SetApplicationWindow(ANativeWindow* window);

  /**
   * @copydoc Dali::Integration::AndroidFramework::GetApplicationWindow()
   */
  ANativeWindow* GetApplicationWindow() const;

  /**
   * @copydoc Dali::Integration::AndroidFramework::OnTerminate()
   */
  void OnTerminate();

  /**
   * @copydoc Dali::Integration::AndroidFramework::OnPause()
   */
  void OnPause();

  /**
   * @copydoc Dali::Integration::AndroidFramework::OnResume()
   */
  void OnResume();

  /**
   * @copydoc Dali::Integration::AndroidFramework::OnWindowCreated()
   */
  void OnWindowCreated(ANativeWindow* window);

  /**
   * @copydoc Dali::Integration::AndroidFramework::OnWindowDestroyed()
   */
  void OnWindowDestroyed(ANativeWindow* window);

  /**
   * @copydoc Dali::Integration::AndroidFramework::Get()
   */
  static Dali::Integration::AndroidFramework& Get();

  /**
   * @brief Sets an internal framework.
   */
  void SetFramework(FrameworkAndroidInterface* framework)
  {
    mFramework = framework;
  }

  /**
   * @brief Gets an internal framework.
   *
   * @return a pointer to the internal framework
   */
  FrameworkAndroidInterface* GetFramework()
  {
    return mFramework;
  }

  /**
   * Virtual destructor.
   */
  virtual ~AndroidFramework();

  // Not copyable or movable
  AndroidFramework(const AndroidFramework&) = delete;            ///< Deleted copy constructor
  AndroidFramework(AndroidFramework&&)      = delete;            ///< Deleted move constructor
  AndroidFramework& operator=(const AndroidFramework&) = delete; ///< Deleted copy assignment operator
  AndroidFramework& operator=(AndroidFramework&&) = delete;      ///< Deleted move assignment operator

private:
  AndroidFramework(Dali::Integration::AndroidFramework* androidFramework);
  Dali::Integration::AndroidFramework* mAndroidFramework;
  FrameworkAndroidInterface*           mFramework;

  android_app*    mNativeApplication;
  ANativeWindow*  mWindow;
  AAssetManager*  mAssets;
  std::string     mInternalDataPath;
  AConfiguration* mConfiguration;
  JavaVM*         mJVM;

public:
  static AndroidFramework& GetImplementation(Dali::Integration::AndroidFramework& androidFramework)
  {
    return *androidFramework.mImpl;
  }
  static FrameworkAndroidInterface& GetFramework(Dali::Integration::AndroidFramework& androidFramework)
  {
    return *androidFramework.mImpl->mFramework;
  }
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTEGRATION_ANDROID_FRAMEWORK_IMPL_H
