#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_LRU_CACHE_CONTAINER_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_LRU_CACHE_CONTAINER_H

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
 */

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <limits> // for std::numeric_limits
#include <unordered_map>

namespace Dali::TextAbstraction::Internal
{
/**
 * @brief Helper class to cache as LRU algirhtm.
 * It implement as double-linked-list with header and footer.
 *
 * HEADER <-> n(LatestId) <-> n <-> ... <-> n(OldestId) <-> FOOTER <-> n(FreeId) <-> n <-> .. <-> n <-> HEADER
 *
 * @note This container cannot control the KeyType and ElementType construct and destruct timming.
 *
 * @todo Could we make "iteration" system here?
 * @todo Could we move it into dali-core/devel-api?
 *
 * @tparam KeyType The type of the key that pairwise with element.
 * @tparam ElementType The type of the data that the container holds
 * @tparam KeyHash The custom hash funcion of KeyType. Default is std::hash
 * @tparam KeyEqual The custom equal function of KeyType. Default is std::equal_to
 */
template<class KeyType, class ElementType, class KeyHash = std::hash<KeyType>, class KeyEqual = std::equal_to<KeyType>>
class LRUCacheContainer
{
public:
  // Constructor
  LRUCacheContainer(std::size_t maxNumberOfCache = std::numeric_limits<std::size_t>::max() - 2u)
  : mCacheMaxSize(maxNumberOfCache)
  {
  }

  // Destructor
  ~LRUCacheContainer() = default;

  LRUCacheContainer(const LRUCacheContainer& rhs) = default;
  LRUCacheContainer(LRUCacheContainer&& rhs)      = default;

  LRUCacheContainer& operator=(const LRUCacheContainer& rhs) = default;
  LRUCacheContainer& operator=(LRUCacheContainer&& rhs) noexcept = default;

public:
  // Public struct area.
  using CacheId          = std::size_t; ///< The id of cached element. It can be used until element poped out.
  using CacheIdContainer = std::unordered_map<KeyType, CacheId, KeyHash, KeyEqual>;

  /**
   * @brief Special CacheId for header.
   */
  static constexpr CacheId CACHE_HEADER_ID = std::numeric_limits<std::size_t>::max();
  /**
   * @brief Special CacheId for footer.
   */
  static constexpr CacheId CACHE_FOOTER_ID = std::numeric_limits<std::size_t>::max() - 1u;

public:
  // iterator.
  /**
   * @brief Iterator of this LRU cache.
   *
   * Add, Get and Clear invalidate the itertator's validation.
   * Erase and Pop validate the iterator.
   *
   * Range based iteration doesn't supported.
   */
  struct iterator
  {
  public:
    iterator(LRUCacheContainer& owner, const CacheId& id)
    : owner(owner),
      id(id)
    {
    }

    // copy constructor & assign
    iterator(const iterator& rhs)
    : owner(rhs.owner),
      id(rhs.id)
    {
    }

    iterator& operator=(const iterator& rhs)
    {
      this->owner = rhs.owner;
      this->id    = rhs.id;
      return *this;
    }

    // Move constructor & assign
    iterator(iterator&& rhs) noexcept
    : owner(rhs.owner),
      id(rhs.id)
    {
    }

    iterator& operator=(iterator&& rhs) noexcept
    {
      this->owner = rhs.owner;
      this->id    = rhs.id;
      return *this;
    }

    // Prefix increment
    iterator& operator++()
    {
      id = owner.mData[id].next;
      return *this;
    }

    // Postfix increment
    iterator operator++(int)
    {
      iterator temp = *this;
      ++(*this);
      return temp;
    }

    // Prefix decrement
    iterator& operator--()
    {
      id = owner.mData[id].prev;
      return *this;
    }

    // Postfix decrement
    iterator operator--(int)
    {
      iterator temp = *this;
      --(*this);
      return temp;
    }

    bool operator==(const iterator& rhs)
    {
      return id == rhs.id && (&owner) == (&rhs.owner);
    }
    bool operator!=(const iterator& rhs)
    {
      return id != rhs.id || (&owner) != (&rhs.owner);
    }

  public:
    LRUCacheContainer& owner; // The reference of owner of this iterator.
    CacheId            id;
  };

public:
  // Public API area.

