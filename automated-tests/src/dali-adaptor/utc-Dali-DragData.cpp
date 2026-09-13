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
#include <dali/public-api/adaptor-framework/drag-data.h>

using namespace Dali;

void utc_dali_drag_data_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_drag_data_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliDragDataDefaultConstructorP(void)
{
  DragAndDrop::DragData data;

  DALI_TEST_EQUALS(data.GetDataCount(), 0u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliDragDataCopyConstructorP(void)
{
  DragAndDrop::DragData original;
  original.AddData("text/plain", "hello");
  original.AddData("text/html", "<p>world</p>");

  DragAndDrop::DragData copy(original);

  DALI_TEST_EQUALS(copy.GetDataCount(), 2u, TEST_LOCATION);
  DALI_TEST_EQUALS(copy.GetMimeType(0), Dali::String("text/plain"), TEST_LOCATION);
  DALI_TEST_EQUALS(copy.GetData(0), Dali::String("hello"), TEST_LOCATION);
  DALI_TEST_EQUALS(copy.GetMimeType(1), Dali::String("text/html"), TEST_LOCATION);
  DALI_TEST_EQUALS(copy.GetData(1), Dali::String("<p>world</p>"), TEST_LOCATION);

  END_TEST;
}

int UtcDaliDragDataMoveConstructorP(void)
{
  DragAndDrop::DragData original;
  original.AddData("text/plain", "move test");

  DragAndDrop::DragData moved(std::move(original));

  DALI_TEST_EQUALS(moved.GetDataCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(moved.GetMimeType(0), Dali::String("text/plain"), TEST_LOCATION);
  DALI_TEST_EQUALS(moved.GetData(0), Dali::String("move test"), TEST_LOCATION);

  END_TEST;
}

int UtcDaliDragDataCopyAssignmentP(void)
{
  DragAndDrop::DragData original;
  original.AddData("image/png", "png-data");

  DragAndDrop::DragData copy;
  copy = original;

  DALI_TEST_EQUALS(copy.GetDataCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(copy.GetMimeType(0), Dali::String("image/png"), TEST_LOCATION);
  DALI_TEST_EQUALS(copy.GetData(0), Dali::String("png-data"), TEST_LOCATION);

  END_TEST;
}

int UtcDaliDragDataMoveAssignmentP(void)
{
  DragAndDrop::DragData original;
  original.AddData("application/json", "{\"key\":\"value\"}");

  DragAndDrop::DragData moved;
  moved = std::move(original);

  DALI_TEST_EQUALS(moved.GetDataCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(moved.GetMimeType(0), Dali::String("application/json"), TEST_LOCATION);
  DALI_TEST_EQUALS(moved.GetData(0), Dali::String("{\"key\":\"value\"}"), TEST_LOCATION);

  END_TEST;
}

int UtcDaliDragDataAddDataP(void)
{
  DragAndDrop::DragData data;

  data.AddData("text/plain", "first");
  DALI_TEST_EQUALS(data.GetDataCount(), 1u, TEST_LOCATION);

  data.AddData("text/html", "second");
  DALI_TEST_EQUALS(data.GetDataCount(), 2u, TEST_LOCATION);

  data.AddData("application/xml", "third");
  DALI_TEST_EQUALS(data.GetDataCount(), 3u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliDragDataAddMultipleMimeTypesP(void)
{
  DragAndDrop::DragData data;

  data.AddData("text/plain;charset=utf-8", "utf8 text");
  data.AddData("text/html;charset=utf-8", "<html>");
  data.AddData("image/png", "binary-png-data");

  DALI_TEST_EQUALS(data.GetDataCount(), 3u, TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetMimeType(0), Dali::String("text/plain;charset=utf-8"), TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetMimeType(1), Dali::String("text/html;charset=utf-8"), TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetMimeType(2), Dali::String("image/png"), TEST_LOCATION);

  END_TEST;
}

int UtcDaliDragDataGetMimeTypeInvalidIndexN(void)
{
  DragAndDrop::DragData data;
  data.AddData("text/plain", "data");

  Dali::String result = data.GetMimeType(100);
  DALI_TEST_EQUALS(result, Dali::String(""), TEST_LOCATION);

  END_TEST;
}

int UtcDaliDragDataGetDataInvalidIndexN(void)
{
  DragAndDrop::DragData data;
  data.AddData("text/plain", "data");

  Dali::String result = data.GetData(100);
  DALI_TEST_EQUALS(result, Dali::String(""), TEST_LOCATION);

  END_TEST;
}

int UtcDaliDragDataEmptyStringsP(void)
{
  DragAndDrop::DragData data;

  data.AddData("", "");
  DALI_TEST_EQUALS(data.GetDataCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetMimeType(0), Dali::String(""), TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetData(0), Dali::String(""), TEST_LOCATION);

  END_TEST;
}

int UtcDaliDragDataLargeDataP(void)
{
  DragAndDrop::DragData data;

  std::string largeDataStr(10000, 'a');
  data.AddData("text/plain", Dali::String(largeDataStr.c_str()));

  DALI_TEST_EQUALS(data.GetDataCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetData(0), Dali::String(largeDataStr.c_str()), TEST_LOCATION);

  END_TEST;
}

int UtcDaliDragDataUnicodeP(void)
{
  DragAndDrop::DragData data;

  data.AddData("text/plain;charset=utf-8", "한글 테스트");
  data.AddData("text/html;charset=utf-8", "<p>日本語</p>");

  DALI_TEST_EQUALS(data.GetDataCount(), 2u, TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetData(0), Dali::String("한글 테스트"), TEST_LOCATION);
  DALI_TEST_EQUALS(data.GetData(1), Dali::String("<p>日本語</p>"), TEST_LOCATION);

  END_TEST;
}

int UtcDaliDragDataSequentialAccessP(void)
{
  DragAndDrop::DragData data;

  const char* mimeTypes[] = {"type1", "type2", "type3"};
  const char* datas[]     = {"data1", "data2", "data3"};

  for(int i = 0; i < 3; ++i)
  {
    data.AddData(mimeTypes[i], datas[i]);
  }

  for(uint32_t i = 0; i < data.GetDataCount(); ++i)
  {
    DALI_TEST_EQUALS(data.GetMimeType(i), Dali::String(mimeTypes[i]), TEST_LOCATION);
    DALI_TEST_EQUALS(data.GetData(i), Dali::String(datas[i]), TEST_LOCATION);
  }

  END_TEST;
}
