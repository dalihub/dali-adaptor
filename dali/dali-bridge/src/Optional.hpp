/*
 * Copyright 2017  Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP

/**
 * Minimalistic implementation of standard library std::optional (c++17) for c++11 compiler.
 *
 * After project conversion to C++17 standard, this template class will be deleted and
 * Optional will point to std::optional.
 *
 * Allowed operations (note, to make code simplier, than original, value class must have accessible copy and move constructor):
 *  - constructing empty (valueless) object
 *  - copying Optional (with and without value)
 *  - moving Optional (with and without value)
 *  - querying, if Optional has value (via explicit operator bool), for example:
 *        Optional<int> v = ...;
 *        if (v) ... // if v has value, then do something
 *  - accessing value (via operator *), for example:
 *        Optional<int> v = ...;
 *        auto z = *v; // z now has the same int, as v (copied)
 *        auto &y = *v; // y now has REFERENCE to int inside v, so modifying y modifies v
 */

#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

template < typename A >
class Optional
{
  /// \cond
  union
  {
    A place;
  };
  bool hasValue = false;
  /// \endcond
public:
  /**
   * @brief Empty constructor.
   * Creates empty Optional object, which will be false in boolean context.
   * So:
   *   \code{.cpp}
   *   Optional<int> o;
   *   if (o) printf("1\n");
   *   \endcode
   * won't print 1.
   */
  Optional() {}

  /**
   * @brief Single element constructor, when implicit convertion can be applied.
   *
   * This constructor will be selected, when type of given argument (U) is
   * implicitly convertable to expected type A. In other words following
   * code must be valid:
   *   \code{.cpp}
   *   A foo() {
   *     return U();
   *   }
   *   \endcode
   *
   * @param a value held by Optional object will be initialized from a.
   */
  template < typename U = A, typename std::enable_if<
                                 std::is_convertible< U&&, A >::value &&
                                     std::is_constructible< A, U&& >::value &&
                                     !std::is_same< typename std::decay< U >::type, Optional< A > >::value,
                                 int* >::type = nullptr >
  constexpr Optional( U&& a )
      : place( std::forward< U >( a ) ), hasValue( true )
  {
  }

  /**
   * @brief Single element constructor, when only explicit convertion can be applied.
   *
   * This constructor will be selected, when type of given argument (U) is
   * convertable to expected type A.
   *
   * @param a value held by Optional object will be initialized from a.
   */
  template < typename U = A, typename std::enable_if<
                                 !std::is_convertible< U&&, A >::value &&
                                     std::is_constructible< A, U&& >::value &&
                                     !std::is_same< typename std::decay< U >::type, Optional< A > >::value,
                                 int* >::type = nullptr >
  explicit constexpr Optional( U&& a )
      : place( std::forward< U >( a ) ), hasValue( true )
  {
  }

  /**
   * @brief Copy constructor.
   *
   * @param v Optional value to copy from. Will cause to copy data held by object v,
   * if v has data.
   */
  Optional( const Optional& v ) : hasValue( v.hasValue )
  {
    if( hasValue )
      new( &place ) A( v.place );
  }

  /**
   * @brief Move constructor.
   *
   * @param v Optional value to copy from. Will move data help by v, if any.
   * After construction \code{.cpp} bool(v) \endcode will be false.
   */
  Optional( Optional&& v ) : hasValue( v.hasValue )
  {
    if( hasValue )
      new( &place ) A( std::move( v.place ) );
  }

  /**
   * @brief Destructor.
   */
  ~Optional()
  {
    if( hasValue )
    {
      place.~A();
    }
  }

  /**
   * @brief Explicit bool operator
   *
   * Will return true if and only if object is helding data.
   */
  explicit operator bool() const
  {
    return hasValue;
  }

  /**
   * @brief Accessor (*) operator
   *
   * Will return modifiable reference to held object. Will assert, if not object is held.
   */
  A& operator*()
  {
    assert( hasValue );
    return place;
  }

  /**
   * @brief Accessor (*) const operator
   *
   * Will return const reference to held object. Will assert, if not object is held.
   */
  const A& operator*() const
  {
    assert( hasValue );
    return place;
  }

  /**
   * @brief Accessor (->) operator
   *
   * Will return pointer to held object allowing access to the value's members.
   * Will assert, if not object is held.
   */
  A* operator->()
  {
    assert( hasValue );
    return &place;
  }

  /**
   * @brief Accessor (->) operator
   *
   * Will return pointer to (const) held object allowing access to the value's members.
   * Will assert, if not object is held.
   */
  const A* operator->() const
  {
    assert( hasValue );
    return &place;
  }

  /**
   * @brief Assignment operator
   *
   * Will copy held value from v, if any.
   *
   * @param v Value to copy from
   */
  Optional& operator=( const Optional& v )
  {
    if( this != &v )
    {
      if( hasValue != v.hasValue )
      {
        if( v.hasValue )
          new( &place ) A( v.place );
        else
          place.~A();
        hasValue = v.hasValue;
      }
      else if( hasValue )
      {
        place = v.place;
      }
    }
    return *this;
  }

  /**
   * @brief Assignment move operator
   *
   * Will move held value from v, if any. In all cases v won't held a value
   * after assignment is done.
   *
   * @param v Value to copy from
   */
  Optional& operator=( Optional&& v )
  {
    if( this != &v )
    {
      if( hasValue != v.hasValue )
      {
        if( v.hasValue )
          new( &place ) A( std::move( v.place ) );
        else
          place.~A();
        hasValue = v.hasValue;
      }
      else if( hasValue )
      {
        place = std::move( v.place );
      }
    }
    return *this;
  }

  /**
   * @brief Assignment operator from value of type held.
   *
   * Will initialize held value from given parameter a.
   * Type of the parameter must be the same (barring cv convertions),
   * as the type of the value held.
   *
   * @param a Value to copy from
   */
  template < class U, class = typename std::enable_if<
                          std::is_same< typename std::remove_reference< U >::type, A >::value &&
                          std::is_constructible< A, U >::value &&
                          std::is_assignable< A&, U >::value >::type >
  Optional& operator=( U&& a )
  {
    if( hasValue )
      place = std::forward< U >( a );
    else
    {
      hasValue = true;
      new( &place ) A( std::forward< U >( a ) );
    }
    return *this;
  }
};

#endif
