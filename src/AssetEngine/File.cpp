#include <pch.h>

File::~File() { Close(); }

bool File::Open(const std::string& filename, Mode mode)
{
    assert(!IsOpen() && "File stream is already opened");
    Close();

    const char  *fmode = "";
    switch (mode)
    {
        case READ:  fmode = "rb";  break;
        case WRITE: fmode = "r+b"; break;
    }

    errno_t err = fopen_s(&mpFile, filename.c_str(), fmode);

    if (mode == WRITE && (err != 0 || mpFile == nullptr))
        err = fopen_s(&mpFile, filename.c_str(), "wb+");

    if (err != 0 || mpFile == nullptr)
        return false;

    mMode = mode; 
    return true;
}

UINT64 File::Read(UINT8 *buffer, UINT64 size, UINT64 count)
{
    assert(IsOpen() && "File not opened for reading");
    if (buffer == nullptr || size == 0)
        return 0;

    UINT64 totalBytes = size * count;
    UINT64 bytesRead = 0;

    while (totalBytes > 0)
    {
        UINT64 chunkSize = ComputeChunkSize(totalBytes);
        UINT64 read = fread_s(buffer + bytesRead, size - bytesRead, 1, chunkSize, mpFile);

        if (read == 0)
        {
            if (feof(mpFile))
                break;
        }

        if (mIsEncrypted)
            DecryptBuffer(buffer + bytesRead, read);

        bytesRead += read;
        totalBytes -= read;
    }

    assert(bytesRead <= size * count && "Read overflow detected");
    return bytesRead;
}

UINT64 File::Write(const UINT8 *buffer, UINT64 size, UINT64 count)
{
    assert(IsOpen() && "File not opened for writing");
    if (buffer == nullptr || size == 0)
        return 0;

    UINT64 totalBytes = size * count;
    UINT64 bytesWritten = 0;

    while (totalBytes > 0)
    {
        UINT64 chunkSize = ComputeChunkSize(totalBytes);

        if (mIsEncrypted)
        {
            std::vector<UINT8> temp(buffer + bytesWritten, buffer + bytesWritten + chunkSize);
            EncryptBuffer(temp.data(), chunkSize);
            UINT64 written = fwrite(temp.data(), 1, chunkSize, mpFile);
            if (written != chunkSize)
                break;
            bytesWritten += written;
        }
        else
        {
            UINT64 written = fwrite(buffer + bytesWritten, 1, chunkSize, mpFile);
            if (written != chunkSize)
                break;
            bytesWritten += written;
        }

        totalBytes -= chunkSize;
    }

    assert(bytesWritten == size * count && "Not all bytes written");
    return bytesWritten;
}


void File::Close()
{
    if (mpFile == nullptr)
        return;
    fclose(mpFile);
    mpFile = nullptr;
}

INT64 File::Seek(INT64 offset, int origin)
{
    assert(IsOpen() && "Cannot seek: file is not open.");

    int result = _fseeki64(mpFile, offset, origin);
    assert(result == 0 && "File::Seek failed to move cursor.");

    INT64 pos = _ftelli64(mpFile);
    assert(pos >= 0 && "File::Seek failed to get current position.");

    return pos;
}


UINT64 File::ComputeChunkSize(UINT64 remainingBytes) const
{
    UINT64 chunkSize;
    if (remainingBytes > MAX_BUFFER_SIZE) chunkSize = MAX_BUFFER_SIZE;
    else chunkSize = remainingBytes;
    return chunkSize;
}

bool File::OpenRead(const std::string& filename)
{
    return Open(filename, READ);
}

bool File::OpenWrite(const std::string& filename)
{
    return Open(filename, WRITE);
}

UINT64 File::GetSize()
{
    assert(IsOpen() && "File not opened");

    INT64 currentPos = Seek(0, SEEK_CUR);
    INT64 fileSize = Seek(0, SEEK_END);
    Seek(currentPos, SEEK_SET);

    return static_cast<UINT64>(fileSize);
}

std::string File::GetKey() const
{
    return mKey;
}

void File::SetKey(const std::string& key)
{
    mKey = key;
}

void File::EncryptBuffer(UINT8* buffer, UINT64 size)
{
    if (mKey.empty() || buffer == nullptr || size == 0)
        return;

    const UINT64 keyLength = mKey.size();

    for (UINT64 i = 0; i < size; ++i)
    {
        buffer[i] ^= static_cast<UINT8>(mKey[i % keyLength]);
    }
}

void File::DecryptBuffer(UINT8* buffer, UINT64 size)
{
    EncryptBuffer(buffer, size);    // ff c'est du XOR ;)
}