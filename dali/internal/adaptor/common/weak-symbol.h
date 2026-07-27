#ifndef DALI_INTERNAL_ADAPTOR_WEAK_SYMBOL_H
#define DALI_INTERNAL_ADAPTOR_WEAK_SYMBOL_H

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
 */

// MSVC/COFF has no direct equivalent of an ELF weak function definition.
// Profiles using MSVC must therefore select exactly one factory implementation.
#if defined(_WIN32)
#define DALI_ADAPTOR_WEAK_SYMBOL
#elif defined(__GNUC__) || defined(__clang__)
#define DALI_ADAPTOR_WEAK_SYMBOL __attribute__((weak))
#else
#define DALI_ADAPTOR_WEAK_SYMBOL
#endif

#endif // DALI_INTERNAL_ADAPTOR_WEAK_SYMBOL_H
