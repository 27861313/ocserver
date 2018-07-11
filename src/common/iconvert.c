#include "iconvert.h"

//---------------------------------------------------------------
//function: 
//          lendian_u16 
//Access:
//           public  
//Parameter:
//          [in] uint16_i v - 
//Returns:
//          uint16_i - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
inline uint16_i lendian_u16(uint16_i v)
{
	return ((v & 0xff00) >> 8) | ((v & 0x00ff) << 8);
}

//---------------------------------------------------------------
//function: 
//          lendian_u32 
//Access:
//           public  
//Parameter:
//          [in] uint32_i v - 
//Returns:
//          uint32_i - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
inline uint32_i lendian_u32(uint32_i v)
{
	return ((v & 0x000000FF) << 24) |
		((v & 0x0000FF00) << 8) |
		((v & 0x00FF0000) >> 8) |
		((v & 0xFF000000) >> 24);
}

//---------------------------------------------------------------
//function: 
//          lendian_f32 
//Access:
//           public  
//Parameter:
//          [in] float v - 
//Returns:
//          float - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
inline float lendian_f32(float v)
{
	struct IFValue
	{
		uint32_i _i32;
		float    _f32;
	};
	struct IFValue i;
	i._i32 = v;
	i._f32 = lendian_u32(i._i32);
	return i._f32;
}

//---------------------------------------------------------------
//function: 
//          v_touint32 
//Access:
//           public  
//Parameter:
//          [in] void * poffset - 
//Returns:
//          uint32_i - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
inline uint32_i v_touint32(void* poffset)
{
	if (poffset == NULL) return 0;
	uint32_i res = 0xffffffff;
	res &= (int64_i)poffset;
	return res;
}

//---------------------------------------------------------------
//function: 
//          v_toint32 
//Access:
//           public  
//Parameter:
//          [in] void * poffset - 
//Returns:
//          int32_i - 
//Remarks:
//          ...
//author:    Administrator[mirliang]
//---------------------------------------------------------------
inline int32_i v_toint32(void* poffset)
{
	if (NULL == poffset)	return 0;
	int32_i res = 0xffffffff;
	res &= ((int64_i)poffset);
	return res;
}