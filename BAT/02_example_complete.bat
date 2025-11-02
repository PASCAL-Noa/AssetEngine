@echo off
REM ============================================================================
REM Asset Engine - Example 02 : Complete Demonstration
REM ============================================================================
REM Demonstration complete de toutes les fonctionnalites CLI (7 etapes + bonus)
REM Niveau : Avance
REM Duree : ~2-3 minutes (avec pauses interactives)
REM ============================================================================

echo ========================================
echo Asset Engine - Example 02 : Complete
echo ========================================
echo.

REM ============================================================================
REM CONFIGURATION
REM ============================================================================
set EXE=..\ide\AssetEngine\x64\Release\AssetEngine.exe
set ARCHIVE=my_game_assets.asset
set ASSETS_DIR=game_assets
set EXTRACT_DIR=extracted_assets

REM ============================================================================
REM ETAPE 1 : Preparation des assets
REM ============================================================================
echo [STEP 1/7] Preparation des assets de demonstration
echo.

if not exist %ASSETS_DIR% mkdir %ASSETS_DIR%

REM Creation de fichiers texte
echo Game Title: My Awesome Game > %ASSETS_DIR%\game_info.txt
echo Version: 1.0.0 >> %ASSETS_DIR%\game_info.txt
echo Author: Student Developer >> %ASSETS_DIR%\game_info.txt

echo Player 1 Health: 100 > %ASSETS_DIR%\player_data.txt
echo Player 1 Score: 0 >> %ASSETS_DIR%\player_data.txt

echo Level 1: Forest > %ASSETS_DIR%\levels.txt
echo Level 2: Castle >> %ASSETS_DIR%\levels.txt
echo Level 3: Dragon Lair >> %ASSETS_DIR%\levels.txt

REM Creation de fichiers binaires (fake textures)
powershell -Command "[System.IO.File]::WriteAllBytes('%ASSETS_DIR%\texture_hero.png', @(0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52))"
powershell -Command "[System.IO.File]::WriteAllBytes('%ASSETS_DIR%\texture_enemy.png', @(0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52))"
powershell -Command "[System.IO.File]::WriteAllBytes('%ASSETS_DIR%\sound_jump.wav', @(0xFF,0xD8,0xFF,0xE0,0x00,0x10,0x4A,0x46,0x49,0x46,0x00,0x01))"

echo [OK] 6 fichiers assets crees dans %ASSETS_DIR%\
echo.
pause

REM ============================================================================
REM ETAPE 2 : Afficher l'aide
REM ============================================================================
echo.
echo [STEP 2/7] Affichage de l'aide
echo.

%EXE% -help

echo.
pause

REM ============================================================================
REM ETAPE 3 : Creation de l'archive DEPUIS DOSSIER
REM ============================================================================
echo.
echo [STEP 3/7] Creation de l'archive depuis le dossier %ASSETS_DIR%\
echo.
echo [INFO] Utilisation de la feature "create depuis dossier" :
echo        AssetEngine.exe create archive.asset dossier\
echo.

if exist %ARCHIVE% del %ARCHIVE%

%EXE% create %ARCHIVE% %ASSETS_DIR%

if %errorlevel% neq 0 (
    echo [ERROR] Echec de creation de l'archive
    goto error
)

echo.
echo [SUCCESS] Archive creee automatiquement avec tous les fichiers du dossier
echo.
pause

REM ============================================================================
REM ETAPE 4 : Afficher le contenu de l'archive
REM ============================================================================
echo.
echo [STEP 4/7] Affichage du contenu de l'archive
echo.

%EXE% list %ARCHIVE%

if %errorlevel% neq 0 (
    echo [ERROR] Echec de lecture de l'archive
    goto error
)

echo.
pause

REM ============================================================================
REM ETAPE 5 : Verifier l'integrite de l'archive
REM ============================================================================
echo.
echo [STEP 5/7] Verification de l'integrite (CRC32)
echo.

%EXE% validate %ARCHIVE%

if %errorlevel% neq 0 (
    echo [ERROR] Archive corrompue (CRC32 mismatch)
    goto error
)

echo.
echo [SUCCESS] Archive integre (CRC32 OK)
echo.
pause

REM ============================================================================
REM ETAPE 6 : Extraire un fichier specifique
REM ============================================================================
echo.
echo [STEP 6/7] Extraction d'un fichier specifique
echo.

