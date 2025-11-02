#include <pch.h>

// ============================================================================
// TOGGLE MODE: Uncomment ONE of the following defines
// ============================================================================

#define MODE_CLI       // CLI Production (default)
// #define MODE_TESTS  // Unit Tests (Memory + Archive)

// ============================================================================

#ifdef MODE_CLI

// ============================================================================
// CLI MODE - PRODUCTION
// ============================================================================

using namespace DebugUtils;
namespace fs = std::filesystem;

std::vector<std::string> CollectFilesFromDirectory(const std::string& dirPath)
{
    std::vector<std::string> files;

    if (!fs::exists(dirPath))
    {
        std::cerr << "[ERROR] Path does not exist: " << dirPath << "\n";
        return files;
    }

    if (!fs::is_directory(dirPath))
    {
        std::cerr << "[ERROR] Not a directory: " << dirPath << "\n";
        return files;
    }

    try
    {
        for (const auto& entry : fs::directory_iterator(dirPath))
        {
            if (entry.is_regular_file())
            {
                files.push_back(entry.path().string());
            }
        }
    }
    catch (const fs::filesystem_error& e)
    {
        std::cerr << "[ERROR] Failed to read directory: " << e.what() << "\n";
    }

    return files;
}

void PrintUsage()
{
    std::cout << "========================================\n";
    std::cout << "Asset Engine - CLI v1.0\n";
    std::cout << "========================================\n\n";

    std::cout << "Usage:\n";
    std::cout << "  AssetEngine.exe <command> [arguments]\n";
    std::cout << "  AssetEngine.exe -help | --help | help\n\n";

    std::cout << "Commands:\n";
    std::cout << "  create <archive> <file1> [file2] ...    Create new archive from files\n";
    std::cout << "  add <archive> <file1> [file2] ...       Add files to existing archive\n";
    std::cout << "  list <archive>                          Display archive contents\n";
    std::cout << "  extract <archive> <filename> <output>   Extract specific file\n";
    std::cout << "  extractall <archive> <outputdir>        Extract all files\n";
    std::cout << "  validate <archive>                      Verify archive integrity (CRC32)\n";
    std::cout << "  remove <archive> <filename>             Remove file (soft delete)\n";
    std::cout << "  removeall <archive>                     Remove all files (empty archive)\n";
    std::cout << "  rename <archive> <oldname> <newname>    Rename file in archive\n";
    std::cout << "  compact <archive>                       Compact archive (reclaim space)\n\n";

    std::cout << "Examples:\n";
    std::cout << "  AssetEngine.exe create game.asset textures/*.png sounds/*.wav\n";
    std::cout << "  AssetEngine.exe list game.asset\n";
    std::cout << "  AssetEngine.exe validate game.asset\n";
    std::cout << "  AssetEngine.exe extract game.asset logo.png extracted_logo.png\n";
    std::cout << "  AssetEngine.exe extractall game.asset output_folder\n\n";
}

