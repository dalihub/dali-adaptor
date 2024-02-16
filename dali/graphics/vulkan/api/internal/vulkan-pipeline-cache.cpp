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

#include <dali/graphics/vulkan/api/internal/vulkan-pipeline-cache.h>
#include <dali/graphics/vulkan/api/internal/vulkan-api-pipeline-impl.h>
#include <dali/graphics/vulkan/api/vulkan-api-pipeline.h>
#include <dali/graphics/vulkan/internal/vulkan-debug.h>

namespace Dali
{
namespace Graphics
{
namespace VulkanAPI
{

namespace
{
const int32_t CACHE_AGE_LIMIT = 5; // Number of frames a pipeline can stay alive with no handles.
}

PipelineCache::PipelineCache()
{
  mThreadPool = std::make_unique<Dali::ThreadPool>();
  mThreadPool->Initialize(0u); // max threads
}

PipelineCache::~PipelineCache() = default;

Internal::Pipeline* PipelineCache::GetPipeline( const VulkanAPI::PipelineFactory& factory ) const
{
  auto hashCode = factory.GetHashCode();
  auto iter = mCacheMap.find( hashCode );
  if( iter == mCacheMap.end() )
  {
    return nullptr;
  }

  auto& entries = iter->second;

  // return quickly if there's only one entry
  if( entries.size() == 1 )
  {
    return entries[0].pipelineImpl.get();
  }

  for( auto&& entry : entries )
  {
    if( !std::memcmp( &entry.info, &factory.GetCreateInfo(), sizeof( entry.info ) ) )
    {
      return entry.pipelineImpl.get();
    }
  }

  return nullptr;
}

bool PipelineCache::SavePipeline( const VulkanAPI::PipelineFactory& factory, std::unique_ptr< Internal::Pipeline > pipeline )
{
  // hash factory
  auto hashCode = factory.GetHashCode();
  auto iter = mCacheMap.find( hashCode );
  std::vector< CacheEntry >* cacheEntries{ nullptr };

  if( iter == mCacheMap.end() )
  {
    mCacheMap[hashCode] = std::vector< CacheEntry >{};
    cacheEntries = &mCacheMap[hashCode];
  }
  else
  {
    cacheEntries = &iter->second;
  }

  // pass the ownership to the cache
  CacheEntry entry;
  entry.pipelineImpl = std::move( pipeline );
  entry.pipelineImpl->Reference();
  entry.age = 0;
  entry.info = std::unique_ptr< PipelineFactory::Info >( new PipelineFactory::Info( factory.GetCreateInfo() ) );
  cacheEntries->push_back( std::move( entry ) );
  return true;
}

Dali::UniqueFutureGroup PipelineCache::Compile( bool parallel )
{
  std::vector<Internal::Pipeline*> pipelinesToRemove;
  std::vector<Dali::Task> tasks;
  for( auto& item : mCacheMap )
  {
    for( auto& pipeline : item.second )
    {
      auto pipelineImpl = pipeline.pipelineImpl.get();

      // initialise now
      bool isInitialized = pipelineImpl->Initialise();
      if( !isInitialized )
      {
        int32_t refCount = pipeline.pipelineImpl->GetReferenceCount();

        // if this is a pre-existing pipeline, check if it's still in use - if the cache is the only
        // entity holding a reference to it after CACHE_AGE_LIMIT frames, then can drop it.
        if( refCount > 1 )
        {
          pipeline.age = 0u;
        }
        else
        {
          ++pipeline.age;

          if( pipeline.age > CACHE_AGE_LIMIT )
          {
            pipelinesToRemove.push_back( pipeline.pipelineImpl.get() );
          }
        }
      }
      else
      {
        pipeline.age = 0u;

        // If pipeline has just been initialized then schedule the compilation
        auto workerFunc = [&, pipelineImpl](int workerIndex)
        {
          pipelineImpl->Compile();
        };

        if(parallel)
        {
          // Queue it
          tasks.emplace_back( workerFunc );
        }
        else
        {
          // Execute it
          workerFunc(0);
        }
      }
    }
  }

  for( auto& pipeline : pipelinesToRemove )
  {
    pipeline->Dereference();
  }

  if( !tasks.empty() )
  {
    return mThreadPool->SubmitTasks( tasks, 0u );
  }
  return {};
}

bool PipelineCache::RemovePipeline( Internal::Pipeline* pipeline )
{
  auto hashCode = pipeline->GetHashCode();
  auto iter = mCacheMap.find( hashCode );
  if( iter == mCacheMap.end() )
  {
    return false;
  }

  // delete pipeline
  if( iter->second.size() == 1 )
  {
    iter->second.clear();
    mCacheMap.erase( iter );
  }
  else
  {
    int32_t i = 0;
    for( auto&& entry : iter->second )
    {
      if( entry.pipelineImpl.get() == pipeline )
      {
        iter->second.erase( iter->second.begin() + i );
        if( iter->second.empty() )
        {
          mCacheMap.erase( iter );
        }
        break;
      }
      ++i;
    }
  }

  return true;
}

uint32_t PipelineCache::GetCacheSize() const
{
  auto retval = 0u;
  for( auto& item : mCacheMap )
  {
    retval += uint32_t(item.second.size());
  }
  return retval;
}

} // VulkanAPI
} // Graphics
} // Dali
