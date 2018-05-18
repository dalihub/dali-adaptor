#ifndef __DALI_APPLICATION_EXTENSIONS_H__
#define __DALI_APPLICATION_EXTENSIONS_H__

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
class Application;

/**
 * @brief An Application extension class object should be created by DaliApplication.
 *
 * It provides some extension methods to DaliApplication.
 *
 */
class DALI_ADAPTOR_API ApplicationExtensions
{
public:

 /**
   * @brief The default constructor.
   *
   */
  ApplicationExtensions();

 /**
   * @brief The constructor accept an instance of Dali::Application.
   *
   * This can be initialized by new keyword.
   */
  ApplicationExtensions(Dali::Application* application);

 /**
   * @brief Destructor
   *
   */
  ~ApplicationExtensions();


 /**
   * @brief Called when the framework is initialised.
   * @SINCE_1_2.7
   */
  void Init();

  /**
    * @brief Called when the framework is started.
    */
   void Start();

 /**
   * @brief Called when the framework is terminated.
   * @SINCE_1_2.7
   */
  void Terminate();

 /**
   * @brief Called when the framework is paused.
   * @SINCE_1_2.7
   */
  void Pause();

 /**
   * @brief Called when the framework resumes from a paused state.
   * @SINCE_1_2.7
   */
  void Resume();

 /**
   * @brief Called when the framework informs the application that the language of the device has changed.
   * @SINCE_1_2.7
   */
  void LanguageChange();

private:
  Dali::Application* mApplication;
};

} // namespace Dali

#endif // ___DALI_APPLICATION_EXTENSIONS_H__
