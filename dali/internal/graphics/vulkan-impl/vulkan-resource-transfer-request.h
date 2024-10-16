#pragma once

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

#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/graphics/vulkan/vulkan-hpp-wrapper.h>

namespace Dali::Graphics::Vulkan
{
class BufferImpl;
class Image;

enum class TransferRequestType
{
  BUFFER_TO_IMAGE,
  IMAGE_TO_IMAGE,
  BUFFER_TO_BUFFER,
  IMAGE_TO_BUFFER,
  USE_TBM_SURFACE,
  LAYOUT_TRANSITION_ONLY,
  UNDEFINED
};

/**
 * Structure describing blitting request Buffer to Image
 */
struct ResourceTransferRequest
{
  ResourceTransferRequest(TransferRequestType type)
  : requestType(type)
  {
  }

  TransferRequestType requestType;

  struct
  {
    BufferImpl*         srcBuffer{nullptr}; /// Source buffer
    Image*              dstImage{nullptr};  /// Destination image
    vk::BufferImageCopy copyInfo{};         /// Vulkan specific copy info
  } bufferToImageInfo;

  struct
  {
    Image*        srcImage{nullptr}; /// Source image
    Image*        dstImage{nullptr}; /// Destination image
    vk::ImageCopy copyInfo{};        /// Vulkan specific copy info
  } imageToImageInfo;

  struct
  {
    Image*          image;
    vk::ImageLayout srcLayout;
    vk::ImageLayout dstLayout;
  } imageLayoutTransitionInfo;

  struct
  {
    Image* srcImage{nullptr}; /// Source image
  } useTBMSurfaceInfo;

  bool deferredTransferMode{true}; // Vulkan implementation prefers deferred mode

  // delete copy
  ResourceTransferRequest(const ResourceTransferRequest&)            = delete;
  ResourceTransferRequest& operator=(const ResourceTransferRequest&) = delete;
  ResourceTransferRequest& operator=(ResourceTransferRequest&& obj)  = delete;

  ResourceTransferRequest(ResourceTransferRequest&& obj)
  {
    requestType          = obj.requestType;
    deferredTransferMode = obj.deferredTransferMode;

    if(requestType == TransferRequestType::BUFFER_TO_IMAGE)
    {
      bufferToImageInfo.srcBuffer = obj.bufferToImageInfo.srcBuffer;
      bufferToImageInfo.dstImage  = obj.bufferToImageInfo.dstImage;
      bufferToImageInfo.copyInfo  = obj.bufferToImageInfo.copyInfo;
    }
    else if(requestType == TransferRequestType::IMAGE_TO_IMAGE)
    {
      imageToImageInfo.srcImage = obj.imageToImageInfo.srcImage;
      imageToImageInfo.dstImage = obj.imageToImageInfo.dstImage;
      imageToImageInfo.copyInfo = obj.imageToImageInfo.copyInfo;
    }
  }
};

} // namespace Dali::Graphics::Vulkan
