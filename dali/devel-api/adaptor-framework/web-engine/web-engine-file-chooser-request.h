#ifndef DALI_WEB_ENGINE_FILE_CHOOSER_REQUEST_H
#define DALI_WEB_ENGINE_FILE_CHOOSER_REQUEST_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <vector>

namespace Dali
{
/**
 * @brief A class WebEngineFileChooserRequest for file chooser.
 */
class WebEngineFileChooserRequest
{
public:
  /**
   * @brief Constructor.
   */
  WebEngineFileChooserRequest() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineFileChooserRequest() = default;

  /**
   * @brief if it is allowed to select multiple files or not.
   *
   * @return true if it is allowed to select multiple files,
   *         false otherwise
   */
  virtual bool MultipleFilesAllowed() const = 0;

  /**
   * @brief Queries the list of accepted MIME types.
   *
   * Possible MIME types are:
   * - "audio\/\*": All sound files are accepted
   * - "video\/\*": All video files are accepted
   * - "image\/\*": All image files are accepted
   * - standard IANA MIME type (see http://www.iana.org/assignments/media-types/ for a complete list)
   *
   * @return The list of accepted MIME types.
   */
  virtual std::vector<std::string> AcceptedMimetypes() const = 0;

  /**
   * @brief Cancels the file chooser request.
   *
   * @return true if successful, false otherwise
   */
  virtual bool Cancel() = 0;

  /**
   * @brief Chooses the files list.
   *
   * @return true if successful, false otherwise
   */
  virtual bool ChooseFiles(const std::vector<std::string> files) = 0;

  /**
   * @brief Chooses a file.
   *
   * @return true if successful, false otherwise
   */
  virtual bool ChooseFile(const std::string file) = 0;
};
} // namespace Dali

#endif // DALI_WEB_ENGINE_FILE_CHOOSER_REQUEST_H
