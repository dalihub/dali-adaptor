#ifndef __DALI_SLP_PLATFORM_ASSIMP_STUBS_H__
#define __DALI_SLP_PLATFORM_ASSIMP_STUBS_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

namespace Dali
{

namespace SlpPlatform
{

// Stubbed out assimp classes

class AssimpProxy
{
public:
  AssimpProxy()
  {
  }

  ~AssimpProxy()
  {
  }
};

class AssimpModelBuilder : public ModelBuilder
{
public:
  AssimpModelBuilder(AssimpProxy* importer, const std::string& name)
  {
  }

  virtual ~AssimpModelBuilder()
  {
  }

  virtual bool Build(ModelData model)
  {
    return false;
  }

  virtual const std::string& GetModelName()
  {
    static std::string s;
    return s;
  }
};

} // namespace SlpPlatform

} // namespace Dali

#endif // __DALI_SLP_PLATFORM_ASSIMP_STUBS_H__