int main(int argc, const char* argv[])
{
    std::ios_base::sync_with_stdio(false);
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);

    if (argc < 2)
    {
        PrintUsage();
        return 1;
    }

    std::string command = argv[1];

    // HELP
    if (command == "-help" || command == "--help" || command == "help")
    {
        PrintUsage();
        std::cout.flush();
        return 0;
    }

    // CREATE
    if (command == "create")
    {
        if (argc < 3)
        {
            std::cerr << "[ERROR] Usage: create <archive> [file1|dir] ...\n";
            std::cerr << "[HELP] Examples:\n";
            std::cerr << "  create game.asset file1.txt file2.txt\n";
            std::cerr << "  create game.asset assets/\n";
            std::cerr << "  create game.asset assets/ config.txt\n";
            return 1;
        }

        std::string archivePath = argv[2];
        std::vector<std::string> filePaths;

        for (int i = 3; i < argc; i++)
        {
            std::string arg = argv[i];

            if (fs::is_directory(arg))
            {
                auto dirFiles = CollectFilesFromDirectory(arg);
                if (!dirFiles.empty())
                {
                    filePaths.insert(filePaths.end(), dirFiles.begin(), dirFiles.end());
                    std::cout << "[INFO] Collected " << dirFiles.size()
                              << " files from directory: " << arg << "\n";
                }
                else
                {
                    std::cerr << "[WARNING] Directory is empty or unreadable: " << arg << "\n";
                }
            }
            else if (fs::exists(arg))
            {
                filePaths.push_back(arg);
            }
            else
            {
                std::cerr << "[WARNING] File not found (skipped): " << arg << "\n";
            }
        }

        Archive archive;
        if (!archive.Create(filePaths))
        {
            std::cerr << "[ERROR] Failed to create archive\n";
            return 1;
        }

        remove(archivePath.c_str());
        rename("temp_archive.asset", archivePath.c_str());

        if (filePaths.empty())
        {
            std::cout << "[OK] Empty archive created: " << archivePath << "\n";
        }
        else
        {
            std::cout << "[OK] Archive created: " << archivePath
                      << " (" << filePaths.size() << " files)\n";
        }

        std::cout.flush();
        std::cerr.flush();

        return 0;
    }

    // ADD
    if (command == "add")
    {
        if (argc < 4)
        {
            std::cerr << "[ERROR] Usage: add <archive> <file1> [file2] ...\n";
            return 1;
        }

        std::string archivePath = argv[2];
        std::vector<std::string> filePaths;

        for (int i = 3; i < argc; i++)
            filePaths.push_back(argv[i]);

        Archive archive;
        if (!archive.Open(archivePath, Mode::WRITE))
        {
            std::cerr << "[ERROR] Failed to open archive: " << archivePath << "\n";
            return 1;
        }

        for (const auto& file : filePaths)
        {
            if (!archive.AddFile(file))
            {
                std::cerr << "[ERROR] Failed to add file: " << file << "\n";
                archive.Close();
                return 1;
            }
            std::cout << "[OK] Added: " << file << "\n";
        }

        archive.Close();
        std::cout << "[OK] All files added\n";
        std::cout.flush();
        return 0;
    }

    // REMOVE
    if (command == "remove")
    {
        if (argc < 4)
        {
            std::cerr << "[ERROR] Usage: remove <archive> <filename>\n";
            return 1;
        }

        std::string archivePath = argv[2];
        std::string filename = argv[3];

        Archive archive;
        if (!archive.Open(archivePath, Mode::WRITE))
        {
            std::cerr << "[ERROR] Failed to open archive: " << archivePath << "\n";
            return 1;
        }

        if (!archive.RemoveFileByName(filename))
        {
            std::cerr << "[ERROR] Failed to remove file: " << filename << "\n";
            archive.Close();
            return 1;
        }

        archive.Close();
        std::cout << "[OK] File removed (soft delete): " << filename << "\n";
        std::cout.flush();
        return 0;
    }

    // REMOVEALL
    if (command == "removeall")
    {
        if (argc < 3)
        {
            std::cerr << "[ERROR] Usage: removeall <archive>\n";
            return 1;
        }

        std::string archivePath = argv[2];

        Archive archive;
        if (!archive.Open(archivePath, Mode::WRITE))
        {
            std::cerr << "[ERROR] Failed to open archive: " << archivePath << "\n";
            return 1;
        }

        if (!archive.RemoveAll())
        {
            std::cerr << "[ERROR] Failed to remove all files\n";
            archive.Close();
            return 1;
        }

        archive.Close();
        std::cout << "[OK] Archive emptied\n";
        return 0;
    }

    // RENAME
    if (command == "rename")
    {
        if (argc < 5)
        {
            std::cerr << "[ERROR] Usage: rename <archive> <oldname> <newname>\n";
            return 1;
        }

        std::string archivePath = argv[2];
        std::string oldName = argv[3];
        std::string newName = argv[4];

        Archive archive;
        if (!archive.Open(archivePath, Mode::WRITE))
        {
            std::cerr << "[ERROR] Failed to open archive: " << archivePath << "\n";
            return 1;
        }

        if (!archive.RenameFileByName(oldName, newName))
        {
            std::cerr << "[ERROR] Failed to rename file\n";
            archive.Close();
            return 1;
        }

        archive.Close();
        std::cout << "[OK] File renamed: " << oldName << " -> " << newName << "\n";
        return 0;
    }

    // COMPACT
    if (command == "compact")
    {
        if (argc < 3)
        {
            std::cerr << "[ERROR] Usage: compact <archive>\n";
            return 1;
        }

        std::string archivePath = argv[2];

        Archive archive;
        if (!archive.Open(archivePath, Mode::WRITE))
        {
            std::cerr << "[ERROR] Failed to open archive: " << archivePath << "\n";
            return 1;
        }

        if (!archive.Compact())
        {
            std::cerr << "[ERROR] Failed to compact archive\n";
            archive.Close();
            return 1;
        }

        archive.Close();
        std::cout << "[OK] Archive compacted\n";
        return 0;
    }

    // LIST
    if (command == "list")
    {
        if (argc < 3)
        {
            std::cerr << "[ERROR] Usage: list <archive>\n";
            return 1;
        }

        std::string archivePath = argv[2];

        Archive archive;
        if (!archive.Open(archivePath, Mode::READ))
        {
            std::cerr << "[ERROR] Failed to open archive: " << archivePath << "\n";
            return 1;
        }

        archive.List();
        archive.Close();
        std::cout.flush();
        return 0;
    }

    // VALIDATE
    if (command == "validate")
    {
        if (argc < 3)
        {
            std::cerr << "[ERROR] Usage: validate <archive>\n";
            return 1;
        }

        std::string archivePath = argv[2];

        Archive archive;
        if (!archive.Open(archivePath, Mode::READ))
        {
            std::cerr << "[ERROR] Failed to open archive: " << archivePath << "\n";
            return 1;
        }

        if (!archive.Validate())
        {
            std::cerr << "[ERROR] Archive validation failed\n";
            archive.Close();
            return 1;
        }

        archive.Close();
        std::cout << "[OK] Archive validated (CRC32 OK)\n";
        std::cout.flush();
        return 0;
    }

    // EXTRACT
    if (command == "extract")
    {
        if (argc < 5)
        {
            std::cerr << "[ERROR] Usage: extract <archive> <filename> <output>\n";
            return 1;
        }

        std::string archivePath = argv[2];
        std::string filename = argv[3];
        std::string outputPath = argv[4];

        Archive archive;
        if (!archive.Open(archivePath, Mode::READ))
        {
            std::cerr << "[ERROR] Failed to open archive: " << archivePath << "\n";
            return 1;
        }

        if (!archive.ExtractByName(filename, outputPath))
        {
            std::cerr << "[ERROR] Failed to extract file: " << filename << "\n";
            archive.Close();
            return 1;
        }

        archive.Close();
        std::cout << "[OK] Extracted: " << filename << " -> " << outputPath << "\n";
        std::cout.flush();
        return 0;
    }

    // EXTRACTALL
    if (command == "extractall")
    {
        if (argc < 4)
        {
            std::cerr << "[ERROR] Usage: extractall <archive> <outputdir>\n";
            return 1;
        }

        std::string archivePath = argv[2];
        std::string outputDir = argv[3];

        Archive archive;
        if (!archive.Open(archivePath, Mode::READ))
        {
            std::cerr << "[ERROR] Failed to open archive: " << archivePath << "\n";
            return 1;
        }

        if (!archive.ExtractAll(outputDir))
        {
            std::cerr << "[ERROR] Failed to extract all files\n";
            archive.Close();
            return 1;
        }

        archive.Close();
        std::cout << "[OK] All files extracted to: " << outputDir << "\n";
        std::cout.flush();
        return 0;
    }

    // Unknown command
    std::cerr << "[ERROR] Unknown command: " << command << "\n\n";
    std::cerr << "Try 'AssetEngine.exe -help' for more information.\n\n";
    return 1;
}

