#include <pch.h>

bool Blob::Realloc(UINT64 newCapacity)
{
	if (newCapacity == mCapacity) return true;

	UINT8* temp = (UINT8*)realloc(mpData, newCapacity * sizeof(UINT8));
	if (temp == nullptr)
	{
		std::cout << "[ERROR][BLOB] Realloc failed" << std::endl;
		return false;
	}

	mpData = temp;
	mCapacity = newCapacity;

	return true;
}

Blob::Blob() :
	mpData(nullptr),
	mCapacity(256),
	mSize(0)
{
	mpData = (UINT8*)malloc(mCapacity * sizeof(UINT8));
	if (mpData == nullptr)
	{
		std::cout << "[ERROR][BLOB] malloc failed" << std::endl;
		mCapacity = 0;
		return;
	}
}

Blob::Blob(const UINT8* pData, UINT64 amount) :
	mpData(nullptr),
	mCapacity(amount),
	mSize(amount)
{
	mpData = (UINT8*)malloc(mCapacity * sizeof(UINT8));
	if (mpData == nullptr)
	{
		std::cout << "[ERROR][BLOB] malloc failed" << std::endl;
		mSize = 0;
		mCapacity = 0;
		return;
	}
	if (pData != nullptr)
	{
		memcpy(mpData, pData, amount * sizeof(UINT8));
	}
}

Blob::~Blob()
{
	free(mpData);
	mpData = nullptr;
}

// Move constructor
Blob::Blob(Blob&& other) noexcept :
	mpData(other.mpData),
	mCapacity(other.mCapacity),
	mSize(other.mSize)
{
	// Steal resources from other
	other.mpData = nullptr;
	other.mCapacity = 0;
	other.mSize = 0;
}

// Move assignment
Blob& Blob::operator=(Blob&& other) noexcept
{
	if (this != &other)
	{
		// Free existing resources
		free(mpData);

		// Steal resources from other
		mpData = other.mpData;
		mCapacity = other.mCapacity;
		mSize = other.mSize;

		// Leave other in valid empty state
		other.mpData = nullptr;
		other.mCapacity = 0;
		other.mSize = 0;
	}
	return *this;
}

UINT8* Blob::GetData()
{
	return mpData;
}

const UINT8* Blob::GetData() const
{
	return mpData;
}

UINT64 Blob::GetSize() const
{
	return mSize;
}

UINT64 Blob::GetCapacity() const
{
	return mCapacity;
}

void Blob::Resize(UINT64 newSize)
{
	if (newSize > mCapacity)
	{
		Reserve(newSize);
	}
	mSize = newSize;
}

bool Blob::Shrink()
{
	if (mSize == mCapacity) return true;

	UINT64 newCapacity = (mSize > 0) ? mSize : 1;
	return Realloc(newCapacity);
}

void Blob::Clear()
{
	mSize = 0;
}

bool Blob::Reserve(UINT64 capacity)
{
	if (capacity <= mCapacity) return true;
	return Realloc(capacity);
}
