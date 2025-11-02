#ifndef SAFEFORMAT_H__
#define SAFEFORMAT_H__

class SafeFormat
{
public:
    // === CRC32 Utilities ===
    static UINT32 CalculateCRC32(const UINT8* data, UINT64 size);

    // === SAFE File Validation ===
    static bool Validate(const std::string& filename, Stream::Header& outHeader, Blob& outData);

    // === SAFE File Writing ===
    static bool WriteSafeFile(const std::string& filename, const UINT8* data, UINT64 size, UINT32 version = 1);

private:
    static const UINT32 CRC32_TABLE[256];
};

#endif // !SAFEFORMAT_H__
