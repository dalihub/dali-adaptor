#ifndef DALI_INTERNAL_UPDATE_STATUS_LOGGER_H
#define DALI_INTERNAL_UPDATE_STATUS_LOGGER_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class EnvironmentOptions;

/**
 * This outputs the status of the update as required.
 */
class UpdateStatusLogger
{
public:

  /**
   * Create the update-status-logger.
   * @param[in] environmentOptions environment options
   */
  UpdateStatusLogger( const EnvironmentOptions& environmentOptions );

  /**
   * Non-virtual destructor; UpdateThread is not suitable as a base class.
   */
  ~UpdateStatusLogger();

  /**
   * Optionally output the update thread status.
   * @param[in] keepUpdatingStatus Whether the update-thread requested further updates.
   */
  void Log( unsigned int keepUpdatingStatus );

private: // Data

  unsigned int mStatusLogInterval;   ///< Interval in frames between status debug prints
  unsigned int mStatusLogCount;      ///< Used to count frames between status debug prints
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_UPDATE_STATUS_LOGGER_H
