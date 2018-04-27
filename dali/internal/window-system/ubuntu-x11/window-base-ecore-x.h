#ifndef DALI_INTERNAL_WINDOWSYSTEM_ECOREX_WINDOW_BASE_ECORE_X_H
#define DALI_INTERNAL_WINDOWSYSTEM_ECOREX_WINDOW_BASE_ECORE_X_H

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
#include <dali/internal/window-system/common/window-base.h>

// EXTERNAL HEADERS
#include <Ecore.h>
#include <Ecore_X.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

class Window;
class WindowRenderSurface;
class WindowRenderSurfaceEcoreX;

/**
 * WindowBaseEcoreX class provides an WindowBase EcoreX implementation.
 */
class WindowBaseEcoreX : public WindowBase
{
public:

  /**
   * @brief Constructor
   */
  WindowBaseEcoreX( Window* window, WindowRenderSurface* windowRenderSurface );

  /**
   * @brief Destructor
   */
  virtual ~WindowBaseEcoreX();

public:

  /**
   * @brief Called when the window property is changed.
   */
  Eina_Bool OnWindowPropertyChanged( void* data, int type, void* event );

  /**
   * Called when the window receives a delete request
   */
  void OnDeleteRequest();

public:

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Initialize()
   */
  virtual void Initialize() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::ShowIndicator()
   */
  virtual void ShowIndicator( Dali::Window::IndicatorVisibleMode visibleMode, Dali::Window::IndicatorBgOpacity opacityMode ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetIndicatorProperties()
   */
  virtual void SetIndicatorProperties( bool isShow, Dali::Window::WindowOrientation lastOrientation ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::IndicatorTypeChanged()
   */
  virtual void IndicatorTypeChanged( IndicatorInterface::Type type ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetClass()
   */
  virtual void SetClass( std::string name, std::string className ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Raise()
   */
  virtual void Raise() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Lower()
   */
  virtual void Lower() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Activate()
   */
  virtual void Activate() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetAvailableOrientations()
   */
  virtual void SetAvailableOrientations( const std::vector< Dali::Window::WindowOrientation >& orientations ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetPreferredOrientation()
   */
  virtual void SetPreferredOrientation( Dali::Window::WindowOrientation orientation ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetAcceptFocus()
   */
  virtual void SetAcceptFocus( bool accept ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Show()
   */
  virtual void Show() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Hide()
   */
  virtual void Hide() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetSupportedAuxiliaryHintCount()
   */
  virtual unsigned int GetSupportedAuxiliaryHintCount() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetSupportedAuxiliaryHint()
   */
  virtual std::string GetSupportedAuxiliaryHint( unsigned int index ) const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::AddAuxiliaryHint()
   */
  virtual unsigned int AddAuxiliaryHint( const std::string& hint, const std::string& value ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::RemoveAuxiliaryHint()
   */
  virtual bool RemoveAuxiliaryHint( unsigned int id ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetAuxiliaryHintValue()
   */
  virtual bool SetAuxiliaryHintValue( unsigned int id, const std::string& value ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetAuxiliaryHintValue()
   */
  virtual std::string GetAuxiliaryHintValue( unsigned int id ) const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetAuxiliaryHintId()
   */
  virtual unsigned int GetAuxiliaryHintId( const std::string& hint ) const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetInputRegion()
   */
  virtual void SetInputRegion( const Rect< int >& inputRegion ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetType()
   */
  virtual void SetType( Dali::Window::Type type ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetNotificationLevel()
   */
  virtual bool SetNotificationLevel( Dali::Window::NotificationLevel::Type level ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetNotificationLevel()
   */
  virtual Dali::Window::NotificationLevel::Type GetNotificationLevel() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetOpaqueState()
   */
  virtual void SetOpaqueState( bool opaque ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetScreenOffMode()
   */
  virtual bool SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetScreenOffMode()
   */
  virtual Dali::Window::ScreenOffMode::Type GetScreenOffMode() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetBrightness()
   */
  virtual bool SetBrightness( int brightness ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetBrightness()
   */
  virtual int GetBrightness() const override;

protected:

  // Undefined
  WindowBaseEcoreX(const WindowBaseEcoreX&) = delete;

  // Undefined
  WindowBaseEcoreX& operator=(const WindowBaseEcoreX& rhs) = delete;

private:

  Dali::Vector< Ecore_Event_Handler* > mEcoreEventHandler;

  Window*                              mWindow;
  WindowRenderSurfaceEcoreX*           mWindowSurface;
  Ecore_X_Window                       mEcoreWindow;

  bool                                 mRotationAppSet:1;
};

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_ECOREX_WINDOW_BASE_ECORE_X_H
