#include "iringbuffer.h"
#include "iatom.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------
//function:
//          iocringbuffer_create
//Access:
//           public
//Parameter:
//          [in] int32_i length -
//Returns:
//          iocringbuffer * -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
iocringbuffer *iocringbuffer_create(int32_i length)
{

	iocringbuffer *buffer = (iocringbuffer *)malloc(sizeof(iocringbuffer));
	memset((char *)buffer, 0, sizeof(iocringbuffer));
	buffer->_length = length + 1;
#ifdef OC_MULTITHREADING
	buffer->_posv._pos._start = 0;
	buffer->_posv._pos._end = 0;
#else
	buffer->_start = 0;
	buffer->_end = 0;
#endif

	buffer->_buffer = malloc(buffer->_length);

	return buffer;
}

void iocringbuffer_clean(iocringbuffer *buffer)
{
#ifdef OC_MULTITHREADING
		buffer->_posv._pos._start = 0;
		buffer->_posv._pos._end = 0;
#else
		buffer->_start = 0;
		buffer->_end = 0;
#endif


}
//---------------------------------------------------------------
//function:
//          iocringbuffer_release
//Access:
//           public
//Parameter:
//          [in] iocringbuffer * buffer -
//Returns:
//          void -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
void iocringbuffer_release(iocringbuffer *buffer)
{
	if (buffer)
	{
		free(buffer->_buffer);
		free(buffer);
	}
}

//---------------------------------------------------------------
//function:
//          iocringbuffer_read
//Access:
//           public
//Parameter:
//          [in] iocringbuffer * buffer -
//          [in] char * target -
//          [in] int32_i amount -
//Returns:
//          int32_i -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i iocringbuffer_read(iocringbuffer *buffer, char *target, int32_i amount)
{
#ifdef OC_MULTITHREADING
	int32_i readbyte = 0;
	iocringbuffer_posval currentPos;
	iocringbuffer_posval newPos;

	do
	{
		//���ǰ��߳��ó�ȥ
		memcpy((char *)&currentPos._val, (char *)&buffer->_posv._val, sizeof(int64_i));
		memcpy((char *)&newPos._val, (char *)&buffer->_posv._val, sizeof(int64_i));

		iocringbuffer_will_read(&newPos, amount, buffer->_length);

		readbyte = newPos._pos._start - currentPos._pos._start;
		memcpy(target, iocringbuffer_starts_at(buffer, &currentPos), readbyte);

		if (newPos._pos._start == newPos._pos._end)
		{
			memset((char *)&newPos._val, 0, sizeof(int64_i));
		}
	} while (!CAS(&buffer->_posv._val, currentPos._val, newPos._val));

#else

	void *result = memcpy(target, iocringbuffer_starts_at(buffer), amount);
	if (result == NULL)
		return -1;

	iocringbuffer_commit_read(buffer, amount);

	if (buffer->_end == buffer->_start)
	{
		buffer->_start = buffer->_end = 0;
	}

#endif
	return readbyte;
}

//---------------------------------------------------------------
//function:
//          iocringbuffer_write
//Access:
//           public
//Parameter:
//          [in] iocringbuffer * buffer -
//          [in] char * data -
//          [in] int32_i length -
//Returns:
//          int32_i -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i iocringbuffer_write(iocringbuffer *buffer, char *data, int32_i length, Boolean still)
{
#ifdef OC_MULTITHREADING
	iocringbuffer_posval currentPos;
	iocringbuffer_posval newPos;

	do
	{
		memcpy((char *)&(currentPos._val), (char *)&(buffer->_posv._val), sizeof(int64_i));
		memcpy((char *)&newPos._val, (char *)&buffer->_posv._val, sizeof(int64_i));
		if (iocringbuffer_available_data(&currentPos, buffer->_length) == 0)
		{
			memset((char *)&newPos._val, 0, sizeof(int64_i));
		}

		if (length > iocringbuffer_available_space(&newPos, buffer->_length) && still == FALSE)
		{
			return -1;
		}

		iocringbuffer_will_write(&newPos, length, buffer->_length);

	} while (!CAS(&buffer->_posv._val, currentPos._val, newPos._val));
	//д������
	char *cc = buffer->_buffer;
	int aa = currentPos._pos._end;
	char *xx = iocringbuffer_ends_at(buffer, &currentPos);
	memcpy(iocringbuffer_ends_at(buffer, &currentPos), data, length);

#else

	if (iocringbuffer_available_data(buffer) == 0)
	{
		buffer->_start = buffer->_end = 0;
	}

	if (length > iocringbuffer_available_space(buffer))
	{
		return -1;
	}

	void *result = memcpy(iocringbuffer_ends_at(buffer), data, length);
	if (result == NULL)
		return -1;

	iocringbuffer_commit_write(buffer, length);

#endif
	return length;
}

