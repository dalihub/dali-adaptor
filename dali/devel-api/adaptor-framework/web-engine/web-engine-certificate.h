#ifndef DALI_WEB_ENGINE_CERTIFICATE_H
#define DALI_WEB_ENGINE_CERTIFICATE_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <string>

namespace Dali
{
/**
 * @brief A class WebEngineCertificate for certificate of web engine.
 */
class WebEngineCertificate
{
public:
  /**
   * @brief Constructor.
   */
  WebEngineCertificate() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineCertificate() = default;

  /**
   * @brief Allow the site access about certificate error.
   *
   * @param[in] allowed A value to decide policy
   */
  virtual void Allow(bool allowed) = 0;

  /**
   * @brief Returns information whether the certificate comes from main frame.
   *
   * @return true if the certificate comes from main frame, false otherwise
   */
  virtual bool IsFromMainFrame() const = 0;

  /**
   * @brief Query certificate's PEM data.
   *
   * @return A certificate itself in the PEM format.
   */
  virtual std::string GetPem() const = 0;

  /**
   * @brief Query if the context loaded with a given certificate is secure.
   *
   * @return true if the context is secure, false otherwise
   */
  virtual bool IsContextSecure() const = 0;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_CERTIFICATE_H
