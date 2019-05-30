#ifndef FEEDBACK_PLUGIN_H
#define FEEDBACK_PLUGIN_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/feedback-plugin.h>

namespace Dali
{

namespace Plugin
{

/**
 * Plays feedback effects for Dali-Toolkit UI Controls.
 */
class DaliFeedback : public Dali::FeedbackPlugin
{

public: // Construction & Destruction

  /**
   * Constructor
   */
  DaliFeedback();

  /**
   * Destructor
   */
  virtual ~DaliFeedback();

public: // FeedbackPlugin overrides

  /**
   * @copydoc Dali::Integration::FeedbackPlugin::PlayHaptic()
   */
  void PlayHaptic( const std::string& filePath );

  /**
   * @copydoc Dali::FeedbackPlugin::PlayHapticMonotone()
   */
  void PlayHapticMonotone( unsigned int duration );

  /**
   * @copydoc Dali::FeedbackPlugin::StopHaptic()
   */
  void StopHaptic();

  /**
   * @copydoc Dali::FeedbackPlugin::PlaySound()
   */
  int PlaySound( const std::string& fileName );

  /**
   * @copydoc Dali::FeedbackPlugin::StopSound()
   */
  void StopSound( int handle );

  /**
   * @copydoc Dali::FeedbackPlugin::PlayFeedbackPattern()
   */
  void PlayFeedbackPattern( int type, int pattern );
};

}  // namespace Plugin

}  // namespace Dali

#endif // FEEDBACK_PLUGIN_H
