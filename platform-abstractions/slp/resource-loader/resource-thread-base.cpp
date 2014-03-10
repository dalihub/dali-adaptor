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

#include <dali/integration-api/debug.h>
#include "resource-thread-base.h"
#include "slp-logging.h"

using namespace std;
using namespace Dali::Integration;
using boost::mutex;
using boost::unique_lock;
using boost::scoped_ptr;

namespace Dali
{

namespace SlpPlatform
{

ResourceThreadBase::ResourceThreadBase(ResourceLoader& resourceLoader)
: mResourceLoader(resourceLoader), mPaused(false)
{
#if defined(DEBUG_ENABLED)
  mLogFilter = Debug::Filter::New(Debug::Concise, false, "LOG_RESOURCE_THREAD_BASE");
#endif

  mThread = new boost::thread(boost::bind(&ResourceThreadBase::ThreadLoop, this));
}

ResourceThreadBase::~ResourceThreadBase()
{
  TerminateThread();

#if defined(DEBUG_ENABLED)
  delete mLogFilter;
#endif
}

void ResourceThreadBase::TerminateThread()
{
  if (mThread)
  {
    // wake thread
    mCondition.notify_all();
    // wait for thread to exit
    mThread->join();
    // delete thread instance
    delete mThread;
    // mark thread terminated
    mThread = NULL;
  }
}

void ResourceThreadBase::AddRequest(const ResourceRequest& request, const RequestType type)
{
  bool wasEmpty = false;
  bool wasPaused = false;

  {
    // Lock while adding to the request queue
    unique_lock<mutex> lock( mMutex );

    wasEmpty = mQueue.empty();
    wasPaused = mPaused;

    mQueue.push_back( make_pair(request, type) );
  }

  if( wasEmpty && !wasPaused )
  {
    // Wake-up the thread
    mCondition.notify_all();
  }
}

void ResourceThreadBase::CancelRequest( Integration::ResourceId resourceId )
{
  {
    // Lock while searching and removing from the request queue:
    unique_lock<mutex> lock( mMutex );

    for( RequestQueueIter iterator = mQueue.begin();
         iterator != mQueue.end();
         ++iterator )
    {
      if( ((*iterator).first).GetId() == resourceId )
      {
        iterator = mQueue.erase( iterator );
        break;
      }
    }
  }
}

void ResourceThreadBase::Pause()
{
  unique_lock<mutex> lock( mMutex );
  mPaused = true;
}

void ResourceThreadBase::Resume()
{
  // Clear the paused flag and if we weren't running already, also wake up the background thread:
  bool wasPaused = false;
  {
    unique_lock<mutex> lock( mMutex );
    wasPaused = mPaused;
    mPaused = false;
  }

  // If we were paused, wake up the background thread and give it a
  // chance to do some work:
  if( wasPaused )
  {
    mCondition.notify_all();
  }
}

//----------------- Called from separate thread (mThread) -----------------

void ResourceThreadBase::ThreadLoop()
{
  InstallLogging();

  while( !mResourceLoader.IsTerminating() )
  {
    WaitForRequests();

    if ( !mResourceLoader.IsTerminating() )
    {
      ProcessNextRequest();
    }
  }
}

void ResourceThreadBase::WaitForRequests()
{
  unique_lock<mutex> lock(mMutex);

  if( mQueue.empty() || mPaused == true )
  {
    // Waiting for a wake up from resource loader control thread
    // This will be to process a new request or terminate
    mCondition.wait(lock);
  }
}

void ResourceThreadBase::ProcessNextRequest()
{
  ResourceRequest* request(NULL);
  RequestType type(RequestLoad);

  {
    // lock the queue and extract the next request
    unique_lock<mutex> lock(mMutex);

    if (!mQueue.empty())
    {
      const RequestInfo & front = mQueue.front();
      request = new ResourceRequest( front.first );
      type = front.second;
      mQueue.pop_front();
    }
  } // unlock the queue

  // process request outside of lock
  if ( NULL != request )
  {
    std::auto_ptr<ResourceRequest> deleter( request );
    switch( type )
    {
      case RequestLoad:
      {
        Load(*request);
      }
      break;

      case RequestDecode:
      {
        Decode(*request);
      }
      break;

      case RequestSave:
      {
        Save(*request);
      }
      break;
    }
  }
}

void ResourceThreadBase::InstallLogging()
{
  // resource loading thread will send its logs to SLP Platform's LogMessage handler.
  Dali::Integration::Log::InstallLogFunction(Dali::SlpPlatform::LogMessage);
}

void ResourceThreadBase::UninstallLogging()
{
  // uninstall it on resource loading thread.
  Dali::Integration::Log::UninstallLogFunction();
}

void ResourceThreadBase::Decode(const Integration::ResourceRequest& request)
{
  DALI_LOG_TRACE_METHOD(mLogFilter);
  DALI_LOG_WARNING("Resource Decoding from a memory buffer not supported for this type.");
  ///! If you need this for a subclassed thread, look to ResourceThreadImage::Decode() for an example implementation.
}

} // namespace SlpPlatform

} // namespace Dali

