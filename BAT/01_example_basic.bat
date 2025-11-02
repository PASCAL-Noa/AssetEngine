@echo off
REM ============================================================================
REM Asset Engine - Example 01 : Basic Usage
REM ============================================================================
REM Exemple basique pour debuter avec Asset Engine (6 etapes)
REM Niveau : Debutant
REM Duree : ~40 secondes
REM ============================================================================

echo ========================================
echo Asset Engine - Example 01 : Basic Usage
echo ========================================
echo.

set EXE=..\ide\AssetEngine\x64\Release\AssetEngine.exe

REM ============================================================================
REM 1. CREER DES FICHIERS DE TEST
REM ============================================================================
echo [1] Creation de fichiers de test...

mkdir my_assets
echo Hello World! > my_assets\hello.txt
echo Asset Engine Demo > my_assets\readme.txt
echo Test data > my_assets\data.txt

echo [OK] 3 fichiers crees
echo.

REM ============================================================================
REM 2. CREER UNE ARCHIVE
REM ============================================================================
echo [2] Creation de l'archive...

%EXE% create my_archive.asset my_assets\hello.txt my_assets\readme.txt my_assets\data.txt

echo.

REM ============================================================================
REM 3. AFFICHER LE CONTENU
REM ============================================================================
echo [3] Contenu de l'archive :
echo.

%EXE% list my_archive.asset

echo.

REM ============================================================================
REM 4. VERIFIER L'INTEGRITE
REM ============================================================================
echo [4] Verification de l'integrite :
echo.

%EXE% validate my_archive.asset

echo.

REM ============================================================================
REM 5. EXTRAIRE UN FICHIER
REM ============================================================================
echo [5] Extraction de hello.txt...

%EXE% extract my_archive.asset hello.txt extracted_hello.txt

echo.
echo [CONTENT] Contenu extrait :
type extracted_hello.txt

echo.
echo.

REM ============================================================================
REM 6. BONUS : CREER ARCHIVE DEPUIS DOSSIER
REM ============================================================================
echo [6] BONUS : Creation archive depuis dossier complet...
echo.

%EXE% create my_archive_from_folder.asset my_assets

echo.
echo [RESULT] Archive creee automatiquement avec tous les fichiers du dossier
%EXE% list my_archive_from_folder.asset

echo.
echo.

REM ============================================================================
REM FIN
REM ============================================================================
echo ========================================
echo DEMONSTRATION TERMINEE !
echo ========================================
echo.
echo Fichiers crees :
echo   - my_archive.asset (archive depuis fichiers individuels)
echo   - my_archive_from_folder.asset (archive depuis dossier)
echo   - extracted_hello.txt (fichier extrait)
echo.

pause
