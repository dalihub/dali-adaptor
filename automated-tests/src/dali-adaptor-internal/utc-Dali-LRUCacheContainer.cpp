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

#include <iostream>
#include <string>

#include <dali-test-suite-utils.h>
#include <dali/internal/text/text-abstraction/plugin/lru-cache-container.h>

using namespace Dali::TextAbstraction::Internal;
using TestLRUCacheIntInt    = LRUCacheContainer<int, int>;
using TestLRUCacheIntString = LRUCacheContainer<int, std::string>;

namespace
{
template<typename K, typename E>
void TestLRUCacheExist(LRUCacheContainer<K, E>& cache, const K& key, bool expectExist, const char* location)
{
  auto iter = cache.Find(key);
  DALI_TEST_EQUALS((iter != cache.End()), expectExist, location);
}

template<typename K, typename E>
void TestLRUCachePop(LRUCacheContainer<K, E>& cache, const E& expectElement, const char* location)
{
  auto popElement = cache.Pop();
  DALI_TEST_EQUALS(popElement, expectElement, location);
}
} // namespace

void utc_dali_internal_lru_cache_container_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_internal_lru_cache_container_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliLRUCacheContainerPushPopTest(void)
{
  TestLRUCacheIntInt cache(3);

  tet_infoline("Test LRUCache Push and Pop");

  DALI_TEST_EQUALS(cache.IsEmpty(), true, TEST_LOCATION);
  DALI_TEST_EQUALS(cache.IsFull(), false, TEST_LOCATION);

  cache.Push(1111, 111);
  DALI_TEST_EQUALS(cache.IsEmpty(), false, TEST_LOCATION);

  cache.Push(2222, 222);
  cache.Push(3333, 333);
  DALI_TEST_EQUALS(cache.IsFull(), true, TEST_LOCATION);

  cache.Push(4444, 444);
  DALI_TEST_EQUALS(cache.IsFull(), true, TEST_LOCATION);

  TestLRUCacheExist(cache, 1111, false, TEST_LOCATION);
  TestLRUCacheExist(cache, 2222, true, TEST_LOCATION);
  TestLRUCacheExist(cache, 3333, true, TEST_LOCATION);
  TestLRUCacheExist(cache, 4444, true, TEST_LOCATION);

  TestLRUCachePop(cache, 222, TEST_LOCATION);
  DALI_TEST_EQUALS(cache.IsFull(), false, TEST_LOCATION);

  TestLRUCachePop(cache, 333, TEST_LOCATION);
  DALI_TEST_EQUALS(cache.IsEmpty(), false, TEST_LOCATION);
  DALI_TEST_EQUALS(cache.IsFull(), false, TEST_LOCATION);

  cache.Push(5555, 555);
  cache.Push(6666, 666);

  // Replace exist key
  cache.Push(5555, 777);
  DALI_TEST_EQUALS(cache.IsFull(), true, TEST_LOCATION);

  // Change element
  DALI_TEST_EQUALS(cache.Get(5555), 777, TEST_LOCATION);
  cache.Get(5555) = 888;
  DALI_TEST_EQUALS(cache.Get(5555), 888, TEST_LOCATION);

  TestLRUCachePop(cache, 444, TEST_LOCATION);

  TestLRUCacheExist(cache, 2222, false, TEST_LOCATION);
  TestLRUCacheExist(cache, 3333, false, TEST_LOCATION);
  TestLRUCacheExist(cache, 4444, false, TEST_LOCATION);

  TestLRUCachePop(cache, 666, TEST_LOCATION);
  TestLRUCachePop(cache, 888, TEST_LOCATION);
  DALI_TEST_EQUALS(cache.IsEmpty(), true, TEST_LOCATION);

  END_TEST;
}

int UtcDaliLRUCacheContainerPushPopTest2(void)
{
  TestLRUCacheIntString cache(3);

  tet_infoline("Test LRUCache Push and Pop 2");

  DALI_TEST_EQUALS(cache.IsEmpty(), true, TEST_LOCATION);
  DALI_TEST_EQUALS(cache.IsFull(), false, TEST_LOCATION);

  cache.Push(1111, "111");
  DALI_TEST_EQUALS(cache.IsEmpty(), false, TEST_LOCATION);

  cache.Push(2222, "222");
  cache.Push(3333, "333");
  DALI_TEST_EQUALS(cache.IsFull(), true, TEST_LOCATION);

  cache.Push(4444, "444");
  DALI_TEST_EQUALS(cache.IsFull(), true, TEST_LOCATION);

  TestLRUCacheExist(cache, 1111, false, TEST_LOCATION);
  TestLRUCacheExist(cache, 2222, true, TEST_LOCATION);
  TestLRUCacheExist(cache, 3333, true, TEST_LOCATION);
  TestLRUCacheExist(cache, 4444, true, TEST_LOCATION);

  TestLRUCachePop(cache, std::string("222"), TEST_LOCATION);
  DALI_TEST_EQUALS(cache.IsFull(), false, TEST_LOCATION);

  TestLRUCachePop(cache, std::string("333"), TEST_LOCATION);
  DALI_TEST_EQUALS(cache.IsEmpty(), false, TEST_LOCATION);
  DALI_TEST_EQUALS(cache.IsFull(), false, TEST_LOCATION);

  cache.Push(5555, "555");
  cache.Push(6666, "666");

  // Replace exist key
  cache.Push(5555, "777");
  DALI_TEST_EQUALS(cache.IsFull(), true, TEST_LOCATION);

  // Change element
  DALI_TEST_EQUALS(cache.Get(5555), "777", TEST_LOCATION);
  cache.Get(5555) = "888";
  DALI_TEST_EQUALS(cache.Get(5555), "888", TEST_LOCATION);

  TestLRUCachePop(cache, std::string("444"), TEST_LOCATION);

  TestLRUCacheExist(cache, 2222, false, TEST_LOCATION);
  TestLRUCacheExist(cache, 3333, false, TEST_LOCATION);
  TestLRUCacheExist(cache, 4444, false, TEST_LOCATION);

  TestLRUCachePop(cache, std::string("666"), TEST_LOCATION);
  TestLRUCachePop(cache, std::string("888"), TEST_LOCATION);
  DALI_TEST_EQUALS(cache.IsEmpty(), true, TEST_LOCATION);

  END_TEST;
}

int UtcDaliLRUCacheContainerPopEmptyNegative(void)
{
  TestLRUCacheIntInt cache(3);

  tet_infoline("Test LRUCache Pop empty");

  try
  {
    cache.Pop();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // Asserted
  }

  END_TEST;
}

int UtcDaliLRUCacheContainerGetInvalidNegative(void)
{
  TestLRUCacheIntInt cache(3);

  tet_infoline("Test LRUCache Get with invalid key");

  cache.Push(111, 1);
  cache.Push(222, 2);
  cache.Push(333, 3);
  cache.Push(444, 4);

  try
  {
    cache.Get(111);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // Asserted
  }

  END_TEST;
}
