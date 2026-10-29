/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dali-adaptor-common-testcases.h"

#include <cstring>
#include <exception>
#include <iostream>
#include <string>

namespace
{
struct UnsupportedTest
{
  const char* name;
  const char* reason;
};

constexpr UnsupportedTest WINDOWS_UNSUPPORTED_TESTS[] = {
  {"UtcDaliDownloadImageP", "The Windows curl dependency was built without file:// protocol support."},
  {"UtcDaliGlWindowNew1", "This TC loads the Linux libdali2-adaptor-gl-window-addon.so addon."},
  {"UtcDaliGlWindowNew2", "This TC loads the Linux libdali2-adaptor-gl-window-addon.so addon."},
};

const char* GetUnsupportedReason(const char* name)
{
  for(const auto& unsupported : WINDOWS_UNSUPPORTED_TESTS)
  {
    if(std::strcmp(name, unsupported.name) == 0)
    {
      return unsupported.reason;
    }
  }
  return nullptr;
}

testcase* FindTest(const std::string& name)
{
  for(testcase* test = gDaliAdaptorCommonTests; test->name; ++test)
  {
    if(name == test->name)
    {
      return test;
    }
  }
  return nullptr;
}

int RunTest(testcase& test)
{
  int result = 1;
  if(test.startup)
  {
    test.startup();
  }
  try
  {
    result = test.function();
  }
  catch(const char*)
  {
    result = 1;
  }
  catch(const std::exception& exception)
  {
    std::cerr << "Unhandled exception: " << exception.what() << '\n';
    result = 1;
  }
  catch(...)
  {
    std::cerr << "Unknown unhandled exception\n";
    result = 1;
  }
  if(test.cleanup)
  {
    test.cleanup();
  }
  return result;
}
} // unnamed namespace

int main(int argc, char** argv)
{
  if(argc == 2 && std::strcmp(argv[1], "--list") == 0)
  {
    for(testcase* test = gDaliAdaptorCommonTests; test->name; ++test)
    {
      const char* unsupportedReason = GetUnsupportedReason(test->name);
      if(unsupportedReason)
      {
        std::cout << test->name << "|common,mock,windows-unsupported|" << unsupportedReason << '\n';
      }
      else
      {
        std::cout << test->name << "|common,mock|Existing DALi adaptor common UTC." << '\n';
      }
    }
    return 0;
  }

  std::string testName;
  for(int index = 1; index < argc; ++index)
  {
    if(std::strcmp(argv[index], "--test") == 0 && index + 1 < argc)
    {
      testName = argv[++index];
    }
    else if(std::strcmp(argv[index], "--timeout-ms") == 0 && index + 1 < argc)
    {
      ++index;
    }
    else
    {
      std::cerr << "Unknown or incomplete argument: " << argv[index] << '\n';
      return 2;
    }
  }

  testcase* test = FindTest(testName);
  if(!test)
  {
    std::cerr << "Unknown test: " << testName << '\n';
    return 2;
  }

  std::cout << "[ RUN      ] " << test->name << '\n';
  const int result = RunTest(*test);
  std::cout << (result == 0 ? "[       OK ] " : "[  FAILED  ] ") << test->name << '\n';
  return result;
}
