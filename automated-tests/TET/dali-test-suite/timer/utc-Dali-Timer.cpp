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

#include <iostream>

#include <stdlib.h>
#include <tet_api.h>

#include <dali/dali.h>
#include <Ecore.h>

#include <dali-test-suite-utils.h>

#include <dali/public-api/adaptor-framework/common/timer.h>

using namespace Dali;


/**
 * small class to test timer signal
 */
class TimerTestClass : public ConnectionTracker
{
public:

  TimerTestClass(bool repeat):mTimerCalled(false),mReturnContiune(repeat) {}

  bool Tick()
  {
    tet_printf("timer ticked\n");
    mTimerCalled = true;
    // quit the main loop otherwise we'll never return to tet
    ecore_main_loop_quit();
    return mReturnContiune;
  }
  bool mTimerCalled;    // whether tick has been called or not
  bool mReturnContiune; // whether to return true / false to continue

};


static void Startup();
static void Cleanup();

extern "C" {
  void (*tet_startup)() = Startup;
  void (*tet_cleanup)() = Cleanup;
}

static void UtcDaliTimerCreation();
static void UtcDaliTimerUnitializedStart();
static void UtcDaliTimerUnitializedStop();
static void UtcDaliTimerUnitializedGetInterval();
static void UtcDaliTimerUnitializedSetInterval();
static void UtcDaliTimerUnitializedIsRunning();
static void UtcDaliTimerUnitializedSignalTick();
static void UtcDaliTimerSetInterval();
static void UtcDaliTimerCopyConstructor();
static void UtcDaliTimerAssignmentOperator();
static void UtcDaliTimerIsRunning();
static void UtcDaliTimerSignalTickContinue();
static void UtcDaliTimerSignalTickStop();
static void UtcDaliTimerReset();




enum {
  POSITIVE_TC_IDX = 0x01,
  NEGATIVE_TC_IDX,
};

// Add test functionality for all APIs in the class (Positive and Negative)
extern "C" {
  struct tet_testlist tet_testlist[] = {
    { UtcDaliTimerCreation, POSITIVE_TC_IDX },
    { UtcDaliTimerUnitializedStart, NEGATIVE_TC_IDX},
    { UtcDaliTimerUnitializedStop, NEGATIVE_TC_IDX},
    { UtcDaliTimerUnitializedGetInterval , NEGATIVE_TC_IDX},
    { UtcDaliTimerUnitializedSetInterval, NEGATIVE_TC_IDX},
    { UtcDaliTimerUnitializedIsRunning, NEGATIVE_TC_IDX},
    { UtcDaliTimerUnitializedSignalTick, NEGATIVE_TC_IDX},
    { UtcDaliTimerSetInterval, POSITIVE_TC_IDX },
    { UtcDaliTimerCopyConstructor, POSITIVE_TC_IDX },
    { UtcDaliTimerAssignmentOperator, POSITIVE_TC_IDX},
    { UtcDaliTimerIsRunning, POSITIVE_TC_IDX},
    { UtcDaliTimerSignalTickContinue, POSITIVE_TC_IDX },
    { UtcDaliTimerSignalTickStop, POSITIVE_TC_IDX },
    { UtcDaliTimerReset, POSITIVE_TC_IDX },
    { NULL, 0 }
  };
}

// Called only once before first test is run.
static void Startup()
{
  ecore_init();
}

// Called only once after last test is run
static void Cleanup()
{
  ecore_shutdown();
}


// Positive test case for a method
static void UtcDaliTimerCreation()
{
 // TestApplication application;
  tet_printf("timer creation \n");
  Timer timer = Timer::New(300);

  DALI_TEST_CHECK( timer );

  DALI_TEST_CHECK( timer.GetInterval() == 300);

}

static void UtcDaliTimerUnitializedStart()
{
  tet_printf("unintialized timer start \n");

  Timer *timer = new Timer;
  DALI_TEST_CHECK(timer != NULL);

  try
  {
    timer->Start();
  }
  catch(Dali::DaliException& e)
  {
    DALI_TEST_ASSERT(e, "timer", TEST_LOCATION);
  }
}

static void UtcDaliTimerUnitializedStop()
{
  tet_printf("unintialized timer stop \n");

  Timer *timer = new Timer;
  DALI_TEST_CHECK(timer != NULL);

  try
  {
    timer->Stop();
  }
  catch(Dali::DaliException& e)
  {
    DALI_TEST_ASSERT(e, "timer", TEST_LOCATION);
  }
}

