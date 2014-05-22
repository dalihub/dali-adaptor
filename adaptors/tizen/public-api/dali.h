#ifndef __DALI_H__
#define __DALI_H__

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

#include <dali/public-api/dali-core.h>
#include <dali/public-api/dali-adaptor-capi-internal.h>

// Application / UI Framework adaption

// These defines come from running pkg-config --cflags with the correct package config file

#if defined(DALI_LIVEBOX_PLUGIN)
#include <dali/public-api/adaptor-framework/livebox-plugin.h>
#endif

#if defined(DALI_APPLICATION)
#include <dali/public-api/adaptor-framework/application.h>
#endif

#include <dali/public-api/adaptor-framework/common/accessibility-action-handler.h>
#include <dali/public-api/adaptor-framework/common/accessibility-gesture-handler.h>
#include <dali/public-api/adaptor-framework/common/event-feeder.h>
#include <dali/public-api/adaptor-framework/common/feedback-plugin.h>
#include <dali/public-api/adaptor-framework/common/physical-keyboard.h>
#include <dali/public-api/adaptor-framework/common/tilt-sensor.h>

#endif //__DALI_H__
