#include "ikcpproto.h"
#include <stdio.h>
#include <stdlib.h>

inline Boolean kcpserver_isconnect(const char *data, const int32_i length)
{
	if (length != IKCPCONNECT_PACKSIZE)
		return FALSE;
	if ( (unsigned char)data[0] != IKCPCONNECT_START_BIT ||
		 (unsigned char)data[1] != IKCPCONNECT_SECOND_BIT ||
		 (unsigned char)data[7] != IKCPCONNECT_END_BIT )
		 	return FALSE;
			 
	int32_i seq = 0;
	memcpy(&seq, data + IKCPCONNECT_DATA_POS, IKCPCONNECT_DATA_SIZE);
	if (seq < 30000 || seq > 9000000)
		return FALSE;
	return TRUE;
}

inline void kcpserver_connect_finish_pack(char *outdata, const kcp_conv_t conv)
{
	memset(outdata, 0, IKCPCONNECT_PACKSIZE);
	outdata[0] = IKCPCONNECT_START_BIT;
	outdata[1] = IKCPCONNECT_SECOND_BIT;
	memcpy(outdata + 2, (char *)&conv, IKCPCONNECT_DATA_SIZE);
	outdata[7] = IKCPCONNECT_END_BIT;
}

inline int32_i kcpserver_disconnect_pack(char *outdata, const kcp_conv_t conv)
{
	outdata[0] = IKCPCONNECT_START_BIT;
	outdata[1] = IKCPCONNECT_SECOND_BIT;
	memcpy(outdata + 2, (char *)&conv, IKCPCONNECT_DATA_SIZE);
	outdata[6] = 0x01;
	outdata[7] = IKCPCONNECT_END_BIT;
	return IKCPCONNECT_PACKSIZE;
}