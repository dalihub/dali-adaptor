#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_BASE_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_BASE_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/internal/window-system/common/indicator-interface.h>

// EXTERNAL INCLUDES
#include <string>
#include <vector>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * WindowBase interface
 */
class WindowBase
{
public:

  /**
   * @brief Default constructor
   */
  WindowBase() = default;

  /**
   * @brief Destructor
   */
  virtual ~WindowBase() = default;

public:

  /**
   * Second stage initialization
   */
  virtual void Initialize() = 0;

  /**
   * @copydoc Dali::Window::ShowIndicator()
   */
  virtual void ShowIndicator( Dali::Window::IndicatorVisibleMode visibleMode, Dali::Window::IndicatorBgOpacity opacityMode ) = 0;

  /**
   * Set the indicator properties on the window
   */
  virtual void SetIndicatorProperties( bool isShow, Dali::Window::WindowOrientation lastOrientation ) = 0;

  /**
   * @copydoc Dali::Internal::Adaptor::IndicatorInterface::Observer::IndicatorTypeChanged()
   */
  virtual void IndicatorTypeChanged( IndicatorInterface::Type type ) = 0;

  /**
   * @copydoc Dali::Window::SetClass()
   */
  virtual void SetClass( std::string name, std::string className ) = 0;

  /**
   * @copydoc Dali::Window::Raise()
   */
  virtual void Raise() = 0;

  /**
   * @copydoc Dali::Window::Lower()
   */
  virtual void Lower() = 0;

  /**
   * @copydoc Dali::Window::Activate()
   */
  virtual void Activate() = 0;

  /**
   * @copydoc Dali::Window::SetAvailableOrientations()
   */
  virtual void SetAvailableOrientations( const std::vector< Dali::Window::WindowOrientation >& orientations ) = 0;

  /**
   * @copydoc Dali::Window::SetPreferredOrientation()
   */
  virtual void SetPreferredOrientation( Dali::Window::WindowOrientation orientation ) = 0;

  /**
   * @copydoc Dali::Window::SetAcceptFocus()
   */
  virtual void SetAcceptFocus( bool accept ) = 0;

  /**
   * @copydoc Dali::Window::Show()
   */
  virtual void Show() = 0;

  /**
   * @copydoc Dali::Window::Hide()
   */
  virtual void Hide() = 0;

  /**
   * @copydoc Dali::Window::GetSupportedAuxiliaryHintCount()
   */
  virtual unsigned int GetSupportedAuxiliaryHintCount() const = 0;

  /**
   * @copydoc Dali::Window::GetSupportedAuxiliaryHint()
   */
  virtual std::string GetSupportedAuxiliaryHint( unsigned int index ) const = 0;

  /**
   * @copydoc Dali::Window::AddAuxiliaryHint()
   */
  virtual unsigned int AddAuxiliaryHint( const std::string& hint, const std::string& value ) = 0;

  /**
   * @copydoc Dali::Window::RemoveAuxiliaryHint()
   */
  virtual bool RemoveAuxiliaryHint( unsigned int id ) = 0;

  /**
   * @copydoc Dali::Window::SetAuxiliaryHintValue()
   */
  virtual bool SetAuxiliaryHintValue( unsigned int id, const std::string& value ) = 0;

  /**
   * @copydoc Dali::Window::GetAuxiliaryHintValue()
   */
  virtual std::string GetAuxiliaryHintValue( unsigned int id ) const = 0;

  /**
   * @copydoc Dali::Window::GetAuxiliaryHintId()
   */
  virtual unsigned int GetAuxiliaryHintId( const std::string& hint ) const = 0;

  /**
   * @copydoc Dali::Window::SetInputRegion()
   */
  virtual void SetInputRegion( const Rect< int >& inputRegion ) = 0;

  /**
   * @copydoc Dali::Window::SetType()
   */
  virtual void SetType( Dali::Window::Type type ) = 0;

  /**
   * @copydoc Dali::Window::SetNotificationLevel()
   */
  virtual bool SetNotificationLevel( Dali::Window::NotificationLevel::Type level ) = 0;

  /**
   * @copydoc Dali::Window::GetNotificationLevel()
   */
  virtual Dali::Window::NotificationLevel::Type GetNotificationLevel() const = 0;

  /**
   * @copydoc Dali::Window::SetOpaqueState()
   */
  virtual void SetOpaqueState( bool opaque ) = 0;

  /**
   * @copydoc Dali::Window::SetScreenOffMode()
   */
  virtual bool SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode) = 0;

  /**
   * @copydoc Dali::Window::GetScreenOffMode()
   */
  virtual Dali::Window::ScreenOffMode::Type GetScreenOffMode() const = 0;

  /**
   * @copydoc Dali::Window::SetBrightness()
   */
  virtual bool SetBrightness( int brightness ) = 0;

  /**
   * @copydoc Dali::Window::GetBrightness()
   */
  virtual int GetBrightness() const = 0;

protected:

  // Undefined
  WindowBase(const WindowBase&) = delete;

  // Undefined
  WindowBase& operator=(const WindowBase& rhs) = delete;

};

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_BASE_H
