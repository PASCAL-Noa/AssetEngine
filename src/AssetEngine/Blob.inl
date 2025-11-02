#include "pch.h"

template<typename T>
inline bool Blob::Append(const T* pData, UINT64 amount)
{
	if (pData == nullptr || amount == 0) return false;

	UINT64 bytesToAdd = amount * sizeof(T);
	UINT64 newSize = mSize + bytesToAdd;

	while (mCapacity < newSize)
	{
		mCapacity *= 2;
	}
	if (!Realloc(mCapacity)) return false;

	memcpy(mpData + mSize, pData, bytesToAdd);
	mSize = newSize;

	return true;
}  

template<typename T>
inline bool Blob::Add(const T* pData, UINT64 atIndex, UINT64 amount)
{
	if (pData == nullptr || amount == 0) return false;

	UINT64 atByte = atIndex * sizeof(T);
	if (atByte == mSize)
	{
		return Append<T>(pData, amount);
	}

	UINT64 bytesToAdd = amount * sizeof(T);

	assert(atByte <= mSize && "[ERROR][BLOB] Add position out of bounds: ");
	UINT64 newSize = mSize + bytesToAdd;

	while (mCapacity < newSize)
	{
		mCapacity *= 2;
	}
	if (!Realloc(mCapacity)) return false;

	UINT64 bytesAfter = mSize - atByte;
	UINT8* temp = (UINT8*)malloc(bytesAfter);
	assert(temp != nullptr && "[ERROR][BLOB] Add: temp malloc failed");

	memcpy(temp, mpData + atByte, bytesAfter);
	memcpy(mpData + atByte, pData, bytesToAdd);
	memcpy(mpData + atByte + bytesToAdd, temp, bytesAfter);

	free(temp);
	mSize = newSize;
	return true;
}

template<typename T>
inline bool Blob::OverWrite(const T* pData, UINT64 atIndex, UINT64 amount)
{
	if (pData == nullptr || amount == 0) return false;

	UINT64 atByte = atIndex * sizeof(T);
	UINT64 bytesToWrite = amount * sizeof(T);
	assert(atByte + bytesToWrite <= mSize && "[ERROR][BLOB] OverWrite: Out of Bounds");

	memcpy(mpData + atByte, pData, bytesToWrite);
	return true;
}

template<typename T>
inline bool Blob::Remove(UINT64 atIndex, UINT64 amount)
{
	if (amount == 0) return true;

	UINT64 atByte = atIndex * sizeof(T);
	UINT64 bytesToRemove = amount * sizeof(T);

	assert(atByte < mSize && "[ERROR][BLOB] Remove index out of bounds: byte position");
	assert(atByte + bytesToRemove <= mSize && "[ERROR][BLOB] Remove range exceeds blob size");

	UINT64 bytesAfter = mSize - (atByte + bytesToRemove);
	if (bytesAfter > 0)
	{
		memmove(mpData + atByte, mpData + atByte + bytesToRemove, bytesAfter);
	}

	mSize -= bytesToRemove;
	return true;
}

template<typename T>
inline T* Blob::At(UINT64 index)
{
	UINT64 byteOffset = index * sizeof(T);
	assert(byteOffset + sizeof(T) <= mSize && "[ERROR][BLOB] At: index out of bounds");
	return reinterpret_cast<T*>(mpData + byteOffset);
}

template<typename T>
inline const T* Blob::At(UINT64 index) const
{
	UINT64 byteOffset = index * sizeof(T);
	if (byteOffset + sizeof(T) > mSize)
	{
		std::cout << "[ERROR][BLOB] At: index out of bounds" << std::endl;
		return nullptr;
	}
	return reinterpret_cast<const T*>(mpData + byteOffset);
}
