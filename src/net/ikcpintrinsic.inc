#include <common/iconvert.h>
#include <arpa/inet.h>

static uint32_i kcpconnection_hashcode(void *key)
{
	if (key == NULL)
		return 0;
	return v_touint32(key);
}

static int32_i kcpconnection_equals(void *key1, void *key2)
{
	uint32_i k1 = 0;
    if(key1 != NULL)	
	    k1 = v_touint32(key1);
	uint32_i k2 = 0;
    if(key2 != NULL)	
		k2 = v_touint32(key2);

	if (k1 > k2)
		return 1;
	else if (k1 < k2)
		return -1;
	else
		return 0;
}

static uint32_i kcpiperrs_hashcode(void *key)
{
	if (key == NULL)
		return 0;
	return (uint32_i)inet_addr((const char *)key);
}

static int32_i kcpiperrs_equals(void *key1, void *key2)
{
	uint32_i k1 = 0;
	if (key1 != NULL)
		k1 = (uint32_i)inet_addr((const char *)key1);
	uint32_i k2 = 0;
	if(key2 != NULL)
		k2 = (uint32_i)inet_addr((const char *)key2);
	
	if (k1 > k2)
		return 1;
	else if (k1 < k2)
		return -1;
	else
		return 0;
}