void iocringbuffer_writebyted(iocringbuffer *buffer, uint32_i size)
{
#ifdef OC_MULTITHREADING
		buffer->_posv._pos._end += size;
#else
		buffer->_end += size;
#endif
}

void iocringbuffer_readbyted(iocringbuffer *buffer, uint32_i size)
{
#ifdef OC_MULTITHREADING
		buffer->_posv._pos._start += size;
		if (buffer->_posv._pos._start >= buffer->_posv._pos._end)
				goto _IOCRINGBUFFER_CLEAN;
#else
		buffer->_start += size;
		if (buffer->_start >= buffer->_end)
				goto _IOCRINGBUFFER_CLEAN;
#endif

_IOCRINGBUFFER_CLEAN:
		iocringbuffer_clean(buffer);
}

uint32_i iocringbuffer_remainbytes(iocringbuffer *buffer)
{
#ifdef OC_MULTITHREADING
		return buffer->_length - buffer->_posv._pos._end - 1; 
#else
		return buffer->_length - buffer->_end - 1; 
#endif
}

uint32_i iocringbuffer_bytes(iocringbuffer *buffer)
{
#ifdef OC_MULTITHREADING
	//printf("end : %d start: %d\n", buffer->_posv._pos._end, buffer->_posv._pos._start);
		return buffer->_posv._pos._end - buffer->_posv._pos._start;
#else
		return buffer->_end - buffer->_start;
#endif
}

char*  iocringbuffer_getwritepos(iocringbuffer *buffer)
{
#ifdef OC_MULTITHREADING
		return buffer->_buffer + buffer->_posv._pos._end;
#else
		return buffer->_buffer + buffer->_end;
#endif
}

char*  iocringbuffer_getreadpos(iocringbuffer *buffer)
{
#ifdef OC_MULTITHREADING
		return buffer->_buffer + buffer->_posv._pos._start;
#else
		return buffer->_buffer + buffer->_start;
#endif
}

//---------------------------------------------------------------
//function:
//          iocringbuffer_gets
//Access:
//           public
//Parameter:
//          [in] iocringbuffer * buffer -
//          [in] int32_i amount -
//          [in] char * targetchar -
//Returns:
//          int32_i -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i iocringbuffer_gets(iocringbuffer *buffer, int32_i amount, char *targetchar)
{
	return iocringbuffer_read(buffer, targetchar, amount);
}

//---------------------------------------------------------------
//function:
//          iocringbuffer_getint16
//Access:
//           public
//Parameter:
//          [in] iocringbuffer * buffer -
//          [in] int16_i * target -
//Returns:
//          int32_i -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i iocringbuffer_getint16(iocringbuffer *buffer, int16_i *target)
{
	return iocringbuffer_read(buffer, (char *)target, sizeof(int16_i));
}

//---------------------------------------------------------------
//function:
//          iocringbuffer_getuint16
//Access:
//           public
//Parameter:
//          [in] iocringbuffer * buffer -
//          [in] uint16_i * target -
//Returns:
//          int32_i -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i iocringbuffer_getuint16(iocringbuffer *buffer, uint16_i *target)
{
	return iocringbuffer_read(buffer, (char *)target, sizeof(uint16_i));
}

//---------------------------------------------------------------
//function:
//          iocringbuffer_getint32
//Access:
//           public
//Parameter:
//          [in] iocringbuffer * buffer -
//          [in] int32_i * target -
//Returns:
//          int32_i -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i iocringbuffer_getint32(iocringbuffer *buffer, int32_i *target)
{
	return iocringbuffer_read(buffer, (char *)target, sizeof(int32_i));
}

//---------------------------------------------------------------
//function:
//          iocringbuffer_getuint32
//Access:
//           public
//Parameter:
//          [in] iocringbuffer * buffer -
//          [in] uint32_i * target -
//Returns:
//          int32_i -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i iocringbuffer_getuint32(iocringbuffer *buffer, uint32_i *target)
{
	return iocringbuffer_read(buffer, (char *)target, sizeof(uint32_i));
}

//---------------------------------------------------------------
//function:
//          iocringbuffer_getint64
//Access:
//           public
//Parameter:
//          [in] iocringbuffer * buffer -
//          [in] int64_i * target -
//Returns:
//          int32_i -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i iocringbuffer_getint64(iocringbuffer *buffer, int64_i *target)
{
	return iocringbuffer_read(buffer, (char *)target, sizeof(int64_i));
	return 0;
}

//---------------------------------------------------------------
//function:
//          iocringbuffer_getuint64
//Access:
//           public
//Parameter:
//          [in] iocringbuffer * buffer -
//          [in] uint64_i * target -
//Returns:
//          int32_i -
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int32_i iocringbuffer_getuint64(iocringbuffer *buffer, uint64_i *target)
{
	return iocringbuffer_read(buffer, (char *)target, sizeof(uint64_i));
}
