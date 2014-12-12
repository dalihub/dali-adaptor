#ifndef __DALI_SLP_PLATFORM_MODEL_BUILDER_H__
#define __DALI_SLP_PLATFORM_MODEL_BUILDER_H__

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

#include <dali/public-api/modeling/model-data.h>

namespace Dali
{
namespace SlpPlatform
{

/**
 * @brief Provides a means of creating a Model object.
 *
 * @sa ResourceThreadModel::CreateModelBuilder
 */
class ModelBuilder
{
public:
  /**
   * @brief Allow implementing classes to be deleted through a pointer to this interface.
   */
  virtual ~ModelBuilder() {}

  /**
   * @brief Build the given model
   * @param[in] model - the model to build
   */
  virtual bool Build( ModelData model ) = 0;

  /**
   * @brief Get the model name (usually the basename of the file resource)
   */
  virtual const std::string& GetModelName() = 0;
};

}//SlpPlatform
}//Dali

#endif // __DALI_SLP_PLATFORM_MODEL_BUILDER_H__
