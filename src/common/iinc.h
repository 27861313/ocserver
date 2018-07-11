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

#ifndef OC_IINCLUDE_H
#define OC_IINCLUDE_H

#undef _MSC_VER

#if !defined NULL
#define NULL 0
#endif

#if !defined CLOCK_PROCESS_CPUTIME_ID
#define CLOCK_PROCESS_CPUTIME_ID 2
#endif

#if !defined CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

#if !defined __OC_BOOLEAN__
#define __OC_BOOLEAN__
typedef unsigned char Boolean;
#if !defined TRUE
#define TRUE 1
#endif
#if !defined FALSE
#define FALSE 0
#endif
#endif

//=====================================================================
// OC 32BIT INTEGER DEFINITION
//=====================================================================
#ifndef __OC_INTEGER_32_BITS__
#define __OC_INTEGER_32_BITS__

#if defined(_WIN64) || defined(WIN64) || defined(__amd64__) ||      \
	defined(__x86_64) || defined(__x86_64__) || defined(_M_IA64) || \
	defined(_M_AMD64)
typedef unsigned int _UINT32;
typedef int _INT32;
#elif defined(_WIN32) || defined(WIN32) || defined(__i386__) || \
	defined(__i386) || defined(_M_X86)
typedef unsigned long _UINT32;
typedef long _INT32;
#elif defined(__MACOS__)
typedef UInt32 _UINT32;
typedef SInt32 _INT32;
#elif defined(__APPLE__) && defined(__MACH__)
#include <sys/types.h>
typedef u_int32_t _UINT32;
typedef int32_t _INT32;
#elif defined(__BEOS__)
#include <sys/inttypes.h>
typedef u_int32_t _UINT32;
typedef int32_t _INT32;
#elif (defined(_MSC_VER) || defined(__BORLANDC__)) && (!defined(__MSDOS__))
typedef unsigned __int32 _UINT32;
typedef __int32 _INT32;
#elif defined(__GNUC__)
#include <stdint.h>
typedef uint32_t _UINT32;
typedef int32_t _INT32;
#else
typedef unsigned int _UINT32;
typedef int _INT32;
#endif
#endif // __OC_INTEGER_32_BITS__

//=====================================================================
// Integer Definition
//=====================================================================
#ifndef __OC_IINT8_DEFINED
#define __OC_IINT8_DEFINED
typedef char int8_i;
#endif

#ifndef __OC_IUINT8_DEFINED
#define __OC_IUINT8_DEFINED
typedef unsigned char uint8_i;
#endif

#ifndef __OC_IUINT16_DEFINED
#define __OC_IUINT16_DEFINED
typedef unsigned short uint16_i;
#endif

#ifndef __OC_IINT16_DEFINED
#define __OC_IINT16_DEFINED
typedef short int16_i;
#endif

#ifndef __OC_IINT32_DEFINED
#define __OC_IINT32_DEFINED
typedef _INT32 int32_i;
#endif

#ifndef __OC_IUINT32_DEFINED
#define __OC_IUINT32_DEFINED
typedef _UINT32 uint32_i;
#endif

#ifndef __OC_IINT64_DEFINED
#define __OC_IINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64 int64_i;
#else
typedef long long int64_i;
#endif
#endif

#ifndef __OC_IUINT64_DEFINED
#define __OC_IUINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int64 uint64_i;
#else
typedef unsigned long long uint64_i;
#endif
#endif

#define OC_MULTITHREADING 1 //多线程模式
//#undef  OC_MULTITHREADING
//#define OC_C_XX_
#undef OC_C_XX_

#define IOC_VERSION_MAJOR 1
#define IOC_VERSION_MINOR 0
#define IOC_VERSION_BUGFIX 0

#define IOC_VERSION "1.0.0"

#ifdef _DEBUG_
#define SYSLOG_ERROR(info)     \
	{                          \
		syslog(LOG_ERR, info); \
		printf(info);          \
		printf("\n");          \
	}
#else
#define SYSLOG_ERROR(info) syslog(LOG_ERR, info);
#endif

#endif