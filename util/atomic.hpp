// Copyright 2011 Branan Purvine-Riley and Adam Johnson
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SENSE_UTIL_ATOMIC_HPP
#define SENSE_UTIL_ATOMIC_HPP

#if defined(__GNUC__)
  #define fetchAndIncrementP(var) __sync_fetch_and_add(var, 1)
  #define fetchAndDecrementP(var) __sync_fetch_and_add(var, -1)
  #define compareAndSwapP(var, old_val, new_val) __sync_bool_compare_and_swap(var, old_val, new_val)
  #define compareAndSwapPointerP(var, old_val, new_val) compareAndSwapP(var, old_val, new_val)
#elif defined(_MSC_VER)
  #include <windows.h>
  #define fetchAndIncrementP(var) InterlockedExchangeAdd(&var, 1)
  #define fetchAndDecrementP(var) InterlockedExchangeAdd(&var, -1)
  #define compareAndSwapP(var, old_val, new_val) InterlockedCompareExchange((DWORD*)var, (DWORD)new_val, (DWORD)old_val)==(DWORD)old_val
  #ifdef _M_AMD64
    #define compareAndSwapPointerP(var, old_val, new_val) InterlockedCompareExchange64((QWORD*)var, (QWORD)new_val, (QWORD)old_val)==(QWORD)old_val
  #else
    #define compareAndSwapPointerP(var, old_val, new_val) compareAndSwapP(var, old_val, new_val)
  #endif
#else
#error "Sorry, don't know how to do atomic operations on your compiler"
#endif

#define fetchAndIncrement(var) fetchAndIncrementP(&(var))
#define fetchAndDecrement(var) fetchAndDecrementP(&(var))
#define compareAndSwap(var, old_val, new_val) compareAndSwapP(&(var), old_val, new_val)
#define compareAndSwapPointer(var, old_val, new_val) compareAndSwapPointerP(&(var), old_val, new_val)

#endif // SENSE_UTIL_ATOMIC_HPP
