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

// Use image pipeline barriers instead of semaphores between dependent FBOs
//#define ENABLE_FBO_BARRIER 1

#if !defined(ENABLE_FBO_BARRIER)
// If not using barriers, should use semaphore to block start of dependent cmd buf
// to ensure that the rendering doesn't tear
#define ENABLE_FBO_SEMAPHORE 1
#endif
