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
 *
 */

#import <Foundation/Foundation.h>
#include "extern-definitions.h"

#include "callback-manager-mac.h"

#include <unordered_map>
#include <optional>

namespace
{
NSString *EventName = @"Dali::Internal::Adaptor::CallbackManager";
}

using Dali::Internal::Adaptor::CocoaCallbackManager;

// This is the observer that processes callback events
@interface CallbackObserver : NSObject
- (CallbackObserver *) init;
- (void) ReceiveCallback:(NSNotification *) aNotification;
@end

// DaliCallback is the Objective-C object holding the information to execute the callback
@interface DaliCallback : NSObject
- (DaliCallback *) initWithImpl:(CocoaCallbackManager::Impl*) impl
                   withCallback:(Dali::CallbackBase *) callback;
- (void) ExecuteCallback;
@end

namespace Dali::Internal::Adaptor
{

namespace Detail
{
// Helper class to implement the callbacks containers
// The boolean value corresponds to the hasReturnValue parameter
struct CallbackContainer final : public std::unordered_map<CallbackBase*, bool>
{
  using Parent = std::unordered_map<CallbackBase*, bool>;
  using iterator = Parent::iterator;
  using const_iterator = Parent::const_iterator;
  using value_type = Parent::value_type;
  using key_type = Parent::key_type;
  using size_type = Parent::size_type;

  ~CallbackContainer() { Clear(); }

  void RemoveCallback(const_iterator item)
  {
    delete item->first;
    erase(item);
  }

  bool RemoveCallback(CallbackBase *callback)
  {
    if (auto it(find(callback)); it != end())
    {
      RemoveCallback(it);
      return true;
    }

    return false;
  }

  void Clear()
  {
    for (auto [cb, dummy]: *this)
    {
      delete cb;
    }

    clear();
  }

  // Execute the callback if it is present. The first item in the
  // return value tells either the callback was executed or not.
  // The second item gives the return value of the callback itself,
  // if any.
  std::pair<const_iterator, std::optional<bool>>
  Execute(CallbackBase *callback) const
  {
    std::optional<bool> retValue;

    auto it(find(callback));
    if (it != end())
    {
      retValue = Execute(it);
    }

    return std::make_pair(it, retValue);
  }

  std::optional<bool> Execute(const_iterator it) const
  {
    auto [callback, hasReturnValue] = *it;
    if (hasReturnValue)
    {
      return CallbackBase::ExecuteReturn<bool>(*callback);
    }
    else
    {
      CallbackBase::Execute(*callback);
    }

    return std::optional<bool>();
  }
};
}

// Internal implementation of the CallbackManager
struct CocoaCallbackManager::Impl final
{
  CFRunLoopObserverContext mObserverContext;

  Detail::CallbackContainer mCallbacks, mIdleEntererCallbacks;
  CFRef<CFRunLoopObserverRef> mIdleObserver;
  CallbackObserver *mObserver;

  Impl();
  ~Impl();

  Impl(const Impl &) = delete;
  Impl &operator=(const Impl&) = delete;
  Impl(const Impl &&) = delete;
  Impl &operator=(const Impl&&) = delete;

