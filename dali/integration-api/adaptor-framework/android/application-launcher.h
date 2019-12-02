#ifndef DALI_INTEGRATION_APPLICATION_LAUNCHER_H
#define DALI_INTEGRATION_APPLICATION_LAUNCHER_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#include <jni.h>
#include <dali/devel-api/adaptor-framework/application-devel.h>

using namespace Dali;

// Wraps DALi application instance for DaliView JNI interface
struct ApplicationLauncher
{
  static RefObject* applicationObject;
  ApplicationLauncher( Application& application )
  {
    applicationObject = application.GetObjectPtr();
  }
};

// Static DALi application instance for DaliView
RefObject* ApplicationLauncher::applicationObject;

namespace
{

// JNI native "nativeOnCreate" callback when DaliView is created
jlong OnCreate(JNIEnv *jenv, jobject obj)
{
  // Extra reference to prevent finalize clearing the app
  ApplicationLauncher::applicationObject->Reference();

  // not blocking, does nothing except for the setting of the running flag
  DevelApplication::DownCast( ApplicationLauncher::applicationObject ).MainLoop();

  return reinterpret_cast<jlong>( ApplicationLauncher::applicationObject );
};

}

// JNI daliview library on load entry function. Registers nativeOnCreate callback for DaliView Java class.
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
  JNIEnv* env = nullptr;
  if( vm->GetEnv( reinterpret_cast<void **>( &env ), JNI_VERSION_1_6) != JNI_OK )
  {
    return JNI_ERR;
  }

  // Find DaliView Java class. JNI_OnLoad is called from the correct class loader context for this to work.
  jclass clazz = env->FindClass( "com/sec/daliview/DaliView" );
  if( clazz == nullptr )
  {
    return JNI_ERR;
  }

  // Register your class' native methods.
  static const JNINativeMethod methods[] =
  {
      { "nativeOnCreate", "()J", (void *)&OnCreate },
  };

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK)
    return rc;

  return JNI_VERSION_1_6;
};

// This macro facilitates creation of DALi application and launch when DaliView is created.
// The DALi application controller is passed to the application created by DaliView JNI library.
#define RUN(Controller) Application application = Application::New(); \
                        Controller controller( application ); \
                        ApplicationLauncher launcher( application ); \

#endif // DALI_INTEGRATION_APPLICATION_LAUNCHER_H

