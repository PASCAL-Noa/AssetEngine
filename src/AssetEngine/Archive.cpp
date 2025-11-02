#include "pch.h"

namespace
{
    struct FileData
    {
        FileHeader header;
        std::string filename;
        Blob data;
        UINT64 id;
    };
}

Archive::~Archive()
{
    Close();

    if (m_ownsStream && m_stream != nullptr)
    {
        delete m_stream;
        m_stream = nullptr;
    }
}

bool Archive::Open(const std::string& archivePath, Mode mode)
{
    if (IsOpen())
        Close();

    m_archivePath = archivePath;

    File* file = new File();
    if (!file->Open(archivePath, mode))
    {
        delete file;
        return false;
    }

    m_stream = file;
    m_ownsStream = true;

    if (mode == Mode::READ)
    {
        if (!ReadArchiveHeader())
        {
            Close();
            return false;
        }

        if (!ReadMaps())
        {
            Close();
            return false;
        }
    }
    else
    {
        if (ReadArchiveHeader() && ReadMaps())
        {
        }
        else
        {
            m_header.magic[0] = 'A';
            m_header.magic[1] = 'S';
            m_header.magic[2] = 'E';
            m_header.magic[3] = 'T';
            m_header.version = 4;
            m_header.fileCount = 0;
            m_header.dataOffset = sizeof(ArchiveHeader);
        }
    }

    return true;
}

bool Archive::OpenStream(Stream* stream)
{
    if (stream == nullptr || !stream->IsOpen())
        return false;

    if (IsOpen())
        Close();

    m_stream = stream;
    m_ownsStream = false;  

    if (ReadArchiveHeader() && ReadMaps())
    {
    }
    else
    {
        m_header.magic[0] = 'A';
        m_header.magic[1] = 'S';
        m_header.magic[2] = 'E';
        m_header.magic[3] = 'T';
        m_header.version = 4;
        m_header.fileCount = 0;
        m_header.dataOffset = sizeof(ArchiveHeader);
    }

    return true;
}

void Archive::Close()
{
    if (m_stream != nullptr)
    {
        m_stream->Close();
    }
    m_nameToOffset.clear();
    m_idToOffset.clear();
    m_archivePath.clear();
}

bool Archive::IsOpen() const
{
    return m_stream != nullptr && m_stream->IsOpen();
}

UINT64 Archive::GenerateFileID(const std::string& filename)
{
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    UINT32 hash = SafeFormat::CalculateCRC32((const UINT8*)filename.c_str(), filename.size());

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<UINT64> dis(0, 0xFFFFFFFF);
    UINT64 random = dis(gen);

    return (timestamp << 32) | (hash ^ (random & 0xFFFFFFFF));
}

