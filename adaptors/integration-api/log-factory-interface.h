#ifndef DALI_ADAPTOR_LOG_FACTORY_INTERFACE_H
#define DALI_ADAPTOR_LOG_FACTORY_INTERFACE_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

namespace Dali
{

class LogFactoryInterface
{
public:
  /**
   * @brief Install a log function for this thread.
   *
   * Only need to use once per thread, before any processing occurs.
   */
  virtual void InstallLogFunction() const = 0;
};

} // namespace Dali


#endif //DALI_ADAPTOR_LOG_FACTORY_INTERFACE_H
