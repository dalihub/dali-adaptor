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

#include <dali/internal/accessibility/bridge/navigation-sorting.h>
#include <test-harness.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
#define CHECK(condition)                                                                                     \
  do                                                                                                         \
  {                                                                                                          \
    if(!(condition))                                                                                         \
    {                                                                                                        \
      throw std::runtime_error(std::string(#condition) + " at line " + std::to_string(__LINE__));              \
    }                                                                                                        \
  } while(false)

using Dali::Bounds;
using Dali::Accessibility::Internal::SortChildrenFromTopLeft;

struct Node
{
  Bounds bounds;
  int    reads{0};
};

Bounds ReadBounds(Node* node)
{
  ++node->reads;
  return node->bounds;
}

int ChangingBounds(void)
{
  // With live reads, this pair compares false at the insertion guard and true
  // on the next call, allowing an unguarded insertion to move before begin().
  Node               content{{0.f, 10.f, 10.f, 10.f}};
  Node               title{{0.f, 20.f, 10.f, 10.f}};
  std::vector<Node*> children{&content, &title};
  CHECK(SortChildrenFromTopLeft(children, [&title](Node* node)
  {
    auto bounds = ReadBounds(node);
    if(node == &title && node->reads > 1)
    {
      bounds.y = -1.76174493e28f;
    }
    return bounds;
  }));
  CHECK((children == std::vector<Node*>{&content, &title}));
  CHECK(content.reads == 1 && title.reads == 1);
  return 0;
}

int HorizontalSnapshot(void)
{
  Node               right{{100.f, 0.f, 10.f, 10.f}};
  Node               left{{0.f, 0.f, 10.f, 10.f}};
  Node               lower{{-20.f, 50.f, 10.f, 10.f}};
  std::vector<Node*> children{&lower, &right, &left};
  CHECK(SortChildrenFromTopLeft(children, [](Node* node)
  {
    auto bounds = ReadBounds(node);
    if(node->reads > 1)
    {
      bounds.x      = std::numeric_limits<float>::quiet_NaN();
      bounds.height = 0.f;
    }
    return bounds;
  }));
  CHECK((children == std::vector<Node*>{&left, &right, &lower}));
  CHECK(left.reads == 1 && right.reads == 1 && lower.reads == 1);
  return 0;
}

int ZeroArea(void)
{
  Node               title{{960.f, -1.76174493e28f, 0.f, 0.f}};
  Node               content{{0.f, 10.f, 10.f, 10.f}};
  Node               zeroWidth{{0.f, 0.f, 0.f, 10.f}};
  Node               zeroHeight{{0.f, 0.f, 10.f, -0.f}};
  std::vector<Node*> children{&title, &content, nullptr, &zeroWidth, &zeroHeight};
  CHECK(SortChildrenFromTopLeft(children, ReadBounds));
  CHECK((children == std::vector<Node*>{&content}));
  CHECK(title.reads == 1 && content.reads == 1);
  children = {&title, &zeroWidth, &zeroHeight};
  CHECK(SortChildrenFromTopLeft(children, ReadBounds));
  CHECK(children.empty());
  return 0;
}

int InvalidBounds(void)
{
  const float invalidValues[] = {std::numeric_limits<float>::quiet_NaN(),
                                 std::numeric_limits<float>::infinity(),
                                 -std::numeric_limits<float>::infinity()};
  const auto checkInvalid = [](Bounds bounds)
  {
    Node               lower{{0.f, 100.f, 10.f, 10.f}};
    Node               empty{{0.f, 0.f, 0.f, 0.f}};
    Node               invalid{bounds};
    Node               upper{{0.f, 0.f, 10.f, 10.f}};
    std::vector<Node*> children{&lower, &empty, &invalid, &upper};
    const auto         original = children;
    CHECK(!SortChildrenFromTopLeft(children, ReadBounds));
    // A bad rectangle must not discard nodes (or their subtrees), including
    // the empty node already observed before the bad rectangle.
    CHECK(children == original);
  };
  for(float value : invalidValues)
  {
    checkInvalid({value, 0.f, 10.f, 10.f});
    checkInvalid({0.f, value, 10.f, 10.f});
    checkInvalid({0.f, 0.f, value, 10.f});
    checkInvalid({0.f, 0.f, 10.f, value});
  }
  checkInvalid({0.f, 0.f, -1.f, 10.f});
  checkInvalid({0.f, 0.f, 10.f, -1.f});
  return 0;
}

int LineGrouping(void)
{
  // Preserve the existing first-item center rule, even for overlapping rows.
  Node               first{{100.f, 0.f, 10.f, 100.f}};
  Node               sameLine{{0.f, 10.f, 10.f, 10.f}};
  Node               nextLine{{50.f, 20.f, 10.f, 100.f}};
  Node               sameNextLine{{10.f, 30.f, 10.f, 20.f}};
  std::vector<Node*> children{&sameNextLine, &nextLine, &sameLine, &first};
  CHECK(SortChildrenFromTopLeft(children, ReadBounds));
  CHECK((children == std::vector<Node*>{&sameLine, &first, &sameNextLine, &nextLine}));
  return 0;
}

int Ties(void)
{
  std::vector<Node>  nodes(128, Node{{0.f, 0.f, 10.f, 10.f}});
  std::vector<Node*> children;
  for(auto& node : nodes)
  {
    children.push_back(&node);
  }
  std::reverse(children.begin(), children.end());
  const auto original = children;
  CHECK(SortChildrenFromTopLeft(children, ReadBounds));
  CHECK(children == original);
  for(const auto& node : nodes)
  {
    CHECK(node.reads == 1);
  }
  return 0;
}

int FreshSnapshot(void)
{
  Node               first{{0.f, 0.f, 10.f, 10.f}};
  Node               second{{0.f, 20.f, 10.f, 10.f}};
  std::vector<Node*> children{&second, &first};
  CHECK(SortChildrenFromTopLeft(children, ReadBounds));
  CHECK(children.front() == &first);
  first.bounds.y = 100.f;
  CHECK(SortChildrenFromTopLeft(children, ReadBounds));
  CHECK(children.front() == &second);
  CHECK(first.reads == 2 && second.reads == 2);
  return 0;
}

int LargeFinite(void)
{
  const float        maximum = std::numeric_limits<float>::max();
  Node               high{{0.f, maximum, 10.f, maximum}};
  Node               low{{0.f, maximum / 2.f, 10.f, maximum}};
  Node               negative{{0.f, -1.76174493e28f, 10.f, 10.f}};
  std::vector<Node*> children{&high, &low, &negative};
  CHECK(SortChildrenFromTopLeft(children, ReadBounds));
  CHECK((children == std::vector<Node*>{&negative, &low, &high}));
  return 0;
}

int Empty(void)
{
  std::vector<Node*> children;
  CHECK(SortChildrenFromTopLeft(children, [](Node*) -> Bounds
  {
    throw std::runtime_error("Empty list must not query bounds");
  }));
  CHECK(children.empty());
  return 0;
}

int GetterFailure(void)
{
  Node               lower{{0.f, 100.f, 10.f, 10.f}};
  Node               upper{{0.f, 0.f, 10.f, 10.f}};
  std::vector<Node*> children{&lower, &upper};
  const auto         original = children;
  bool               caught   = false;
  try
  {
    SortChildrenFromTopLeft(children, [&upper](Node* node)
    {
      if(node == &upper)
      {
        throw std::runtime_error("Bounds query failed");
      }
      return ReadBounds(node);
    });
  }
  catch(const std::runtime_error&)
  {
    caught = true;
  }
  CHECK(caught);
  CHECK(children == original);
  return 0;
}

template<int (*Test)()>
static int RunTest()
{
  try
  {
    return Test();
  }
  catch(const std::exception& error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
} // namespace

int main(int argc, char** argv)
{
  testcase tests[] = {
    {"ChangingBounds", RunTest<ChangingBounds>, nullptr, nullptr},
    {"HorizontalSnapshot", RunTest<HorizontalSnapshot>, nullptr, nullptr},
    {"ZeroArea", RunTest<ZeroArea>, nullptr, nullptr},
    {"InvalidBounds", RunTest<InvalidBounds>, nullptr, nullptr},
    {"LineGrouping", RunTest<LineGrouping>, nullptr, nullptr},
    {"Ties", RunTest<Ties>, nullptr, nullptr},
    {"FreshSnapshot", RunTest<FreshSnapshot>, nullptr, nullptr},
    {"LargeFinite", RunTest<LargeFinite>, nullptr, nullptr},
    {"Empty", RunTest<Empty>, nullptr, nullptr},
    {"GetterFailure", RunTest<GetterFailure>, nullptr, nullptr},
    {nullptr, nullptr, nullptr, nullptr}};
  return TestHarness::RunTests(argc, argv, tests);
}