#endif // MODE_CLI

// ============================================================================

#ifdef MODE_TESTS

// ============================================================================
// TEST MODE - UNIT TESTS (Memory + Archive)
// ============================================================================

using namespace DebugUtils;

// ============================================================================
// MEMORY TESTS
// ============================================================================

void Test1_Memory_Basic_Text()
{
    PrintTitle("Test 1A: Memory Basic (TEXT)");

    Memory mem;

    const char* text = "Hello AssetsEngine!";
    mem.Write((const UINT8*)text, strlen(text) + 1);

    PrintInfo("After Write:");
    std::cout << "  Position: " << mem.GetPosition() << "\n";
    std::cout << "  Blob size: " << mem.GetBlob()->GetSize() << "\n";
    std::cout << "  Content: " << mem.GetBlob()->GetData() << "\n";

    mem.Seek(0, SEEK_SET);
    UINT8 buffer[64] = { 0 };
    UINT64 bytesRead = mem.Read(buffer, strlen(text) + 1);

    std::cout << "  Bytes read: " << bytesRead << "\n";
    std::cout << "  Read content: " << buffer << "\n";

    mem.Seek(6, SEEK_SET);
    UINT8 buffer2[64] = { 0 };
    mem.Read(buffer2, 13);
    std::cout << "  After seek(6): " << buffer2 << "\n";

    PrintSuccess("Test 1A PASSED\n");
}

void Test1_Memory_Basic_Binary()
{
    PrintTitle("Test 1B: Memory Basic (BINARY)");

    Memory mem;

    UINT8 binaryData[32];
    GeneratePatternData(binaryData, 32);

    mem.Write(binaryData, 32);

    PrintInfo("After Write:");
    std::cout << "  Position: " << mem.GetPosition() << "\n";
    std::cout << "  Blob size: " << mem.GetBlob()->GetSize() << "\n";
    std::cout << "  Content (hex):\n";
    PrintAsHex(mem);

    mem.Seek(0, SEEK_SET);
    UINT8 buffer[32] = { 0 };
    UINT64 bytesRead = mem.Read(buffer, 32);

    std::cout << "  Bytes read: " << bytesRead << "\n";
    std::cout << "  Read content (hex):\n";
    PrintAsHex(buffer, 32);

    if (CompareData(binaryData, buffer, 32))
        PrintSuccess("Data matches!");
    else
        PrintError("Data mismatch!");

    PrintSuccess("Test 1B PASSED\n");
}

