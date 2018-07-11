
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

#ifndef OC_IRINGBUFFER_H
#define OC_IRINGBUFFER_H

#include "iinc.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef OC_MULTITHREADING

	typedef struct IOCRINGBUFFER_POS iocringbuffer_pos;
	typedef union IOCRINGBUFFER_POSVAL iocringbuffer_posval;

	struct IOCRINGBUFFER_POS
	{
		int32_i _start; // 读
		int32_i _end;   // 写
	};

	union IOCRINGBUFFER_POSVAL {
		int64_i _val;
		iocringbuffer_pos _pos;
	};

	struct IOCRINGBUFFER
	{
		char *_buffer;
		int32_i _length;
		iocringbuffer_posval _posv;
	};
#else
	struct IOCRINGBUFFER
	{
			char *_buffer;
			int32_i _length;
			int32_i _start;
			int32_i _end;
	};
#endif

	typedef struct IOCRINGBUFFER iocringbuffer;

	// @function 创建一个环形缓冲区
	// @param   int32_i          缓冲区字节数
	// @return  iocringbuffer    缓冲区对象
	iocringbuffer *iocringbuffer_create(int32_i length);

	void iocringbuffer_clean(iocringbuffer *buffe);

	// @function 销毁一个环形缓冲区
	// @param   iocringbuffer     缓冲区对象
	void iocringbuffer_release(iocringbuffer *buffer);

	// @function 读取缓冲区数据
	// @param   iocringbuffer     缓冲区对象
	// @param   char*             out
	// @param   int32_i           读取字节数
	// @return  int32_i           实际读取字节数
	int32_i iocringbuffer_read(iocringbuffer *buffer, char *target, int32_i amount);

	// @function 写数据到缓冲区
	// @param   iocringbuffer     缓冲区对象
	// @param   char*             需要写的数据
	// @param   int32_i           写入字节数
	// @param   still             false�ռ䲻�㲻��д�룬true�ռ䲻����Ȼд
	// @return  int32_i           实际写入字节数
	int32_i iocringbuffer_write(iocringbuffer *buffer, char *data, int32_i length, Boolean still);

	// @function 读取字符串
	// @param   iocringbuffer     缓冲区对象
	// @param   int32_i           读取字节数
	// @param   char*             out
	// @return  int32_i           实际读取字节数
	int32_i iocringbuffer_gets(iocringbuffer *buffer, int32_i amount, char *targetchar);

	// @function 读取 short
	// @param   iocringbuffer     缓冲区对象
	// @param   int16_i*          out
	// @return  int32_i           实际读取字节数
	int32_i iocringbuffer_getint16(iocringbuffer *buffer, int16_i *target);

	// @function 读取 ushort
	// @param   iocringbuffer     缓冲区对象
	// @param   uint16_i*         out
	// @return  int32_i           实际读取字节数
	int32_i iocringbuffer_getuint16(iocringbuffer *buffer, uint16_i *target);

	// @function 读取 int32
	// @param   iocringbuffer     缓冲区对象
	// @param   int32_i*          out
	// @return  int32_i           实际读取字节数
	int32_i iocringbuffer_getint32(iocringbuffer *buffer, int32_i *target);

	// @function 读取 uint32
	// @param   iocringbuffer     缓冲区对象
	// @param   uint32_i*         out
	// @return  int32_i           实际读取字节数
	int32_i iocringbuffer_getuint32(iocringbuffer *buffer, uint32_i *target);

	// @function 读取 int64
	// @param   iocringbuffer     缓冲区对象
	// @param   char*             out
	// @return  int32_i           实际读取字节数
	int32_i iocringbuffer_getint64(iocringbuffer *buffer, int64_i *target);

	// @function 读取 uint64
	// @param   iocringbuffer     缓冲区对象
	// @param   char*             out
	// @return  int32_i           实际读取字节数
	int32_i iocringbuffer_getuint64(iocringbuffer *buffer, uint64_i *target);

	// @function 缓冲区是否为空
	// @param   iocringbuffer     缓冲区对象
	// @return  int32_i           0. 非空 1. 空
	int32_i iocringbuffer_empty(iocringbuffer *buffer);

	// @function 缓冲区是否满
	// @param   iocringbuffer     缓冲区对象
	// @return  int32_i           0.非满  1.满
	int32_i iocringbuffer_full(iocringbuffer *buffer);

	// @function 减少读出字节数
	// @param   iocringbuffer     缓冲区对象
	// @return  int32_i           字节数
	void iocringbuffer_readbyted(iocringbuffer *buffer, uint32_i size);

	// @function 添加新写入字数标记
	// @param   iocringbuffer     缓冲区对象
	// @param   uint32_i          新写入字数
	// @return  void              
	void iocringbuffer_writebyted(iocringbuffer *buffer, uint32_i size);

	// @function 已经写入的字节数
	// @param   iocringbuffer     缓冲区对象
	// @return  uint32_i          字节数             
	uint32_i iocringbuffer_bytes(iocringbuffer *buffer);

	// @function 读取剩余空间
	// @param   iocringbuffer     缓冲区对象
	// @return  int32_i           剩余空间字节数
	uint32_i iocringbuffer_remainbytes(iocringbuffer *buffer);

	// @function 获取写位置
	// @param   iocringbuffer     缓冲区对象
	// @return  int32_i           剩余空间字节数
	char*  iocringbuffer_getwritepos(iocringbuffer *buffer);

	// @function 获取读位置
	// @param   iocringbuffer     缓冲区对象
	// @return  int32_i           剩余空间字节数
	char*  iocringbuffer_getreadpos(iocringbuffer *buffer);




