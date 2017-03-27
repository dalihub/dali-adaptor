/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <dali/dali.h>
#include <dali-test-suite-utils.h>
#include "public-api/dali-wearable.h"
#include <appcore-watch/watch_app.h>
#include <appcore-watch/watch_app_extension.h>
#include <stdlib.h>
#define TIMEZONE_BUFFER_MAX 102

using namespace Dali;

void utc_dali_watchtime_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_watchtime_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{

struct MyTestApp : public ConnectionTracker
{
  MyTestApp( WatchApplication& app)
  : initCalled( false ),
    mApplication( app )
  {
    mApplication.InitSignal().Connect( this, &MyTestApp::Create );
  }

  void Create(Application& app)
  {
    initCalled = true;
  }

  void Quit()
  {
    mApplication.Quit();
  }

  // Data
  bool initCalled;
  WatchApplication&  mApplication;
};

} // unnamed namespace

int UtcDaliWatchTimeNew(void)
{
  WatchTime watchTime;
  WatchTime *watchTimeRef = &watchTime;

  DALI_TEST_CHECK( watchTimeRef );

  END_TEST;
}

int UtcDaliWatchTimeGetHour(void)
{
  int ret, hour;
  WatchTime watchTime;
  watch_time_h watch_time = {0,};

  ret = watch_time_get_current_time(&watch_time);

  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  ret = watch_time_get_hour(watch_time, &hour);

  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  DALI_TEST_CHECK( watchTime.GetHour() == hour );

  END_TEST;
}

int UtcDaliWatchTimeGetHour24(void)
{
  int ret, hour24;
  WatchTime watchTime;
  watch_time_h watch_time = {0,};

  ret = watch_time_get_current_time(&watch_time);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  ret = watch_time_get_hour24(watch_time, &hour24);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  DALI_TEST_CHECK( watchTime.GetHour24() == hour24 );

  END_TEST;
}

int UtcDaliWatchTimeGetMinute(void)
{
  int ret, minute;
  WatchTime watchTime;
  watch_time_h watch_time = {0,};

  ret = watch_time_get_current_time(&watch_time);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  ret = watch_time_get_minute(watch_time, &minute);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  DALI_TEST_CHECK( watchTime.GetMinute() == minute );

  END_TEST;
}

int UtcDaliWatchTimeGetSecond(void)
{
  int ret, second;
  WatchTime watchTime;
  watch_time_h watch_time = {0,};

  ret = watch_time_get_current_time(&watch_time);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  ret = watch_time_get_second(watch_time, &second);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  DALI_TEST_CHECK( watchTime.GetSecond() == second );

  END_TEST;
}

int UtcDaliWatchTimeGetMillisecond(void)
{
  int ret, millisecond;
  WatchTime watchTime;
  watch_time_h watch_time = {0,};

  ret = watch_time_get_current_time(&watch_time);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  ret = watch_time_get_millisecond(watch_time, &millisecond);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  DALI_TEST_CHECK( watchTime.GetMillisecond() == millisecond );

  END_TEST;
}

int UtcDaliWatchTimeGetYear(void)
{
  int ret, year;
  WatchTime watchTime;
  watch_time_h watch_time = {0,};

  ret = watch_time_get_current_time(&watch_time);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  ret = watch_time_get_year(watch_time, &year);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  DALI_TEST_CHECK( watchTime.GetYear() == year );

  END_TEST;
}

int UtcDaliWatchTimeGetMonth(void)
{
  int ret, month;
  WatchTime watchTime;
  watch_time_h watch_time = {0,};

  ret = watch_time_get_current_time(&watch_time);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  ret = watch_time_get_month(watch_time, &month);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  DALI_TEST_CHECK( watchTime.GetMonth() == month );

  END_TEST;
}

int UtcDaliWatchTimeGetDay(void)
{
  int ret, day;
  WatchTime watchTime;
  watch_time_h watch_time = {0,};

  ret = watch_time_get_current_time(&watch_time);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  ret = watch_time_get_day(watch_time, &day);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  DALI_TEST_CHECK( watchTime.GetDay() == day );

  END_TEST;
}

int UtcDaliWatchTimeGetDayOfWeek(void)
{
  int ret, dayOfWeek;
  WatchTime watchTime;
  watch_time_h watch_time = {0,};

  ret = watch_time_get_current_time(&watch_time);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  ret = watch_time_get_day_of_week(watch_time, &dayOfWeek);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  DALI_TEST_CHECK( watchTime.GetDayOfWeek() == dayOfWeek );

  END_TEST;
}

int UtcDaliWatchTimeGetUtcTime(void)
{
  int ret;
  struct tm *utcTime = (struct tm *)calloc( 1, sizeof( struct tm ) );
  WatchTime watchTime;
  watch_time_h watch_time = {0,};

  ret = watch_time_get_current_time(&watch_time);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  ret = watch_time_get_day(watch_time, utcTime);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  DALI_TEST_CHECK( watchTime.GetUtcTime().tm_sec == (*utcTime).tm_sec );

  END_TEST;
}

int UtcDaliWatchTimeGetUtcTimeStamp(void)
{
  int ret;
  time_t *timeStamp = (time_t *)calloc( 1, sizeof( time_t ) );
  WatchTime watchTime;
  watch_time_h watch_time = {0,};

  ret = watch_time_get_current_time(&watch_time);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  ret = watch_time_get_day(watch_time, timeStamp);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  DALI_TEST_CHECK( watchTime.GetUtcTimeStamp() == *timeStamp );

  END_TEST;
}

int UtcDaliWatchTimeGetTimeZone(void)
{
  int ret;
  char *timeZone[TIMEZONE_BUFFER_MAX] = {0,};
  WatchTime watchTime;
  watch_time_h watch_time = {0,};

  ret = watch_time_get_current_time(&watch_time);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  ret = watch_time_get_day(watch_time, timeZone);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  DALI_TEST_CHECK( watchTime.GetTimeZone() == timeZone );

  END_TEST;
}

int UtcDaliWatchTimeGetDaylightSavingTimeStatus(void)
{
  int ret;
  bool daylight;
  WatchTime watchTime;
  watch_time_h watch_time = {0,};

  ret = watch_time_get_current_time(&watch_time);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  ret = watch_time_get_day(watch_time, &daylight);
  DALI_TEST_CHECK( ret == APP_ERROR_NONE );

  DALI_TEST_CHECK( watchTime.GetDaylightSavingTimeStatus() == daylight );

  END_TEST;
}