// ============================================================================
void Test2_Memory_ExternalBlob_Text()
{
    PrintTitle("Test 2A: Memory External Blob (TEXT)");

    const char* data = "External Data";
    Blob externalBlob((const UINT8*)data, strlen(data) + 1);

    std::cout << "External Blob size: " << externalBlob.GetSize() << "\n";

    Memory mem(&externalBlob);

    UINT8 buffer[64] = { 0 };
    mem.Read(buffer, strlen(data) + 1);
    std::cout << "Read from Memory: " << buffer << "\n";

    mem.Seek(0, SEEK_SET);
    const char* modified = "MODIFIED!!!!";
    mem.SetWriteMode(Memory::WriteMode::TRUNCATE);
    mem.Write((const UINT8*)modified, strlen(modified) + 1);

    std::cout << "External Blob after write: " << externalBlob.GetData() << "\n";
    std::cout << "External Blob size: " << externalBlob.GetSize() << "\n";

    PrintSuccess("Test 2A PASSED\n");
}

void Test2_Memory_ExternalBlob_Binary()
{
    PrintTitle("Test 2B: Memory External Blob (BINARY)");

    UINT8 binaryData[16];
    GeneratePatternData(binaryData, 16, 0x10);
    Blob externalBlob(binaryData, 16);

    PrintInfo("Initial Blob:");
    std::cout << "  Size: " << externalBlob.GetSize() << "\n";
    PrintAsHex(externalBlob);

    Memory mem(&externalBlob);

    UINT8 buffer[16] = { 0 };
    mem.Read(buffer, 16);
    PrintInfo("Read from Memory:");
    PrintAsHex(buffer, 16);

    mem.Seek(0, SEEK_SET);
    UINT8 newPattern[12];
    GeneratePatternData(newPattern, 12, 0xFF);

    mem.SetWriteMode(Memory::WriteMode::TRUNCATE);
    mem.Write(newPattern, 12);

    PrintInfo("External Blob after write:");
    std::cout << "  Size: " << externalBlob.GetSize() << "\n";
    PrintAsHex(externalBlob);

    PrintSuccess("Test 2B PASSED\n");
}

// ============================================================================
void Test3_Memory_To_File_Text()
{
    PrintTitle("Test 3A: Memory -> File (TEXT)");

    Memory mem;
    const char* data = "This is my asset data in RAM!";
    mem.Write((const UINT8*)data, strlen(data) + 1);

    PrintInfo("Data in Memory:");
    std::cout << "  " << mem.GetBlob()->GetData() << "\n";
    std::cout << "  Size: " << mem.GetBlob()->GetSize() << " bytes\n";

    bool saved = SafeFormat::WriteSafeFile(
        "asset_text.safe",
        mem.GetBlob()->GetData(),
        mem.GetBlob()->GetSize()
    );

    if (saved)
        PrintSuccess("Saved to asset_text.safe");
    else
        PrintError("Failed to save");

    Stream::Header header;
    Blob loadedBlob;

    if (SafeFormat::Validate("asset_text.safe", header, loadedBlob))
    {
        PrintSuccess("File validated!");
        std::cout << "  Loaded data: " << loadedBlob.GetData() << "\n";
        std::cout << "  Size: " << loadedBlob.GetSize() << " bytes\n";
        std::cout << "  CRC32: 0x" << std::hex << header.checksum << std::dec << "\n";
    }
    else
    {
        PrintError("Validation failed");
    }

    PrintSuccess("Test 3A PASSED\n");
}

void Test3_Memory_To_File_Binary()
{
    PrintTitle("Test 3B: Memory -> File (BINARY)");

    Memory mem;

    Blob* fakeImage = CreateFakeImageBlob(16, 16);

    PrintInfo("Fake Image Created:");
    std::cout << "  Size: " << fakeImage->GetSize() << " bytes\n";
    std::cout << "  First 64 bytes:\n";
    HexDump(*fakeImage, 64);

    bool saved = SafeFormat::WriteSafeFile(
        "asset_image.safe",
        fakeImage->GetData(),
        fakeImage->GetSize()
    );

    if (saved)
        PrintSuccess("Saved to asset_image.safe");
    else
        PrintError("Failed to save");

    Stream::Header header;
    Blob loadedBlob;

    if (SafeFormat::Validate("asset_image.safe", header, loadedBlob))
    {
        PrintSuccess("File validated!");
        std::cout << "  Loaded size: " << loadedBlob.GetSize() << " bytes\n";
        std::cout << "  CRC32: 0x" << std::hex << header.checksum << std::dec << "\n";

        if (CompareBlobs(*fakeImage, loadedBlob))
            PrintSuccess("Data integrity: OK");
        else
            PrintError("Data integrity: FAILED");

        DataStats stats = AnalyzeData(loadedBlob.GetData(), loadedBlob.GetSize());
        PrintStats(stats);
    }
    else
    {
        PrintError("Validation failed");
    }

    delete fakeImage;
    PrintSuccess("Test 3B PASSED\n");
}

