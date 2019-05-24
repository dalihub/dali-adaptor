#ifndef DALI_INTERNAL_ADAPTOR_PERFORMANCE_SERVER_H
#define DALI_INTERNAL_ADAPTOR_PERFORMANCE_SERVER_H

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

// EXTERNAL INCLDUES
#include <dali/public-api/common/dali-vector.h>
#include <dali/devel-api/threading/mutex.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/frame-time-stats.h>
#include <dali/internal/network/common/network-performance-server.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/internal/system/common/performance-marker.h>
#include <dali/internal/system/common/stat-context-manager.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class EnvironmentOptions;
class StatContext;
/**
 * Concrete implementation of performance interface.
 * Adaptor classes should never include this file, they
 * just need to include the abstract class performance-interface.h
 */
class PerformanceServer : public PerformanceInterface, public StatContextLogInterface
{
public:

  /**
   * @brief Constructor
   * @param[in] adaptorServices adaptor internal services
   * @param[in] environmentOptions environment options
   */
  PerformanceServer( AdaptorInternalServices& adaptorServices,
                     const EnvironmentOptions& environmentOptions );

  /**
   * Destructor
   */
  virtual ~PerformanceServer();

  /**
   * @copydoc PerformanceLogger::AddContext()
   */
  virtual ContextId AddContext( const char* name );

  /**
   * @copydoc PerformanceLogger::RemoveContext()
   */
  virtual void RemoveContext( ContextId contextId );

  /**
   * @copydoc PerformanceInterface::AddMarker( MarkerType markerType )
   */
  virtual void AddMarker( MarkerType markerType );

  /**
   * @copydoc PerformanceLogger::AddMarker( MarkerType markerType, ContextId contextId )
   */
  virtual void AddMarker( MarkerType markerType, ContextId contextId );

  /**
   * @copydoc PerformanceInterface::SetLogging()
   */
  virtual void SetLogging( unsigned int statisticsLogOptions,
                           unsigned int timeStampOutput,
                           unsigned int logFrequency );

  /**
   * @copydoc PerformanceLogger::SetLoggingFrequency()
   */
  virtual void SetLoggingFrequency( unsigned int logFrequency, ContextId contextId );

  /**
   * @copydoc PerformanceLogger::EnableLogging()
   */
  virtual void EnableLogging( bool enable, ContextId contextId );

public: //StatLogInterface

  /**
   * @copydoc StatLogInterface::LogContextStatistics()
   */
  virtual void LogContextStatistics( const char* const text );

private:

  /**
   * @brief log the marker out to kernel/ DALi log
   * @param[in] marker performance marker
   * @param[in] description marker description
   */
  void LogMarker( const PerformanceMarker& marker, const char* const description );

private:

  const EnvironmentOptions& mEnvironmentOptions;          ///< environment options
  TraceInterface& mKernelTrace;                           ///< kernel trace interface
  TraceInterface& mSystemTrace;                           ///< system trace interface
  Dali::Mutex mLogMutex;                                  ///< mutex

#if defined(NETWORK_LOGGING_ENABLED)
  NetworkPerformanceServer mNetworkServer;                ///< network server
  bool mNetworkControlEnabled;                            ///< Whether network control is enabled
#endif

  StatContextManager mStatContextManager;                 ///< Stat context manager
  unsigned int mStatisticsLogBitmask;                     ///< statistics log level
  unsigned int mPerformanceOutputBitmask;                 ///< performance marker output

  bool mLoggingEnabled:1;                                 ///< whether logging update / render to a log is enabled
  bool mLogFunctionInstalled:1;                           ///< whether the log function is installed
};


} // namespace Internal

} // namespace Adaptor

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_PERFORMANCE_SERVER_H