std::string Archive::GetBasename(const std::string& path)
{
    size_t lastSlash = path.find_last_of("/\\");
    return (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
}

std::string Archive::GetUniqueFilename(const std::string& baseFilename) const
{
    if (m_nameToOffset.find(baseFilename) == m_nameToOffset.end())
        return baseFilename;

    size_t dotPos = baseFilename.find_last_of('.');
    std::string name = (dotPos != std::string::npos) ? baseFilename.substr(0, dotPos) : baseFilename;
    std::string ext = (dotPos != std::string::npos) ? baseFilename.substr(dotPos) : "";

    int counter = 1;
    std::string candidateName;
    while (true)
    {
        candidateName = name + "(" + std::to_string(counter) + ")" + ext;

        if (m_nameToOffset.find(candidateName) == m_nameToOffset.end())
            return candidateName;

        counter++;
    }
}

bool Archive::ReadArchiveHeader()
{
    assert(m_stream->IsOpen() && "Archive file must be open");

    UINT64 bytesRead = m_stream->Read((UINT8*)&m_header, sizeof(ArchiveHeader), 1);
    if (bytesRead != sizeof(ArchiveHeader))
        return false;

    if (m_header.magic[0] != 'A' || m_header.magic[1] != 'S' ||
        m_header.magic[2] != 'E' || m_header.magic[3] != 'T')
        return false;

    return true;
}

bool Archive::WriteArchiveHeader()
{
    assert(m_stream->IsOpen() && "Archive file must be open");

    UINT64 written = m_stream->Write((const UINT8*)&m_header, sizeof(ArchiveHeader), 1);
    return (written == sizeof(ArchiveHeader));
}

bool Archive::ReadMaps()
{
    m_nameToOffset.clear();
    m_idToOffset.clear();

    m_stream->Seek(sizeof(ArchiveHeader), SEEK_SET);

    for (UINT32 i = 0; i < m_header.fileCount; i++)
    {
        MapEntry entry;
        m_stream->Read((UINT8*)&entry, sizeof(MapEntry), 1);
        m_nameToOffset[std::string(entry.key)] = entry.offset;
    }

    for (UINT32 i = 0; i < m_header.fileCount; i++)
    {
        MapEntry entry;
        m_stream->Read((UINT8*)&entry, sizeof(MapEntry), 1);
        UINT64 id = std::stoull(entry.key);
        m_idToOffset[id] = entry.offset;
    }

    return true;
}

bool Archive::WriteMaps()
{
    UINT64 newMapSize = m_header.fileCount * sizeof(MapEntry) * 2;
    UINT64 oldMapSize = (m_header.dataOffset - sizeof(ArchiveHeader));

    if (newMapSize > oldMapSize)
    {
        return RebuildArchive();
    }

    m_stream->Seek(sizeof(ArchiveHeader), SEEK_SET);

    for (const auto& [name, offset] : m_nameToOffset)
    {
        MapEntry entry = {};
        strncpy_s(entry.key, name.c_str(), 255);
        entry.offset = offset;
        m_stream->Write((UINT8*)&entry, sizeof(MapEntry), 1);
    }

    for (const auto& [id, offset] : m_idToOffset)
    {
        MapEntry entry = {};
        snprintf(entry.key, 256, "%llu", id);
        entry.offset = offset;
        m_stream->Write((UINT8*)&entry, sizeof(MapEntry), 1);
    }

    return true;
}

bool Archive::RebuildArchive()
{
    std::vector<FileData> allFiles;

    for (const auto& [name, oldOffset] : m_nameToOffset)
    {
        FileData fd;
        if (!ReadFileHeader(oldOffset, fd.header, fd.filename))
            continue;

        if (!(fd.header.flags & FILE_ACTIVE))
            continue;

        UINT64 blobOffset = oldOffset + sizeof(FileHeader);
        m_stream->Seek(blobOffset, SEEK_SET);

        fd.data.Reserve(fd.header.dataSize);
        UINT64 remaining = fd.header.dataSize;
        UINT64 offset = 0;

        while (remaining > 0)
        {
            UINT64 chunkSize = (remaining > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : remaining;
            UINT64 bytesRead = m_stream->Read(fd.data.GetData() + offset, chunkSize, 1);
            if (bytesRead == 0)
                break;
            offset += bytesRead;
            remaining -= bytesRead;
        }
        fd.data.Resize(fd.header.dataSize);

        fd.id = fd.header.id;
        allFiles.push_back(std::move(fd));
    }

    m_stream->Close();

    if (!static_cast<File*>(m_stream)->OpenWrite(m_archivePath))
        return false;

    UINT64 newMapSize = m_header.fileCount * sizeof(MapEntry) * 2;

    m_stream->Seek(0, SEEK_SET);
    m_stream->Write((const UINT8*)&m_header, sizeof(ArchiveHeader), 1);

    m_stream->Seek(newMapSize, SEEK_CUR);
    m_header.dataOffset = m_stream->Seek(0, SEEK_CUR);

    m_nameToOffset.clear();
    m_idToOffset.clear();

    for (const auto& fd : allFiles)
    {
        UINT64 newFileOffset = m_stream->Seek(0, SEEK_CUR);

        if (!WriteFileHeader(fd.filename, fd.id, fd.data.GetSize(), fd.header.flags, fd.header.checksum))
            continue;

        m_stream->Write(fd.data.GetData(), fd.data.GetSize(), 1);

        m_nameToOffset[fd.filename] = newFileOffset;
        m_idToOffset[fd.id] = newFileOffset;
    }

    m_stream->Seek(sizeof(ArchiveHeader), SEEK_SET);

    for (const auto& [name, offset] : m_nameToOffset)
    {
        MapEntry entry = {};
        strncpy_s(entry.key, name.c_str(), 255);
        entry.offset = offset;
        m_stream->Write((UINT8*)&entry, sizeof(MapEntry), 1);
    }

    for (const auto& [id, offset] : m_idToOffset)
    {
        MapEntry entry = {};
        snprintf(entry.key, 256, "%llu", id);
        entry.offset = offset;
        m_stream->Write((UINT8*)&entry, sizeof(MapEntry), 1);
    }

    m_stream->Seek(0, SEEK_SET);
    WriteArchiveHeader();

    return true;
}

bool Archive::ReadFileHeader(UINT64 offset, FileHeader& header, std::string& filename) const
{
    return ReadFileHeader(*m_stream, offset, header, filename);
}

bool Archive::ReadFileHeader(Stream& stream, UINT64 offset, FileHeader& header, std::string& filename) const
{
    stream.Seek(offset, SEEK_SET);

    UINT64 bytesRead = stream.Read((UINT8*)&header, sizeof(FileHeader), 1);
    if (bytesRead != sizeof(FileHeader))
        return false;

    if (header.magic[0] != 'F' || header.magic[1] != 'I' ||
        header.magic[2] != 'L' || header.magic[3] != 'E')
        return false;

    filename = std::string(header.filename);

    return true;
}

bool Archive::WriteFileHeader(const std::string& filename, UINT64 id, UINT64 dataSize, UINT8 flags, UINT32 checksum)
{
    if (filename.length() >= 256)
    {
        std::cerr << "[ERROR] Filename too long (max 255 chars): " << filename << "\n";
        return false;
    }

    FileHeader header;
    header.magic[0] = 'F';
    header.magic[1] = 'I';
    header.magic[2] = 'L';
    header.magic[3] = 'E';
    header.id = id;
    header.dataSize = dataSize;

    memset(header.filename, 0, 256);
    strncpy_s(header.filename, filename.c_str(), 255);

    header.flags = flags;
    header.checksum = checksum;
    memset(header.padding, 0, sizeof(header.padding));

    UINT64 headerWritten = m_stream->Write((UINT8*)&header, sizeof(FileHeader), 1);
    if (headerWritten != sizeof(FileHeader))
        return false;

    return true;
}

bool Archive::WriteFileWithHeader(const std::string& filePath, UINT64& outID, UINT64& outSize)
{
    File inputFile;
    if (!inputFile.OpenRead(filePath))
        return false;

    inputFile.Seek(0, SEEK_END);
    UINT64 fileSize = inputFile.Seek(0, SEEK_CUR);
    inputFile.Seek(0, SEEK_SET);

    Blob fileData;
    fileData.Reserve(fileSize);

    UINT8 buffer[MAX_BUFFER_SIZE];
    UINT64 remaining = fileSize;

    while (remaining > 0)
    {
        UINT64 chunkSize = (remaining > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : remaining;
        UINT64 bytesRead = inputFile.Read(buffer, chunkSize, 1);
        if (bytesRead == 0)
            break;
        fileData.Append(buffer, bytesRead);
        remaining -= bytesRead;
    }

    inputFile.Close();

    UINT32 checksum = SafeFormat::CalculateCRC32(fileData.GetData(), fileData.GetSize());

    UINT64 dataWritten = m_stream->Write(fileData.GetData(), fileData.GetSize(), 1);
    if (dataWritten != fileData.GetSize())
        return false;

    outID = GenerateFileID(filePath);
    outSize = fileData.GetSize();

    return true;
}

bool Archive::Create(const std::vector<std::string>& filePaths)
{
    m_archivePath = "temp_archive.asset";  

    if (m_stream == nullptr || !m_stream->IsOpen())
    {
        File* file = new File();
        if (!file->OpenWrite("temp_archive.asset"))
        {
            delete file;
            return false;
        }
        m_stream = file;
        m_ownsStream = true;
    }
    else if (!static_cast<File*>(m_stream)->OpenWrite("temp_archive.asset"))
    {
        return false;
    }

    m_header.magic[0] = 'A';
    m_header.magic[1] = 'S';
    m_header.magic[2] = 'E';
    m_header.magic[3] = 'T';
    m_header.version = 4;
    m_header.fileCount = static_cast<UINT32>(filePaths.size());
    m_header.dataOffset = 0;

    m_stream->Write((const UINT8*)&m_header, sizeof(ArchiveHeader), 1);

    UINT64 estimatedMapSize = filePaths.size() * sizeof(MapEntry) * 2;
    m_stream->Seek(estimatedMapSize, SEEK_CUR);

    m_header.dataOffset = m_stream->Seek(0, SEEK_CUR);

    for (const auto& filePath : filePaths)
    {
        UINT64 fileHeaderOffset = m_stream->Seek(0, SEEK_CUR);
        std::string basename = GetBasename(filePath);
        std::string uniqueName = GetUniqueFilename(basename);

        File inputFile;
        if (!inputFile.OpenRead(filePath))
            continue;

        inputFile.Seek(0, SEEK_END);
        UINT64 fileSize = inputFile.Seek(0, SEEK_CUR);
        inputFile.Seek(0, SEEK_SET);

        Blob fileData;
        fileData.Reserve(fileSize);

        UINT8 buffer[MAX_BUFFER_SIZE];
        UINT64 remaining = fileSize;

        while (remaining > 0)
        {
            UINT64 chunkSize = (remaining > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : remaining;
            UINT64 bytesRead = inputFile.Read(buffer, chunkSize, 1);
            if (bytesRead == 0)
                break;
            fileData.Append(buffer, bytesRead);
            remaining -= bytesRead;
        }

        inputFile.Close();

        UINT32 checksum = SafeFormat::CalculateCRC32(fileData.GetData(), fileData.GetSize());
        UINT64 id = GenerateFileID(uniqueName);

        if (!WriteFileHeader(uniqueName, id, fileData.GetSize(), FILE_ACTIVE, checksum))
            continue;

        UINT64 dataWritten = m_stream->Write(fileData.GetData(), fileData.GetSize(), 1);
        if (dataWritten != fileData.GetSize())
            continue;

        m_nameToOffset[uniqueName] = fileHeaderOffset;
        m_idToOffset[id] = fileHeaderOffset;
    }

    m_stream->Seek(sizeof(ArchiveHeader), SEEK_SET);
    WriteMaps();

    m_stream->Seek(0, SEEK_SET);
    WriteArchiveHeader();

    Close();
    return true;
}

bool Archive::List() const
{
    std::cout << "==========================================\n";
    std::cout << "Archive: " << m_archivePath << "\n";
    std::cout << "Files: " << m_header.fileCount << " active\n";
    std::cout << "==========================================\n";

    UINT32 index = 1;
    for (const auto& [name, offset] : m_nameToOffset)
    {
        FileHeader header;
        std::string filename;
        if (!ReadFileHeader(offset, header, filename))
            continue;

        std::cout << "[" << index << "] " << filename
            << " (ID: " << header.id
            << ", " << header.dataSize << " bytes"
            << ", CRC32: 0x" << std::hex << std::uppercase << header.checksum << std::dec << ")\n";
        index++;
    }

    std::cout << "==========================================\n";

    return true;
}

bool Archive::Extract(UINT64 fileID, const std::string& outputPath) const
{
    auto it = m_idToOffset.find(fileID);
    if (it == m_idToOffset.end())
    {
        std::cerr << "[ERROR] File ID " << fileID << " not found in archive\n";
        return false;
    }

    FileHeader header;
    std::string filename;
    if (!ReadFileHeader(it->second, header, filename))
    {
        std::cerr << "[ERROR] Failed to read file header for ID " << fileID << "\n";
        return false;
    }

    if (!(header.flags & FILE_ACTIVE))
    {
        std::cerr << "[ERROR] File ID " << fileID << " is marked as deleted\n";
        return false;
    }

    UINT64 blobOffset = it->second + sizeof(FileHeader);
    m_stream->Seek(blobOffset, SEEK_SET);

    bool isEncrypted = (header.flags & FILE_ENCRYPTED) != 0;
    if (isEncrypted)
    {
        if (m_encryptionKey.empty())
        {
            std::cerr << "[ERROR] File is encrypted but no decryption key provided\n";
            return false;
        }

        File* fileStream = static_cast<File*>(m_stream);
        fileStream->EnableEncryption(true);
        fileStream->SetKey(m_encryptionKey);
    }

    Blob fileData;
    fileData.Reserve(header.dataSize);

    UINT8 buffer[MAX_BUFFER_SIZE];
    UINT64 remaining = header.dataSize;

    while (remaining > 0)
    {
        UINT64 chunkSize = (remaining > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : remaining;
        UINT64 bytesRead = m_stream->Read(buffer, chunkSize, 1);
        if (bytesRead == 0)
            break;
        fileData.Append(buffer, bytesRead);
        remaining -= bytesRead;
    }

    if (isEncrypted)
    {
        File* fileStream = static_cast<File*>(m_stream);
        fileStream->EnableEncryption(false);
    }

    UINT32 calculatedCRC = SafeFormat::CalculateCRC32(fileData.GetData(), fileData.GetSize());
    if (calculatedCRC != header.checksum)
    {
        std::cerr << "[ERROR] CRC32 mismatch for file ID " << fileID << " (expected 0x"
                  << std::hex << header.checksum << ", got 0x" << calculatedCRC << std::dec << ")\n";
        return false;
    }

    File outputFile;
    if (!outputFile.OpenWrite(outputPath))
    {
        std::cerr << "[ERROR] Failed to open output file: " << outputPath << "\n";
        return false;
    }

    UINT64 written = outputFile.Write(fileData.GetData(), fileData.GetSize(), 1);
    outputFile.Close();

    return (written == fileData.GetSize());
}

bool Archive::ExtractByName(const std::string& filename, const std::string& outputPath) const
{
    auto it = m_nameToOffset.find(filename);
    if (it == m_nameToOffset.end())
        return false;

    FileHeader header;
    std::string name;
    if (!ReadFileHeader(it->second, header, name))
        return false;

    return Extract(header.id, outputPath);
}

bool Archive::ExtractAll(const std::string& outputDir) const
{
    _mkdir(outputDir.c_str());

    for (const auto& [name, offset] : m_nameToOffset)
    {
        FileHeader header;
        std::string filename;
        if (!ReadFileHeader(offset, header, filename))
            continue;

        std::string outputPath = outputDir + "/" + filename;

        UINT64 blobOffset = offset + sizeof(FileHeader);
        m_stream->Seek(blobOffset, SEEK_SET);

        Blob fileData;
        fileData.Reserve(header.dataSize);

        UINT8 buffer[MAX_BUFFER_SIZE];
        UINT64 remaining = header.dataSize;

        while (remaining > 0)
        {
            UINT64 chunkSize = (remaining > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : remaining;
            UINT64 bytesRead = m_stream->Read(buffer, chunkSize, 1);
            if (bytesRead == 0)
                break;
            fileData.Append(buffer, bytesRead);
            remaining -= bytesRead;
        }

        UINT32 calculatedCRC = SafeFormat::CalculateCRC32(fileData.GetData(), fileData.GetSize());
        if (calculatedCRC != header.checksum)
        {
            std::cout << "[SKIP] " << filename << " (CRC32 mismatch)\n";
            continue;
        }

        File outputFile;
        if (outputFile.OpenWrite(outputPath))
        {
            outputFile.Write(fileData.GetData(), fileData.GetSize(), 1);
            outputFile.Close();
        }
    }

    return true;
}

bool Archive::Validate() const
{
    std::cout << "Validating archive: " << m_archivePath << "\n";
    std::cout << "Files to check: " << m_header.fileCount << "\n";

    bool allValid = true;

    for (const auto& [name, offset] : m_nameToOffset)
    {
        FileHeader header;
        std::string filename;
        if (!ReadFileHeader(offset, header, filename))
        {
            std::cout << "[FAIL] " << name << " (invalid file header)\n";
            allValid = false;
            continue;
        }

        UINT64 blobOffset = offset + sizeof(FileHeader);
        m_stream->Seek(blobOffset, SEEK_SET);

        Blob fileData;
        fileData.Reserve(header.dataSize);

        UINT8 buffer[MAX_BUFFER_SIZE];
        UINT64 remaining = header.dataSize;

        while (remaining > 0)
        {
            UINT64 chunkSize = (remaining > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : remaining;
            UINT64 bytesRead = m_stream->Read(buffer, chunkSize, 1);
            if (bytesRead == 0)
                break;
            fileData.Append(buffer, bytesRead);
            remaining -= bytesRead;
        }

        UINT32 calculatedCRC = SafeFormat::CalculateCRC32(fileData.GetData(), fileData.GetSize());

        if (calculatedCRC == header.checksum)
        {
            std::cout << "[OK] " << filename << "\n";
        }
        else
        {
            std::cout << "[FAIL] " << filename << " (CRC mismatch, expected 0x"
                << std::hex << header.checksum << ", got 0x" << calculatedCRC << std::dec << ")\n";
            allValid = false;
        }
    }

    std::cout << (allValid ? "Archive is valid.\n" : "Archive has errors.\n");

    return allValid;
}

bool Archive::AddFile(const std::string& filePath)
{
    UINT64 discardedID;
    return AddFile(filePath, discardedID);
}

bool Archive::AddFile(const std::string& filePath, UINT64& outGeneratedID)
{
    if (m_stream == nullptr || static_cast<File*>(m_stream)->GetMode() != Mode::WRITE)
        return false;

    m_stream->Seek(0, SEEK_END);
    UINT64 newFileOffset = m_stream->Seek(0, SEEK_CUR);

    std::string basename = GetBasename(filePath);
    std::string uniqueName = GetUniqueFilename(basename);
    UINT64 fileID = GenerateFileID(uniqueName);

    File sourceFile;
    if (!sourceFile.OpenRead(filePath))
        return false;

    UINT64 sourceSize = sourceFile.GetSize();
    Blob fileData;
    fileData.Reserve(sourceSize);
    UINT8 buffer[MAX_BUFFER_SIZE];
    UINT64 remaining = sourceSize;

    while (remaining > 0)
    {
        UINT64 chunkSize = (remaining > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : remaining;
        UINT64 bytesRead = sourceFile.Read(buffer, chunkSize, 1);
        if (bytesRead == 0)
            break;
        fileData.Append(buffer, bytesRead);
        remaining -= bytesRead;
    }
    sourceFile.Close();

    UINT32 checksum = SafeFormat::CalculateCRC32(fileData.GetData(), fileData.GetSize());

    UINT8 flags = FILE_ACTIVE;
    if (m_encryptionEnabled)
        flags |= FILE_ENCRYPTED;

    if (!WriteFileHeader(uniqueName, fileID, fileData.GetSize(), flags, checksum))
        return false;

    if (m_encryptionEnabled && !m_encryptionKey.empty())
    {
        File* fileStream = static_cast<File*>(m_stream);
        fileStream->EnableEncryption(true);
        fileStream->SetKey(m_encryptionKey);
    }

    m_stream->Write(fileData.GetData(), fileData.GetSize(), 1);

    if (m_encryptionEnabled)
    {
        File* fileStream = static_cast<File*>(m_stream);
        fileStream->EnableEncryption(false);
    }

    m_nameToOffset[uniqueName] = newFileOffset;
    m_idToOffset[fileID] = newFileOffset;
    m_header.fileCount++;

    m_stream->Seek(0, SEEK_SET);
    WriteArchiveHeader();
    WriteMaps();

    outGeneratedID = fileID;
    return true;
}

bool Archive::AddFile(const std::vector<std::string>& filePaths)
{
    if (m_stream == nullptr || static_cast<File*>(m_stream)->GetMode() != Mode::WRITE)
        return false;

    if (filePaths.empty())
        return false;

    for (const auto& filePath : filePaths)
    {
        m_stream->Seek(0, SEEK_END);
        UINT64 newFileOffset = m_stream->Seek(0, SEEK_CUR);

        std::string basename = GetBasename(filePath);
        std::string uniqueName = GetUniqueFilename(basename);
        UINT64 fileID = GenerateFileID(uniqueName);

        File sourceFile;
        if (!sourceFile.OpenRead(filePath))
            continue;  

        UINT64 sourceSize = sourceFile.GetSize();
        Blob fileData;
        fileData.Reserve(sourceSize);
        UINT8 buffer[MAX_BUFFER_SIZE];
        UINT64 remaining = sourceSize;

        while (remaining > 0)
        {
            UINT64 chunkSize = (remaining > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : remaining;
            UINT64 bytesRead = sourceFile.Read(buffer, chunkSize, 1);
            if (bytesRead == 0)
                break;
            fileData.Append(buffer, bytesRead);
            remaining -= bytesRead;
        }
        sourceFile.Close();

        UINT32 checksum = SafeFormat::CalculateCRC32(fileData.GetData(), fileData.GetSize());

        if (!WriteFileHeader(uniqueName, fileID, fileData.GetSize(), FILE_ACTIVE, checksum))
            continue;

        m_stream->Write(fileData.GetData(), fileData.GetSize(), 1);

        m_nameToOffset[uniqueName] = newFileOffset;
        m_idToOffset[fileID] = newFileOffset;
        m_header.fileCount++;
    }

    m_stream->Seek(0, SEEK_SET);
    WriteArchiveHeader();
    WriteMaps();

    return true;
}

bool Archive::RemoveFile(UINT64 fileID)
{
    if (m_stream == nullptr || static_cast<File*>(m_stream)->GetMode() != Mode::WRITE)
        return false;

    auto it = m_idToOffset.find(fileID);
    if (it == m_idToOffset.end())
        return false;

    UINT64 offset = it->second;

    FileHeader header;
    std::string filename;
    if (!ReadFileHeader(offset, header, filename))
        return false;

    header.flags &= ~FILE_ACTIVE;
    header.flags |= FILE_DELETED;

    m_stream->Seek(offset, SEEK_SET);
    m_stream->Write((const UINT8*)&header, sizeof(FileHeader), 1);

    m_nameToOffset.erase(filename);
    m_idToOffset.erase(fileID);
    m_header.fileCount--;

    m_stream->Seek(0, SEEK_SET);
    WriteArchiveHeader();
    WriteMaps();

    return true;
}

bool Archive::RemoveFileByName(const std::string& filename)
{
    if (m_stream == nullptr || static_cast<File*>(m_stream)->GetMode() != Mode::WRITE)
        return false;

    auto it = m_nameToOffset.find(filename);
    if (it == m_nameToOffset.end())
        return false;

    UINT64 offset = it->second;

    FileHeader header;
    std::string name;
    if (!ReadFileHeader(offset, header, name))
        return false;

    return RemoveFile(header.id);
}

bool Archive::RemoveAll()
{
    if (m_stream == nullptr || static_cast<File*>(m_stream)->GetMode() != Mode::WRITE)
        return false;

    if (m_nameToOffset.empty())
        return true;  

    int removedCount = 0;

    for (const auto& [name, offset] : m_nameToOffset)
    {
        FileHeader header;
        std::string filename;
        if (!ReadFileHeader(offset, header, filename))
            continue;

        if (!(header.flags & FILE_ACTIVE))
            continue;

        header.flags &= ~FILE_ACTIVE;
        header.flags |= FILE_DELETED;

        m_stream->Seek(offset, SEEK_SET);
        m_stream->Write((UINT8*)&header, sizeof(FileHeader), 1);

        removedCount++;
    }

    m_stream->Seek(0, SEEK_SET);
    WriteArchiveHeader();

    WriteMaps();

    std::cout << removedCount << " file(s) soft-deleted\n";

    if (!Compact())
    {
        std::cerr << "[ERROR] Auto-compact failed\n";
        return false;
    }


    return true;
}

bool Archive::RenameFile(UINT64 fileID, const std::string& newName)
{
    if (m_stream == nullptr || static_cast<File*>(m_stream)->GetMode() != Mode::WRITE)
        return false;

    if (newName.length() >= 256)
    {
        std::cerr << "[ERROR] Filename too long (max 255 chars)\n";
        return false;
    }

    auto it = m_idToOffset.find(fileID);
    if (it == m_idToOffset.end())
        return false;

    UINT64 offset = it->second;

    FileHeader header;
    std::string oldName;
    if (!ReadFileHeader(offset, header, oldName))
        return false;

    memset(header.filename, 0, 256);
    strncpy_s(header.filename, newName.c_str(), 255);

    m_stream->Seek(offset, SEEK_SET);
    m_stream->Write((UINT8*)&header, sizeof(FileHeader), 1);

    m_nameToOffset.erase(oldName);
    m_nameToOffset[newName] = offset;

    m_stream->Seek(sizeof(ArchiveHeader), SEEK_SET);
    WriteMaps();

    return true;
}

bool Archive::RenameFileByName(const std::string& oldName, const std::string& newName)
{
    if (m_stream == nullptr || static_cast<File*>(m_stream)->GetMode() != Mode::WRITE)
        return false;

    auto it = m_nameToOffset.find(oldName);
    if (it == m_nameToOffset.end())
        return false;

    UINT64 offset = it->second;

    FileHeader header;
    std::string name;
    if (!ReadFileHeader(offset, header, name))
        return false;

    return RenameFile(header.id, newName);
}

bool Archive::Compact()
{
    m_stream->Close();

    File oldArchive;
    if (!oldArchive.OpenRead(m_archivePath))
        return false;

    ArchiveHeader oldHeader;
    UINT64 bytesRead = oldArchive.Read((UINT8*)&oldHeader, sizeof(ArchiveHeader), 1);
    if (bytesRead != sizeof(ArchiveHeader))
    {
        oldArchive.Close();
        return false;
    }

    std::string tempPath = m_archivePath + ".tmp";
    File newArchive;
    if (!newArchive.OpenWrite(tempPath))
    {
        oldArchive.Close();
        return false;
    }

    ArchiveHeader newHeader;
    newHeader.magic[0] = 'A';
    newHeader.magic[1] = 'S';
    newHeader.magic[2] = 'E';
    newHeader.magic[3] = 'T';
    newHeader.version = 4;
    newHeader.fileCount = 0; 
    newHeader.dataOffset = 0;

    newArchive.Write((const UINT8*)&newHeader, sizeof(ArchiveHeader), 1);

    UINT64 estimatedMapSize = m_header.fileCount * sizeof(MapEntry) * 2;
    newArchive.Seek(estimatedMapSize, SEEK_CUR);

    std::unordered_map<std::string, UINT64> newNameToOffset;
    std::unordered_map<UINT64, UINT64> newIdToOffset;
    int skippedCorrupted = 0;
    int skippedDeleted = 0;

    for (const auto& [name, oldOffset] : m_nameToOffset)
    {
        UINT64 newOffset = newArchive.Seek(0, SEEK_CUR);

        FileHeader header;
        std::string filename;
        if (!ReadFileHeader(oldArchive, oldOffset, header, filename))
            continue;

        if (!(header.flags & FILE_ACTIVE))
        {
            skippedDeleted++;
            continue;
        }

        UINT64 blobOffset = oldOffset + sizeof(FileHeader);
        UINT64 blobSize = header.dataSize;

        oldArchive.Seek(blobOffset, SEEK_SET);

        Blob blob;
        blob.Reserve(blobSize);
        UINT8 buffer[MAX_BUFFER_SIZE];
        UINT64 remaining = blobSize;

        while (remaining > 0)
        {
            UINT64 chunkSize = (remaining > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : remaining;
            UINT64 bytesRead = oldArchive.Read(buffer, chunkSize, 1);
            if (bytesRead == 0)
                break;
            blob.Append(buffer, bytesRead);
            remaining -= bytesRead;
        }

        UINT32 calculatedCRC = SafeFormat::CalculateCRC32(blob.GetData(), blob.GetSize());
        if (calculatedCRC != header.checksum)
        {
            std::cout << "[SKIP] " << filename << " (CRC32 mismatch)\n";
            skippedCorrupted++;
            continue;
        }

        newArchive.Write((UINT8*)&header, sizeof(FileHeader), 1);
        newArchive.Write(blob.GetData(), blob.GetSize(), 1);

        newNameToOffset[name] = newOffset;
        newIdToOffset[header.id] = newOffset;
        newHeader.fileCount++; 
    }

    if (skippedDeleted > 0)
        std::cout << skippedDeleted << " deleted file(s) removed during compact\n";

    if (skippedCorrupted > 0)
        std::cout <<  skippedCorrupted << " corrupted file(s) skipped during compact\n";

    UINT64 finalMapSize = newHeader.fileCount * sizeof(MapEntry) * 2;
    newHeader.dataOffset = sizeof(ArchiveHeader) + finalMapSize;

    newArchive.Seek(sizeof(ArchiveHeader), SEEK_SET);

    for (const auto& [name, offset] : newNameToOffset)
    {
        MapEntry entry = {};
        strncpy_s(entry.key, name.c_str(), 255);
        entry.offset = offset;
        newArchive.Write((UINT8*)&entry, sizeof(MapEntry), 1);
    }

    for (const auto& [id, offset] : newIdToOffset)
    {
        MapEntry entry = {};
        snprintf(entry.key, 256, "%llu", id);
        entry.offset = offset;
        newArchive.Write((UINT8*)&entry, sizeof(MapEntry), 1);
    }

    newArchive.Seek(0, SEEK_SET);
    newArchive.Write((UINT8*)&newHeader, sizeof(ArchiveHeader), 1);

    oldArchive.Close();
    newArchive.Close();

    remove(m_archivePath.c_str());
    rename(tempPath.c_str(), m_archivePath.c_str());

    if (m_stream != nullptr && static_cast<File*>(m_stream)->GetMode() == Mode::READ)
    {
        if (!static_cast<File*>(m_stream)->OpenRead(m_archivePath))
            return false;
    }
    else if (m_stream != nullptr && static_cast<File*>(m_stream)->GetMode() == Mode::WRITE)
    {
        if (!static_cast<File*>(m_stream)->OpenWrite(m_archivePath))
            return false;
    }

    if (!ReadArchiveHeader())
        return false;

    if (!ReadMaps())
        return false;

    return true;
}

void Archive::EnableEncryption(bool enable)
{
    m_encryptionEnabled = enable;
}

void Archive::SetEncryptionKey(const std::string& key)
{
    m_encryptionKey = key;
}

bool Archive::IsEncryptionEnabled() const
{
    return m_encryptionEnabled;
}
