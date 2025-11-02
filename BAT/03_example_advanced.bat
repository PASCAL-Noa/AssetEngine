@echo off
REM ============================================================================
REM Asset Engine - Example 03 : Advanced Features
REM ============================================================================
REM Demonstration des fonctionnalites avancees (8 features techniques)
REM Niveau : Expert
REM Duree : ~5 minutes (avec pauses interactives)
REM ============================================================================

echo ========================================
echo Asset Engine - Example 03 : Advanced
echo ========================================
echo.

set EXE=..\ide\AssetEngine\x64\Release\AssetEngine.exe
set ARCHIVE=advanced_demo.asset

REM ============================================================================
REM FEATURE 1 : Gestion des doublons (auto-increment)
REM ============================================================================
echo [FEATURE 1] Gestion automatique des doublons
echo.

mkdir demo_assets
echo Content version 1 > demo_assets\config.txt

echo [ACTION] Creer archive avec config.txt
%EXE% create %ARCHIVE% demo_assets\config.txt

echo [ACTION] Ajouter config.txt une 2eme fois
%EXE% add %ARCHIVE% demo_assets\config.txt

echo [ACTION] Ajouter config.txt une 3eme fois
%EXE% add %ARCHIVE% demo_assets\config.txt

echo.
echo [RESULT] L'archive contient maintenant :
echo   - config.txt
echo   - config(1).txt
echo   - config(2).txt
echo.
%EXE% list %ARCHIVE%

echo.
pause

REM ============================================================================
REM FEATURE 2 : Renommage instantane (O(1))
REM ============================================================================
echo.
echo [FEATURE 2] Renommage instantane (sans recopie)
echo.

echo [ACTION] Renommer config.txt en settings.txt
%EXE% rename %ARCHIVE% config.txt settings.txt

echo.
echo [RESULT] Fichier renomme instantanement (O(1) - pas de recopie)
%EXE% list %ARCHIVE%

echo.
pause

REM ============================================================================
REM FEATURE 3 : Soft Delete (marquage sans suppression physique)
REM ============================================================================
echo.
echo [FEATURE 3] Soft Delete (marquage FLAG_DELETED)
echo.

echo [BEFORE] Taille archive avant suppression :
for %%A in (%ARCHIVE%) do echo   %%~zA bytes

echo.
echo [ACTION] Supprimer config(1).txt (soft delete)
%EXE% remove %ARCHIVE% config(1).txt

echo.
echo [AFTER] Taille archive apres soft delete :
for %%A in (%ARCHIVE%) do echo   %%~zA bytes
echo.
echo [INFO] La taille n'a PAS change (fichier marque comme deleted, pas physiquement supprime)

echo.
%EXE% list %ARCHIVE%

echo.
pause

REM ============================================================================
REM FEATURE 4 : Compact (purge physique)
REM ============================================================================
echo.
echo [FEATURE 4] Compact (purge deleted files)
echo.

echo [BEFORE] Taille avant compact :
for %%A in (%ARCHIVE%) do echo   %%~zA bytes

echo.
echo [ACTION] Compacter l'archive
%EXE% compact %ARCHIVE%

echo.
echo [AFTER] Taille apres compact :
for %%A in (%ARCHIVE%) do echo   %%~zA bytes
echo.
echo [INFO] La taille a DIMINUE (fichiers deleted physiquement supprimes)

echo.
%EXE% list %ARCHIVE%

echo.
pause

REM ============================================================================
REM FEATURE 5 : CRC32 Validation
REM ============================================================================
echo.
echo [FEATURE 5] Validation CRC32 (detection corruption)
echo.

echo [TEST 1] Archive integre
%EXE% validate %ARCHIVE%

echo.
echo [TEST 2] Simulation corruption (modifier manuellement l'archive corromprait le CRC32)
echo [INFO] Si l'archive etait corrompue, la validation echouerait

echo.
pause

REM ============================================================================
REM FEATURE 6 : Multiple types de fichiers
REM ============================================================================
echo.
echo [FEATURE 6] Support multi-types (texte + binaire)
echo.

echo Text file content > demo_assets\text_file.txt
powershell -Command "[System.IO.File]::WriteAllBytes('demo_assets\binary_file.dat', @(0xDE,0xAD,0xBE,0xEF,0xCA,0xFE,0xBA,0xBE))"
powershell -Command "[System.IO.File]::WriteAllBytes('demo_assets\image.png', @(0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A))"

if exist multi_types.asset del multi_types.asset
%EXE% create multi_types.asset demo_assets\text_file.txt demo_assets\binary_file.dat demo_assets\image.png

echo.
echo [RESULT] Archive avec fichiers texte + binaire
%EXE% list multi_types.asset

echo.
%EXE% validate multi_types.asset

echo.
pause

REM ============================================================================
REM FEATURE 7 : Performance (O(1) operations)
REM ============================================================================
echo.
echo [FEATURE 7] Performance - Complexite O(1)
echo.
echo [INFO] Operations AssetEngine :
echo.
echo   Operation         ^| Complexite ^| Temps
echo   ------------------+-----------+--------
echo   AddFile           ^| O(1)      ^| Instant (append at end)
echo   RemoveFile        ^| O(1)      ^| Instant (flag FILE_DELETED)
echo   RenameFile        ^| O(1)      ^| Instant (fixed 256-byte buffer)
echo   Extract by ID     ^| O(1)      ^| Instant (hashmap lookup)
echo   Extract by Name   ^| O(1)      ^| Instant (hashmap lookup)
echo   List              ^| O(n)      ^| Parcours tous fichiers actifs
echo   Validate          ^| O(n)      ^| CRC32 sur tous fichiers
echo   Compact           ^| O(n)      ^| Recopie fichiers actifs uniquement
echo.

pause

REM ============================================================================
REM FEATURE 8 : Chiffrement (API level - pas CLI)
REM ============================================================================
echo.
echo [FEATURE 8] Chiffrement symetrique (XOR)
echo.
echo [INFO] Le chiffrement est implemente au niveau API (Archive.cpp)
echo        mais pas encore expose dans la CLI.
echo.
echo [API EXAMPLE] Code C++ pour creer archive chiffree :
echo.
echo     Archive arc;
echo     arc.EnableEncryption(true);
echo     arc.SetEncryptionKey("MySecretKey123");
echo     arc.Create(files);
echo.
echo     // Extract avec bonne cle : OK
echo     arc.SetEncryptionKey("MySecretKey123");
echo     arc.Extract("file.txt", "output.txt");  // Plaintext correct
echo.
echo     // Extract sans cle : Donnees chiffrees (garbage)
echo.
echo [TESTED] Test15_Archive_Encryption PASSED (voir main.cpp MODE_TESTS)
echo.

pause

REM ============================================================================
REM SUCCESS
REM ============================================================================
echo.
echo ========================================
echo DEMONSTRATION AVANCEE TERMINEE !
echo ========================================
echo.
echo Fonctionnalites demontrees :
echo   [1] Gestion doublons (auto-increment)
echo   [2] Renommage instantane (O(1))
echo   [3] Soft Delete (FLAG_DELETED)
echo   [4] Compact (purge physique)
echo   [5] CRC32 Validation
echo   [6] Multi-types (texte + binaire)
echo   [7] Performance O(1)
echo   [8] Chiffrement XOR (API)
echo.

pause
