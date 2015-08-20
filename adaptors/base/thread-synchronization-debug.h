#ifndef __DALI_INTERNAL_THREAD_SYNCHRONIZATION_DEBUG_H__
#define __DALI_INTERNAL_THREAD_SYNCHRONIZATION_DEBUG_H__

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
#ifdef DEBUG_ENABLED

#define ENABLE_LOG_IN_COLOR 1
#define ENABLE_VSYNC_COUNTER_LOGGING 1
#define ENABLE_UPDATE_COUNTER_LOGGING 1
#define ENABLE_VSYNC_THREAD_LOGGING 1
#define ENABLE_UPDATE_THREAD_LOGGING 1
#define ENABLE_RENDER_THREAD_LOGGING 1
#define ENABLE_EVENT_LOGGING 1

Debug::Filter* gLogFilter = Debug::Filter::New( Debug::NoLogging, true, "LOG_THREAD_SYNC" );

#define LOG_THREAD_SYNC(level, color, format, args...) \
    if( gLogFilter && gLogFilter->IsEnabledFor( level ) ) { Dali::Integration::Log::LogMessage( Dali::Integration::Log::DebugInfo, "%s" format "%s\n", color, ## args, COLOR_CLEAR ); }

#define LOG_THREAD_SYNC_TRACE(color) \
    Dali::Integration::Log::TraceObj debugTraceObj( ( gLogFilter && gLogFilter->IsEnabledFor( Debug::Concise ) ) ? gLogFilter : NULL, "%s%s%s", color, __FUNCTION__, COLOR_CLEAR )

#define LOG_THREAD_SYNC_TRACE_FMT(color, format, args...) \
    Dali::Integration::Log::TraceObj debugTraceObj( ( gLogFilter && gLogFilter->IsEnabledFor( Debug::Concise ) ) ? gLogFilter : NULL, "%s%s: " format "%s", color, __FUNCTION__, ## args, COLOR_CLEAR )

#else // DEBUG_ENABLED

#define LOG_THREAD_SYNC(color, format, args...)
#define LOG_THREAD_SYNC_TRACE(color)
#define LOG_THREAD_SYNC_TRACE_FMT(color, format, args...)

#endif // DEBUG_ENABLED

#ifdef ENABLE_LOG_IN_COLOR
#define COLOR_RED            "\033[31m"
#define COLOR_YELLOW         "\033[33m"
#define COLOR_BLUE           "\033[34m"
#define COLOR_LIGHT_RED      "\033[91m"
#define COLOR_LIGHT_YELLOW   "\033[93m"
#define COLOR_LIGHT_BLUE     "\033[94m"
#define COLOR_WHITE          "\033[97m"
#define COLOR_CLEAR          "\033[0m"
#else
#define COLOR_RED
#define COLOR_YELLOW
#define COLOR_BLUE
#define COLOR_LIGHT_RED
#define COLOR_LIGHT_YELLOW
#define COLOR_LIGHT_BLUE
#define COLOR_WHITE
#define COLOR_CLEAR
#endif

#ifdef ENABLE_VSYNC_COUNTER_LOGGING
#define LOG_VSYNC_COUNTER_VSYNC(format, args...)    LOG_THREAD_SYNC(Debug::Verbose, COLOR_LIGHT_RED, "%s: " format, __FUNCTION__, ## args)
#define LOG_VSYNC_COUNTER_UPDATE(format, args...)   LOG_THREAD_SYNC(Debug::Verbose, COLOR_LIGHT_YELLOW, "%s: " format, __FUNCTION__, ## args)
#else
#define LOG_VSYNC_COUNTER_VSYNC(format, args...)
#define LOG_VSYNC_COUNTER_UPDATE(format, args...)
#endif

#ifdef ENABLE_UPDATE_COUNTER_LOGGING
#define LOG_UPDATE_COUNTER_UPDATE(format, args...)  LOG_THREAD_SYNC(Debug::Verbose, COLOR_YELLOW, "%s: " format, __FUNCTION__, ## args)
#define LOG_UPDATE_COUNTER_RENDER(format, args...)  LOG_THREAD_SYNC(Debug::Verbose, COLOR_LIGHT_BLUE, "%s: " format, __FUNCTION__, ## args)
#else
#define LOG_UPDATE_COUNTER_UPDATE(format, args...)
#define LOG_UPDATE_COUNTER_RENDER(format, args...)
#endif

#ifdef ENABLE_VSYNC_THREAD_LOGGING
#define LOG_VSYNC(format, args...)             LOG_THREAD_SYNC(Debug::General, COLOR_RED, "%s: " format, __FUNCTION__, ## args)
#define LOG_VSYNC_TRACE                        LOG_THREAD_SYNC_TRACE(COLOR_RED)
#define LOG_VSYNC_TRACE_FMT(format, args...)   LOG_THREAD_SYNC_TRACE_FMT(COLOR_RED, format, ## args)
#else
#define LOG_VSYNC(format, args...)
#define LOG_VSYNC_TRACE
#define LOG_VSYNC_TRACE_FMT(format, args...)
#endif

#ifdef ENABLE_UPDATE_THREAD_LOGGING
#define LOG_UPDATE(format, args...)            LOG_THREAD_SYNC(Debug::General, COLOR_YELLOW, "%s: " format, __FUNCTION__, ## args)
#define LOG_UPDATE_TRACE                       LOG_THREAD_SYNC_TRACE(COLOR_YELLOW)
#define LOG_UPDATE_TRACE_FMT(format, args...)  LOG_THREAD_SYNC_TRACE_FMT(COLOR_YELLOW, format, ## args)
#else
#define LOG_UPDATE(format, args...)
#define LOG_UPDATE_TRACE
#define LOG_UPDATE_TRACE_FMT(format, args...)
#endif

#ifdef ENABLE_RENDER_THREAD_LOGGING
#define LOG_RENDER(format, args...)            LOG_THREAD_SYNC(Debug::General, COLOR_BLUE, "%s: " format, __FUNCTION__, ## args)
#define LOG_RENDER_TRACE                       LOG_THREAD_SYNC_TRACE(COLOR_BLUE)
#define LOG_RENDER_TRACE_FMT(format, args...) LOG_THREAD_SYNC_TRACE_FMT(COLOR_BLUE, format, ## args)
#else
#define LOG_RENDER(format, args...)
#define LOG_RENDER_TRACE
#define LOG_RENDER_TRACE_FMT(format, args...)
#endif

#ifdef ENABLE_EVENT_LOGGING
#define LOG_EVENT(format, args...)             LOG_THREAD_SYNC(Debug::Concise, COLOR_WHITE, "%s: " format, __FUNCTION__, ## args)
#define LOG_EVENT_TRACE                        LOG_THREAD_SYNC_TRACE(COLOR_WHITE)
#define LOG_EVENT_TRACE_FMT(format, args...)   LOG_THREAD_SYNC_TRACE_FMT(COLOR_WHITE, format, ## args)
#else
#define LOG_EVENT(format, args...)
#define LOG_EVENT_TRACE
#define LOG_EVENT_TRACE_FMT(format, args...)
#endif
} // unnamed namespace

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_THREAD_SYNCHRONIZATION_DEBUG_H__