static void UtcDaliTimerUnitializedGetInterval()
{
  tet_printf("unintialized get interval \n");

  Timer *timer = new Timer;
  DALI_TEST_CHECK(timer != NULL);

  try
  {
    timer->GetInterval();
  }
  catch(Dali::DaliException& e)
  {
    DALI_TEST_ASSERT(e, "timer", TEST_LOCATION);
  }
}

static void UtcDaliTimerUnitializedSetInterval()
{
  tet_printf("unintialized set interval \n");

  Timer *timer = new Timer;
  DALI_TEST_CHECK(timer != NULL);

  try
  {
    timer->SetInterval(10);
  }
  catch(Dali::DaliException& e)
  {
    DALI_TEST_ASSERT(e, "timer", TEST_LOCATION);
  }
}

static void UtcDaliTimerUnitializedIsRunning()
{
  tet_printf("unintialized is running \n");

  Timer *timer = new Timer;
  DALI_TEST_CHECK(timer != NULL);

  try
  {
    timer->IsRunning();
  }
  catch(Dali::DaliException& e)
  {
    DALI_TEST_ASSERT(e, "timer", TEST_LOCATION);
  }
}


static void UtcDaliTimerUnitializedSignalTick()
{
  tet_printf("unintialized SignalTick \n");

  Timer *timer = new Timer;
  DALI_TEST_CHECK(timer != NULL);

  try
  {
    TimerTestClass testClass(true);// = new TimerTestClass(true);

    timer->TickSignal().Connect(&testClass, &TimerTestClass::Tick);
  }
  catch(Dali::DaliException& e)
  {
    DALI_TEST_ASSERT(e, "timer", TEST_LOCATION);
  }
}

static void UtcDaliTimerSetInterval()
{
  tet_printf("timer set interval \n");
  Timer timer = Timer::New(10);

  DALI_TEST_CHECK( timer.GetInterval() == 10);

  timer.SetInterval(5000);

  DALI_TEST_CHECK( timer.GetInterval() == 5000);

}

static void UtcDaliTimerCopyConstructor()
{
  tet_printf("timer copy constructor \n");
  Timer timer = Timer::New(10);

  Timer anotherTimer( timer );

  DALI_TEST_CHECK( anotherTimer.GetInterval() == 10);
}

static void UtcDaliTimerAssignmentOperator()
{
  tet_printf("assignmnet constructor \n");

  Timer timer = Timer::New(10);

  DALI_TEST_CHECK( timer );

  Timer anotherTimer = Timer::New(40);

  DALI_TEST_CHECK(anotherTimer.GetInterval() == 40);

  tet_printf("timer 1 interval %d, \n",anotherTimer.GetInterval());
  tet_printf("timer 2 interval %d, \n",timer.GetInterval());

  DALI_TEST_CHECK(timer != anotherTimer);

  timer = anotherTimer;

  DALI_TEST_CHECK(timer == anotherTimer);

  tet_printf("timer 1 interval %d, \n",timer.GetInterval());
  tet_printf("timer 2 interval %d, \n",anotherTimer.GetInterval());

  DALI_TEST_CHECK(timer.GetInterval() == 40);

}

static void UtcDaliTimerIsRunning()
{
  tet_printf("timer is running \n");

  Timer timer = Timer::New(100);

  timer.Start();

  DALI_TEST_CHECK( timer.IsRunning() );

  timer.Stop();

  DALI_TEST_CHECK( timer.IsRunning() == false );

}

static void UtcDaliTimerSignalTickContinue()
{
  tet_printf("timer call back\n");

  Timer timer = Timer::New(100);
  TimerTestClass testClass(true);

  timer.TickSignal().Connect(&testClass, &TimerTestClass::Tick);

  timer.Start();

  ecore_main_loop_begin();

  DALI_TEST_CHECK( testClass.mTimerCalled );

}

static void UtcDaliTimerSignalTickStop()
{
  Timer timer = Timer::New(100);
  TimerTestClass testClass(false);

  timer.TickSignal().Connect(&testClass, &TimerTestClass::Tick);

  timer.Start();

  ecore_main_loop_begin();

  DALI_TEST_CHECK( testClass.mTimerCalled );

}

static void UtcDaliTimerReset()
{
  Timer timer = Timer::New(100);

  DALI_TEST_CHECK(timer);

  timer.Reset();

  DALI_TEST_CHECK(!timer);

}
