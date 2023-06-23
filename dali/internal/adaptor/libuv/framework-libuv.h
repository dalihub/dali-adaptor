#ifndef DALI_INTERNAL_ADAPTOR_LIBUV_FRAMEWORK_H
#define DALI_INTERNAL_ADAPTOR_LIBUV_FRAMEWORK_H

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
 * FrameworkLibuv class provides an Framework Libuv implementation.
 */
class FrameworkLibuv : public Framework
{
public:
  /**
   * @copydoc Dali::Internal::Adaptor::Framework::Framework()
   */
  FrameworkLibuv(Observer& observer, TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread);

  /**
   * Destructor
   */
  ~FrameworkLibuv();

public:
  /**
   * @copydoc Dali::Internal::Adaptor::Framework::Run()
   */
  void Run() override;

  /**
   * @copydoc Dali::Internal::Adaptor::Framework::Quit()
   */
  void Quit() override;

private:
  // Undefined
  FrameworkLibuv(const FrameworkLibuv&);
  FrameworkLibuv& operator=(FrameworkLibuv&);

private: // impl members
  struct Impl;
  Impl* mImpl;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_LIBUV_FRAMEWORK_H
