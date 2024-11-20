#ifndef DALI_GRAPHICS_VULKAN_HANDLE_H
#define DALI_GRAPHICS_VULKAN_HANDLE_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

namespace Dali::Graphics::Vulkan
{

template<class T>
class Handle
{
public:
  Handle();

  explicit Handle(T* object);

  Handle(const Handle& handle);

  Handle& operator=(const Handle& handle);

  Handle& operator=(Handle&& handle) noexcept;

  Handle(Handle&& handle) noexcept;

  ~Handle();

  operator bool() const;

  T* operator->() const
  {
    return mObject;
  }

  [[nodiscard]] uint32_t GetRefCount() const
  {
    return mObject->GetRefCount();
  }

  T& operator*() const
  {
    return *mObject;
  }

  template<class K>
  Handle<K> StaticCast()
  {
    return Handle<K>(static_cast<K*>(mObject));
  }

  template<class K>
  bool operator==(const Handle<K>& object) const
  {
    return mObject == &*object;
  }

  template<class K>
  bool operator!=(const Handle<K>& object) const
  {
    return !(mObject == &*object);
  }

  template<class K>
  Handle<K> DynamicCast();

  void Reset()
  {
    if(mObject)
    {
      mObject->Release();
      mObject = nullptr;
    }
  }

private:
  T* mObject{nullptr};
};

template<class K, class T>
static Handle<K> VkTypeCast(const Handle<T>& inval)
{
  return Handle<K>(static_cast<K*>(&*inval));
}

template<class T>
Handle<T>::Handle(T* object)
: mObject(object)
{
  if(mObject)
  {
    mObject->Retain();
  }
}

template<class T>
Handle<T>::Handle()
: mObject(nullptr)
{
}

template<class T>
Handle<T>::Handle(const Handle& handle)
{
  mObject = handle.mObject;
  if(mObject)
  {
    mObject->Retain();
  }
}

template<class T>
Handle<T>::Handle(Handle&& handle) noexcept
{
  mObject        = handle.mObject;
  handle.mObject = nullptr;
}

template<class T>
Handle<T>::operator bool() const
{
  return mObject != nullptr;
}

template<class T>
Handle<T>& Handle<T>::operator=(Handle&& handle) noexcept
{
  if(mObject)
  {
    mObject->Release();
  }
  mObject        = handle.mObject;
  handle.mObject = nullptr;
  return *this;
}

template<class T>
Handle<T>& Handle<T>::operator=(const Handle<T>& handle)
{
  if(handle.mObject != this)
  {
    mObject = handle.mObject;
    if(mObject)
    {
      mObject->Retain();
    }
  }
  return *this;
}

template<class T>
Handle<T>::~Handle()
{
  if(mObject)
  {
    mObject->Release();
  }
}

template<class T>
template<class K>
Handle<K> Handle<T>::DynamicCast()
{
  auto val = dynamic_cast<K*>(mObject);
  if(val)
  {
    return Handle<K>(val);
  }
  return Handle<K>();
}

template<typename T, typename... Args>
Handle<T> MakeRef(Args&&... args)
{
  return Handle<T>(new T(std::forward<Args>(args)...));
}

template<typename T, typename... Args>
Handle<T> NewRef(Args&&... args)
{
  return Handle<T>(T::New(std::forward<Args>(args)...));
}

class VkSharedResource // E.g. render pass
{
public:
  VkSharedResource() = default;

  virtual ~VkSharedResource() = default;

  void Release()
  {
    OnRelease(--mRefCount);

    if(mRefCount == 0)
    {
      // orphaned
      if(!Destroy())
      {
        delete this;
      }
    }
  }

  void Retain()
  {
    OnRetain(++mRefCount);
  }

  uint32_t GetRefCount()
  {
    return mRefCount;
  }

  virtual bool Destroy()
  {
    return OnDestroy();
  }

  virtual void OnRetain(uint32_t refcount)
  {
  }

  virtual void OnRelease(uint32_t refcount)
  {
  }

  virtual bool OnDestroy()
  {
    return false;
  }

private:
  std::atomic_uint mRefCount{0u};
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_HANDLE_H