  bool ProcessIdle();
  void EnqueueNotification(DaliCallback *callback) const;
  inline bool AddIdleEntererCallback(CallbackBase *callback);
  bool AddIdleCallback(CallbackBase *callback, bool hasReturnValue);

private:
  static void IdleEnterObserverCallback(
    CFRunLoopObserverRef observer,
    CFRunLoopActivity activity,
    void *info
  );
};

CocoaCallbackManager::Impl::Impl()
  : mObserverContext{0, this, nullptr, nullptr, 0}
  // mIdleObserver is configured to receive a notification
  // when to run loop is about to sleep
  , mIdleObserver(MakeRef(CFRunLoopObserverCreate(
      kCFAllocatorDefault,
      kCFRunLoopBeforeWaiting,
      true,
      0,
      IdleEnterObserverCallback,
      &mObserverContext)))
{
  CFRunLoopAddObserver(CFRunLoopGetMain(), mIdleObserver.get(), kCFRunLoopCommonModes);
  mObserver = [[CallbackObserver alloc] init];
}

CocoaCallbackManager::Impl::~Impl()
{
  CFRunLoopRemoveObserver(CFRunLoopGetMain(), mIdleObserver.get(), kCFRunLoopCommonModes);
  auto *center = [NSNotificationCenter defaultCenter];
  [center removeObserver:mObserver name:EventName object:nil];
}

bool CocoaCallbackManager::Impl::ProcessIdle()
{
  auto ret = !mCallbacks.empty();
  for (auto it(cbegin(mCallbacks)), e(cend(mCallbacks)); it != e; ++it)
  {
    if (!mCallbacks.Execute(it).value_or(false))
    {
      mCallbacks.RemoveCallback(it);
    }
  }

  return ret;
}

void CocoaCallbackManager::Impl::EnqueueNotification(DaliCallback *callback) const
{
  auto *notification = [NSNotification notificationWithName:EventName object:callback];
  auto *queue = [NSNotificationQueue defaultQueue];
  [queue enqueueNotification:notification postingStyle:NSPostWhenIdle coalesceMask:0 forModes:nil];
}

bool CocoaCallbackManager::Impl::AddIdleEntererCallback(CallbackBase *callback)
{
  return mIdleEntererCallbacks.emplace(callback, true).second;
}

void CocoaCallbackManager::Impl::IdleEnterObserverCallback(
  CFRunLoopObserverRef observer,
  CFRunLoopActivity activity,
  void *info
)
{
  auto *pImpl = reinterpret_cast<Impl*>(info);

  for (auto it(cbegin(pImpl->mIdleEntererCallbacks)),
      e(cend(pImpl->mIdleEntererCallbacks)); it != e; ++it)
  {
    if (!pImpl->mIdleEntererCallbacks.Execute(it).value_or(false))
    {
      pImpl->mIdleEntererCallbacks.RemoveCallback(it);
    }
  }
}

bool CocoaCallbackManager::Impl::AddIdleCallback(
    CallbackBase *callback, bool hasReturnValue)
{
  if (mCallbacks.emplace(callback, hasReturnValue).second)
  {
    auto *daliCallback = [[DaliCallback alloc] initWithImpl:this
                                               withCallback:callback];
    EnqueueNotification(daliCallback);
    return true;
  }

  return false;
}

CocoaCallbackManager::CocoaCallbackManager()
  : mImpl(std::make_unique<CocoaCallbackManager::Impl>())
  , mRunning(false)
{
}

CocoaCallbackManager::~CocoaCallbackManager() = default;

bool CocoaCallbackManager::AddIdleCallback(CallbackBase *callback, bool hasReturnValue)
{
  return mRunning && mImpl->AddIdleCallback(callback, hasReturnValue);
}

void CocoaCallbackManager::RemoveIdleCallback(CallbackBase *callback)
{
  mImpl->mCallbacks.RemoveCallback(callback);
}

bool CocoaCallbackManager::ProcessIdle()
{
  return mImpl->ProcessIdle();
}

void CocoaCallbackManager::ClearIdleCallbacks()
{
  mImpl->mCallbacks.Clear();
}

bool CocoaCallbackManager::AddIdleEntererCallback(CallbackBase* callback)
{
  return mRunning && mImpl->AddIdleEntererCallback(callback);;
}

void CocoaCallbackManager::RemoveIdleEntererCallback(CallbackBase* callback)
{
  mImpl->mIdleEntererCallbacks.RemoveCallback(callback);
}

void CocoaCallbackManager::Start()
{
  DALI_ASSERT_DEBUG( mRunning == false );
  mRunning = true;
}

void CocoaCallbackManager::Stop()
{
  DALI_ASSERT_DEBUG( mRunning == true );
  mRunning = false;
}

}

@implementation DaliCallback
{
  CocoaCallbackManager::Impl *mImpl;
  Dali::CallbackBase *mCallback;
}

- (DaliCallback *) initWithImpl:(CocoaCallbackManager::Impl *) impl
                   withCallback:(Dali::CallbackBase *) callback
{
  self = [super init];
  if (self)
  {
    mImpl = impl;
    mCallback = callback;
  }
  return self;
}

- (void) ExecuteCallback
{
  // Look for the callback inside the list.
  // If it is not there, then it was either called by ProcessIdle
  // or was removed by RemoveCallback.
  if (auto [iter, shouldKeep] = mImpl->mCallbacks.Execute(mCallback);
      iter != mImpl->mCallbacks.end())
  {
    if (!shouldKeep.value_or(false))
    {
      mImpl->mCallbacks.RemoveCallback(iter);
    }
    else
    {
      mImpl->EnqueueNotification(self);
    }
  }
}
@end

@implementation CallbackObserver
- (CallbackObserver *) init
{
  self = [super init];
  if (self)
  {
    auto *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self
               selector:@selector(ReceiveCallback:)
                   name:EventName
                 object:nil];
  }
  return self;
}

- (void) ReceiveCallback:(NSNotification *)aNotification
{
  DaliCallback *callback = [aNotification object];
  [callback ExecuteCallback];
}
@end