// ============================================================================
// ARCHIVE TESTS 
// ============================================================================

void Test4_Archive_Create_Empty()
{
    PrintTitle("Test 4: Archive Create (EMPTY)");

    std::vector<std::string> files = {}; 
    Archive arc;

    if (!arc.Create(files))
    {
        PrintError("Failed to create empty archive");
        return;
    }

    PrintSuccess("Empty archive created: temp_archive.asset");

    remove("test_empty.asset");
    rename("temp_archive.asset", "test_empty.asset");

    if (!arc.Open("test_empty.asset", Mode::READ))
    {
        PrintError("Failed to open empty archive");
        return;
    }

    arc.List();
    arc.Close();

    PrintSuccess("Test 4 PASSED\n");
}

void Test5_Archive_Create_Files()
{
    PrintTitle("Test 5: Archive Create (WITH FILES)");

    File f1, f2;
    f1.OpenWrite("test_file1.txt");
    const char* content1 = "Test content 1";
    f1.Write((const UINT8*)content1, strlen(content1) + 1, 1);
    f1.Close();

    f2.OpenWrite("test_file2.txt");
    const char* content2 = "Test content 2 longer";
    f2.Write((const UINT8*)content2, strlen(content2) + 1, 1);
    f2.Close();

    std::vector<std::string> files = { "test_file1.txt", "test_file2.txt" };
    Archive arc;

    if (!arc.Create(files))
    {
        PrintError("Failed to create archive");
        return;
    }

    PrintSuccess("Archive created: temp_archive.asset");

    remove("test_create.asset");
    rename("temp_archive.asset", "test_create.asset");

    if (!arc.Open("test_create.asset", Mode::READ))
    {
        PrintError("Failed to open archive");
        return;
    }

    arc.List();
    arc.Close();

    PrintSuccess("Test 5 PASSED\n");
}

void Test6_Archive_Add_Files()
{
    PrintTitle("Test 6: Archive Add Files");

    File f1;
    f1.OpenWrite("base_file.txt");
    const char* content = "Base content";
    f1.Write((const UINT8*)content, strlen(content) + 1, 1);
    f1.Close();

    std::vector<std::string> files = { "base_file.txt" };
    Archive arc;
    arc.Create(files);
    remove("test_add.asset");
    rename("temp_archive.asset", "test_add.asset");

    File f2, f3;
    f2.OpenWrite("added_file1.txt");
    const char* content2 = "Added content 1";
    f2.Write((const UINT8*)content2, strlen(content2) + 1, 1);
    f2.Close();

    f3.OpenWrite("added_file2.txt");
    const char* content3 = "Added content 2";
    f3.Write((const UINT8*)content3, strlen(content3) + 1, 1);
    f3.Close();

    if (!arc.Open("test_add.asset", Mode::WRITE))
    {
        PrintError("Failed to open archive");
        return;
    }

    if (!arc.AddFile("added_file1.txt"))
    {
        PrintError("Failed to add file 1");
        arc.Close();
        return;
    }
    PrintSuccess("Added file 1");

    if (!arc.AddFile("added_file2.txt"))
    {
        PrintError("Failed to add file 2");
        arc.Close();
        return;
    }
    PrintSuccess("Added file 2");

    arc.Close();

    arc.Open("test_add.asset", Mode::READ);
    arc.List();
    arc.Close();

    PrintSuccess("Test 6 PASSED\n");
}

void Test7_Archive_Add_Duplicates()
{
    PrintTitle("Test 7: Archive Add Duplicates (Auto-Increment)");

    File f;
    f.OpenWrite("duplicate_test.txt");
    const char* content = "Duplicate content";
    f.Write((const UINT8*)content, strlen(content) + 1, 1);
    f.Close();

    std::vector<std::string> files = { "duplicate_test.txt" };
    Archive arc;
    arc.Create(files);
    remove("test_duplicates.asset");
    rename("temp_archive.asset", "test_duplicates.asset");

    arc.Open("test_duplicates.asset", Mode::WRITE);
    arc.AddFile("duplicate_test.txt");
    arc.AddFile("duplicate_test.txt");
    arc.AddFile("duplicate_test.txt");
    arc.Close();

    arc.Open("test_duplicates.asset", Mode::READ);
    arc.List();
    arc.Close();

    PrintSuccess("Test 7 PASSED\n");
}

