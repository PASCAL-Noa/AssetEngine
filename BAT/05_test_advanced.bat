@echo off
REM ============================================================================
REM Asset Engine - Test : Advanced Suite
REM ============================================================================
REM Suite de tests avances (10 tests : Create, Add, Remove, Rename, Compact, etc.)
REM Type : Tests d'integration
REM Duree : ~2 minutes (automatise)
REM ============================================================================

echo ========================================
echo Asset Engine - Test : Advanced Suite
echo ========================================
echo.

set EXE=..\ide\AssetEngine\x64\Release\AssetEngine.exe
set ARCHIVE=test_advanced.asset
set TESTDIR=testfiles_advanced
set EXTRACT_DIR=extracted_all

REM ============================================================
REM SETUP : Creation des fichiers de test
REM ============================================================
echo [SETUP] Creation fichiers de test...
if not exist %TESTDIR% mkdir %TESTDIR%

echo Test text file 1 > %TESTDIR%\file1.txt
echo Test text file 2 with more content > %TESTDIR%\file2.txt
echo Small > %TESTDIR%\small.txt

powershell -Command "[System.IO.File]::WriteAllBytes('%TESTDIR%\binary.dat', @(0x00,0xFF,0xDE,0xAD,0xBE,0xEF,0xCA,0xFE,0xBA,0xBE))"
powershell -Command "[System.IO.File]::WriteAllBytes('%TESTDIR%\image.png', @(0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52))"
powershell -Command "[System.IO.File]::WriteAllBytes('%TESTDIR%\photo.jpg', @(0xFF,0xD8,0xFF,0xE0,0x00,0x10,0x4A,0x46,0x49,0x46,0x00,0x01))"

echo [OK] Fichiers crees
echo.


REM ============================================================
REM TEST 1 : CREATE ARCHIVE
REM ============================================================
echo ========================================
echo TEST 1: Create archive avec fichiers varies
echo ========================================

if exist %ARCHIVE% del %ARCHIVE%
echo [ACTION] Create archive
%EXE% create %ARCHIVE% %TESTDIR%\file1.txt %TESTDIR%\binary.dat %TESTDIR%\image.png %TESTDIR%\photo.jpg
if %errorlevel% neq 0 goto fail

echo [LIST] Contents:
%EXE% list %ARCHIVE%

echo [VALIDATE]
%EXE% validate %ARCHIVE%
if %errorlevel% neq 0 goto fail
echo [PASS]
echo.


REM ============================================================
REM TEST 2 : EXTRACT FICHIERS (binaire + image)
REM ============================================================
echo ========================================
echo TEST 2: Extract fichiers specifiques
echo ========================================

echo [ACTION] Extract binary.dat
if exist extracted_binary.dat del extracted_binary.dat
%EXE% extract %ARCHIVE% binary.dat extracted_binary.dat
if %errorlevel% neq 0 goto fail
if not exist extracted_binary.dat goto fail

echo [ACTION] Extract image.png
if exist extracted_image.png del extracted_image.png
%EXE% extract %ARCHIVE% image.png extracted_image.png
if %errorlevel% neq 0 goto fail
if not exist extracted_image.png goto fail

echo [VALIDATE]
%EXE% validate %ARCHIVE%
if %errorlevel% neq 0 goto fail
echo [PASS]
echo.


REM ============================================================
REM TEST 3 : ADD DUPLICATES
REM ============================================================
echo ========================================
echo TEST 3: Add duplicates (auto-increment)
echo ========================================

echo [ACTION] Add file1.txt x3
%EXE% add %ARCHIVE% %TESTDIR%\file1.txt
%EXE% add %ARCHIVE% %TESTDIR%\file1.txt
%EXE% add %ARCHIVE% %TESTDIR%\file1.txt

echo [LIST]
%EXE% list %ARCHIVE%
echo Verifier presence de : file1.txt, file1(1).txt, file1(2).txt

echo [VALIDATE]
%EXE% validate %ARCHIVE%
if %errorlevel% neq 0 goto fail
echo [PASS]
echo.


REM ============================================================
REM TEST 4 : ADD DUPLICATE PNG
REM ============================================================
echo ========================================
echo TEST 4: Add duplicate PNG
echo ========================================

echo [ACTION] Add image.png
%EXE% add %ARCHIVE% %TESTDIR%\image.png

echo [LIST]
%EXE% list %ARCHIVE%
echo Verifier presence de : image(1).png

echo [VALIDATE]
%EXE% validate %ARCHIVE%
if %errorlevel% neq 0 goto fail
echo [PASS]
echo.