  /**
   * @brief Push an element into the cache. It will be marked as recent
   * If it is already existed key, it will replace element.
   *
   * @param[in] key The key to push
   * @param[in] element The element to push
   * @warning This method pop oldest elements if the user attempts to push
   * more elements than the maximum size specified in the constructor
   */
  void Push(const KeyType& key, const ElementType& element)
  {
    const auto iter = mCacheId.find(key);

    // If already exist key, just replace element, and return.
    if(iter != mCacheId.end())
    {
      const CacheId id = iter->second;

      // Mark as recently used.
      InternalPop(id);
      InternalInsertAfterHeader(id);

      mData[id].element = element;
      return;
    }

    if(DALI_UNLIKELY(IsFull()))
    {
      // Pop latest element automatically.
      Pop();
    }

    if(DALI_UNLIKELY(mNumberOfElements == mData.size()))
    {
      InternalReserve(mNumberOfElements == 0 ? 1 : (mNumberOfElements << 1));
    }

    ++mNumberOfElements;

    const CacheId id = mFreeId;

    // Mark as recently used.
    InternalPop(id);
    InternalInsertAfterHeader(id);

    // Copy element
    mData[id].element = element;

    // Store cache iterator.
    mData[id].cacheIdIterator = mCacheId.emplace(key, id).first;
  }

  /**
   * @brief Pops an element off the oldest used element.
   * After pop, CacheId relative with this element cannot be used.
   * Access by poped element's CacheId is Undefined Behavior.
   *
   * @return A copy of the element
   * @warning This method asserts if the container is empty
   */
  ElementType Pop()
  {
    DALI_ASSERT_ALWAYS(!IsEmpty() && "Reading from empty container");

    const CacheId id = mOldestId;
    InternalPop(id);
    InternalInsertAfterFooter(id);

    --mNumberOfElements;

    // Erase cache id.
    mCacheId.erase(mData[id].cacheIdIterator);

    return mData[id].element;
  }

  /**
   * @brief Get an element by the key. It will be marked as recent
   *
   * @param[in] key The key of element
   * @return A reference of the element
   * @warning This method asserts if invalid key inputed
   */
  ElementType& Get(const KeyType& key)
  {
    const auto iter = mCacheId.find(key);
    DALI_ASSERT_ALWAYS((iter != mCacheId.end()) && "Try to get invalid key");

    const auto id = iter->second;

    // Mark as recently used.
    InternalPop(id);
    InternalInsertAfterHeader(id);

    return mData[id].element;
  }

  /**
   * @brief Get an key by iterator. It will not be marked as recent
   *
   * @param[in] iter The iterator of element
   * @return A reference of the key
   * @warning This method don't check iterator validation
   */
  const KeyType& GetKey(iterator iter)
  {
    const auto id = iter.id;

    return mData[id].cacheIdIterator->first;
  }

  /**
   * @brief Get an element by iterator. It will not be marked as recent
   *
   * @param[in] iter The iterator of element
   * @return A reference of the element
   * @warning This method don't check iterator validation
   */
  ElementType& GetElement(iterator iter)
  {
    const auto id = iter.id;

    return mData[id].element;
  }

  /**
   * @brief Find an element by the key. It will not be marked as recent
   *
   * @param[in] key The key of element
   * @return A iterator of cache node. If key not exist, return End()
   */
  iterator Find(const KeyType& key)
  {
    if(mCacheId.find(key) == mCacheId.end())
    {
      return End();
    }

    const auto id = mCacheId[key];

    return iterator(*this, id);
  }

  /**
   * @brief Clear all data
   */
  void Clear()
  {
    mCacheId.clear();
    mCacheId.rehash(0);
    mData.clear();
    mData.shrink_to_fit();

    mNumberOfElements = 0;
    mLatestId         = CACHE_FOOTER_ID;
    mOldestId         = CACHE_HEADER_ID;
    mFreeId           = CACHE_HEADER_ID;
  }

  /**
   * @brief Predicate to determine if the container is empty
   *
   * @return true if the container is empty
   */
  bool IsEmpty() const
  {
    return mNumberOfElements == 0;
  }

  /**
   * @brief Predicate to determine if the container is full
   *
   * @return true if the container is full
   */
  bool IsFull() const
  {
    return (mNumberOfElements == mCacheMaxSize);
  }

  iterator Begin()
  {
    return iterator(*this, mLatestId);
  }

  iterator End()
  {
    return iterator(*this, CACHE_FOOTER_ID);
  }

  /**
   * @brief Remove cache item by iterator.
   *
   * @param[in] iter The iterator what we want to remove.
   * @return iterator The next iterator after remove
   * @warning This method don't check iterator validation
   */
  iterator Erase(iterator iter)
  {
    const auto id     = iter.id;
    const auto nextId = mData[id].next;
    InternalPop(id);
    InternalInsertAfterFooter(id);

    --mNumberOfElements;

    // Erase cache id.
    mCacheId.erase(mData[id].cacheIdIterator);

    return iterator(*this, nextId);
  }

  /**
   * @brief Get a count of the elements in the container
   *
   * @return the number of elements in the container.
   */
  std::size_t Count() const
  {
    return mNumberOfElements;
  }

private:
  // Private API area.

