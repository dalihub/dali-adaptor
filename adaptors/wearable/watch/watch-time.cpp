/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <wearable/watch/watch-time.h>

// EXTERNAL INCLUDES
#ifdef APPCORE_WATCH_AVAILABLE
#include <watch_app.h>
#include <watch_app_extension.h>
#endif

namespace Dali
{

struct WatchTime::Impl
{
  Impl(void *time_handle)
  : mTimeHandle(time_handle)
  {
  }

  void *mTimeHandle;
};

WatchTime::WatchTime(void *time_handle)
{
  mImpl = new Impl(time_handle);
}

WatchTime::~WatchTime()
{
  if( mImpl )
  {
    delete mImpl;
    mImpl = NULL;
  }
}

#ifdef APPCORE_WATCH_AVAILABLE

WatchTime::WatchTime()
{
  watch_time_h watch_time = {0,};

  watch_time_get_current_time(&watch_time);
  mImpl = new Impl(watch_time);
}

int WatchTime::GetHour() const
{
  int hour;

  watch_time_get_hour(reinterpret_cast<watch_time_h>(mImpl->mTimeHandle), &hour);
  return hour;
}

int WatchTime::GetHour24() const
{
  int hour24;

  watch_time_get_hour24(reinterpret_cast<watch_time_h>(mImpl->mTimeHandle), &hour24);
  return hour24;
}

int WatchTime::GetMinute() const
{
  int minute;

  watch_time_get_minute(reinterpret_cast<watch_time_h>(mImpl->mTimeHandle), &minute);
  return minute;
}

int WatchTime::GetSecond() const
{
  int second;

  watch_time_get_second(reinterpret_cast<watch_time_h>(mImpl->mTimeHandle), &second);
  return second;
}

int WatchTime::GetMillisecond() const
{
  int millisecond;

  watch_time_get_millisecond(reinterpret_cast<watch_time_h>(mImpl->mTimeHandle), &millisecond);
  return millisecond;
}

int WatchTime::GetYear() const
{
  int year;

  watch_time_get_year(reinterpret_cast<watch_time_h>(mImpl->mTimeHandle), &year);
  return year;
}

int WatchTime::GetMonth() const
{
  int month;

  watch_time_get_month(reinterpret_cast<watch_time_h>(mImpl->mTimeHandle), &month);
  return month;
}

int WatchTime::GetDay() const
{
  int day;

  watch_time_get_day(reinterpret_cast<watch_time_h>(mImpl->mTimeHandle), &day);
  return day;
}

int WatchTime::GetDayOfWeek() const
{
  int dayOfWeek;

  watch_time_get_day_of_week(reinterpret_cast<watch_time_h>(mImpl->mTimeHandle), &dayOfWeek);
  return dayOfWeek;
}

struct tm WatchTime::GetUtcTime() const
{
  struct tm UtcTime;

  watch_time_get_utc_time(reinterpret_cast<watch_time_h>(mImpl->mTimeHandle), &UtcTime);
  return UtcTime;
}

time_t WatchTime::GetUtcTimeStamp() const
{
  time_t UtcTimeStamp;

  watch_time_get_utc_timestamp(reinterpret_cast<watch_time_h>(mImpl->mTimeHandle), &UtcTimeStamp);
  return UtcTimeStamp;
}

const char* WatchTime::GetTimeZone() const
{
  char* timeZone;

  watch_time_get_time_zone(reinterpret_cast<watch_time_h>(mImpl->mTimeHandle), &timeZone);
  return timeZone;
}

bool WatchTime::GetDaylightSavingTimeStatus() const
{
  bool daylight;

  watch_time_get_daylight_time_status(reinterpret_cast<watch_time_h>(mImpl->mTimeHandle), &daylight);
  return daylight;
}

#else
WatchTime::WatchTime()
  :mImpl(NULL)
{
}

int WatchTime::GetHour() const
{
  return 0;
}

int WatchTime::GetHour24() const
{
  return 0;
}

int WatchTime::GetMinute() const
{
  return 0;
}

int WatchTime::GetSecond() const
{
  return 0;
}

int WatchTime::GetMillisecond() const
{
  return 0;
}

int WatchTime::GetYear() const
{
  return 0;
}

int WatchTime::GetMonth() const
{
  return 0;
}

int WatchTime::GetDay() const
{
  return 0;
}

int WatchTime::GetDayOfWeek() const
{
  return 0;
}

struct tm WatchTime::GetUtcTime() const
{
  time_t zero = time(0);
  return *localtime(&zero);
}

time_t WatchTime::GetUtcTimeStamp() const
{
  return 0;
}

const char* WatchTime::GetTimeZone() const
{
  return 0;
}

bool WatchTime::GetDaylightSavingTimeStatus() const
{
  return 0;
}

#endif

} // namespace Dali
