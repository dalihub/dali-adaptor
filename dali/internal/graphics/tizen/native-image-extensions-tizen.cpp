
/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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


// CLASS HEADER
#include <dali/internal/graphics/common/native-image-extensions.h>
#include <dali/integration-api/debug.h>

namespace
{
// function pointers assigned in InitializeEglImageKHR
PFN_vkGetInstanceProcAddr get_proc_addr= 0
PFN_vkCreateImageFromNativeBufferTIZEN create_presentable_image = 0;
} // unnamed namespace


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

EglImageExtensions::EglImageExtensions(EglImplementation* eglImpl)
: mEglImplementation(eglImpl),
  mImageKHRInitialized(false),
  mImageKHRInitializeFailed(false)
{
  DALI_ASSERT_ALWAYS( eglImpl && "EGL Implementation not instantiated" );
}

EglImageExtensions::~EglImageExtensions()
{
}

/**
 * Constructor
 */
NativeImageExtensions::NativeImageExtensions(void* vkdevice)
: mvkImage(NULL),
  mvkImageView(NULL),
  mvkSampler(NULL),
  mImageKHRInitialized(false),
  mImageKHRInitializeFailed(false)
{
   mvkDevice = mDevice;
}

/**
 * Destructor
 */
NativeImageExtensions::~NativeImageExtensions()
{

}

// load extension function proc
void  NativeImageExtensions::Initialize()
{

}

// create vkImage
void* NativeImageExtensions::CreateImage(auto tbmsurface)
{

}

// Destroy resource
void  NativeImageExtensions::DestroyImage()
{

}

 // create vkImageView and vkSampler
void  NativeImageExtensions::TargetTexture()
{

}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
