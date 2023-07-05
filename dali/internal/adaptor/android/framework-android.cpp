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

// CLASS HEADER
#include <dali/internal/adaptor/android/framework-android.h>

// EXTERNAL INCLUDES
#include <android_native_app_glue.h>
#include <unistd.h>
#include <queue>
#include <unordered_set>

#include <dali/devel-api/events/key-event-devel.h>
#include <dali/devel-api/events/touch-point.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/android/android-framework.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/events/key-event.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/android/android-framework-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
// Copied from x server
static unsigned int GetCurrentMilliSeconds(void)
{
  struct timeval tv;

  struct timespec  tp;
  static clockid_t clockid;

  if(!clockid)
  {
#ifdef CLOCK_MONOTONIC_COARSE
    if(clock_getres(CLOCK_MONOTONIC_COARSE, &tp) == 0 &&
       (tp.tv_nsec / 1000) <= 1000 && clock_gettime(CLOCK_MONOTONIC_COARSE, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC_COARSE;
    }
    else
#endif
      if(clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC;
    }
    else
    {
      clockid = ~0L;
    }
  }
  if(clockid != ~0L && clock_gettime(clockid, &tp) == 0)
  {
    return (tp.tv_sec * 1000) + (tp.tv_nsec / 1000000L);
  }

  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

/// Recursively removes constraints from an actor and all it's children.
void RemoveAllConstraints(Dali::Actor actor)
{
  if(actor)
  {
    const auto childCount = actor.GetChildCount();
    for(auto i = 0u; i < childCount; ++i)
    {
      Dali::Actor child = actor.GetChildAt(i);
      RemoveAllConstraints(child);
    }
    actor.RemoveConstraints();
  }
}

/// Removes constraints from all actors in all windows.
void RemoveAllConstraints(const Dali::WindowContainer& windows)
{
  for(auto& window : windows)
  {
    RemoveAllConstraints(window.GetRootLayer());
  }
}

} // Unnamed namespace

/**
 * Impl to hide android data members
 */
struct FrameworkAndroid::Impl
{
  struct IdleCallback
  {
    int   timestamp;
    int   timeout;
    int   id;
    void* data;
    bool (*callback)(void* data);

    IdleCallback(int timeout, int id, void* data, bool (*callback)(void* data))
    : timestamp(GetCurrentMilliSeconds() + timeout),
      timeout(timeout),
      id(id),
      data(data),
      callback(callback)
    {
    }

    bool operator()()
    {
      return callback(data);
    }

    bool operator<(const IdleCallback& rhs) const
    {
      return timestamp > rhs.timestamp;
    }
  };

  // Constructor

  Impl(FrameworkAndroid* framework)
  : mFinishRequested(false),
    mIdleId(0),
    mIdleReadPipe(-1),
    mIdleWritePipe(-1)

  {
    AndroidFramework::GetImplementation(AndroidFramework::Get()).SetFramework(framework);
  }

  ~Impl()
  {
    AndroidFramework::GetImplementation(AndroidFramework::Get()).SetFramework(nullptr);
  }

  void OnIdle()
  {
    // Dequeue the pipe
    int8_t msg = -1;
    read(mIdleReadPipe, &msg, sizeof(msg));

    unsigned int ts = GetCurrentMilliSeconds();

    if(!mIdleCallbacks.empty())
    {
      IdleCallback callback = mIdleCallbacks.top();
      if(callback.timestamp <= ts)
      {
        mIdleCallbacks.pop();

        // Callback wasn't removed
        if(mRemovedIdleCallbacks.find(callback.id) == mRemovedIdleCallbacks.end())
        {
          if(callback()) // keep the callback
          {
            AddIdle(callback.timeout, callback.data, callback.callback, callback.id);
          }
        }

        // Callback cane be also removed during the callback call
        auto i = mRemovedIdleCallbacks.find(callback.id);
        if(i != mRemovedIdleCallbacks.end())
        {
          mRemovedIdleCallbacks.erase(i);
        }
      }
    }

    if(mIdleCallbacks.empty())
    {
      mRemovedIdleCallbacks.clear();
    }
  }

  unsigned int AddIdle(int timeout, void* data, bool (*callback)(void* data), unsigned int existingId = 0)
  {
    unsigned int chosenId;
    if(existingId)
    {
      chosenId = existingId;
    }
    else
    {
      ++mIdleId;
      if(mIdleId == 0)
      {
        ++mIdleId;
      }
      chosenId = mIdleId;
    }

    mIdleCallbacks.push(IdleCallback(timeout, chosenId, data, callback));

    // To wake up the idle pipe and to trigger OnIdle
    int8_t msg = 1;
    write(mIdleWritePipe, &msg, sizeof(msg));

    return chosenId;
  }

  void RemoveIdle(unsigned int id)
  {
    if(id != 0)
    {
      mRemovedIdleCallbacks.insert(id);
    }
  }

  int GetIdleTimeout()
  {
    int timeout = -1;

    if(!mIdleCallbacks.empty())
    {
      IdleCallback idleTimeout = mIdleCallbacks.top();
      timeout                  = idleTimeout.timestamp - GetCurrentMilliSeconds();
      if(timeout < 0)
      {
        timeout = 0;
      }
    }

    return timeout;
  }

  // Data
  bool mFinishRequested;

  int                               mIdleReadPipe;
  int                               mIdleWritePipe;
  unsigned int                      mIdleId;
  std::priority_queue<IdleCallback> mIdleCallbacks;
  std::unordered_set<int>           mRemovedIdleCallbacks;

  // Static methods

  /**
   * Called by the native activity loop when the application APP_CMD_INIT_WINDOW event is processed.
   */
  static void NativeWindowCreated(FrameworkAndroid* framework, ANativeWindow* window)
  {
    if(framework)
    {
      framework->AppStatusHandler(APP_WINDOW_CREATED, window);
    }
  }

  /**
   * Called by the native activity loop when the application APP_CMD_DESTROY event is processed.
   */
  static void NativeWindowDestroyed(FrameworkAndroid* framework, ANativeWindow* window)
  {
    if(framework)
    {
      framework->AppStatusHandler(APP_WINDOW_DESTROYED, window);
    }
  }

  /**
   * Called by the native activity loop when the application APP_CMD_INIT_WINDOW event is processed.
   */
  static void NativeAppPaused(FrameworkAndroid* framework)
  {
    if(framework)
    {
      framework->AppStatusHandler(APP_PAUSE, nullptr);
    }
  }

  /**
   * Called by the native activity loop when the application APP_CMD_TERM_WINDOW event is processed.
   */
  static void NativeAppResumed(FrameworkAndroid* framework)
  {
    if(framework)
    {
      framework->AppStatusHandler(APP_RESUME, nullptr);
    }
  }

  /**
   * Called by the native activity loop when the application input touch event is processed.
   */
  static void NativeAppTouchEvent(FrameworkAndroid* framework, Dali::TouchPoint& touchPoint, int64_t timeStamp)
  {
    Dali::Adaptor::Get().FeedTouchPoint(touchPoint, timeStamp);
  }

  /**
   * Called by the native activity loop when the application input key event is processed.
   */
  static void NativeAppKeyEvent(FrameworkAndroid* framework, Dali::KeyEvent& keyEvent)
  {
    Dali::Adaptor::Get().FeedKeyEvent(keyEvent);
  }

  /**
   * Called by the native activity loop when the application APP_CMD_DESTROY event is processed.
   */
  static void NativeAppDestroyed(FrameworkAndroid* framework)
  {
    if(framework)
    {
      framework->AppStatusHandler(APP_DESTROYED, nullptr);
    }
  }

  /*
  Order of events:

  APP_CMD_START
  APP_CMD_RESUME
  APP_CMD_INIT_WINDOW
  APP_CMD_GAINED_FOCUS

  APP_CMD_PAUSE
  APP_CMD_LOST_FOCUS
  APP_CMD_SAVE_STATE
  APP_CMD_STOP
  APP_CMD_TERM_WINDOW
*/

  static void HandleAppCmd(struct android_app* app, int32_t cmd)
  {
    FrameworkAndroid* framework = static_cast<FrameworkAndroid*>(AndroidFramework::GetImplementation(AndroidFramework::Get()).GetFramework());
    switch(cmd)
    {
      case APP_CMD_SAVE_STATE:
        break;
      case APP_CMD_START:
        break;
      case APP_CMD_STOP:
        break;
      case APP_CMD_RESUME:
        break;
      case APP_CMD_PAUSE:
        break;
      case APP_CMD_INIT_WINDOW:
        // The window is being shown, get it ready.
        AndroidFramework::Get().SetApplicationWindow(app->window);
        Dali::Internal::Adaptor::FrameworkAndroid::Impl::NativeWindowCreated(framework, app->window);
        Dali::Internal::Adaptor::FrameworkAndroid::Impl::NativeAppResumed(framework);
        break;
      case APP_CMD_TERM_WINDOW:
        // The window is being hidden or closed, clean it up.
        AndroidFramework::Get().SetApplicationWindow(nullptr);
        Dali::Internal::Adaptor::FrameworkAndroid::Impl::NativeAppPaused(framework);
        Dali::Internal::Adaptor::FrameworkAndroid::Impl::NativeWindowDestroyed(framework, app->window);
        break;
      case APP_CMD_GAINED_FOCUS:
        break;
      case APP_CMD_LOST_FOCUS:
        break;
      case APP_CMD_DESTROY:
        Dali::Internal::Adaptor::FrameworkAndroid::Impl::NativeAppPaused(framework);
        Dali::Internal::Adaptor::FrameworkAndroid::Impl::NativeAppDestroyed(framework);
        break;
    }
  }

  static int32_t HandleAppInput(struct android_app* app, AInputEvent* event)
  {
    FrameworkAndroid* framework = static_cast<FrameworkAndroid*>(AndroidFramework::GetImplementation(AndroidFramework::Get()).GetFramework());

    if(AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
    {
      int32_t                deviceId  = AInputEvent_getDeviceId(event);
      float                  x         = AMotionEvent_getX(event, 0);
      float                  y         = AMotionEvent_getY(event, 0);
      Dali::PointState::Type state     = Dali::PointState::DOWN;
      int32_t                action    = AMotionEvent_getAction(event);
      int64_t                timeStamp = AMotionEvent_getEventTime(event);

      switch(action & AMOTION_EVENT_ACTION_MASK)
      {
        case AMOTION_EVENT_ACTION_DOWN:
          break;
        case AMOTION_EVENT_ACTION_UP:
          state = Dali::PointState::UP;
          break;
        case AMOTION_EVENT_ACTION_MOVE:
          state = Dali::PointState::MOTION;
          break;
        case AMOTION_EVENT_ACTION_CANCEL:
          state = Dali::PointState::INTERRUPTED;
          break;
        case AMOTION_EVENT_ACTION_OUTSIDE:
          state = Dali::PointState::LEAVE;
          break;
      }

      Dali::TouchPoint point(deviceId, state, x, y);
      Dali::Internal::Adaptor::FrameworkAndroid::Impl::NativeAppTouchEvent(framework, point, timeStamp);
      return 1;
    }
    else if(AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
    {
      int32_t deviceId  = AInputEvent_getDeviceId(event);
      int32_t keyCode   = AKeyEvent_getKeyCode(event);
      int32_t action    = AKeyEvent_getAction(event);
      int64_t timeStamp = AKeyEvent_getEventTime(event);

      Dali::KeyEvent::State state = Dali::KeyEvent::DOWN;
      switch(action)
      {
        case AKEY_EVENT_ACTION_DOWN:
          break;
        case AKEY_EVENT_ACTION_UP:
          state = Dali::KeyEvent::UP;
          break;
      }

      std::string keyName = "";
      switch(keyCode)
      {
        case 4:
          keyName = "XF86Back";
          break;
        default:
          break;
      }
      Dali::KeyEvent keyEvent = Dali::DevelKeyEvent::New(keyName, "", "", keyCode, 0, timeStamp, state, "", "", Device::Class::NONE, Device::Subclass::NONE);
      Dali::Internal::Adaptor::FrameworkAndroid::Impl::NativeAppKeyEvent(framework, keyEvent);
      return 1;
    }

    return 0;
  }

  static void HandleAppIdle(struct android_app* app, struct android_poll_source* source)
  {
    FrameworkAndroid* framework = static_cast<FrameworkAndroid*>(AndroidFramework::GetImplementation(AndroidFramework::Get()).GetFramework());
    if(framework && framework->mImpl)
    {
      framework->mImpl->OnIdle();
    }
  }
};

FrameworkAndroid::FrameworkAndroid(Framework::Observer& observer, Framework::TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread)
: Framework(observer, taskObserver, argc, argv, type, useUiThread),
  mInitialised(false),
  mImpl(NULL)
{
  mImpl = new Impl(this);
}

FrameworkAndroid::~FrameworkAndroid()
{
  if(mRunning)
  {
    Quit();
  }

  delete mImpl;
  mImpl = nullptr;
}

void FrameworkAndroid::Run()
{
  struct android_app* app = AndroidFramework::Get().GetNativeApplication();
  app->onAppCmd           = FrameworkAndroid::Impl::HandleAppCmd;
  app->onInputEvent       = FrameworkAndroid::Impl::HandleAppInput;

  struct android_poll_source* source;
  struct android_poll_source  idlePollSource;
  idlePollSource.id      = LOOPER_ID_USER;
  idlePollSource.app     = app;
  idlePollSource.process = Impl::HandleAppIdle;

  int idlePipe[2];
  if(pipe(idlePipe))
  {
    DALI_LOG_ERROR("Failed to open idle pipe\n");
    return;
  }

  mImpl->mIdleReadPipe  = idlePipe[0];
  mImpl->mIdleWritePipe = idlePipe[1];
  ALooper_addFd(app->looper,
                idlePipe[0],
                LOOPER_ID_USER,
                ALOOPER_EVENT_INPUT,
                NULL,
                &idlePollSource);

  mRunning = true;

  // Read all pending events.
  int events;
  int idleTimeout = -1;

  while(true)
  {
    if(mImpl)
    {
      idleTimeout = mImpl->GetIdleTimeout();
    }

    int id = ALooper_pollAll(idleTimeout, NULL, &events, (void**)&source);

    // Process the error.
    if(id == ALOOPER_POLL_ERROR)
    {
      DALI_LOG_ERROR("ALooper error\n");
      Quit();
      std::abort();
    }

    // Process the timeout, trigger OnIdle.
    if(id == ALOOPER_POLL_TIMEOUT)
    {
      int8_t msg = 1;
      write(mImpl->mIdleWritePipe, &msg, sizeof(msg));
    }

    // Process the application event.
    if(id >= 0 && source != NULL)
    {
      source->process(app, source);
    }

    // Check if we are exiting.
    if(app->destroyRequested)
    {
      break;
    }
  }

  while(!mImpl->mIdleCallbacks.empty())
  {
    mImpl->mIdleCallbacks.pop();
  }

  mImpl->mRemovedIdleCallbacks.clear();
  mImpl->mIdleId = 0;

  ALooper_removeFd(app->looper, idlePipe[0]);
  if(mImpl)
  {
    mImpl->mIdleReadPipe  = -1;
    mImpl->mIdleWritePipe = -1;
  }
  close(idlePipe[0]);
  close(idlePipe[1]);

  mRunning = false;
}

unsigned int FrameworkAndroid::AddIdle(int timeout, void* data, bool (*callback)(void* data))
{
  if(mImpl)
  {
    return mImpl->AddIdle(timeout, data, callback);
  }

  return 0;
}

void FrameworkAndroid::RemoveIdle(unsigned int id)
{
  if(mImpl)
  {
    mImpl->RemoveIdle(id);
  }
}

void FrameworkAndroid::Quit()
{
  struct android_app* app = AndroidFramework::Get().GetNativeApplication();
  if(app && !app->destroyRequested && !mImpl->mFinishRequested)
  {
    mImpl->mFinishRequested = true;
    ANativeActivity_finish(app->activity);
  }
}

bool FrameworkAndroid::AppStatusHandler(int type, void* data)
{
  Dali::Adaptor* adaptor = nullptr;
  switch(type)
  {
    case APP_WINDOW_CREATED:
    {
      if(!mInitialised)
      {
        mObserver.OnInit();
        mInitialised = true;
      }

      mObserver.OnSurfaceCreated(data);
      break;
    }

    case APP_RESET:
    {
      mObserver.OnReset();
      break;
    }

    case APP_RESUME:
    {
      mObserver.OnResume();
      adaptor = &Dali::Adaptor::Get();
      adaptor->Resume();
      break;
    }

    case APP_WINDOW_DESTROYED:
    {
      mObserver.OnSurfaceDestroyed(data);
      break;
    }

    case APP_PAUSE:
    {
      adaptor = &Dali::Adaptor::Get();
      adaptor->Pause();
      mObserver.OnPause();
      break;
    }

    case APP_LANGUAGE_CHANGE:
    {
      mObserver.OnLanguageChanged();
      break;
    }

    case APP_DESTROYED:
    {
      adaptor = &Dali::Adaptor::Get();
      // Need to remove constraints before Terminate is called as the constraint function
      // can be destroyed before the constraints get a chance to clean up.
      RemoveAllConstraints(adaptor->GetWindows());
      mObserver.OnTerminate();
      mInitialised = false;
      break;
    }

    default:
    {
      break;
    }
  }

  return true;
}

/**
 * Impl for Pre-Initailized using UI Thread.
 */
struct UIThreadLoader::Impl
{
  // Constructor

  Impl(void *data)
  {
  }

  ~Impl()
  {
  }

  void Run(Runner runner)
  {
  }

private:
  // Undefined
  Impl(const Impl& impl);
  Impl& operator=(const Impl& impl);
};

/**
 * UI Thread loader to support Pre-Initailized using UI Thread.
 */
UIThreadLoader::UIThreadLoader(int* argc, char*** argv)
: mArgc(argc),
  mArgv(argv),
  mImpl(nullptr)
{
}

UIThreadLoader::~UIThreadLoader()
{
}

void UIThreadLoader::Run(Runner runner)
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
