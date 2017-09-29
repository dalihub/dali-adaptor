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

// CLASS HEADER
#include "widget-impl.h"

// EXTERNAL INCLUDES
#include <system_info.h>
#include <string.h>

#include <dali/integration-api/debug.h>
#include <dali/public-api/math/vector2.h>

// INTERNAL INCLUDES
#include <adaptor.h>

namespace Dali
{
namespace Internal
{

namespace Adaptor
{

namespace
{

static bool IsWidgetFeatureEnabled()
{
  static bool feature = false;
  static bool retrieved = false;
  int ret;

  if(retrieved == true)
    return feature;

  ret = system_info_get_platform_bool("http://tizen.org/feature/shell.appwidget", &feature);
  if(ret != SYSTEM_INFO_ERROR_NONE)
  {
    DALI_LOG_ERROR("failed to get system info\n"); /* LCOV_EXCL_LINE */
    return false; /* LCOV_EXCL_LINE */
  }

  retrieved = true;

  return feature;
}

} // anonymous namespace

Dali::Widget Widget::New( const std::string& id )
{
  if(!IsWidgetFeatureEnabled())
  {
    DALI_LOG_ERROR("not supported");
    Dali::Widget handle(NULL);
    return handle;
  }

  if( id.size() < 1 )
  {
    DALI_LOG_ERROR("class id is NULL");
    Dali::Widget handle(NULL);
    return handle;
  }

  WidgetPtr widget ( new Widget( id ) );
  Dali::Widget handle( widget.Get() );
  return handle;
}

static int OnInit(widget_base_instance_h instance_h, bundle *content, int w, int h, void *classData)
{
  char *id;
  widget_base_context_get_id(instance_h, &id);
  widget_base_class_on_create(instance_h, content, w, h);

  Dali::Window window = Dali::Adaptor::Get().GetWindow();
  Any nativeHandle = window.GetNativeHandle();
  Ecore_Wl_Window * wlWindow = AnyCast<Ecore_Wl_Window*>( nativeHandle );
  widget_base_context_window_bind( instance_h, id, wlWindow);
  window.SetSize( Dali::Window::WindowSize( w, h ) );

  Internal::Adaptor::Widget* widget = static_cast< Internal::Adaptor::Widget* >( classData );
  widget->mCreateSignal.Emit( std::string(id), content, window );

  return 0;
}

static int OnDestroy(widget_base_instance_h instance_h, widget_base_destroy_type_e reason, bundle *content, void *classData)
{
  char *id;
  widget_base_context_get_id(instance_h, &id);

  Dali::Widget::WidgetTerminateType destroyReason = Dali::Widget::WidgetTerminateType::TEMPORARY;

  if(reason == WIDGET_BASE_DESTROY_TYPE_PERMANENT)
  {
    destroyReason = Dali::Widget::WidgetTerminateType::PERMANENT;
  }

  Internal::Adaptor::Widget* widget = static_cast< Internal::Adaptor::Widget* >( classData );
  widget->mTerminateSignal.Emit(std::string(id), content, destroyReason );

  widget_base_class_on_destroy(instance_h, reason, content);

  return 0;
}

static int OnPause(widget_base_instance_h instance_h, void *classData)
{
  char *id;
  widget_base_context_get_id(instance_h, &id);

  widget_base_class_on_pause(instance_h);

  Internal::Adaptor::Widget* widget = static_cast< Internal::Adaptor::Widget* >( classData );
  widget->mPauseSignal.Emit(id);

  return 0;
}

static int OnResume(widget_base_instance_h instance_h, void *classData)
{
  char *id;
  widget_base_context_get_id(instance_h, &id);

  widget_base_class_on_resume(instance_h);

  Internal::Adaptor::Widget* widget = static_cast< Internal::Adaptor::Widget* >( classData );
  widget->mResumeSignal.Emit(id);

  return 0;
}

static int OnResize(widget_base_instance_h instance_h, int w, int h, void *classData)
{
  char *id;
  widget_base_context_get_id(instance_h, &id);

  widget_base_class_on_resize(instance_h, w, h);

  Internal::Adaptor::Widget* widget = static_cast< Internal::Adaptor::Widget* >( classData );
  Dali::Window window = Dali::Adaptor::Get().GetWindow();
  window.SetSize( Dali::Window::WindowSize(w, h) );
  widget->mResizeSignal.Emit(id, window);

  return 0;
}

static int OnUpdate(widget_base_instance_h instance_h, bundle *content, int force, void *classData)
{
  char *id;
  widget_base_context_get_id(instance_h, &id);

  widget_base_class_on_update(instance_h, content, force);

  Internal::Adaptor::Widget* widget = static_cast< Internal::Adaptor::Widget* >( classData );
  widget->mUpdateSignal.Emit(id, content, force);

  return 0;
}

Widget::Widget( const std::string& id )
: mCreateSignal(),
  mTerminateSignal(),
  mPauseSignal(),
  mResumeSignal(),
  mResizeSignal(),
  mUpdateSignal(),
  mClassId(id),
  mSlotDelegate(this)
{
  widget_base_class cls = widget_base_class_get_default();
  cls.ops.create = OnInit;
  cls.ops.destroy = OnDestroy;
  cls.ops.pause = OnPause;
  cls.ops.resume = OnResume;
  cls.ops.resize = OnResize;
  cls.ops.update = OnUpdate;

  widget_base_class_add(cls, mClassId.c_str(), this);
}

Widget::~Widget()
{
}

} // Adaptor

} // Internal

} // Dali