echo [ACTION] Extraction de game_info.txt
if exist extracted_game_info.txt del extracted_game_info.txt
%EXE% extract %ARCHIVE% game_info.txt extracted_game_info.txt

if %errorlevel% neq 0 (
    echo [ERROR] Echec d'extraction
    goto error
)

echo.
echo [SUCCESS] Fichier extrait
echo [CONTENT] Contenu de extracted_game_info.txt :
echo -------------------------------------------
type extracted_game_info.txt
echo -------------------------------------------
echo.
pause

REM ============================================================================
REM ETAPE 7 : Extraire tous les fichiers
REM ============================================================================
echo.
echo [STEP 7/7] Extraction de tous les fichiers
echo.

if exist %EXTRACT_DIR% rmdir /s /q %EXTRACT_DIR%
mkdir %EXTRACT_DIR%

%EXE% extractall %ARCHIVE% %EXTRACT_DIR%

if %errorlevel% neq 0 (
    echo [ERROR] Echec d'extraction complete
    goto error
)

echo.
echo [SUCCESS] Tous les fichiers extraits dans %EXTRACT_DIR%\
echo [FILES] Liste des fichiers extraits :
dir /b %EXTRACT_DIR%

echo.
pause

REM ============================================================================
REM BONUS : Operations avancees (optionnel)
REM ============================================================================
echo.
echo ========================================
echo BONUS: Operations avancees
echo ========================================
echo.

echo [BONUS 1] Ajout d'un nouveau fichier
echo New file added after creation > %ASSETS_DIR%\new_file.txt
%EXE% add %ARCHIVE% %ASSETS_DIR%\new_file.txt
echo [OK] Fichier ajoute
echo.

echo [BONUS 2] Liste mise a jour
%EXE% list %ARCHIVE%
echo.

echo [BONUS 3] Suppression d'un fichier (soft delete)
%EXE% remove %ARCHIVE% new_file.txt
echo [OK] Fichier supprime (soft delete)
echo.

echo [BONUS 4] Liste apres suppression
%EXE% list %ARCHIVE%
echo.

echo [BONUS 5] Renommer un fichier
%EXE% rename %ARCHIVE% player_data.txt savegame.txt
echo [OK] Fichier renomme
echo.

echo [BONUS 6] Liste apres renommage
%EXE% list %ARCHIVE%
echo.

echo [BONUS 7] Compacter l'archive (purge deleted files)
%EXE% compact %ARCHIVE%
echo [OK] Archive compactee
echo.

pause

REM ============================================================================
REM SUCCESS
REM ============================================================================
echo.
echo ========================================
echo DEMONSTRATION TERMINEE AVEC SUCCES !
echo ========================================
echo.
echo Archive creee       : %ARCHIVE%
echo Fichiers extraits   : %EXTRACT_DIR%\
echo.
echo Toutes les fonctionnalites ont ete demontrees :
echo   [OK] Creation archive depuis dossier
echo   [OK] Affichage du contenu
echo   [OK] Verification integrite (CRC32)
echo   [OK] Extraction fichier specifique
echo   [OK] Extraction complete
echo   [OK] Ajout de fichier
echo   [OK] Suppression de fichier
echo   [OK] Renommage de fichier
echo   [OK] Compactage archive
echo.
goto end

:error
echo.
echo ========================================
echo ERREUR DETECTEE !
echo ========================================
echo.
echo Une erreur s'est produite pendant l'execution.
echo Verifiez que AssetEngine.exe existe dans : %EXE%
echo.

:end
echo.
echo Appuyez sur une touche pour nettoyer et quitter...
pause > nul

REM ============================================================================
REM CLEANUP (optionnel - commenter pour garder les fichiers)
REM ============================================================================
REM echo.
REM echo [CLEANUP] Nettoyage des fichiers de demonstration...
REM if exist %ARCHIVE% del %ARCHIVE%
REM if exist %ASSETS_DIR% rmdir /s /q %ASSETS_DIR%
REM if exist %EXTRACT_DIR% rmdir /s /q %EXTRACT_DIR%
REM if exist extracted_game_info.txt del extracted_game_info.txt
REM echo [OK] Nettoyage termine
REM echo.

echo.
echo ========================================
echo Script termine
echo ========================================
