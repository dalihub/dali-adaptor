#ifndef DALI_INTERNAL_ADAPTOR_ANDROID_FRAMEWORK_INTERFACE_H
#define DALI_INTERNAL_ADAPTOR_ANDROID_FRAMEWORK_INTERFACE_H

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

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * FrameworkAndroid interface class
 */
class FrameworkAndroidInterface
{
public:
  /**
   * Called by the App framework when an application lifecycle event occurs.
   * @param[in] type The type of event occurred.
   * @param[in] data The data of event occurred.
   */
  virtual bool AppStatusHandler(int type, void* data) = 0;

  /**
   * Called by the adaptor when an idle callback is added.
   * @param[in] timeout The timeout of the callback.
   * @param[in] data The data of of the callback.
   * @param[in] callback The callback.
   * @return The callback id.
   */
  virtual unsigned int AddIdle(int timeout, void* data, bool (*callback)(void* data)) = 0;

  /**
   * Called by the adaptor when an idle callback is removed.
   * @param[in] id The callback id.
   */
  virtual void RemoveIdle(unsigned int id) = 0;

protected:
  /**
   * Destructor
   */
  virtual ~FrameworkAndroidInterface() = default;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_ANDROID_FRAMEWORK_INTERFACE_H
