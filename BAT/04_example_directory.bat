@echo off
REM ============================================================================
REM Asset Engine - Example 04 : Create from Directory
REM ============================================================================
REM Demonstration creation d'archive depuis un dossier (std::filesystem C++17)
REM Niveau : Intermediaire
REM Duree : ~2 minutes
REM ============================================================================

echo ========================================
echo Asset Engine - Example 04 : Directory
echo ========================================
echo.

set EXE=..\ide\AssetEngine\x64\Release\AssetEngine.exe

REM ============================================================
REM SETUP : Creation dossier + fichiers de test
REM ============================================================
echo [STEP 1] Creation dossier de test avec fichiers...
if not exist demo_assets mkdir demo_assets

echo Texture Data > demo_assets\texture.png
echo Sound Data > demo_assets\sound.wav
echo Config Settings > demo_assets\config.json
echo Level 1 Data > demo_assets\level1.dat
echo Level 2 Data > demo_assets\level2.dat

echo [OK] 5 fichiers crees dans demo_assets\
echo.
pause


REM ============================================================
REM TEST 1 : Create archive depuis dossier
REM ============================================================
echo.
echo [STEP 2] Creation archive depuis dossier
echo.

if exist game_from_dir.asset del game_from_dir.asset

echo [ACTION] create game_from_dir.asset demo_assets\
%EXE% create game_from_dir.asset demo_assets

echo.
echo [INFO] L'archive contient maintenant tous les fichiers du dossier
echo.
pause


REM ============================================================
REM TEST 2 : Verifier contenu
REM ============================================================
echo.
echo [STEP 3] Verification contenu archive
echo.

echo [ACTION] list game_from_dir.asset
%EXE% list game_from_dir.asset

echo.
pause


REM ============================================================
REM TEST 3 : Valider integrite
REM ============================================================
echo.
echo [STEP 4] Validation integrite (CRC32)
echo.

echo [ACTION] validate game_from_dir.asset
%EXE% validate game_from_dir.asset

echo.
pause


REM ============================================================
REM TEST 4 : Extraire tous les fichiers
REM ============================================================
echo.
echo [STEP 5] Extraction complete
echo.

if exist extracted_from_dir rmdir /s /q extracted_from_dir
mkdir extracted_from_dir

echo [ACTION] extractall game_from_dir.asset extracted_from_dir\
%EXE% extractall game_from_dir.asset extracted_from_dir

echo.
echo [FILES] Fichiers extraits :
dir /b extracted_from_dir

echo.
pause


REM ============================================================
REM TEST 5 : Mix dossier + fichiers individuels
REM ============================================================
echo.
echo [STEP 6] Creation archive mixte (dossier + fichiers)
echo.

echo Standalone Config > standalone.cfg
echo README Content > readme.txt

if exist game_mixed.asset del game_mixed.asset

echo [ACTION] create game_mixed.asset demo_assets\ standalone.cfg readme.txt
%EXE% create game_mixed.asset demo_assets standalone.cfg readme.txt

echo.
echo [LIST] Contenu archive mixte :
%EXE% list game_mixed.asset

echo.
pause


REM ============================================================
REM SUCCESS
REM ============================================================
echo.
echo ========================================
echo DEMONSTRATION TERMINEE !
echo ========================================
echo.
echo Fonctionnalites demontrees :
echo   [OK] Create archive depuis dossier
echo   [OK] Collection automatique fichiers
echo   [OK] Mix dossier + fichiers individuels
echo   [OK] List + Validate + ExtractAll
echo.
echo Archives creees :
echo   - game_from_dir.asset (5 fichiers depuis dossier)
echo   - game_mixed.asset (7 fichiers : dossier + fichiers standalone)
echo.

pause

REM ============================================================
REM CLEANUP (optionnel)
REM ============================================================
REM if exist game_from_dir.asset del game_from_dir.asset
REM if exist game_mixed.asset del game_mixed.asset
REM if exist demo_assets rmdir /s /q demo_assets
REM if exist extracted_from_dir rmdir /s /q extracted_from_dir
REM if exist standalone.cfg del standalone.cfg
REM if exist readme.txt del readme.txt
