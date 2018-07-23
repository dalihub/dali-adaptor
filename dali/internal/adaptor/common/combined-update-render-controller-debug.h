#ifndef __DALI_INTERNAL_COMBINED_UPDATE_RENDER_CONTROLLER_DEBUG_H__
#define __DALI_INTERNAL_COMBINED_UPDATE_RENDER_CONTROLLER_DEBUG_H__

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

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
// Uncomment next line for FULL logging of the ThreadSynchronization class in release mode
//#define RELEASE_BUILD_LOGGING

#ifdef DEBUG_ENABLED

#define ENABLE_LOG_IN_COLOR
#define ENABLE_COUNTER_LOGGING
#define ENABLE_UPDATE_RENDER_THREAD_LOGGING
#define ENABLE_EVENT_LOGGING

#define DEBUG_LEVEL_COUNTER         Debug::Verbose
#define DEBUG_LEVEL_UPDATE_RENDER   Debug::General
#define DEBUG_LEVEL_EVENT           Debug::Concise

Debug::Filter* gLogFilter = Debug::Filter::New( Debug::NoLogging, false, "LOG_THREAD_SYNC" );

#define LOG_THREAD_SYNC(level, color, format, ...) \
  DALI_LOG_INFO( gLogFilter, level, "%s" format "%s\n", color, ## __VA_ARGS__, COLOR_CLEAR )

#define LOG_THREAD_SYNC_TRACE(color) \
  Dali::Integration::Log::TraceObj debugTraceObj( gLogFilter, "%s%s%s", color, __FUNCTION__, COLOR_CLEAR ); \
  if( ! gLogFilter->IsTraceEnabled() ) { LOG_THREAD_SYNC( Debug::Concise, color, "%s", __FUNCTION__ ); }

#define LOG_THREAD_SYNC_TRACE_FMT(color, format, ...) \
  Dali::Integration::Log::TraceObj debugTraceObj( gLogFilter, "%s%s: " format "%s", color, __FUNCTION__, ## __VA_ARGS__, COLOR_CLEAR ); \
  if( ! gLogFilter->IsTraceEnabled() ) { LOG_THREAD_SYNC( Debug::Concise, color, "%s: " format, __FUNCTION__, ## __VA_ARGS__ ); }

#elif defined( RELEASE_BUILD_LOGGING )

#define ENABLE_LOG_IN_COLOR
#define ENABLE_COUNTER_LOGGING
#define ENABLE_UPDATE_RENDER_THREAD_LOGGING
#define ENABLE_EVENT_LOGGING

#define DEBUG_LEVEL_COUNTER     0
#define DEBUG_LEVEL_UPDATE_RENDER      0
#define DEBUG_LEVEL_EVENT       0

#define LOG_THREAD_SYNC(level, color, format, ...) \
  Dali::Integration::Log::LogMessage( Dali::Integration::Log::DebugInfo, "%s" format "%s\n", color, ## __VA_ARGS__, COLOR_CLEAR )

#define LOG_THREAD_SYNC_TRACE(color) \
  Dali::Integration::Log::LogMessage( Dali::Integration::Log::DebugInfo, "%s%s%s\n", color, __FUNCTION__, COLOR_CLEAR )

#define LOG_THREAD_SYNC_TRACE_FMT(color, format, ...) \
  Dali::Integration::Log::LogMessage( Dali::Integration::Log::DebugInfo, "%s%s: " format "%s\n", color, __FUNCTION__, ## __VA_ARGS__, COLOR_CLEAR )

#else

#define LOG_THREAD_SYNC(level, color, format, ...)
#define LOG_THREAD_SYNC_TRACE(color)
#define LOG_THREAD_SYNC_TRACE_FMT(color, format, ...)

#endif // DEBUG_ENABLED

#ifdef ENABLE_LOG_IN_COLOR
#define COLOR_YELLOW         "\033[33m"
#define COLOR_LIGHT_RED      "\033[91m"
#define COLOR_LIGHT_YELLOW   "\033[93m"
#define COLOR_WHITE          "\033[97m"
#define COLOR_CLEAR          "\033[0m"
#else
#define COLOR_YELLOW
#define COLOR_LIGHT_RED
#define COLOR_LIGHT_YELLOW
#define COLOR_WHITE
#define COLOR_CLEAR
#endif

#ifdef ENABLE_COUNTER_LOGGING
#define LOG_COUNTER_EVENT(format, ...)            LOG_THREAD_SYNC(DEBUG_LEVEL_COUNTER, COLOR_LIGHT_RED, "%s: " format, __FUNCTION__, ## __VA_ARGS__)
#define LOG_COUNTER_UPDATE_RENDER(format, ...)    LOG_THREAD_SYNC(DEBUG_LEVEL_COUNTER, COLOR_LIGHT_YELLOW, "%s: " format, __FUNCTION__, ## __VA_ARGS__)
#else
#define LOG_COUNTER_EVENT(format, ...)
#define LOG_COUNTER_UPDATE_RENDER(format, ...)
#endif

#ifdef ENABLE_UPDATE_RENDER_THREAD_LOGGING
#define LOG_UPDATE_RENDER(format, ...)            LOG_THREAD_SYNC(DEBUG_LEVEL_UPDATE_RENDER, COLOR_YELLOW, "%s: " format, __FUNCTION__, ## __VA_ARGS__)
#define LOG_UPDATE_RENDER_TRACE                       LOG_THREAD_SYNC_TRACE(COLOR_YELLOW)
#define LOG_UPDATE_RENDER_TRACE_FMT(format, ...)  LOG_THREAD_SYNC_TRACE_FMT(COLOR_YELLOW, format, ## __VA_ARGS__)
#else
#define LOG_UPDATE_RENDER(format, ...)
#define LOG_UPDATE_RENDER_TRACE
#define LOG_UPDATE_RENDER_TRACE_FMT(format, ...)
#endif

#ifdef ENABLE_EVENT_LOGGING
#define LOG_EVENT(format, ...)             LOG_THREAD_SYNC(DEBUG_LEVEL_EVENT, COLOR_WHITE, "%s: " format, __FUNCTION__, ## __VA_ARGS__)
#define LOG_EVENT_TRACE                        LOG_THREAD_SYNC_TRACE(COLOR_WHITE)
#define LOG_EVENT_TRACE_FMT(format, ...)   LOG_THREAD_SYNC_TRACE_FMT(COLOR_WHITE, format, ## __VA_ARGS__)
#else
#define LOG_EVENT(format, ...)
#define LOG_EVENT_TRACE
#define LOG_EVENT_TRACE_FMT(format, ...)
#endif

} // unnamed namespace

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_COMBINED_UPDATE_RENDER_CONTROLLER_DEBUG_H__
