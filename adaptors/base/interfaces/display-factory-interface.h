#ifndef __DALI_INTERNAL_BASE_DISPLAY_FACTORY_INTERFACE_H__
#define __DALI_INTERNAL_BASE_DISPLAY_FACTORY_INTERFACE_H__

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

namespace Dali
{

class DisplayInterface;

namespace Internal
{
namespace Adaptor
{

/**
 * Factory interface for creating Display implementation
 */
class DisplayFactoryInterface
{
public:
  /**
   * Create an Display implementation
   * @return An implementation of the Display interface
   */
  virtual DisplayInterface* Create() = 0;

  /**
   * Destroy the Displlay implementation
   */
  virtual void Destroy() = 0;

protected:
  /**
   * Virtual protected destructor - no deletion through this interface
   */
  virtual ~DisplayFactoryInterface() {};
};

} // Adaptor
} // Internal
} // Dali

#endif // __DALI_INTERNAL_BASE_DISPLAY_FACTORY_INTERFACE_H__