void Test8_Archive_Remove_File()
{
    PrintTitle("Test 8: Archive Remove File (Soft Delete)");

    File f1, f2, f3;
    f1.OpenWrite("remove_file1.txt");
    f1.Write((const UINT8*)"Content 1", 10, 1);
    f1.Close();

    f2.OpenWrite("remove_file2.txt");
    f2.Write((const UINT8*)"Content 2", 10, 1);
    f2.Close();

    f3.OpenWrite("remove_file3.txt");
    f3.Write((const UINT8*)"Content 3", 10, 1);
    f3.Close();

    std::vector<std::string> files = { "remove_file1.txt", "remove_file2.txt", "remove_file3.txt" };
    Archive arc;
    arc.Create(files);
    remove("test_remove.asset");
    rename("temp_archive.asset", "test_remove.asset");

    PrintInfo("Before remove:");
    arc.Open("test_remove.asset", Mode::READ);
    arc.List();
    arc.Close();

    arc.Open("test_remove.asset", Mode::WRITE);
    if (!arc.RemoveFileByName("remove_file2.txt"))
    {
        PrintError("Failed to remove file");
        arc.Close();
        return;
    }
    PrintSuccess("File removed (soft delete)");
    arc.Close();

    PrintInfo("After remove:");
    arc.Open("test_remove.asset", Mode::READ);
    arc.List();
    arc.Close();

    PrintSuccess("Test 8 PASSED\n");
}

void Test9_Archive_Rename_File()
{
    PrintTitle("Test 9: Archive Rename File (INSTANTANEOUS v5.0)");

    File f1, f2;
    f1.OpenWrite("original_name1.txt");
    f1.Write((const UINT8*)"Content A", 10, 1);
    f1.Close();

    f2.OpenWrite("original_name2.txt");
    f2.Write((const UINT8*)"Content B", 10, 1);
    f2.Close();

    std::vector<std::string> files = { "original_name1.txt", "original_name2.txt" };
    Archive arc;
    arc.Create(files);
    remove("test_rename.asset");
    rename("temp_archive.asset", "test_rename.asset");

    PrintInfo("Before rename:");
    arc.Open("test_rename.asset", Mode::READ);
    arc.List();
    arc.Close();

    arc.Open("test_rename.asset", Mode::WRITE);

    if (!arc.RenameFileByName("original_name1.txt", "renamed_file1.txt"))
    {
        PrintError("Failed to rename file 1");
        arc.Close();
        return;
    }
    PrintSuccess("Renamed: original_name1.txt -> renamed_file1.txt");

    if (!arc.RenameFileByName("original_name2.txt", "new_name.txt"))
    {
        PrintError("Failed to rename file 2");
        arc.Close();
        return;
    }
    PrintSuccess("Renamed: original_name2.txt -> new_name.txt");

    arc.Close();

    PrintInfo("After rename:");
    arc.Open("test_rename.asset", Mode::READ);
    arc.List();
    arc.Close();

    PrintSuccess("Test 9 PASSED\n");
}

void Test10_Archive_Compact()
{
    PrintTitle("Test 10: Archive Compact (Remove Deleted Files)");

    File f1, f2, f3, f4;
    f1.OpenWrite("compact_file1.txt");
    f1.Write((const UINT8*)"Content 1", 10, 1);
    f1.Close();

    f2.OpenWrite("compact_file2.txt");
    f2.Write((const UINT8*)"Content 2", 10, 1);
    f2.Close();

    f3.OpenWrite("compact_file3.txt");
    f3.Write((const UINT8*)"Content 3", 10, 1);
    f3.Close();

    f4.OpenWrite("compact_file4.txt");
    f4.Write((const UINT8*)"Content 4", 10, 1);
    f4.Close();

    std::vector<std::string> files = { "compact_file1.txt", "compact_file2.txt", "compact_file3.txt", "compact_file4.txt" };
    Archive arc;
    arc.Create(files);
    remove("test_compact.asset");
    rename("temp_archive.asset", "test_compact.asset");

    arc.Open("test_compact.asset", Mode::WRITE);
    arc.RemoveFileByName("compact_file2.txt");
    arc.RemoveFileByName("compact_file4.txt");
    arc.Close();

    PrintInfo("Before compact (2 files soft-deleted):");
    arc.Open("test_compact.asset", Mode::READ);
    arc.List();
    arc.Close();

    File sizeCheck;
    sizeCheck.OpenRead("test_compact.asset");
    UINT64 sizeBefore = sizeCheck.GetSize();
    sizeCheck.Close();

    std::cout << "  Size before compact: " << sizeBefore << " bytes\n";

    arc.Open("test_compact.asset", Mode::WRITE);
    if (!arc.Compact())
    {
        PrintError("Failed to compact");
        arc.Close();
        return;
    }
    PrintSuccess("Archive compacted");
    arc.Close();

    sizeCheck.OpenRead("test_compact.asset");
    UINT64 sizeAfter = sizeCheck.GetSize();
    sizeCheck.Close();

    std::cout << "  Size after compact: " << sizeAfter << " bytes\n";
    std::cout << "  Space freed: " << (sizeBefore - sizeAfter) << " bytes\n";

    PrintInfo("After compact:");
    arc.Open("test_compact.asset", Mode::READ);
    arc.List();
    arc.Close();

    PrintSuccess("Test 10 PASSED\n");
}

