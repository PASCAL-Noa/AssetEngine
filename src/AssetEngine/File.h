#pragma once
#include "Stream.h"

enum  Mode
{
    READ,
    WRITE
};

class File : public Stream
{
public:
    File() : mIsEncrypted(false), mMode(READ) {}
    ~File() override;
    
    bool            Open(const std::string& filename, Mode mode);
    UINT64          Read(UINT8 *buffer, UINT64 size, UINT64 count = 1) override;
    UINT64          Write(const UINT8* buffer, UINT64 size, UINT64 count = 1) override;
    INT64           Seek(INT64 offset, int origin = SEEK_SET) override;


    bool            OpenRead(const std::string& filename);
    bool            OpenWrite(const std::string& filename);
    
    std::string     GetKey() const;
    void            SetKey(const std::string& key);
    void            EncryptBuffer(UINT8* buffer, UINT64 size);
    void            DecryptBuffer(UINT8* buffer, UINT64 size);
    void            EnableEncryption(bool enable) { mIsEncrypted = enable; }
    bool            IsEncryptionEnabled() const { return mIsEncrypted; }


    bool            IsOpen() const override { return mpFile != nullptr; }
    void            Close() override;
    UINT64          GetSize() override;
    Mode            GetMode() const { return mMode; }

private:
    UINT64          ComputeChunkSize(UINT64 remainingBytes) const;

    std::string     mKey;
    bool            mIsEncrypted;
    Mode            mMode;
};
