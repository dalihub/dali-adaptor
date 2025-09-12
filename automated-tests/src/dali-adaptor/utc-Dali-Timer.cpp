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

#include <adaptor-test-application.h>
#include <dali-test-suite-utils.h>
#include <dali/dali.h>
#include <dali/internal/system/linux/dali-ecore.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>

using namespace Dali;

void utc_dali_timer_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_timer_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{
bool          ecore_timer_running = false;
Ecore_Task_Cb timer_callback_func = NULL;
const void*   timer_callback_data = NULL;
bool          main_loop_can_run   = false;
intptr_t      timerId             = 0; // intptr_t has the same size as a pointer and is platform independent so this can be returned as a pointer in ecore_timer_add below without compilation warnings
} // namespace

extern "C" {
Ecore_Timer* ecore_timer_add(double        in,
                             Ecore_Task_Cb func,
                             const void*   data)
{
  ecore_timer_running = true;
  timer_callback_func = func;
  timer_callback_data = data;
  timerId += 8;
  return (Ecore_Timer*)timerId;
}

void* ecore_timer_del(Ecore_Timer* timer)
{
  ecore_timer_running = false;
  timer_callback_func = NULL;
  return NULL;
}
}

namespace
{
void test_ecore_main_loop_begin()
{
  if(timer_callback_func != NULL)
  {
    main_loop_can_run = true;
    while(main_loop_can_run)
    {
      if(!timer_callback_func(const_cast<void*>(timer_callback_data)))
        break;
    }
  }
}

void test_ecore_main_loop_quit()
{
  timer_callback_func = NULL;
  main_loop_can_run   = false;
}

/**
 * small class to test timer signal
 */
class TimerTestClass : public ConnectionTracker
{
public:
  TimerTestClass(bool repeat)
  : mTimerCalled(false),
    mReturnContiune(repeat)
  {
  }

  bool Tick()
  {
    tet_printf("timer ticked\n");
    mTimerCalled = true;
    // quit the main loop otherwise we'll never return to tet
    test_ecore_main_loop_quit();
    return mReturnContiune;
  }
  bool mTimerCalled;    // whether tick has been called or not
  bool mReturnContiune; // whether to return true / false to continue
};

} // namespace

// Positive test case for a method
int UtcDaliTimerCreation(void)
{
  AdaptorTestApplication application;

  tet_printf("timer creation \n");
  Timer timer = Timer::New(300);

  DALI_TEST_CHECK(timer);

  DALI_TEST_CHECK(timer.GetInterval() == 300);

  END_TEST;
}

int UtcDaliTimerUnitializedStart(void)
{
  AdaptorTestApplication application;

  tet_printf("unintialized timer start \n");

  Timer* timer = new Timer;
  DALI_TEST_CHECK(timer != NULL);

  try
  {
    timer->Start();
  }
  catch(Dali::DaliException& e)
  {
    DALI_TEST_ASSERT(e, "timer", TEST_LOCATION);
  }
  END_TEST;
}

int UtcDaliTimerUnitializedStop(void)
{
  AdaptorTestApplication application;

  tet_printf("unintialized timer stop \n");

  Timer* timer = new Timer;
  DALI_TEST_CHECK(timer != NULL);

  try
  {
    timer->Stop();
  }
  catch(Dali::DaliException& e)
  {
    DALI_TEST_ASSERT(e, "timer", TEST_LOCATION);
  }
  END_TEST;
}

int UtcDaliTimerUnitializedGetInterval(void)
{
  AdaptorTestApplication application;

  tet_printf("unintialized get interval \n");

  Timer* timer = new Timer;
  DALI_TEST_CHECK(timer != NULL);

  try
  {
    timer->GetInterval();
  }
  catch(Dali::DaliException& e)
  {
    DALI_TEST_ASSERT(e, "timer", TEST_LOCATION);
  }
  END_TEST;
}