void Test11_Archive_RemoveAll()
{
    PrintTitle("Test 11: Archive RemoveAll (Empty Archive + Auto-Compact)");

    File f1, f2, f3;
    f1.OpenWrite("removeall_file1.txt");
    f1.Write((const UINT8*)"Content 1", 10, 1);
    f1.Close();

    f2.OpenWrite("removeall_file2.txt");
    f2.Write((const UINT8*)"Content 2", 10, 1);
    f2.Close();

    f3.OpenWrite("removeall_file3.txt");
    f3.Write((const UINT8*)"Content 3", 10, 1);
    f3.Close();

    std::vector<std::string> files = { "removeall_file1.txt", "removeall_file2.txt", "removeall_file3.txt" };
    Archive arc;
    arc.Create(files);
    remove("test_removeall.asset");
    rename("temp_archive.asset", "test_removeall.asset");

    PrintInfo("Before removeall:");
    arc.Open("test_removeall.asset", Mode::READ);
    arc.List();
    arc.Close();

    arc.Open("test_removeall.asset", Mode::WRITE);
    if (!arc.RemoveAll())
    {
        PrintError("Failed to removeall");
        arc.Close();
        return;
    }
    arc.Close();

    PrintInfo("After removeall:");
    arc.Open("test_removeall.asset", Mode::READ);
    arc.List();
    arc.Close();

    PrintSuccess("Test 11 PASSED\n");
}

void Test12_Archive_Validate()
{
    PrintTitle("Test 12: Archive Validate (CRC32)");

    File f1;
    f1.OpenWrite("validate_file.txt");
    const char* content = "Content to validate";
    f1.Write((const UINT8*)content, strlen(content) + 1, 1);
    f1.Close();

    std::vector<std::string> files = { "validate_file.txt" };
    Archive arc;
    arc.Create(files);
    remove("test_validate.asset");
    rename("temp_archive.asset", "test_validate.asset");

    arc.Open("test_validate.asset", Mode::READ);

    if (!arc.Validate())
    {
        PrintError("Validation failed");
        arc.Close();
        return;
    }

    PrintSuccess("Archive validated (CRC32 OK)");
    arc.Close();

    PrintSuccess("Test 12 PASSED\n");
}

void Test13_Archive_Extract()
{
    PrintTitle("Test 13: Archive Extract Files");

    File f1, f2;
    f1.OpenWrite("extract_source1.txt");
    const char* content1 = "Extract content 1";
    f1.Write((const UINT8*)content1, strlen(content1) + 1, 1);
    f1.Close();

    f2.OpenWrite("extract_source2.txt");
    const char* content2 = "Extract content 2";
    f2.Write((const UINT8*)content2, strlen(content2) + 1, 1);
    f2.Close();

    std::vector<std::string> files = { "extract_source1.txt", "extract_source2.txt" };
    Archive arc;
    arc.Create(files);
    remove("test_extract.asset");
    rename("temp_archive.asset", "test_extract.asset");

    arc.Open("test_extract.asset", Mode::READ);

    if (!arc.ExtractByName("extract_source1.txt", "extracted_output1.txt"))
    {
        PrintError("Failed to extract file 1");
        arc.Close();
        return;
    }
    PrintSuccess("Extracted: extract_source1.txt -> extracted_output1.txt");

    if (!arc.ExtractByName("extract_source2.txt", "extracted_output2.txt"))
    {
        PrintError("Failed to extract file 2");
        arc.Close();
        return;
    }
    PrintSuccess("Extracted: extract_source2.txt -> extracted_output2.txt");

    arc.Close();

    File check1, check2;
    check1.OpenRead("extracted_output1.txt");
    UINT8 buffer1[64] = { 0 };
    check1.Read(buffer1, 64, 1);
    check1.Close();

    check2.OpenRead("extracted_output2.txt");
    UINT8 buffer2[64] = { 0 };
    check2.Read(buffer2, 64, 1);
    check2.Close();

    std::cout << "  Extracted file 1: " << buffer1 << "\n";
    std::cout << "  Extracted file 2: " << buffer2 << "\n";

    PrintSuccess("Test 13 PASSED\n");
}

