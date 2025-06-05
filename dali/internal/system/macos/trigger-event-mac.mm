/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

#include <atomic>
#include <string>
#include <sstream>
#include <type_traits>

#include <dali/internal/system/macos/trigger-event-mac.h>

namespace
{
const char *EventName = "Dali::Internal::Adaptor::Triggerevent_";
}

@interface NotificationObserver : NSObject

-(NotificationObserver *) initTriggerImpl:(Dali::Internal::Adaptor::TriggerEvent::Impl *) impl;

@end

namespace Dali::Internal::Adaptor
{

struct TriggerEvent::Impl final
{
  std::unique_ptr<CallbackBase> mCallback;
  NotificationObserver *mReceiver;
  NSString *mName;
  TriggerEventInterface::Options mOptions;

  Impl(CallbackBase *callback, TriggerEventInterface::Options options)
    : mCallback(callback), mOptions(options)
  {
    const auto myId = mNameId.fetch_add(1, std::memory_order_relaxed);

    std::stringstream ss;
    ss << EventName << myId;
    mName = [NSString stringWithUTF8String:ss.str().c_str()];

    mReceiver = [[NotificationObserver alloc] initTriggerImpl:this];
  }

  Impl(Impl &&) = delete;
  Impl &operator=(Impl &&) = delete;

  ~Impl()
  {
    auto *center = [NSNotificationCenter defaultCenter];
    [center removeObserver:mReceiver];
  }

  void Trigged()
  {
    CallbackBase::Execute( *mCallback );
  }

private:
  // This is incremented each time the class is instatiated to guarantee
  // an unique notification id
  static std::atomic_uint64_t mNameId;
};

std::atomic<uint64_t> TriggerEvent::Impl::mNameId{0};

TriggerEvent::TriggerEvent(CallbackBase *callback, TriggerEventInterface::Options options)
  : mCallback(callback),
    mImpl(std::make_unique<Impl>(MakeCallback(this, &TriggerEvent::Triggered), options))
{
}

TriggerEvent::~TriggerEvent() = default;

void TriggerEvent::Trigger()
{
  auto center = [NSDistributedNotificationCenter defaultCenter];

  // Post a notification to the notification center
  // The run loop will pop the queue and call the notification center
  [center postNotificationName:mImpl->mName object:nil];
}

void TriggerEvent::Triggered()
{
  CallbackBase::Execute(*mCallback);

  if (mImpl->mOptions == TriggerEventInterface::DELETE_AFTER_TRIGGER)
  {
    delete this;
  }
}

}

@implementation NotificationObserver
{
  Dali::Internal::Adaptor::TriggerEvent::Impl *mImpl;
}

-(void) ReceiveNotification: (NSNotification *) aNotification
{
  mImpl->Trigged();
}

-(NotificationObserver *) initTriggerImpl:(Dali::Internal::Adaptor::TriggerEvent::Impl *) impl;
{
  self = [super init];
  if (self)
  {
    mImpl = impl;
    auto center = [NSDistributedNotificationCenter defaultCenter];
    [center addObserver:self
               selector:@selector(ReceiveNotification:)
                   name:impl->mName
                 object:nil
     suspensionBehavior:NSNotificationSuspensionBehaviorDeliverImmediately];
  }

  return self;
}

@end
