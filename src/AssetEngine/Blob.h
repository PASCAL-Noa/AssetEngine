#ifndef BLOB_H__
#define BLOB_H__

class Blob
{
private:
	UINT8* mpData;
	UINT64 mCapacity;
	UINT64 mSize;

	bool Realloc(UINT64 newCapacity);
public:
	Blob();
	Blob(const UINT8* pData, UINT64 amount = 1);
	~Blob();

	//Avoid copies
	Blob(const Blob&) = delete;
	Blob& operator=(const Blob&) = delete;

	// Allow moves (for std::vector compatibility)
	Blob(Blob&& other) noexcept;
	Blob& operator=(Blob&& other) noexcept;

	UINT8* GetData();
	const UINT8* GetData() const;
	UINT64 GetSize() const;
	UINT64 GetCapacity() const;
	void Resize(UINT64 newSize);

	//Free unused memory
	bool Shrink();
	//Reset size
	void Clear();
	//Pre-Allocate memory
	bool Reserve(UINT64 capacity);

	template<typename T = UINT8>
	bool Append(const T* pData, UINT64 amount = 1);
	template<typename T = UINT8>
	bool Add(const T* pData, UINT64 atIndex, UINT64 amount = 1);
	template<typename T = UINT8>
	bool OverWrite(const T* pData, UINT64 atIndex, UINT64 amount = 1);
	//Note: Call Shrink() to free unused memory
	template<typename T = UINT8>
	bool Remove(UINT64 atIndex, UINT64 amount = 1);

	template<typename T = UINT8>
	T* At(UINT64 index);
	template<typename T = UINT8>
	const T* At(UINT64 index) const;
};

#include "Blob.inl"

#endif // !BLOB_H__
