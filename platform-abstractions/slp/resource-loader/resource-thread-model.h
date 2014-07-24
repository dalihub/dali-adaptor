#ifndef __DALI_SLP_PLATFORM_RESOURCE_THREAD_MODEL_H__
#define __DALI_SLP_PLATFORM_RESOURCE_THREAD_MODEL_H__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include <dali/integration-api/resource-cache.h>

#include "resource-thread-base.h"
#include "model-builder.h"

namespace Dali
{

namespace Integration
{
namespace Log
{
class Filter;
}
}

namespace SlpPlatform
{

class AssimpProxy;

class ResourceThreadModel : public ResourceThreadBase
{
public:
  /**
   * Constructor
   * @param[in] resourceLoader A reference to the ResourceLoader
   */
  ResourceThreadModel(ResourceLoader& resourceLoader);

  /**
   * Destructor
   */
  virtual ~ResourceThreadModel();

private:
  /**
   * @copydoc ResourceThreadBase::Load
   */
  virtual void Load(const Integration::ResourceRequest& request);

  /**
   *@copydoc ResourceThreadBase::Save
   */
  virtual void Save(const Integration::ResourceRequest& request);

  /**
   * Create a platform specific ModelBuilder to load a model file
   * param[in] modelFileName The filename of the model
   */
  ModelBuilder* CreateModelBuilder(const std::string& modelFileName);

private:
  AssimpProxy* mModelImporter;          ///< Used for loading models

}; // class ResourceThreadModel

} // namespace SlpPlatform

} // namespace Dali

#endif // __DALI_SLP_PLATFORM_RESOURCE_THREAD_MODEL_H__
