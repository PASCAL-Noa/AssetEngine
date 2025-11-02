#ifndef ARCHIVE_H__
#define ARCHIVE_H__

enum FileFlags : UINT8
{
    FILE_ACTIVE = 0x01,
    FILE_DELETED = 0x02,
    FILE_COMPRESSED = 0x04,
    FILE_ENCRYPTED = 0x08
};

struct ArchiveHeader
{
    char     magic[4];
    UINT32   version;
    UINT32   fileCount;
    UINT64   dataOffset;
};

struct FileHeader
{
    char     magic[4];     
    UINT64   id;
    UINT64   dataSize;
    char     filename[256];   
    UINT8    flags;
    UINT32   checksum;       
    UINT8    padding[3];
};

struct MapEntry
{
    char     key[256];
    UINT64   offset;
};

class Archive
{
public:
    Archive() : m_stream(nullptr), m_ownsStream(false), m_encryptionEnabled(false), m_encryptionKey("") {}
    ~Archive();

    bool Open(const std::string& archivePath, Mode mode);
    bool OpenStream(Stream* stream);
    void Close();
    bool IsOpen() const;

    bool Create(const std::vector<std::string>& filePaths);

    bool List() const;
    bool Validate() const;

    bool Extract(UINT64 fileID, const std::string& outputPath) const;
    bool ExtractByName(const std::string& filename, const std::string& outputPath) const;
    bool ExtractAll(const std::string& outputDir) const;

    bool AddFile(const std::string& filePath);
    bool AddFile(const std::string& filePath, UINT64& outGeneratedID);
    bool AddFile(const std::vector<std::string>& filePaths);
    bool RemoveFile(UINT64 fileID);
    bool RemoveFileByName(const std::string& filename);
    bool RemoveAll(); 
    bool RenameFile(UINT64 fileID, const std::string& newName);
    bool RenameFileByName(const std::string& oldName, const std::string& newName);
    bool Compact();

    void EnableEncryption(bool enable);
    void SetEncryptionKey(const std::string& key);
    bool IsEncryptionEnabled() const;

private:
    static UINT64 GenerateFileID(const std::string& filename);

    bool ReadMaps();
    bool WriteMaps();
    bool RebuildArchive(); 

    bool ReadFileHeader(UINT64 offset, FileHeader& header, std::string& filename) const;
    bool ReadFileHeader(Stream& stream, UINT64 offset, FileHeader& header, std::string& filename) const;
    bool WriteFileHeader(const std::string& filename, UINT64 id, UINT64 dataSize, UINT8 flags, UINT32 checksum);

    bool ReadArchiveHeader();
    bool WriteArchiveHeader();

    bool WriteFileWithHeader(const std::string& filePath, UINT64& outID, UINT64& outSize);

    static std::string GetBasename(const std::string& path);
    std::string GetUniqueFilename(const std::string& baseFilename) const;

    mutable Stream* m_stream;
    bool m_ownsStream;
    ArchiveHeader m_header;
    std::unordered_map<std::string, UINT64> m_nameToOffset;
    std::unordered_map<UINT64, UINT64> m_idToOffset;
    std::string m_archivePath;

    bool m_encryptionEnabled;
    std::string m_encryptionKey;
};

#endif // !ARCHIVE_H__
