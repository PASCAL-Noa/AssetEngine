#ifndef STREAM_H__
#define STREAM_H__


class Stream
{
public:
    Stream() : mpFile(nullptr) {}
    virtual ~Stream() = default;

    virtual UINT64  Read(UINT8 *buffer, UINT64 size, UINT64 count = 1) = 0;
    virtual UINT64  Write(const UINT8 *buffer, UINT64 size, UINT64 count = 1) = 0;
    virtual INT64   Seek(INT64 offset, int origin = SEEK_SET) = 0;

    virtual bool    IsOpen() const = 0;
    virtual void    Close() = 0;
    virtual UINT64  GetSize() = 0;

    std::FILE       *mpFile;

    struct Header
    {
        char        magic[4];
        UINT16      version;
        UINT16      flags;
        UINT64      dataSize;
        UINT64      originalSize;
        UINT32      checksum;
    };

    enum Flags : UINT16
    {
        NONE       = 0x0000,
        COMPRESSED = 0x0001,
        ENCRYPTED  = 0x0002
    };
};

#endif // !STREAM_H__

