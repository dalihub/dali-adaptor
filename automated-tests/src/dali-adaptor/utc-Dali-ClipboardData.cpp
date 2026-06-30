/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include <dali-test-suite-utils.h>
#include <dali/public-api/adaptor-framework/clipboard-data.h>

#include <utility>

using namespace Dali;

namespace
{
const char* const MIME_TYPE_TEXT_PLAIN = "text/plain;charset=utf-8";
const char* const MIME_TYPE_HTML       = "application/xhtml+xml";
const char* const MOVED_FROM_ASSERT    = "Cannot use a moved-from ClipboardData object";
} // namespace

int UtcDaliClipboardDataDefaultConstructorP(void)
{
  ClipboardData data;

  DALI_TEST_EQUALS(data.GetMimeType(), Dali::String(""), TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetContent(), Dali::String(""), TEST_LOCATION);

  END_TEST;
}

int UtcDaliClipboardDataConstructorP(void)
{
  ClipboardData data(MIME_TYPE_TEXT_PLAIN, "hello clipboard");

  DALI_TEST_EQUALS(data.GetMimeType(), Dali::String(MIME_TYPE_TEXT_PLAIN), TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetContent(), Dali::String("hello clipboard"), TEST_LOCATION);

  END_TEST;
}

int UtcDaliClipboardDataSetGetP(void)
{
  ClipboardData data;

  data.SetMimeType(MIME_TYPE_TEXT_PLAIN);
  data.SetContent("plain text");

  DALI_TEST_EQUALS(data.GetMimeType(), Dali::String(MIME_TYPE_TEXT_PLAIN), TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetContent(), Dali::String("plain text"), TEST_LOCATION);

  data.SetMimeType(MIME_TYPE_HTML);
  data.SetContent("<b>html</b>");

  DALI_TEST_EQUALS(data.GetMimeType(), Dali::String(MIME_TYPE_HTML), TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetContent(), Dali::String("<b>html</b>"), TEST_LOCATION);

  END_TEST;
}

int UtcDaliClipboardDataEmptyAndUnicodeP(void)
{
  ClipboardData data("", "");

  DALI_TEST_EQUALS(data.GetMimeType(), Dali::String(""), TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetContent(), Dali::String(""), TEST_LOCATION);

  data.SetMimeType(MIME_TYPE_TEXT_PLAIN);
  data.SetContent("한글 Clipboard");

  DALI_TEST_EQUALS(data.GetMimeType(), Dali::String(MIME_TYPE_TEXT_PLAIN), TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetContent(), Dali::String("한글 Clipboard"), TEST_LOCATION);

  END_TEST;
}

int UtcDaliClipboardDataCopyConstructorP(void)
{
  ClipboardData original(MIME_TYPE_TEXT_PLAIN, "original");
  ClipboardData copy(original);

  original.SetContent("changed");

  DALI_TEST_EQUALS(copy.GetMimeType(), Dali::String(MIME_TYPE_TEXT_PLAIN), TEST_LOCATION);
  DALI_TEST_EQUALS(copy.GetContent(), Dali::String("original"), TEST_LOCATION);
  DALI_TEST_EQUALS(original.GetContent(), Dali::String("changed"), TEST_LOCATION);

  END_TEST;
}

int UtcDaliClipboardDataCopyAssignmentP(void)
{
  ClipboardData original(MIME_TYPE_TEXT_PLAIN, "assigned");
  ClipboardData copy;

  copy = original;

  original.SetMimeType(MIME_TYPE_HTML);
  original.SetContent("<p>changed</p>");

  DALI_TEST_EQUALS(copy.GetMimeType(), Dali::String(MIME_TYPE_TEXT_PLAIN), TEST_LOCATION);
  DALI_TEST_EQUALS(copy.GetContent(), Dali::String("assigned"), TEST_LOCATION);
  DALI_TEST_EQUALS(original.GetMimeType(), Dali::String(MIME_TYPE_HTML), TEST_LOCATION);
  DALI_TEST_EQUALS(original.GetContent(), Dali::String("<p>changed</p>"), TEST_LOCATION);

  END_TEST;
}

int UtcDaliClipboardDataMoveConstructorP(void)
{
  ClipboardData original(MIME_TYPE_TEXT_PLAIN, "move ctor");
  ClipboardData moved(std::move(original));

  DALI_TEST_EQUALS(moved.GetMimeType(), Dali::String(MIME_TYPE_TEXT_PLAIN), TEST_LOCATION);
  DALI_TEST_EQUALS(moved.GetContent(), Dali::String("move ctor"), TEST_LOCATION);

  END_TEST;
}

int UtcDaliClipboardDataMoveAssignmentP(void)
{
  ClipboardData original(MIME_TYPE_TEXT_PLAIN, "move assign");
  ClipboardData moved;

  moved = std::move(original);

  DALI_TEST_EQUALS(moved.GetMimeType(), Dali::String(MIME_TYPE_TEXT_PLAIN), TEST_LOCATION);
  DALI_TEST_EQUALS(moved.GetContent(), Dali::String("move assign"), TEST_LOCATION);

  END_TEST;
}

int UtcDaliClipboardDataMovedFromGetN(void)
{
  ClipboardData original(MIME_TYPE_TEXT_PLAIN, "content");
  ClipboardData moved(std::move(original));
  (void)moved;

  DALI_TEST_ASSERTION(original.GetMimeType(), MOVED_FROM_ASSERT);

  END_TEST;
}

int UtcDaliClipboardDataMovedFromSetN(void)
{
  ClipboardData original(MIME_TYPE_TEXT_PLAIN, "content");
  ClipboardData moved(std::move(original));
  (void)moved;

  DALI_TEST_ASSERTION(original.SetContent("changed"), MOVED_FROM_ASSERT);

  END_TEST;
}

int UtcDaliClipboardDataCopyFromMovedN(void)
{
  ClipboardData original(MIME_TYPE_TEXT_PLAIN, "content");
  ClipboardData moved(std::move(original));
  (void)moved;

  DALI_TEST_ASSERTION(ClipboardData copy(original), MOVED_FROM_ASSERT);

  END_TEST;
}

int UtcDaliClipboardDataCopyAssignFromMovedN(void)
{
  ClipboardData original(MIME_TYPE_TEXT_PLAIN, "content");
  ClipboardData moved(std::move(original));
  (void)moved;

  ClipboardData copy;

  DALI_TEST_ASSERTION(copy = original, MOVED_FROM_ASSERT);

  END_TEST;
}
