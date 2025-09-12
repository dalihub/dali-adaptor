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
 */

// CLASS HEADER
#include <dali/internal/adaptor/androidjni/framework-androidjni.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/application-devel.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/android/android-framework.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/application.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/android/android-framework-impl.h>
#include <dali/internal/adaptor/common/application-impl.h>

using namespace Dali;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Impl to hide android data members
 */
struct FrameworkAndroidJni::Impl
{
  // Constructor

  Impl(FrameworkAndroidJni* framework)
  {
    AndroidFramework::GetImplementation(AndroidFramework::Get()).SetFramework(framework);
  }

  ~Impl()
  {
    AndroidFramework::GetImplementation(AndroidFramework::Get()).SetFramework(nullptr);
  }
};

FrameworkAndroidJni::FrameworkAndroidJni(Framework::Observer& observer, Framework::TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread)
: Framework(observer, taskObserver, argc, argv, type, useUiThread),
  mInitialised(false),
  mImpl(NULL)
{
  mImpl = new Impl(this);
}

FrameworkAndroidJni::~FrameworkAndroidJni()
{
  if(mRunning)
  {
    Quit();
  }

  delete mImpl;
  mImpl = nullptr;
}

void FrameworkAndroidJni::Run()
{
  AndroidFramework::GetImplementation(AndroidFramework::Get()).SetFramework(this);
  mRunning = true;
}

unsigned int FrameworkAndroidJni::AddIdle(int timeout, void* data, bool (*callback)(void* data))
{
  JNIEnv* env    = nullptr;
  JavaVM* javaVM = AndroidFramework::Get().GetJVM();
  if(javaVM == nullptr || javaVM->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
  {
    DALI_LOG_ERROR("Couldn't get JNI env.");
    return 0;
  }

  jclass clazz = env->FindClass("com/sec/daliview/DaliView");
  if(!clazz)
  {
    DALI_LOG_ERROR("Couldn't find com.sec.daliview.DaliView.");
    return 0;
  }

  jmethodID addIdle = env->GetStaticMethodID(clazz, "addIdle", "(JJJ)I");
  if(!addIdle)
  {
    DALI_LOG_ERROR("Couldn't find com.sec.daliview.DaliView.addIdle.");
    return 0;
  }

  jint id = env->CallStaticIntMethod(clazz, addIdle, reinterpret_cast<jlong>(callback), reinterpret_cast<jlong>(data), static_cast<jlong>(timeout));
  return static_cast<unsigned int>(id);
}

void FrameworkAndroidJni::RemoveIdle(unsigned int id)
{
  JNIEnv* env    = nullptr;
  JavaVM* javaVM = AndroidFramework::Get().GetJVM();
  if(javaVM == nullptr || javaVM->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
  {
    DALI_LOG_ERROR("Couldn't get JNI env.");
    return;
  }

  jclass clazz = env->FindClass("com/sec/daliview/DaliView");
  if(!clazz)
  {
    DALI_LOG_ERROR("Couldn't find com.sec.daliview.DaliView.");
    return;
  }

  jmethodID removeIdle = env->GetStaticMethodID(clazz, "removeIdle", "(I)V");
  if(!removeIdle)
  {
    DALI_LOG_ERROR("Couldn't find com.sec.daliview.DaliView.removeIdle.");
    return;
  }

  env->CallStaticVoidMethod(clazz, removeIdle, static_cast<jint>(id));
}

void FrameworkAndroidJni::Quit()
{
  DALI_LOG_ERROR("Quit does nothing for DaliView!");
}

bool FrameworkAndroidJni::AppStatusHandler(int type, void* data)
{
  Dali::Adaptor* adaptor = nullptr;
  switch(type)
  {
    case APP_WINDOW_CREATED:
      if(!mInitialised)
      {
        mObserver.OnInit();
        mInitialised = true;
      }

      mObserver.OnSurfaceCreated(data);
      break;

    case APP_WINDOW_DESTROYED:
      mObserver.OnSurfaceDestroyed(data);
      break;

    case APP_RESET:
      mObserver.OnReset();
      break;

    case APP_RESUME:
      mObserver.OnResume();
      adaptor = &Dali::Adaptor::Get();
      adaptor->Resume();
      break;

    case APP_PAUSE:
      adaptor = &Dali::Adaptor::Get();
      adaptor->Pause();
      mObserver.OnPause();
      break;

    case APP_LANGUAGE_CHANGE:
      mObserver.OnLanguageChanged();
      break;

    case APP_DESTROYED:
      mObserver.OnTerminate();
      mRunning     = false;
      mInitialised = false;
      break;

    default:
      break;
  }

  return true;
}

/**
 * Impl for Pre-Initailized using UI Thread.
 */
struct UIThreadLoader::Impl
{
  // Constructor

  Impl(void* data)
  {
  }

  ~Impl()
  {
  }

  void Run(Runner runner)
  {
  }

private:
  // Undefined
  Impl(const Impl& impl);
  Impl& operator=(const Impl& impl);
};

/**
 * UI Thread loader to support Pre-Initailized using UI Thread.
 */
UIThreadLoader::UIThreadLoader(int* argc, char*** argv)
: mArgc(argc),
  mArgv(argv),
  mImpl(nullptr)
{
}

UIThreadLoader::~UIThreadLoader()
{
}

void UIThreadLoader::Run(Runner runner)
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