void Test14_Archive_ExtractAll()
{
    PrintTitle("Test 14: Archive ExtractAll");

    File f1, f2, f3;
    f1.OpenWrite("extractall_file1.txt");
    f1.Write((const UINT8*)"Content 1", 10, 1);
    f1.Close();

    f2.OpenWrite("extractall_file2.txt");
    f2.Write((const UINT8*)"Content 2", 10, 1);
    f2.Close();

    f3.OpenWrite("extractall_file3.txt");
    f3.Write((const UINT8*)"Content 3", 10, 1);
    f3.Close();

    std::vector<std::string> files = { "extractall_file1.txt", "extractall_file2.txt", "extractall_file3.txt" };
    Archive arc;
    arc.Create(files);
    remove("test_extractall.asset");
    rename("temp_archive.asset", "test_extractall.asset");

    arc.Open("test_extractall.asset", Mode::READ);

    if (!arc.ExtractAll("extracted_all_output"))
    {
        PrintError("Failed to extract all");
        arc.Close();
        return;
    }

    PrintSuccess("All files extracted to: extracted_all_output/");
    arc.Close();

    PrintSuccess("Test 14 PASSED\n");
}

void Test15_Archive_Encryption()
{
    PrintTitle("Test 15: Archive Encryption (XOR)");

    File f1, f2;
    f1.OpenWrite("encrypt_source1.txt");
    const char* content1 = "Top Secret Data";
    f1.Write((const UINT8*)content1, strlen(content1) + 1, 1);
    f1.Close();

    f2.OpenWrite("encrypt_source2.txt");
    const char* content2 = "Confidential Information";
    f2.Write((const UINT8*)content2, strlen(content2) + 1, 1);
    f2.Close();

    std::vector<std::string> files = { "encrypt_source1.txt", "encrypt_source2.txt" };
    Archive arc;

    arc.EnableEncryption(true);
    arc.SetEncryptionKey("MySecretKey123");

    if (!arc.Create(files))
    {
        PrintError("Failed to create encrypted archive");
        return;
    }

    remove("test_encrypted.asset");
    rename("temp_archive.asset", "test_encrypted.asset");

    PrintSuccess("Encrypted archive created");

    arc.SetEncryptionKey("MySecretKey123");
    arc.EnableEncryption(true);
    arc.Open("test_encrypted.asset", Mode::READ);

    if (!arc.ExtractByName("encrypt_source1.txt", "decrypted_output1.txt"))
    {
        PrintError("Failed to extract encrypted file 1");
        arc.Close();
        return;
    }
    PrintSuccess("Extracted (decrypted): encrypt_source1.txt -> decrypted_output1.txt");

    if (!arc.ExtractByName("encrypt_source2.txt", "decrypted_output2.txt"))
    {
        PrintError("Failed to extract encrypted file 2");
        arc.Close();
        return;
    }
    PrintSuccess("Extracted (decrypted): encrypt_source2.txt -> decrypted_output2.txt");

    arc.Close();

    File check1, check2;
    check1.OpenRead("decrypted_output1.txt");
    UINT8 buffer1[64] = { 0 };
    check1.Read(buffer1, 64, 1);
    check1.Close();

    check2.OpenRead("decrypted_output2.txt");
    UINT8 buffer2[64] = { 0 };
    check2.Read(buffer2, 64, 1);
    check2.Close();

    std::cout << "  Decrypted file 1: " << buffer1 << "\n";
    std::cout << "  Decrypted file 2: " << buffer2 << "\n";

    if (strcmp((const char*)buffer1, content1) == 0)
        PrintSuccess("File 1 decryption: OK");
    else
        PrintError("File 1 decryption: FAILED");

    if (strcmp((const char*)buffer2, content2) == 0)
        PrintSuccess("File 2 decryption: OK");
    else
        PrintError("File 2 decryption: FAILED");

    PrintSuccess("Test 15 PASSED\n");
}

// ============================================================================
// MAIN - TEST RUNNER
// ============================================================================

int main(int argc, char* argv[])
{
    std::cout << "========================================\n";
    std::cout << "Asset Engine - Complete Test Suite\n";
    std::cout << "========================================\n\n";

    // Run ALL tests
    try
    {
        // Memory Tests
        Test1_Memory_Basic_Text();
        Test1_Memory_Basic_Binary();
        Test2_Memory_ExternalBlob_Text();
        Test2_Memory_ExternalBlob_Binary();
        Test3_Memory_To_File_Text();
        Test3_Memory_To_File_Binary();

        // Archive Tests
        Test4_Archive_Create_Empty();
        Test5_Archive_Create_Files();
        Test6_Archive_Add_Files();
        Test7_Archive_Add_Duplicates();
        Test8_Archive_Remove_File();
        Test9_Archive_Rename_File();
        Test10_Archive_Compact();
        Test11_Archive_RemoveAll();
        Test12_Archive_Validate();
        Test13_Archive_Extract();
        Test14_Archive_ExtractAll();
        Test15_Archive_Encryption();

        std::cout << "\n========================================\n";
        std::cout << "ALL TESTS PASSED!\n";
        std::cout << "========================================\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "\n[EXCEPTION] " << e.what() << "\n";
        return 1;
    }

    return 0;
}

#endif // MODE_TESTS
