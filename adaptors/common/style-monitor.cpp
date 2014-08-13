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
#include <style-monitor.h>

// INTERNAL INCLUDES
#include <style-monitor-impl.h>

namespace Dali
{

StyleMonitor::StyleMonitor()
{
}

StyleMonitor::StyleMonitor(const StyleMonitor& monitor)
: BaseHandle(monitor)
{
}

StyleMonitor StyleMonitor::StyleMonitor::Get()
{
  return Internal::Adaptor::StyleMonitor::Get();
}

StyleMonitor::~StyleMonitor()
{
}

StyleMonitor StyleMonitor::DownCast( BaseHandle handle )
{
  return StyleMonitor( dynamic_cast<Internal::Adaptor::StyleMonitor*>( handle.GetObjectPtr() ) );
}

std::string StyleMonitor::GetDefaultFontFamily() const
{
  return GetImplementation(*this).GetDefaultFontFamily();
}

float StyleMonitor::GetDefaultFontSize() const
{
  return GetImplementation(*this).GetDefaultFontSize();
}

const std::string& StyleMonitor::GetTheme() const
{
  return GetImplementation(*this).GetTheme();
}

void StyleMonitor::SetTheme(const std::string& themFilePath)
{
  return GetImplementation(*this).SetTheme(themFilePath);
}

StyleMonitor::StyleChangeSignalV2& StyleMonitor::StyleChangeSignal()
{
  return GetImplementation(*this).StyleChangeSignal();
}

StyleMonitor& StyleMonitor::operator=(const StyleMonitor& monitor)
{
  if( *this != monitor )
  {
    BaseHandle::operator=(monitor);
  }
  return *this;
}

StyleMonitor& StyleMonitor::operator=(BaseHandle::NullType* rhs)
{
  DALI_ASSERT_ALWAYS( (rhs == NULL) && "Can only assign NULL pointer to handle");
  Reset();
  return *this;
}

StyleMonitor::StyleMonitor(Internal::Adaptor::StyleMonitor* internal)
: BaseHandle(internal)
{
}

} // namespace Dali
