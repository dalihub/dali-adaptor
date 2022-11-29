#ifndef DALI_ADAPTOR_ACCESSIBILITY_BITSET_H
#define DALI_ADAPTOR_ACCESSIBILITY_BITSET_H

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
#include <algorithm>
#include <array>
#include <functional>
#include <type_traits>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali::Accessibility
{
template<std::size_t>
class BitSet;

namespace Internal
{
// Number of 32-bit chunks required to hold N bits
template<typename Enum, Enum EnumMax, typename = std::enable_if_t<std::is_enum_v<Enum>>>
inline constexpr std::size_t BitSetSize = (static_cast<std::size_t>(EnumMax) + 31u) / 32u;

/**
 * @brief A writable reference to a single bit.
 *
 * C++ does not offer a built-in mechanism to directly access specific bits in integral types, i.e.
 * @code std::uint32_t x = 0; x[5] = true; @endcode is not possible. The BitSet type uses this proxy
 * class to make such operations possible.
 *
 * @see Accessibility::BitSet
 */
class DALI_ADAPTOR_API BitReference
{
  template<std::size_t>
  friend class Accessibility::BitSet;

public:
  using ElementType = std::uint32_t; ///< Integral type used for storing bits.
  using IndexType   = std::size_t;   ///< Type used for indexing.

  BitReference() = delete;

  /**
   * @brief Assigns a new value to this bit.
   *
   * This operator is used in expressions like @code bitset[i] = true; @endcode
   *
   * @param x The new value.
   */
  BitReference& operator=(bool x)
  {
    if(x)
    {
      *mData |= (1U << mIndex);
    }
    else
    {
      *mData &= ~(1U << mIndex);
    }

    return *this;
  }

  /**
   * @brief Assigns a new value to this bit.
   *
   * This operator is used in expressions like @code bitset[i] = otherBitset[j]; @endcode
   *
   * @param reference A reference to the new value.
   */
  BitReference& operator=(const BitReference& reference)
  {
    return (*this = static_cast<bool>(reference));
  }

  /**
   * @brief Queries the value of this bit.
   *
   * @return true if this bit is set to 1, false if it is set to 0.
   */
  operator bool() const
  {
    return *mData & (1U << mIndex);
  }

private:
  BitReference(ElementType* data, IndexType index)
  : mData{data},
    mIndex{index}
  {
    DALI_ASSERT_DEBUG(data && index < 32);
  }

  ElementType* mData;
  IndexType    mIndex;
};

} // namespace Internal

/**
 * @brief A collection of bits stored in 32-bit chunks for convenient serialization.
 *
 * @tparam N Number of 32-bit chunks (the capacity of this BitSet is 32*N).
 */
template<std::size_t N>
class DALI_ADAPTOR_API BitSet
{
public:
  // Types

  using ReferenceType = Internal::BitReference;     ///< @copybrief Dali::Accessibility::Internal::BitReference
  using ElementType   = ReferenceType::ElementType; ///< @copydoc Dali::Accessibility::Internal::BitReference::ElementType
  using IndexType     = ReferenceType::IndexType;   ///< @copydoc Dali::Accessibility::Internal::BitReference::IndexType
  using ArrayType     = std::array<ElementType, N>; ///< An array of N integers that can store 32*N bits.

  // Constructors

  /**
   * @brief Constructor.
   */
  BitSet() = default;

  BitSet(const BitSet&) = default;

  BitSet(BitSet&&) noexcept = default;

  /**
   * @brief Constructs a new BitSet with all bits inialized with values from the specified array.
   *
   * Equivalent to the pseudocode:
   * @code
   * for(i = 0; i < max; ++i) bits[i] = (array[i / 32] >> (i % 32)) & 0x1;
   * @endcode
   *
   * @param array Array with the initial values.
   */
  explicit BitSet(const ArrayType& array)
  {
    std::copy(array.begin(), array.end(), mData.begin());
  }

  /**
   * @copydoc BitSet(const ArrayType&)
   */
  // Useful for deserializing DBus data that comes in as signed integers
  explicit BitSet(const std::array<std::int32_t, N>& array)
  {
    std::transform(array.begin(), array.end(), mData.begin(), [](std::int32_t x) { return static_cast<ElementType>(x); });
  }

  /**
   * @brief Constructs a new BitSet with all bits initialized with bits from the specified integer.
   *
   * This constructor is only available for BitSets with 32-bit capacity. Equivalent to the pseudocode:
   * @code
   * for(i = 0; i < 32; ++i) bits[i] = (data >> i) & 0x1;
   * @endcode
   *
   * @param data 32-bit integer with the initial values.
   */
  template<std::size_t I = N, typename = std::enable_if_t<(I == N && N == 1u)>>
  explicit BitSet(std::uint32_t data)
  {
    mData[0] = data;
  }

  /**
   * @brief Constructs a new BitSet with all bits initialized with bits from the specified integer.
   *
   * This constructor is only available for BitSets with 64-bit capacity. Equivalent to the pseudocode:
   * @code
   * for(i = 0; i < 64; ++i) bits[i] = (data >> i) & 0x1;
   * @endcode
   *
   * @param data 64-bit integer with the initial values.
   */
  template<std::size_t I = N, typename = std::enable_if_t<(I == N && N == 2u)>>
  explicit BitSet(std::uint64_t data)
  {
    mData[0] = static_cast<ElementType>(data);
    mData[1] = static_cast<ElementType>(data >> 32);
  }

  // Non-virtual destructor

  ~BitSet() noexcept = default;

  // Operators

  BitSet& operator=(const BitSet&) = default;

  BitSet& operator=(BitSet&&) noexcept = default;

  /**
   * @brief Checks whether any bits are set to 1.
   *
   * Equivalent to the pseudocode:
   * @code
   * for(i = 0; i < max; ++i)
   *   if(bits[i] == 1)
   *     return true;
   *  return false;
   * @endcode
   *
   * @return true if at least one bit is set to 1, false otherwise.
   */
  explicit operator bool() const
  {
    return std::any_of(mData.begin(), mData.end(), [](ElementType s) { return s != 0u; });
  }

  /**
   * @brief Compares two bitsets for equality.
   *
   * Equivalent to the pseudocode:
   * @code
   * for(i = 0; i < max; ++i)
   *   if(bits[i] != other.bits[i])
   *     return false;
   * return true;
   * @endcode
   *
   * @param other The other operand.
   * @return true if all bits in the two bitsets have the same values, false otherwise.
   */
  bool operator==(const BitSet& other) const
  {
    return std::equal(mData.begin(), mData.end(), other.mData.begin());
  }

  /**
   * @brief Compares two bitsets for inequality.
   *
   * @param other The other operand.
   * @return false if all bits in the two bitsets have the same values, true otherwise.
   *
   * @see BitSet::operator==(const BitSet&)
   */
  // TODO(C++20): Remove this. Having only operator== will be sufficient.
  bool operator!=(const BitSet& other) const
  {
    return !(*this == other);
  }

  /**
   * @brief Computes the bitwise NOT of this bitset.
   *
   * Equivalent to the pseudocode:
   * @code
   * for(i = 0; i < max; ++i) result.bits[i] = ~bits[i]
   * @endcode
   *
   * @return Result of the bitwise NOT operation.
   */
  BitSet operator~() const
  {
    BitSet result;

    std::transform(mData.begin(), mData.end(), result.mData.begin(), std::bit_not<ElementType>{});

    return result;
  }

  /**
   * @brief Computes the bitwise OR of two bitsets.
   *
   * Equivalent to the pseudocode:
   * @code
   * for(i = 0; i < max; ++i) result.bits[i] = bits[i] | other.bits[i];
   * @endcode
   *
   * @param other The other operand.
   * @return Result of the bitwise OR operation.
   */
  BitSet operator|(const BitSet& other) const
  {
    return ApplyBinaryOperator(other, std::bit_or<ElementType>{});
  }

  /**
   * @brief Computes the bitwise AND of two bitsets.
   *
   * Equivalent to the pseudocode:
   * @code
   * for(i = 0; i < max; ++i) result.bits[i] = bits[i] & other.bits[i];
   * @endcode
   *
   * @param other The other operand.
   * @return Result of the bitwise AND operation.
   */
  BitSet operator&(const BitSet& other) const
  {
    return ApplyBinaryOperator(other, std::bit_and<ElementType>{});
  }

  /**
   * @brief Computes the bitwise XOR of two bitsets.
   *
   * Equivalent to the pseudocode:
   * @code
   * for(i = 0; i < max; ++i) result.bits[i] = bits[i] ^ other.bits[i];
   * @endcode
   *
   * @param other The other operand.
   * @return Result of the bitwise XOR operation.
   */
  BitSet operator^(const BitSet& other) const
  {
    return ApplyBinaryOperator(other, std::bit_xor<ElementType>{});
  }

  /**
   * @brief Queries the value of the specified bit.
   *
   * This operator is used in expressions like @code bool b = bitset[i]; @endcode
   *
   * @param index Index of the bit to query.
   * @return true if the specified bit is set to 1, false if it is set to 0.
   */
  bool operator[](IndexType index) const
  {
    // Reuse implicit BitReference::operator bool
    return const_cast<BitSet*>(this)->operator[](index);
  }

  /**
   * @brief Obtains a writable reference to the specified bit.
   *
   * This operator is used in expressions like @code bitset[i] = true; @endcode
   *
   * @param index Index of the bit to query.
   * @return A writable reference to the specified bit.
   *
   * @see BitSet::ReferenceType
   */
  ReferenceType operator[](IndexType index)
  {
    DALI_ASSERT_ALWAYS(index / 32u < mData.size());

    return {&mData[index / 32u], index % 32u};
  }

  // Other methods

  /**
   * @brief Obtains a copy of the internal storage.
   *
   * @return A copy of the internal storage.
   *
   * @see BitSet::ArrayType
   * @see BitSet::BitSet(const ArrayType&)
   */
  ArrayType GetRawData() const
  {
    return mData;
  }

  /**
   * @brief Obtains a copy of the internal storage serialized as a single integer.
   *
   * This method is only available for BitSets with 32-bit capacity.
   *
   * @return A copy of the internal storage.
   *
   * @see BitSet::BitSet(std::uint32_t)
   */
  template<std::size_t I = N, typename = std::enable_if_t<(I == N && N == 1u)>>
  std::uint32_t GetRawData32() const
  {
    return mData[0];
  }

  /**
   * @brief Obtains a copy of the internal storage serialized as a single integer.
   *
   * This method is only available for BitSets with 64-bit capacity.
   *
   * @return A copy of the internal storage.
   *
   * @see BitSet::BitSet(std::uint64_t)
   */
  template<std::size_t I = N, typename = std::enable_if_t<(I == N && N == 2u)>>
  std::uint64_t GetRawData64() const
  {
    return (static_cast<std::uint64_t>(mData[1]) << 32) | mData[0];
  }

private:
  template<typename BinaryOperator>
  BitSet ApplyBinaryOperator(const BitSet& other, BinaryOperator binaryOperator) const
  {
    BitSet result;

    // Same as the pseudocode:
    // for(i = 0; i < max; ++i) output[i] = input1[i] @ input2[i];
    // (substitute '@' with the desired operator)
    std::transform(
      mData.begin(), mData.end(), // input1
      other.mData.begin(),        // input2
      result.mData.begin(),       // output
      binaryOperator);

    return result;
  }

  ArrayType mData{};
};

/**
 * @brief Helper class for storing enumeration values as a BitSet.
 *
 * The enumeration values will be used as indices to the BitSet, so they should be consecutive integers and start from
 * zero. C++ does not offer a way to query the maximum enumeration value using type traits, hence the additional
 * template parameter.
 *
 * @tparam Enum The enumeration type to use.
 * @tparam EnumMax The maximum value for this enumeration.
 *
 * @see Dali::Accessibility::Accessible::GetStates
 * @see Dali::Accessibility::Accessible::GetRoles
 */
template<typename Enum, Enum EnumMax>
class DALI_ADAPTOR_API EnumBitSet : public BitSet<Internal::BitSetSize<Enum, EnumMax>>
{
  static constexpr std::size_t N = Internal::BitSetSize<Enum, EnumMax>;

public:
  // Types

  using IndexType     = typename BitSet<N>::IndexType;     ///< @copydoc Dali::Accessibility::BitSet::IndexType
  using ReferenceType = typename BitSet<N>::ReferenceType; ///< @copydoc Dali::Accessibility::BitSet::ReferenceType

  // Constructors

  using BitSet<N>::BitSet;

  // Operators

  /**
   * @copydoc Dali::Accessibility::BitSet::operator~() const
   */
  EnumBitSet operator~() const
  {
    return BitSet<N>::operator~();
  }

  /**
   * @copydoc Dali::Accessibility::BitSet::operator|(const BitSet&) const
   */
  EnumBitSet operator|(const EnumBitSet& other) const
  {
    return BitSet<N>::operator|(other);
  }

  /**
   * @copydoc Dali::Accessibility::BitSet::operator&(const BitSet&) const
   */
  EnumBitSet operator&(const EnumBitSet& other) const
  {
    return BitSet<N>::operator&(other);
  }

  /**
   * @copydoc Dali::Accessibility::BitSet::operator^(const BitSet&) const
   */
  EnumBitSet operator^(const EnumBitSet& other) const
  {
    return BitSet<N>::operator^(other);
  }

  /**
   * @copydoc Dali::Accessibility::BitSet::operator[](IndexType) const
   */
  bool operator[](Enum index) const
  {
    return BitSet<N>::operator[](static_cast<IndexType>(index));
  }

  /**
   * @copydoc Dali::Accessibility::BitSet::operator[](IndexType)
   */
  ReferenceType operator[](Enum index)
  {
    return BitSet<N>::operator[](static_cast<IndexType>(index));
  }

private:
  // For operators '~|&^'
  EnumBitSet(BitSet<N>&& bitSet)
  : BitSet<N>(bitSet)
  {
  }

  // No data members (non-virtual destructor)
};

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ACCESSIBILITY_BITSET_H
