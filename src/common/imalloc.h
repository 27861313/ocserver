
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

#ifndef OC_IMALLOC_H
#define OC_IMALLOC_H

#define __xstr(s) __cxxstr(s)
#define __cxxstr(s) #s

#if defined(USE_TCMALLOC)
#define IMALLOC_LIB ("tcmalloc-" __xstr(TC_VERSION_MAJOR) "." __xstr(TC_VERSION_MINOR))
#include <google/malloc_extension_c.h>
#include <google/malloc_hook_c.h>
#include <google/tcmalloc.h>
#if (TC_VERSION_MAJOR == 1 && TC_VERSION_MINOR >= 6 || (TC_VERSION_MAJOR > 1))
#define imalloc_size(p) tc_malloc_size(p)
#define imalloc_release() MallocExtension::instance()->ReleaseFreeMemory()
#define imalloc_release_rate(v) MallocExtension::instance()->SetMemoryReleaseRate(v)
#else
#error "Newer version of tcmalloc required"
#endif
#elif defined(USE_JMALLOC)
#define IMALLOC_LIB ("jemalloc-" __xstr(JEMALLOC_VERSION_MAJOR) "." __xstr(JEMALLOC_VERSION_MINOR) "." __xstr(JEMALLOC_VERSION_BUGFIX))
#include <jemalloc/jemalloc.h>
#if (JEMALLOC_VERSION_MAJOR == 2 && JEMALLOC_VERSION_MINOR >= 1) || (JEMALLOC_VERSION_MAJOR > 2)
#define imalloc_size(p) je_malloc_usable_size(p)
#else
#error "Newer version of jemalloc required"
#endif //if j version
#endif // if define USE_TCMALLOC/USE_JMALLOC

#ifndef IMALLOC_LIB
#define IMALLOC_LIB "libc"
#endif

#endif
