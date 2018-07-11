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

#ifndef OC_IWORKER_H
#define OC_IWORKER_H

#include "iinc.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define iocworkerid_release(p) \
	{                          \
		if (p != 0)            \
			free(p);           \
	}

	/*##########################################################################################################
	#    time 1970-01-01 00:00::00 Millisecond  #    work array id       #     Sequence ID                 #
	#    42 Bit Range in 69 year                #   5 Bit Range in 31    #    17 Bit Range in 131071           #
	############################################################################################################*/

#define TWEPOCH_LONG 1288834974657L				//42位最大数
#define WORKERIDBITS 5L							//5位Worker ID
#define SEQUENCEBITS 17L						//12位序列号
#define MAXWORKERID -1L ^ (-1L << WORKERIDBITS) // 去除工作ID
#define SEQUENCEMASK 0x1FFFF					//-1L ^ (-1L << SEQUENCEBITS)   // 空出序列号位
#define TIMESTAMPLEFTSHIFT SEQUENCEBITS + WORKERIDBITS
#define WORKERIDSHIFT SEQUENCEBITS

	struct IOCWORKERID_64BIT
	{
		int64_i _work_id;
		int64_i _sequence;
		int64_i _last_timestamp;
	};

//usngiend int id make
/*#########################################
	#    TTS/Clock      #     Sequence ID     #
	#    24 Bit         #     8 Bit = 0xFF    #
	###########################################*/
#define TWEPOCH_INT 16777215
#define SEQUENCEBITS32 8
#define SEQUENCEMASK32 0xFF

	struct IOCWORKERID_32BIT
	{
		uint32_i _sequence;
		uint32_i _last_timestamp;
	};

	typedef struct IOCWORKERID_64BIT iocworkerid_64b;
	typedef struct IOCWORKERID_32BIT iocworkerid;
	//-------------------------------------------------------------------
	//-64 bit
	// @function 创建64位id生成器
	// @param   int64_i-workid       工作组id
	// @return  iocworkerid_64b    64位id生成器对象
	iocworkerid_64b *iocworker_create_64b(const int64_i workid);
	// @function 生成64位id
	// @param    iocworkerid   生成器对象
	// @return   int64_i          ID
	int64_i iocworker_nextid_64b(iocworkerid_64b *iocid);

	//-------------------------------------------------------------------
	//-32 bit
	// @function  32位id生成器
	// @return  iocworkerid        32bit ID生成器对象
	iocworkerid *iocworker_create();

	// @function  生成32位id
	// @param    iocworkerid    32位ID生成器对象2weiwej位ID生成器对象
	// @return   uint32_i          32位ID
	uint32_i iocworker_nextid(iocworkerid *iocid);

#ifdef __cplusplus
}
#endif

#endif