#ifndef TYPES_H__
#define TYPES_H__

using UINT8 = unsigned char;
using INT8 = signed char;
using UINT16 = unsigned short;
using INT16 = signed short;
using UINT32 = unsigned int;
using INT32 = signed int;
using UINT64 = unsigned long long;
using INT64 = signed long long;



static_assert(sizeof(UINT8)	 == 1, "UINT8 must be 1 byte");
static_assert(sizeof(UINT16) == 2, "UINT16 must be 2 bytes");
static_assert(sizeof(UINT32) == 4, "UINT32 must be 4 bytes");
static_assert(sizeof(UINT64) == 8, "UINT64 must be 8 bytes");

#endif // !TYPES_H__
