#include "pch.h"
#include "Memory.h"


Memory::Memory()
{
	mpBlob = new Blob();
	mPosition = 0;
	mOwnsBlob = true;
	mWriteMode = WriteMode::PRESERVE;
}

Memory::Memory(Blob* blob)
{
	mpBlob = blob;
	mPosition = 0;
	mOwnsBlob = false;
	mWriteMode = WriteMode::PRESERVE;
}

Memory::~Memory()
{
	if (mOwnsBlob && mpBlob != nullptr) delete mpBlob;
}

UINT64 Memory::Read(UINT8* buffer, UINT64 size, UINT64 count)
{
	if (buffer == nullptr || size == 0)
		return 0;

	UINT64 totalBytes = size * count;
	UINT64 available = mpBlob->GetSize() - mPosition;
	UINT64 bytesToRead = (totalBytes < available) ? totalBytes : available;
	if (bytesToRead == 0) return 0;

	memcpy(buffer, mpBlob->GetData() + mPosition, bytesToRead);
	mPosition += bytesToRead;

	return bytesToRead;
}

UINT64 Memory::Write(const UINT8* buffer, UINT64 size, UINT64 count)
{
	if (buffer == nullptr || size == 0)
		return 0;

	UINT64 totalBytes = size * count;

	if (mPosition >= mpBlob->GetSize())
	{
		if (mPosition > mpBlob->GetSize())
		{
			UINT64 gap = mPosition - mpBlob->GetSize();
			std::vector<UINT8> zeros(gap, 0);
			mpBlob->Append(zeros.data(), gap);
		}
		mpBlob->Append(buffer, totalBytes);
	}
	else if (mPosition + totalBytes > mpBlob->GetSize())
	{
		UINT64 overwriteAmount = mpBlob->GetSize() - mPosition;
		mpBlob->OverWrite(buffer, mPosition, overwriteAmount);
		mpBlob->Append(buffer + overwriteAmount, totalBytes - overwriteAmount);
	}
	else
	{
		mpBlob->OverWrite(buffer, mPosition, totalBytes);

		if (mWriteMode == WriteMode::TRUNCATE)
		{
			mpBlob->Resize(mPosition + totalBytes);
		}
	}
	
	mPosition += totalBytes;
	return totalBytes;
}

INT64 Memory::Seek(INT64 offset, int origin)
{
	if (mpBlob == nullptr) return -1;

	INT64 newPosition = 0;

	switch (origin)
	{
	case SEEK_SET:
		newPosition = offset;
		break;
	case SEEK_CUR:
		newPosition = mPosition + offset;
		break;
	case SEEK_END:
		newPosition = mpBlob->GetSize() + offset;
		break;
	default:
		return -1;
	}

	if (newPosition < 0) return -1;

	mPosition = static_cast<UINT64>(newPosition);
	return mPosition;
}

Blob* Memory::GetBlob()
{
	return mpBlob;
}

const Blob* Memory::GetBlob() const
{
	return mpBlob;
}

UINT64 Memory::GetPosition() const
{
	return mPosition;
}

void Memory::ResetPos()
{
	mPosition = 0;
}

void Memory::AttachBlob(Blob* blob, bool ownership)
{
	assert(blob != nullptr && "[ERROR][MEMORY] Try to attach null Blob");

	if (mOwnsBlob && mpBlob != nullptr)
	{
		delete mpBlob;
	}

	mpBlob = blob;
	mOwnsBlob = ownership;
	mPosition = 0;
}

Blob* Memory::DetachBlob()
{
	Blob* blob = mpBlob;
	mpBlob = nullptr;
	mOwnsBlob = false;
	mPosition = 0;
	return blob;
}

void Memory::SetWriteMode(WriteMode mode)
{
	mWriteMode = mode;
}

bool Memory::IsOpen() const
{
	return mpBlob != nullptr;
}

void Memory::Close()
{
	// Memory n'a pas besoin de fermeture (pas de fichier)
}

UINT64 Memory::GetSize()
{
	if (mpBlob == nullptr)
		return 0;
	return mpBlob->GetSize();
}

