#pragma once

class Memory : Stream
{
public:
    Memory();
    Memory(Blob* blob);
    ~Memory() override;
    
    //Like for Blob, avoid copies
    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;

    UINT64  Read(UINT8* buffer, UINT64 size, UINT64 count = 1) override;
    //WriteMode by default: PRESERVE
    UINT64  Write(const UINT8 *buffer, UINT64 size, UINT64 count = 1) override;
    INT64   Seek(INT64 offset, int origin = SEEK_SET) override;

    bool    IsOpen() const override;
    void    Close() override;
    UINT64  GetSize() override;

    Blob* GetBlob();
    const Blob* GetBlob() const;
    UINT64 GetPosition() const;
    void ResetPos();

    //OwnerShip, choice to move Blob from a Memory or not
    //If you attach a Blob on a Memory with a Blob already, it will delete it,
    void AttachBlob(Blob* blob, bool ownership = false);
    Blob* DetachBlob();

    enum class WriteMode
    {
        PRESERVE,
        TRUNCATE
    };
    void SetWriteMode(WriteMode mode);
private:
    Blob   *mpBlob;
    UINT64 mPosition;
    bool mOwnsBlob;
    WriteMode mWriteMode;
};
