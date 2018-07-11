/*
 * Created on Thu Jun 14 2018
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 mr liang
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef OC_IATOM_H
#define OC_IATOM_H

#ifdef __GNUC__
// Atomic functions in GCC are present from version 4.1.0 on
// http://gcc.gnu.org/onlinedocs/gcc-4.1.0/gcc/Atomic-Builtins.html

// Test for GCC >= 4.1.0
#if (__GNUC__ < 4) ||                              \
    ((__GNUC__ == 4) && ((__GNUC_MINOR__ < 1) ||   \
                         ((__GNUC_MINOR__ == 1) && \
                          (__GNUC_PATCHLEVEL__ < 0))))

#error Atomic built-in functions are only available in GCC in versions >= 4.1.0
#endif // end of check for GCC 4.1.0
/// @brief atomically adds a_count to the variable pointed by a_ptr
/// @return the value that had previously been in memory
#define AtomicAdd(a_ptr, a_count) __sync_fetch_and_add(a_ptr, a_count)

#define AtomicAddFetch(a_ptr, a_count) __sync_add_and_fetch(a_ptr, a_count)

/// @brief atomically substracts a_count from the variable pointed by a_ptr
/// @return the value that had previously been in memory
#define AtomicSub(a_ptr, a_count) __sync_fetch_and_sub(a_ptr, a_count)

#define AtomicSubFetch(a_ptr, a_count) __sync_sub_and_fetch(a_ptr, a_count)

/// @brief Compare And Swap
///        If the current value of *a_ptr is a_oldVal, then write a_newVal into *a_ptr
/// @return true if the comparison is successful and a_newVal was written
#define CAS(a_ptr, a_oldVal, a_newVal) __sync_bool_compare_and_swap(a_ptr, a_oldVal, a_newVal)

/// @brief Compare And Swap
///        If the current value of *a_ptr is a_oldVal, then write a_newVal into *a_ptr
/// @return the contents of *a_ptr before the operation
#define CASVal(a_ptr, a_oldVal, a_newVal) __sync_val_compare_and_swap(a_ptr, a_oldVal, a_newVal)

#define AtomicRelease(a_ptr) __sync_lock_release(a_ptr)

#else
#ifdef __GNUC__
#error Atomic functions such as CAS or AtomicAdd are not defined for your compiler. Please add them in atomic_ops.h
#else
#endif
#endif // __GNUC__

#endif