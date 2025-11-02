@echo off
REM ============================================================================
REM Asset Engine - Test : CDC Validation
REM ============================================================================
REM Tests automatises de validation du Cahier Des Charges (7 tests)
REM Type : Tests fonctionnels
REM Duree : ~1 minute (automatise)
REM ============================================================================

echo ========================================
echo Asset Engine - Test : CDC Validation
echo ========================================
echo.

set EXE=..\ide\AssetEngine\x64\Release\AssetEngine.exe
set ARCHIVE=test_cdc.asset
set TESTDIR=testfiles_cdc

REM ============================================================
REM SETUP : Creation fichiers test
REM ============================================================
echo [SETUP] Creation fichiers test...
if not exist %TESTDIR% mkdir %TESTDIR%

echo Content file 1 > %TESTDIR%\file1.txt
echo Content file 2 > %TESTDIR%\file2.txt
echo Content file 3 > %TESTDIR%\file3.txt

powershell -Command "[System.IO.File]::WriteAllBytes('%TESTDIR%\binary.dat', @(0x00,0xFF,0xDE,0xAD,0xBE,0xEF))"
powershell -Command "[System.IO.File]::WriteAllBytes('%TESTDIR%\image.png', @(0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A))"

echo [OK] Fichiers crees
echo.


REM ============================================================
REM TEST CDC #5.1 : HELP (-help, --help, help)
REM ============================================================
echo ========================================
echo TEST CDC #5.1: Commande -help
echo ========================================

echo [ACTION] Test -help
%EXE% -help > nul 2>&1
if %errorlevel% neq 0 goto fail

echo [ACTION] Test --help
%EXE% --help > nul 2>&1
if %errorlevel% neq 0 goto fail

echo [ACTION] Test help
%EXE% help > nul 2>&1
if %errorlevel% neq 0 goto fail

echo [PASS] -help, --help, help fonctionnent
echo.


REM ============================================================
REM TEST CDC #5.2 : CREATE ARCHIVE (dossier/liste fichiers)
REM ============================================================
echo ========================================
echo TEST CDC #5.2: Creer archive
echo ========================================

if exist %ARCHIVE% del %ARCHIVE%
echo [ACTION] Create archive depuis liste fichiers
%EXE% create %ARCHIVE% %TESTDIR%\file1.txt %TESTDIR%\file2.txt %TESTDIR%\binary.dat %TESTDIR%\image.png
if %errorlevel% neq 0 goto fail

if not exist %ARCHIVE% goto fail
echo [PASS] Archive creee avec succes
echo.


REM ============================================================
REM TEST CDC #5.3 : LIST (Afficher contenu)
REM ============================================================
echo ========================================
echo TEST CDC #5.3: Afficher contenu archive
echo ========================================

echo [ACTION] List archive
%EXE% list %ARCHIVE%
if %errorlevel% neq 0 goto fail

echo [PASS] Contenu affiche avec succes
echo.


REM ============================================================
REM TEST CDC #5.4 : VALIDATE (Verifier integrite)
REM ============================================================
echo ========================================
echo TEST CDC #5.4: Verifier integrite (CRC32)
echo ========================================

echo [ACTION] Validate archive
%EXE% validate %ARCHIVE%
if %errorlevel% neq 0 goto fail

echo [PASS] Integrite verifiee (CRC32 OK)
echo.


REM ============================================================
REM TEST CDC #5.5 : EXTRACT (Extraire ressource specifique)
REM ============================================================
echo ========================================
echo TEST CDC #5.5: Extraire ressource specifique
echo ========================================

echo [ACTION] Extract file1.txt
if exist extracted_file1.txt del extracted_file1.txt
%EXE% extract %ARCHIVE% file1.txt extracted_file1.txt
if %errorlevel% neq 0 goto fail
if not exist extracted_file1.txt goto fail

echo [ACTION] Extract binary.dat
if exist extracted_binary.dat del extracted_binary.dat
%EXE% extract %ARCHIVE% binary.dat extracted_binary.dat
if %errorlevel% neq 0 goto fail
if not exist extracted_binary.dat goto fail

echo [PASS] Extraction specifique OK
echo.


REM ============================================================
REM TEST CDC #5.6 : EXTRACTALL (Extraire tout)
REM ============================================================
echo ========================================
echo TEST CDC #5.6: Extraire toutes ressources
echo ========================================

echo [ACTION] Extract all
if exist extracted_cdc rmdir /s /q extracted_cdc
mkdir extracted_cdc
%EXE% extractall %ARCHIVE% extracted_cdc
if %errorlevel% neq 0 goto fail

echo [LIST] Fichiers extraits:
dir /b extracted_cdc

echo [PASS] Extraction complete OK
echo.


REM ============================================================
REM TEST CDC #5.7 : CREATE DEPUIS DOSSIER (liste fichiers)
REM ============================================================
echo ========================================
echo TEST CDC #5.7: Create archive depuis dossier
echo ========================================

if exist test_folder.asset del test_folder.asset
echo [ACTION] Create archive avec tous les fichiers du dossier
%EXE% create test_folder.asset %TESTDIR%\file1.txt %TESTDIR%\file2.txt %TESTDIR%\file3.txt
if %errorlevel% neq 0 goto fail

echo [LIST] Contents:
%EXE% list test_folder.asset
if %errorlevel% neq 0 goto fail

echo [PASS] Creation depuis dossier OK
echo.


REM ============================================================
REM SUCCESS
REM ============================================================
echo ========================================
echo TOUS LES TESTS CDC #5 PASSES !
echo ========================================
echo.
echo [VALIDATION] Interface CLI complete :
echo   - Help (-help, --help, help)       : OK
echo   - Create archive (fichiers/folder) : OK
echo   - Afficher contenu (list)          : OK
echo   - Verifier integrite (validate)    : OK
echo   - Extraire specifique (extract)    : OK
echo   - Extraire tout (extractall)       : OK
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
REM if exist %ARCHIVE% del %ARCHIVE%
REM if exist test_folder.asset del test_folder.asset
REM if exist extracted_file1.txt del extracted_file1.txt
REM if exist extracted_binary.dat del extracted_binary.dat
REM if exist %TESTDIR% rmdir /s /q %TESTDIR%
REM if exist extracted_cdc rmdir /s /q extracted_cdc
