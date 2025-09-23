#ifndef DALI_INTEGRATION_ANDROID_FRAMEWORK_H
#define DALI_INTEGRATION_ANDROID_FRAMEWORK_H

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
#include <string>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/configuration.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>

#ifndef _ANDROID_NATIVE_APP_GLUE_H
extern "C" {
struct android_app;
}
#endif

#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class AndroidFramework;
}

} //namespace Internal DALI_INTERNAL

namespace Integration
{
/**
 * AndroidFramework provides setter/getter for Android native interfaces for Android DALi Adaptor.
 * It is also used to pass Android application events to Android DALi Adaptor.
 */
class DALI_ADAPTOR_API AndroidFramework
{
public:
  /**
   * @brief Create a new Android framework instance. Can be only one per application.
   *
   * @return a reference to the framework
   */
  static AndroidFramework& New();

  /**
   * @brief Delete an Android framework instance.
   */
  static void Delete();

  /**
   * @brief Sets the Android native application glue struct
   * @param[in] application A pointer to the application glue struct
   */
  void SetNativeApplication(android_app* application);

  /**
   * @brief Gets the Android native application glue struct
   * @return the native application glue struct
   */
  android_app* GetNativeApplication() const;

  /**
   * @brief Sets the Android JVM
   * @param[in] jvm A pointer to Android JVM
   */
  void SetJVM(JavaVM* jvm);

  /**
   * @brief Sets the JVM
   * @return A pointer to JVM
   */
  JavaVM* GetJVM() const;

  /**
   *  Sets the Android application assets manager.
   *  @param[in] assets A pointer to assets manager
   */
  void SetApplicationAssets(AAssetManager* assets);

  /**
   * @brief Gets the Android application assets manager.
   * @return The application assets manager
   */
  AAssetManager* GetApplicationAssets() const;

  /**
   *  Sets the Android application internal data path.
   *  @param[in] path A path to the application data path
   */
  void SetInternalDataPath(const std::string& path);

  /**
   *  Gets the Android application internal data path.
   *  @return The application data path
   */
  std::string GetInternalDataPath() const;

  /**
   * @brief Sets the Android application configuration
   * @param[in] configuration A pointer to Android application configuration
   */
  void SetApplicationConfiguration(AConfiguration* configuration);

  /**
   * @brief Gets the Android application configuration
   * @return A Android application configuration
   */
  AConfiguration* GetApplicationConfiguration() const;

  /**
   * @brief Sets the Android application native window
   * @return A native window
   */
  void SetApplicationWindow(ANativeWindow* window);

  /**
   * @brief Gets the Android application native window
   * @param[in] window A native window
   */
  ANativeWindow* GetApplicationWindow() const;

  /**
   * Invoked when the Android application is to be terminated.
   */
  void OnTerminate();

  /**
   * Invoked when the Android application is to be paused.
   */
  void OnPause();

  /**
   * Invoked when the Android application is to be resumed.
   */
  void OnResume();

  /**
   * Invoked when the Android application native window is created.
   */
  void OnWindowCreated(ANativeWindow* window);

  /**
   * Invoked when the Android application native window is deleted.
   */
  void OnWindowDestroyed(ANativeWindow* window);

  /**
   * @brief Returns a reference to the instance of the Android framework used by the current thread.
   *
   * @return A reference to the framework.
   * @note This is only valid in the main thread.
   */
  static AndroidFramework& Get();

  /**
   * @brief Virtual Destructor.
   */
  virtual ~AndroidFramework();

  // Not copyable or movable
  AndroidFramework(const AndroidFramework&)            = delete; ///< Deleted copy constructor
  AndroidFramework(AndroidFramework&&)                 = delete; ///< Deleted move constructor
  AndroidFramework& operator=(const AndroidFramework&) = delete; ///< Deleted copy assignment operator
  AndroidFramework& operator=(AndroidFramework&&)      = delete; ///< Deleted move assignment operator

private:
  /**
   * @brief Create an uninitialized AndroidFramework.
   */
  AndroidFramework();

  Internal::Adaptor::AndroidFramework* mImpl; ///< Implementation object
  friend class Internal::Adaptor::AndroidFramework;
};

} // namespace Integration

} // namespace Dali

#endif // DALI_INTEGRATION_ANDROID_FRAMEWORK_H