int UtcDaliTimerUnitializedSetInterval(void)
{
  AdaptorTestApplication application;

  tet_printf("unintialized set interval \n");

  Timer* timer = new Timer;
  DALI_TEST_CHECK(timer != NULL);

  try
  {
    timer->SetInterval(10);
  }
  catch(Dali::DaliException& e)
  {
    DALI_TEST_ASSERT(e, "timer", TEST_LOCATION);
  }
  END_TEST;
}

int UtcDaliTimerUnitializedIsRunning(void)
{
  AdaptorTestApplication application;

  tet_printf("unintialized is running \n");

  Timer* timer = new Timer;
  DALI_TEST_CHECK(timer != NULL);

  try
  {
    timer->IsRunning();
  }
  catch(Dali::DaliException& e)
  {
    DALI_TEST_ASSERT(e, "timer", TEST_LOCATION);
  }
  END_TEST;
}

int UtcDaliTimerUnitializedSignalTick(void)
{
  AdaptorTestApplication application;

  tet_printf("unintialized SignalTick \n");

  Timer* timer = new Timer;
  DALI_TEST_CHECK(timer != NULL);

  try
  {
    TimerTestClass testClass(true); // = new TimerTestClass(true);

    timer->TickSignal().Connect(&testClass, &TimerTestClass::Tick);
  }
  catch(Dali::DaliException& e)
  {
    DALI_TEST_ASSERT(e, "timer", TEST_LOCATION);
  }
  END_TEST;
}

int UtcDaliTimerSetInterval(void)
{
  AdaptorTestApplication application;

  tet_printf("timer set interval \n");
  Timer timer = Timer::New(10);

  DALI_TEST_CHECK(timer.GetInterval() == 10);

  timer.SetInterval(5000);

  DALI_TEST_CHECK(timer.GetInterval() == 5000);

  END_TEST;
}

int UtcDaliTimerSetInterval02(void)
{
  AdaptorTestApplication application;

  tet_printf("timer set interval 02 \n");
  Timer timer = Timer::New(10);
  timer.SetInterval(20);

  DALI_TEST_CHECK(timer.GetInterval() == 20);
  DALI_TEST_CHECK(timer.IsRunning() == true);

  timer.SetInterval(5000, false);

  DALI_TEST_CHECK(timer.GetInterval() == 5000);
  DALI_TEST_CHECK(timer.IsRunning() == false);

  END_TEST;
}

int UtcDaliTimerSetInterval03(void)
{
  AdaptorTestApplication application;

  tet_printf("UtcDaliTimerSetInterval03 SetInterval and ensure timer restarts \n");
  Timer timer = Timer::New(10);
  timer.SetInterval(20);

  DALI_TEST_CHECK(timer.GetInterval() == 20);
  DALI_TEST_CHECK(timer.IsRunning() == true);

  timer.SetInterval(5000, true);

  DALI_TEST_CHECK(timer.GetInterval() == 5000);
  DALI_TEST_CHECK(timer.IsRunning() == true);

  END_TEST;
}

int UtcDaliTimerCopyConstructor(void)
{
  AdaptorTestApplication application;

  tet_printf("timer copy constructor \n");
  Timer timer = Timer::New(10);

  Timer anotherTimer(timer);

  DALI_TEST_CHECK(anotherTimer.GetInterval() == 10);
  END_TEST;
}

int UtcDaliTimerAssignmentOperator(void)
{
  AdaptorTestApplication application;

  tet_printf("assignmnet constructor \n");

  Timer timer = Timer::New(10);

  DALI_TEST_CHECK(timer);

  Timer anotherTimer = Timer::New(40);

  DALI_TEST_CHECK(anotherTimer.GetInterval() == 40);

  tet_printf("timer 1 interval %d, \n", anotherTimer.GetInterval());
  tet_printf("timer 2 interval %d, \n", timer.GetInterval());

  DALI_TEST_CHECK(timer != anotherTimer);

  timer = anotherTimer;

  DALI_TEST_CHECK(timer == anotherTimer);

  tet_printf("timer 1 interval %d, \n", timer.GetInterval());
  tet_printf("timer 2 interval %d, \n", anotherTimer.GetInterval());

  DALI_TEST_CHECK(timer.GetInterval() == 40);

  END_TEST;
}

