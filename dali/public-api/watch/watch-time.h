#ifndef __DALI_WATCH_TIME_H__
#define __DALI_WATCH_TIME_H__

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

// EXTERNAL INCLUDES
#include <time.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-core.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

/**
 * @brief The WatchTime class is used to get time for the WatchApplication.
 *
 * A WatchTime has a time handle from watch application framework.
 * You can get time(hour, minute, second, millisecond) and date(year, month, day)
 * on receiving timeTick signal.
 * @SINCE_1_1.37
 */
class DALI_IMPORT_API WatchTime
{
public:

  /**
   * @brief Constructor.
   * @SINCE_1_1.37
   */
  WatchTime();

  /**
   * @brief Destructor.
   * @SINCE_1_1.37
   */
  ~WatchTime();

  /**
   * @brief Returns the current hour.
   *
   * @SINCE_1_1.37
   * @return The current hour
   * @note The return value is always positive.
   * @pre The WatchTime needs to be initialized.
   */
  int GetHour() const;

  /**
   * @brief Returns the current hour24.
   *
   * @SINCE_1_1.37
   * @return The current hour(the 24-hour clock)
   * @note The return value is always positive.
   * @pre The WatchTime needs to be initialized.
   */
  int GetHour24() const;

  /**
   * @brief Returns the current minute.
   *
   * @SINCE_1_1.37
   * @return The current minute
   * @note The return value is always positive.
   * @pre The WatchTime needs to be initialized.
   */
  int GetMinute() const;

  /**
   * @brief Returns the current second.
   *
   * @SINCE_1_1.37
   * @return The current second
   * @note The return value is always positive.
   * @pre The WatchTime needs to be initialized.
   */
  int GetSecond() const;

  /**
   * @brief Returns the current millisecond.
   *
   * @SINCE_1_2_32
   * @return The current millisecond
   * @note The return value is always positive.
   * @pre The WatchTime needs to be initialized.
   */
  int GetMillisecond() const;

  /**
   * @brief Returns the current year.
   *
   * @SINCE_1_2_32
   * @return The current year
   * @note The return value is always positive.
   * @pre The WatchTime needs to be initialized.
   */
  int GetYear() const;

  /**
   * @brief Returns the current month.
   *
   * @SINCE_1_2_32
   * @return The current month
   * @note The return value is always positive.
   * @pre The WatchTime needs to be initialized.
   */
  int GetMonth() const;

  /**
   * @brief Returns the current day.
   *
   * @SINCE_1_2_32
   * @return The current day
   * @note The return value is always positive.
   * @pre The WatchTime needs to be initialized.
   */
  int GetDay() const;

  /**
   * @brief Returns the current day of week.
   *
   * @details The value returns from 1 (Sunday) to 7 (Saturday).
   *
   * @SINCE_1_2_32
   * @return The current day of week
   * @note The return value is always positive.
   * @pre The WatchTime needs to be initialized.
   */
  int GetDayOfWeek() const;

  /**
   * @brief Returns the UTC time. (Coordinated Universal Time)
   *
   * @details Regarding struct tm (the return value), please refer to the site :
   * http://www.cplusplus.com/reference/ctime/tm/
   *
   * @SINCE_1_2_32
   * @return The UTC time
   * @pre The WatchTime needs to be initialized.
   */
  struct tm GetUtcTime() const;

  /**
   * @brief Returns the UTC timestamp.
   *
   * @SINCE_1_2_32
   * @return The UTC timestamp
   * @pre The WatchTime needs to be initialized.
   */
  time_t GetUtcTimeStamp() const;

  /**
   * @brief Returns the ID of timezone.
   *
   * @details The timezone ID, according to the IANA(Internet Assigned Numbers Authority)
   * If you want to see more information, please refer to the site :
   * https://en.wikipedia.org/wiki/List_of_tz_database_time_zones/
   *
   * @SINCE_1_2_32
   * @return The ID of timezone
   * @pre The WatchTime needs to be initialized.
   */
  const char* GetTimeZone() const;

  /**
   * @brief Returns the daylight saving time status.
   *
   * @SINCE_1_2_32
   * @return The Daylight Saving Time status
   * @pre The WatchTime needs to be initialized.
   */
  bool GetDaylightSavingTimeStatus() const;

public: // Not intended for application developers
  DALI_INTERNAL WatchTime(void *time_handle);

private:  // Internal Data

  struct Impl;
  Impl* mImpl;
};

/**
 * @}
 */
} // namespace Dali

#endif // __DALI_WATCH_TIME_H__