REM ============================================================
REM TEST 5 : REMOVE FICHIER
REM ============================================================
echo ========================================
echo TEST 5: Remove + Validate
echo ========================================

echo [ACTION] Remove file1(1).txt
%EXE% remove %ARCHIVE% file1(1).txt

echo [LIST]
%EXE% list %ARCHIVE%

echo [VALIDATE]
%EXE% validate %ARCHIVE%
if %errorlevel% neq 0 goto fail
echo [PASS]
echo.


REM ============================================================
REM TEST 6 : ADD SMALL FILE (REUSE SPACE)
REM ============================================================
echo ========================================
echo TEST 6: Add small.txt (reuse espace)
echo ========================================

echo [ACTION] Add small.txt
%EXE% add %ARCHIVE% %TESTDIR%\small.txt

echo [LIST]
%EXE% list %ARCHIVE%
echo Verifier presence de small.txt

echo [VALIDATE]
%EXE% validate %ARCHIVE%
if %errorlevel% neq 0 goto fail
echo [PASS]
echo.


REM ============================================================
REM TEST 7 : RENAME FILES
REM ============================================================
echo ========================================
echo TEST 7: Rename files
echo ========================================

echo [ACTION] Rename file1.txt -> main.txt
%EXE% rename %ARCHIVE% file1.txt main.txt
if %errorlevel% neq 0 goto fail

echo [ACTION] Rename image.png -> logo.png
%EXE% rename %ARCHIVE% image.png logo.png
if %errorlevel% neq 0 goto fail

echo [LIST]
%EXE% list %ARCHIVE%
echo Verifier presence de : main.txt, logo.png

echo [VALIDATE]
%EXE% validate %ARCHIVE%
if %errorlevel% neq 0 goto fail
echo [PASS]
echo.


REM ============================================================
REM TEST 8 : COMPACT + VALIDATE (VERIFICATION SOFT DELETE)
REM ============================================================
echo ========================================
echo TEST 8: Compact + Validate (Soft Delete Verification)
echo ========================================

echo [INFO] Verification soft delete :
echo - TEST 5 a supprime file1(1).txt en soft delete (flag FILE_DELETED)
echo - Le fichier reste physiquement sur le disque
echo - Compact doit le supprimer physiquement
echo.

echo [SIZE BEFORE COMPACT]
for %%A in (%ARCHIVE%) do echo Archive size: %%~zA bytes
echo.

echo [ACTION] Compact
%EXE% compact %ARCHIVE%
if %errorlevel% neq 0 goto fail

echo.
echo [SIZE AFTER COMPACT]
for %%A in (%ARCHIVE%) do echo Archive size: %%~zA bytes
echo.
echo [INFO] La taille APRES devrait etre plus petite (file1(1).txt supprime physiquement)
echo.

echo [LIST]
%EXE% list %ARCHIVE%

echo [VALIDATE]
%EXE% validate %ARCHIVE%
if %errorlevel% neq 0 goto fail
echo [PASS]
echo.


REM ============================================================
REM TEST 9 : EXTRACT ALL
REM ============================================================
echo ========================================
echo TEST 9: ExtractAll + Validate
echo ========================================

echo [ACTION] Extract all
if exist %EXTRACT_DIR% rmdir /s /q %EXTRACT_DIR%
mkdir %EXTRACT_DIR%
%EXE% extractall %ARCHIVE% %EXTRACT_DIR%
if %errorlevel% neq 0 goto fail

echo [LIST] Extracted files:
dir %EXTRACT_DIR%

echo [VALIDATE]
%EXE% validate %ARCHIVE%
if %errorlevel% neq 0 goto fail
echo [PASS]
echo.


REM ============================================================
REM TEST 10 : VALIDATION FINALE
REM ============================================================
echo ========================================
echo TEST 10: Validation finale
echo ========================================

%EXE% validate %ARCHIVE%
if %errorlevel% neq 0 goto fail
echo [PASS]
echo.


REM ============================================================
REM SUCCESS
REM ============================================================
echo ========================================
echo TOUS LES TESTS PASSES !
echo ========================================
echo.
echo Archive finale :
%EXE% list %ARCHIVE%
echo.
goto end


:fail
echo.
echo ========================================
echo TEST ECHOUE !
echo ========================================

:end
echo.
pause


REM ============================================================
REM CLEANUP (facultatif)
REM ============================================================
if exist %ARCHIVE% del %ARCHIVE%
if exist extracted_binary.dat del extracted_binary.dat
if exist extracted_image.png del extracted_image.png
if exist %TESTDIR% rmdir /s /q %TESTDIR%
if exist %EXTRACT_DIR% rmdir /s /q %EXTRACT_DIR%