  /**
   * @brief Allocate cache memory as reserveSize.
   * @note We assume that mFreeId is header.
   *
   * @param reserveSize Reserved size of cache.
   */
  void InternalReserve(std::size_t reserveSize) noexcept
  {
    // Increase mData capacity
    if(reserveSize > mCacheMaxSize)
    {
      reserveSize = mCacheMaxSize;
    }

    CacheId newCreatedIdBegin = mData.size();
    CacheId newCreatedIdEnd   = reserveSize - 1;

    // Make temporary array for re-validate iterator.
    std::vector<KeyType> keyList(mData.size());
    for(auto i = static_cast<std::size_t>(0); i < newCreatedIdBegin; ++i)
    {
      keyList[i] = mData[i].cacheIdIterator->first;
    }

    // Reserve data and cacheid capacity.
    mData.resize(reserveSize);
    mCacheId.rehash(reserveSize);

    // Revalidate each iterator.
    for(auto i = static_cast<std::size_t>(0); i < newCreatedIdBegin; ++i)
    {
      mData[i].cacheIdIterator = mCacheId.find(keyList[i]);
    }

    // Setup new created CacheNode's prev and next id.
    for(auto i = newCreatedIdBegin;; ++i)
    {
      mData[i].prev = DALI_UNLIKELY(i == newCreatedIdBegin) ? CACHE_FOOTER_ID : i - 1;
      mData[i].next = DALI_UNLIKELY(i == newCreatedIdEnd) ? CACHE_HEADER_ID : i + 1;
      if(DALI_UNLIKELY(i == newCreatedIdEnd))
      {
        break;
      }
    }
    mFreeId = newCreatedIdBegin;
  }

  /**
   * @brief Temperary pop of node. After call this, we should call
   * InternalInsertAfterHeader or InternalInsertAfterFooter
   *
   * @param id CacheId that removed temperary.
   */
  void InternalPop(const CacheId& id) noexcept
  {
    const CacheId prev = mData[id].prev;
    const CacheId next = mData[id].next;

    // Disconnect prev -> id. and connect prev -> next
    if(prev == CACHE_HEADER_ID)
    {
      mLatestId = next;
    }
    else if(prev == CACHE_FOOTER_ID)
    {
      mFreeId = next;
    }
    else
    {
      mData[prev].next = next;
    }

    // Disconnect id <- next. and connect prev <- next
    if(next == CACHE_HEADER_ID)
    {
      // Do nothing.
    }
    else if(next == CACHE_FOOTER_ID)
    {
      mOldestId = prev;
    }
    else
    {
      mData[next].prev = prev;
    }
  }

  /**
   * @brief Insert the node after the header. That mean, this id recently used.
   *
   * @param id CacheId that insert after header.
   */
  void InternalInsertAfterHeader(const CacheId& id) noexcept
  {
    const CacheId next = mLatestId;

    // Connect Header -> id.
    mLatestId = id;

    // Connect id <- next
    if(next == CACHE_FOOTER_ID)
    {
      mOldestId = id;
    }
    else
    {
      mData[next].prev = id;
    }

    // Connect Header <- id -> next
    mData[id].prev = CACHE_HEADER_ID;
    mData[id].next = next;
  }

  /**
   * @brief Insert the node after the footer. That mean, this id become free.
   *
   * @param id CacheId that insert after footer.
   */
  void InternalInsertAfterFooter(const CacheId& id) noexcept
  {
    const CacheId next = mFreeId;

    // Connect Footer -> id.
    mFreeId = id;

    // Connect id <- next
    if(next == CACHE_HEADER_ID)
    {
      // Do nothing.
    }
    else
    {
      mData[next].prev = id;
    }

    // Connect Footer <- id -> next
    mData[id].prev = CACHE_FOOTER_ID;
    mData[id].next = next;
  }

private:
  // Private struct area.
  /**
   * @brief Double linked CacheNode that this container used.
   */
  struct CacheNode
  {
    CacheNode()  = default;
    ~CacheNode() = default;

    CacheId     prev{CACHE_FOOTER_ID};
    CacheId     next{CACHE_HEADER_ID};
    ElementType element{};

    using CacheIdIterator = typename CacheIdContainer::iterator;

    CacheIdIterator cacheIdIterator; ///< Note : It only validate until mCacheId rehashing.
  };

private:
  // Private member value area.
  std::size_t mCacheMaxSize{0};     ///< The maximum capacity of cache.
  std::size_t mNumberOfElements{0}; ///< The number of elements.

  CacheId mLatestId{CACHE_FOOTER_ID}; ///< The recently used element id
  CacheId mOldestId{CACHE_HEADER_ID}; ///< The oldest used element id
  CacheId mFreeId{CACHE_HEADER_ID};   ///< The free element id that can be allocated.

  std::unordered_map<KeyType, CacheId, KeyHash, KeyEqual> mCacheId{}; ///< LRU Cache id container
  std::vector<CacheNode>                                  mData{};    ///< The real data container.
};

} // namespace Dali::TextAbstraction::Internal

#endif //DALI_TEXT_ABSTRACTION_INTERNAL_LRU_CACHE_CONTAINER_H
