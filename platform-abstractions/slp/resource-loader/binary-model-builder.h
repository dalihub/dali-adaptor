#ifndef __DALI_SLP_PLATFORM_BINARY_MODEL_BUILDER_H__
#define __DALI_SLP_PLATFORM_BINARY_MODEL_BUILDER_H__

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

#include <string>

#include <dali/public-api/modeling/model-data.h>
#include "model-builder.h"

namespace Dali
{

namespace SlpPlatform
{

/**
 * ModelBuilder specialization, creates a Model object from DALI specific binary model format.
 */
class BinaryModelBuilder : public ModelBuilder
{
public:
  /**
   * Construct a Builder object with the filename of the Model to import
   * @param[in] name - the filename of the model
   */
  BinaryModelBuilder(const std::string& name);

  /**
   * Destructor
   */
  virtual ~BinaryModelBuilder();

public: // ModelBuilder Interface

  /**
   * @copydoc ModelBuilder::Build()
   */
  virtual bool Build(ModelData model);

  /**
   * @copydoc ModelBuilder::GetModelName()
   */
  virtual const std::string& GetModelName();

public: // public interface

  /**
   * Write out an internal format binary representation of the model data
   * @param[in] model The model data for output.
   */
  bool Write(ModelData model);

private:
  const std::string mFilename;
  std::string       mModelName;
}; // class BinaryModelBuilder

} //namespace SlpPlatform

} // Dali

#endif // __DALI_SLP_PLATFORM_BINARY_MODEL_BUILDER_H__