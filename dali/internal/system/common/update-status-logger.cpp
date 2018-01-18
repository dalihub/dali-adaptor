/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/common/update-status-logger.h>

// EXTERNAL INCLUDES
#include <string>
#include <dali/integration-api/core.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/environment-options.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

UpdateStatusLogger::UpdateStatusLogger( const EnvironmentOptions& environmentOptions )
: mStatusLogInterval( environmentOptions.GetUpdateStatusLoggingFrequency() ),
  mStatusLogCount( 0u )
{
}

UpdateStatusLogger::~UpdateStatusLogger()
{
}

void UpdateStatusLogger::Log( unsigned int keepUpdatingStatus )
{
  if ( mStatusLogInterval )
  {
    std::string oss;

    if ( !(++mStatusLogCount % mStatusLogInterval) )
    {
      oss = "UpdateStatusLogging keepUpdating: ";
      oss += (keepUpdatingStatus ? "true":"false");

      if ( keepUpdatingStatus )
      {
        oss += " because: ";
      }

      if ( keepUpdatingStatus & Integration::KeepUpdating::STAGE_KEEP_RENDERING )
      {
        oss += "<Stage::KeepRendering() used> ";
      }

      if ( keepUpdatingStatus & Integration::KeepUpdating::ANIMATIONS_RUNNING )
      {
        oss  +=  "<Animations running> ";
      }

      if ( keepUpdatingStatus & Integration::KeepUpdating::MONITORING_PERFORMANCE )
      {
        oss += "<Monitoring performance> ";
      }

      if ( keepUpdatingStatus & Integration::KeepUpdating::RENDER_TASK_SYNC )
      {
        oss += "<Render task waiting for completion> ";
      }

      DALI_LOG_UPDATE_STATUS( "%s\n", oss.c_str());
    }
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
