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

const Integration::ResourceId NO_REQUEST = Integration::ResourceId(0) - 1;

namespace SlpPlatform
{

namespace
{
const char * const IDLE_PRIORITY_ENVIRONMENT_VARIABLE_NAME = "DALI_RESOURCE_THREAD_IDLE_PRIORITY"; ///@Todo Move this to somewhere that other environment variables are declared and document it there.
} // unnamed namespace

ResourceThreadBase::ResourceThreadBase(ResourceLoader& resourceLoader)
: mResourceLoader( resourceLoader ), mCurrentRequestId( NO_REQUEST ), mPaused( false )
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

    // See if the request is already launched as the current job on the thread:
    //if( mCurrentRequestId == resourceId )
    //{
    //  mThread->interrupt();
    //}
    // Check the pending requests to be cancelled:
    //else
    {
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
  // TODO: Use Environment Options
  const char* threadPriorityIdleRequired = std::getenv( IDLE_PRIORITY_ENVIRONMENT_VARIABLE_NAME );
  if( threadPriorityIdleRequired )
  {
    // if the parameter exists then set up an idle priority for this thread
    struct sched_param sp;
    sp.sched_priority = 0;
    sched_setscheduler(0, SCHED_IDLE, &sp);
    ///@ToDo: change to the corresponding Pthreads call, not this POSIX.1-2001 one with a Linux-specific argument (SCHED_IDLE): int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param);, as specified in the docs for sched_setscheduler(): http://man7.org/linux/man-pages/man2/sched_setscheduler.2.html
  }

  InstallLogging();

  while( !mResourceLoader.IsTerminating() )
  {
    try
    {
      WaitForRequests();

      if ( !mResourceLoader.IsTerminating() )
      {
        ProcessNextRequest();
      }
    }

    catch( boost::thread_interrupted& ex )
    {
      // No problem, thread was just interrupted from the outside to cancel an in-flight request.
      boost::thread_interrupted* disableUnusedVarWarning = &ex;
      ex = *disableUnusedVarWarning;
      // Temporary logging of an unexpected boost::thread_interrupted exception:
      DALI_LOG_ERROR( "boost::thread_interrupted caught in resource thread in build with late cancellation disabled (should not happen). Aborting request with id %u.\n", unsigned(mCurrentRequestId) );
    }

    // Since we have an exception handler here anyway, lets catch everything to avoid killing the process:
    catch( std::exception& ex )
    {
      const char * const what = ex.what();
      DALI_LOG_ERROR( "std::exception caught in resource thread. Aborting request with id %u because of std::exception with reason, \"%s\".\n", unsigned(mCurrentRequestId), what ? what : "null" );

    }
    catch( Dali::DaliException& ex )
    {
      // Probably a failed assert-always:
      DALI_LOG_ERROR( "DaliException caught in resource thread. Aborting request with id %u. Location: \"%s\". Condition: \"%s\".\n", unsigned(mCurrentRequestId), ex.mLocation.c_str(), ex.mCondition.c_str() );
    }
    catch( ... )
    {
      DALI_LOG_ERROR( "Unknown exception caught in resource thread. Aborting request with id %u.\n", unsigned(mCurrentRequestId) );
    }
  }
}

void ResourceThreadBase::WaitForRequests()
{
  unique_lock<mutex> lock( mMutex );

  // Clear the previously current request:
  mCurrentRequestId = NO_REQUEST;

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
      mCurrentRequestId = front.first.GetId();
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

    // Clear the interruption status for derived classes that don't implement on-the-fly cancellation yet:
    boost::this_thread::interruption_point(); ///@warning This can throw an exception.
    // To support cancellation of an in-flight resource, place the above line at key points in derived
    // resource thread classes and the loading / decoding / saving code that they call.
    // See resource-thread-image.cpp and loader-jpeg-turbo.cpp for a conservative example of its use.
    ///@note: The above line will throw an exception so only place it in exception-safe locations.
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

