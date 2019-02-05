#ifndef DALI_INTERNAL_ADAPTOR_BUILDER_IMPL_H
#define DALI_INTERNAL_ADAPTOR_BUILDER_IMPL_H

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


// INTERNAL INCLUDES
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/graphics/common/graphics-factory-interface.h>


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Implementation of the Adaptor Builder class.
 */
class AdaptorBuilder
{
public:

  /**
   * Destructor
   */
  ~AdaptorBuilder() {};

  /**
   * Singleton getter
   */
  static AdaptorBuilder& Get(EnvironmentOptions& environmentOptions);

public:

  /**
   * @return reference to the GraphicsFactory object
   */
  GraphicsFactoryInterface& GetGraphicsFactory() const;


private:

  /**
   * Constructor
   */
  AdaptorBuilder(EnvironmentOptions& environmentOptions);

// Eliminate copy and assigned operations
  AdaptorBuilder(const AdaptorBuilder&) = delete;
  AdaptorBuilder& operator=(AdaptorBuilder&) = delete;


private:
  std::unique_ptr< GraphicsFactoryInterface > mGraphicsFactory;
  EnvironmentOptions& mEnvironmentOptions;
};

} // namespace Internal

} // namespace Adaptor

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_BUILDER_IMPL_H