int UtcDaliTimerMoveConstructor(void)
{
  AdaptorTestApplication application;

  Timer timer = Timer::New(40);
  DALI_TEST_CHECK(timer);
  DALI_TEST_EQUALS(1, timer.GetBaseObject().ReferenceCount(), TEST_LOCATION);
  DALI_TEST_CHECK(timer.GetInterval() == 40);

  Timer moved = std::move(timer);
  DALI_TEST_CHECK(moved);
  DALI_TEST_EQUALS(1, moved.GetBaseObject().ReferenceCount(), TEST_LOCATION);
  DALI_TEST_CHECK(moved.GetInterval() == 40);
  DALI_TEST_CHECK(!timer);

  END_TEST;
}

int UtcDaliTimerMoveAssignmentr(void)
{
  AdaptorTestApplication application;

  Timer timer = Timer::New(40);
  DALI_TEST_CHECK(timer);
  DALI_TEST_EQUALS(1, timer.GetBaseObject().ReferenceCount(), TEST_LOCATION);
  DALI_TEST_CHECK(timer.GetInterval() == 40);

  Timer moved;
  moved = std::move(timer);
  DALI_TEST_CHECK(moved);
  DALI_TEST_EQUALS(1, moved.GetBaseObject().ReferenceCount(), TEST_LOCATION);
  DALI_TEST_CHECK(moved.GetInterval() == 40);
  DALI_TEST_CHECK(!timer);

  END_TEST;
}

int UtcDaliTimerIsRunning(void)
{
  AdaptorTestApplication application;

  tet_printf("timer is running \n");

  Timer timer = Timer::New(100);

  timer.Start();

  DALI_TEST_CHECK(timer.IsRunning());

  timer.Stop();

  DALI_TEST_CHECK(timer.IsRunning() == false);

  END_TEST;
}

int UtcDaliTimerSignalTickContinue(void)
{
  AdaptorTestApplication application;

  tet_printf("timer call back\n");

  Timer          timer = Timer::New(100);
  TimerTestClass testClass(true);

  timer.TickSignal().Connect(&testClass, &TimerTestClass::Tick);

  timer.Start();

  test_ecore_main_loop_begin();

  DALI_TEST_CHECK(testClass.mTimerCalled);

  END_TEST;
}

int UtcDaliTimerSignalTickStop(void)
{
  AdaptorTestApplication application;

  Timer          timer = Timer::New(100);
  TimerTestClass testClass(false);

  timer.TickSignal().Connect(&testClass, &TimerTestClass::Tick);

  timer.Start();

  test_ecore_main_loop_begin();

  DALI_TEST_CHECK(testClass.mTimerCalled);

  END_TEST;
}

int UtcDaliTimerReset(void)
{
  AdaptorTestApplication application;

  Timer timer = Timer::New(100);

  DALI_TEST_CHECK(timer);

  timer.Reset();

  DALI_TEST_CHECK(!timer);

  END_TEST;
}

int UtcDaliTimerDownCastP(void)
{
  AdaptorTestApplication application;

  Timer timer = Timer::New(100);
  Timer cast  = Timer::DownCast(timer);

  DALI_TEST_CHECK(cast);

  END_TEST;
}

int UtcDaliTimerDownCastN(void)
{
  AdaptorTestApplication application;

  Timer timer;
  Timer cast = Timer::DownCast(timer);

  DALI_TEST_CHECK(!cast);

  END_TEST;
}

int UtcDaliTimerPauseN(void)
{
  Timer timer;

  try
  {
    timer.Pause();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliTimerResumeN(void)
{
  Timer timer;

  try
  {
    timer.Resume();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}
