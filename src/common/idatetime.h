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

#ifndef OC_IDATETIME_H
#define OC_IDATETIME_H

#include "iinc.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // @function 获取dida毫秒数
    // @param    void     -
    // @return   uint64_i - 转毫秒
    uint64_i getdida_msec();
    // @function 获取dida纳秒
    // @param    void     -
    // @return   uint64_i - 纳秒
    uint64_i getdida();

    // @function 获取tts
    // @param    void     -
    // @return   uint64_i - 纳秒
    uint64_i getts();

    // @function 获取tts毫秒
    // @param    void     -
    // @return   uint64_i - 毫秒
    uint64_i getts_msec();

    // @function 获取32位内部时钟
    // @return   32 bit clock
    uint32_i getclock32();

#ifdef __cplusplus
}
#endif

#endif