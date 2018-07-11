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

#ifndef OC_ICONVERT_H
#define OC_ICONVERT_H

#include "iinc.h"

#ifdef __cplusplus
extern "C"
{
#endif

	// @function 2个字节高低转换
	// @param    uint16_i  - 数据
	// @return   uint16_i  - 转换后数据
	uint16_i lendian_u16(uint16_i v);

	// @function 4个字节高低转换
	// @param    uint32_i  - 源数据
	// @return   uint32_i  - 转换后数据
	uint32_i lendian_u32(uint32_i v);

	// @function 浮点数高低转换
	// @param    float  - 源数据
	// @return   float  - 转换后数据
	float lendian_f32(float v);

	// @function void* 转换到 uint32_i
	// @param    void*     - 源数据
	// @return   uint32_i  - 转换后数据
	uint32_i v_touint32(void *poffset);

	// @function void* 转换到 int32_i
	// @param    void*     - 源数据
	// @return   int32_i   - 转换后数据
	int32_i v_toint32(void *poffset);

#if __BYTE_ORDER == __BIG_ENDIAN
#define TOLENDIAN_U16(v) \
	{                    \
		return v;        \
	}
#define TOLENDIAN_U32(v) \
	{                    \
		return v;        \
	}
#define TOLENDIAN_F32(v) \
	{                    \
		return v;        \
	}
#else
#define TOLENDIAN_U16(v)       \
	{                          \
		return LEndian_u16(v); \
	}
#define TOLENDIAN_U32(v)       \
	{                          \
		return LEndian_u32(v); \
	}
#define TOLENDIAN_F32(v)         \
	{                            \
		return ToLEndian_f32(v); \
	}
#endif

#ifdef __cplusplus
}
#endif

#endif