#include "iworker.h"
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timex.h>
#include "iinc.h"

int64_i iocworker_timegen_64b()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	int64_i ts = tv.tv_sec;
	ts *= 1000000ULL;
	ts += tv.tv_usec;
	return ts / 1000ULL;
}

int64_i iocworker_tilnetmillsis_64b(int64_i lastTimestamp)
{
	int64_i timestamp = iocworker_timegen_64b();
	while (timestamp <= lastTimestamp) {
		timestamp = iocworker_timegen_64b();
	}
	return timestamp;
}



//---------------------------------------------------------------
//function: 
//          iocworker_create_64b 
//Access:
//           public  
//Parameter:
//          [in] const long workid - 
//Returns:
//          iocworkerid_64b* - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
iocworkerid_64b* iocworker_create_64b(const int64_i workid)
{
	
	if (workid > (MAXWORKERID) || workid < 0)
	{
		return 0;
	}
	iocworkerid_64b *lpworkid = (iocworkerid_64b*)malloc(sizeof(iocworkerid_64b));
	assert(lpworkid);
	lpworkid->_work_id = workid;
	lpworkid->_sequence = 0;
	lpworkid->_last_timestamp = -1;
	return lpworkid;
}

//---------------------------------------------------------------
//function: 
//          iocworker_nextid ��� 64λ worker id
//Access:
//           public  
//Parameter:
//          [in] iocworkerid_64b * iocid - 
//Returns:
//          long - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
int64_i iocworker_nextid_64b(iocworkerid_64b *iocid)
{
	int64_i timestamp = iocworker_timegen_64b();

	if (iocid->_last_timestamp == timestamp)
	{
		int64_i tmplong = 1L;
		tmplong = iocid->_sequence + 1L;
		iocid->_sequence = (iocid->_sequence + 1L) & SEQUENCEMASK;//SEQUENCEMASK;
		if (iocid->_sequence == 0)
		{
			timestamp = iocworker_tilnetmillsis_64b(iocid->_last_timestamp);
		}
	}
	else
		iocid->_sequence = 0;

	iocid->_last_timestamp = timestamp;
	/*
	1.  22 low vacancy
	###################               ##################
	# fff... # ff...  #   ����22Bit   # fff..  # 00... #
	# HBYTE  # LBYTE  #  <<<<<<<<<<<  # HBYTE  # LBYTE #
	#   42      22    #               #  42    #  22   #
	###################               ##################

	2. Worker ID Move the value to more than 17 bits
	Ditto

	3. Assigns the specified value to the specified bit
	FFF00.......................
	or
	.....00000000000000000000
	or
	000001...................
	=
	ID

	*/
	int64_i nextId = ((timestamp - TWEPOCH_LONG) << TIMESTAMPLEFTSHIFT)\
		| (iocid->_work_id << WORKERIDSHIFT) | iocid->_sequence;
	return nextId;
}


iocworkerid* iocworker_create()
{
	iocworkerid *lpworkid = (iocworkerid*)malloc(sizeof(iocworkerid));
	assert(lpworkid);
	lpworkid->_sequence = 0;
	lpworkid->_last_timestamp = 0;
	return lpworkid;
}

/*#######################################################################################################
#########################################################################################################
########################################################################################################*/

uint32_i iocworker_timegen()
{
	struct timespec dida; 
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &dida);
	return (uint32_i)(((dida.tv_sec * 1000000) + (dida.tv_nsec / 1000) / 1000ULL) & 0xfffffffful);
}

uint32_i iocworker_tilnetmillsis(uint32_i lastTimestamp)
{
	uint32_i timestamp = iocworker_timegen();
	while (timestamp <= lastTimestamp) {
		timestamp = iocworker_timegen();
	}
	return timestamp;
}


//---------------------------------------------------------------
//function: 
//          iocworker_nextid ��ȡ 32λ worker id
//Access:
//           public  
//Parameter:
//          [in] iocworkerid * iocid - 
//Returns:
//          unsigned int - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
uint32_i iocworker_nextid(iocworkerid *iocid)
{
	uint32_i timestamp = iocworker_timegen();
	if (iocid->_last_timestamp == timestamp)
	{
		iocid->_sequence = (iocid->_sequence + 1) & SEQUENCEMASK32;
		if (iocid->_sequence == 0)
		{
			timestamp = iocworker_tilnetmillsis(iocid->_last_timestamp);
		}
	}
	else
		iocid->_sequence = 0;
	iocid->_last_timestamp = timestamp;

	//Principle same
	uint32_i nextId_32 = ((timestamp - TWEPOCH_INT) << SEQUENCEBITS32) | iocid->_sequence;
	return nextId_32;
}