#ifdef OC_MULTITHREADING

	int32_i iocringbuffer_available_data(iocringbuffer_posval *pos, int32_i length);

	int32_i iocringbuffer_available_space(iocringbuffer_posval *pos, int32_i length);

#define iocringbuffer_full(B) (iocringbuffer_available_space(B, B->_length) == 0)

#define iocringbuffer_empty(B) (iocringbuffer_available_data((B), B->_length) == 0)

#define iocringbuffer_available_data(B, A) ( \
	(B)->_pos._end % A - (B)->_pos._start)

#define iocringbuffer_available_space(B, A) ( \
	A - (B)->_pos._end - 1)

#define iocringbuffer_will_read(B, A, C) ( \
	(B)->_pos._start = (A) > ((C) - ((B)->_pos._start)) ? (C) : ((B)->_pos._start + A))

#define iocringbuffer_will_write(B, A, C) ( \
	(B)->_pos._end = (A) > ((C)-1 - ((B)->_pos._end)) ? (B)->_pos._end + ((C)-1 - ((B)->_pos._end)) : ((B)->_pos._end + (A)))

#define iocringbuffer_starts_at(B, A) ( \
	(B)->_buffer + (A)->_pos._start)

#define iocringbuffer_ends_at(B, A) ( \
	(B)->_buffer + (A)->_pos._end)
#else

int32_i iocringbuffer_available_data(iocringbuffer *buffer);

int32_i iocringbuffer_available_space(iocringbuffer *buffer);

#define iocringbuffer_available_data(B) ( \
	(B)->_end % (B)->_length - (B)->_start)

#define iocringbuffer_available_space(B) ( \
	(B)->_length - (B)->_end - 1)

#define iocringbuffer_full(B) (iocringbuffer_available_space(B) == 0)

#define iocringbuffer_empty(B) (iocringbuffer_available_data((B)) == 0)

#define iocringbuffer_starts_at(B) ( \
	(B)->_buffer + (B)->_start)

#define iocringbuffer_ends_at(B) ( \
	(B)->_buffer + (B)->_end)

#define iocringbuffer_commit_read(B, A) ( \
	(B)->_start = ((B)->_start + (A)) % (B)->_length)

#define iocringbuffer_commit_write(B, A) ( \
	(B)->_end = ((B)->_end + (A)) % (B)->_length)

#endif

/*
int iocringbuffer_empty(iocringbuffer * buffer);

int iocringbuffer_full(iocringbuffer * buffer);



int32_i iocringbuffer_available_data(iocringbuffer * buffer);



int32_i iocringbuffer_available_space(iocringbuffer * buffer);



#define iocringbuffer_available_data(B) (\
		(B)->_end % (B)->_length - (B)->_start)



#define iocringbuffer_available_space(B) (\
		(B)->_length - (B)->_end - 1)



#define iocringbuffer_full(B) (iocringbuffer_available_space(B) == 0)

#define iocringbuffer_empty(B) (iocringbuffer_available_data((B)) == 0)



#define iocringbuffer_starts_at(B) (\
		(B)->_buffer + (B)->_start)

#define iocringbuffer_ends_at(B) (\
		(B)->_buffer + (B)->_end)



#define iocringbuffer_commit_read(B, A) (\
		(B)->_start = ((B)->_start + (A)) % (B)->_length)

#define iocringbuffer_commit_write(B, A) (\
		(B)->_end = ((B)->_end + (A)) % (B)->_length)*/
#ifdef __cplusplus
}
#endif

#endif
