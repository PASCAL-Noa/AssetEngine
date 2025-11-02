# Asset Engine - Guide des Exemples BAT

Ce document fournit des exemples complets d'utilisation d'Asset Engine via scripts Windows Batch (`.BAT`).

---

## 📁 Fichiers Exemples Disponibles

Tous les fichiers exemples sont situés à la racine du projet :

| Fichier | Description | Niveau | Durée |
|---------|-------------|--------|-------|
| `01_example_basic.bat` | Usage basique (6 étapes + bonus dossier) | ⭐ Débutant | ~40s |
| `02_example_complete.bat` | Démonstration complète (avec création depuis dossier) | ⭐⭐⭐ Avancé | ~2-3min |
| `03_example_advanced.bat` | Fonctionnalités avancées (8 features techniques) | ⭐⭐⭐ Expert | ~5min |
| `04_example_directory.bat` | **NOUVEAU** : Focus création depuis dossier | ⭐⭐ Intermédiaire | ~2min |
| `test_cdc_validation.bat` | Tests validation CDC (automatisé) | ⭐⭐ Intermédiaire | ~1min |
| `test_advanced.bat` | Suite tests avancés (10 tests) | ⭐⭐⭐ Expert | ~2min |

---

## 🚀 Démarrage Rapide : 01_example_basic.bat

### Ce qu'il fait

1. Crée 3 fichiers de test
2. Crée l'archive `my_archive.asset`
3. Affiche le contenu de l'archive
4. Valide l'intégrité (CRC32)
5. Extrait un fichier spécifique

### Code Source

```batch
@echo off
set EXE=ide\AssetEngine\x64\Release\AssetEngine.exe

REM 1. Créer fichiers test
mkdir my_assets
echo Hello World! > my_assets\hello.txt
echo Asset Engine Demo > my_assets\readme.txt
echo Test data > my_assets\data.txt

REM 2. Créer archive
%EXE% create my_archive.asset my_assets\hello.txt my_assets\readme.txt my_assets\data.txt

REM 3. Lister contenu
%EXE% list my_archive.asset

REM 4. Valider intégrité
%EXE% validate my_archive.asset

REM 5. Extraire fichier
%EXE% extract my_archive.asset hello.txt extracted_hello.txt
type extracted_hello.txt
```

### Sortie Attendue

```
[1] Creation de fichiers de test...
[OK] 3 fichiers crees

[2] Creation de l'archive...
[OK] Archive created: my_archive.asset

[3] Contenu de l'archive :
==========================================
Archive: my_archive.asset
Files: 3 active
==========================================
[1] hello.txt (ID: 3966262814250682967, 15 bytes, CRC32: 0xED8C2EB5)
[2] readme.txt (ID: 3966262818530046219, 20 bytes, CRC32: 0x42B9AE45)
[3] data.txt (ID: 3966262814965804635, 12 bytes, CRC32: 0x72C3E0A1)
==========================================

[4] Verification de l'integrite :
Validating archive: my_archive.asset
Files to check: 3
[OK] hello.txt
[OK] readme.txt
[OK] data.txt
Archive is valid.
[OK] Archive validated (CRC32 OK)

[5] Extraction de hello.txt...
[OK] Extracted: hello.txt -> extracted_hello.txt
[CONTENT] Contenu extrait :
Hello World!
```

---

## 🎓 Exemple Complet : 02_example_complete.bat

### Fonctionnalités Démontrées

#### Fonctions Principales (Étapes 1-7)
1. **Préparation Assets**: Crée 6 fichiers (texte + binaires)
2. **Affichage Aide**: Montre l'aide complète CLI
3. **Création Archive**: Crée archive depuis dossier
4. **Listage Contenu**: Affiche détails (ID, taille, CRC32)
5. **Validation Intégrité**: Vérifie checksums CRC32
6. **Extraction Spécifique**: Extrait un seul fichier
7. **Extraction Complète**: Extrait tous les fichiers

#### Fonctions Avancées (Bonus)
- **Add**: Ajouter fichier à archive existante
- **Remove**: Suppression soft (FLAG_DELETED)
- **Rename**: Renommage instantané (O(1), buffer fixe)
- **Compact**: Purge physique fichiers supprimés

### Sections Clés du Code

```batch
@echo off
set EXE=ide\AssetEngine\x64\Release\AssetEngine.exe
set ARCHIVE=my_game_assets.asset

REM Créer assets test (texte + binaire)
mkdir game_assets
echo Game Title: My Awesome Game > game_assets\game_info.txt
powershell -Command "[System.IO.File]::WriteAllBytes('game_assets\texture_hero.png', ...)"

REM Créer archive
%EXE% create %ARCHIVE% game_assets\*.txt game_assets\*.png

REM Lister contenu
%EXE% list %ARCHIVE%

REM Valider intégrité
%EXE% validate %ARCHIVE%

REM Extraire spécifique
%EXE% extract %ARCHIVE% game_info.txt extracted_game_info.txt

REM Extraire tout
%EXE% extractall %ARCHIVE% extracted_assets

REM Avancé: Ajouter
echo New file > game_assets\new_file.txt
%EXE% add %ARCHIVE% game_assets\new_file.txt

REM Avancé: Supprimer (soft delete)
%EXE% remove %ARCHIVE% new_file.txt

REM Avancé: Renommer (O(1))
%EXE% rename %ARCHIVE% player_data.txt savegame.txt

REM Avancé: Compacter
%EXE% compact %ARCHIVE%
```

---

## 🔬 Fonctionnalités Avancées : 03_example_advanced.bat

### Démonstrations

#### 1. Gestion Doublons (Auto-increment)

```batch
REM Ajouter le même fichier 3 fois
%EXE% create advanced_demo.asset demo_assets\config.txt
%EXE% add advanced_demo.asset demo_assets\config.txt
%EXE% add advanced_demo.asset demo_assets\config.txt

REM Résultat:
REM   - config.txt
REM   - config(1).txt
REM   - config(2).txt
```

#### 2. Renommage Instantané (O(1))

```batch
REM Renommer sans copie données (buffer fixe 256 octets)
%EXE% rename advanced_demo.asset config.txt settings.txt

REM Complexité: O(1) - pas de réécriture fichier
```

#### 3. Soft Delete (FLAG_DELETED)

```batch
REM Avant: 1000 octets
%EXE% remove advanced_demo.asset config(1).txt
REM Après: 1000 octets (même taille - fichier marqué, pas physiquement supprimé)
```

#### 4. Compact (Purge Physique)

```batch
REM Avant: 1000 octets
%EXE% compact advanced_demo.asset
REM Après: 700 octets (fichiers supprimés physiquement retirés)
```

#### 5. Validation CRC32

```batch
%EXE% validate advanced_demo.asset

REM Sortie:
REM Validating archive: advanced_demo.asset
REM Files to check: 2
REM [OK] config(2).txt
REM [OK] settings.txt
REM Archive is valid.
```

#### 6. Types Multiples

```batch
REM Texte + Binaire dans même archive
echo Texte > demo_assets\text.txt
powershell -Command "[System.IO.File]::WriteAllBytes('demo_assets\binary.dat', @(0xDE,0xAD,0xBE,0xEF))"

%EXE% create multi_types.asset demo_assets\text.txt demo_assets\binary.dat
```

#### 7. Métriques Performance (Complexité)

| Opération | Complexité | Vitesse |
|-----------|-----------|---------|
| AddFile | O(1) | Instantané (append à la fin) |
| RemoveFile | O(1) | Instantané (flag FILE_DELETED) |
| RenameFile | O(1) | Instantané (buffer fixe 256 octets) |
| Extract par ID | O(1) | Instantané (lookup hashmap) |
| Extract par Nom | O(1) | Instantané (lookup hashmap) |
| List | O(n) | Parcours fichiers actifs |
| Validate | O(n) | CRC32 sur tous fichiers |
| Compact | O(n) | Copie fichiers actifs uniquement |

#### 8. Chiffrement (Niveau API)

```batch
REM Note: Le chiffrement est implémenté au niveau API (Archive.cpp)
REM Pas encore exposé dans CLI

REM Exemple API C++:
REM     Archive arc;
REM     arc.EnableEncryption(true);
REM     arc.SetEncryptionKey("MySecretKey123");
REM     arc.Create(files);
REM
REM Testé: Test15_Archive_Encryption PASSED (voir main.cpp MODE_TESTS)
```

---

## 📚 Cas d'Usage Réels

### Cas 1 : Packager Assets Jeu

```batch
@echo off
REM Packager tous les assets jeu dans une archive unique

set GAME_ARCHIVE=my_game.asset

AssetEngine.exe create %GAME_ARCHIVE% ^
    assets\textures\*.png ^
    assets\sounds\*.wav ^
    assets\music\*.mp3 ^
    assets\data\*.json

AssetEngine.exe validate %GAME_ARCHIVE%

if %errorlevel% equ 0 (
    echo [SUCCESS] Assets jeu packages et valides
) else (
    echo [ERROR] Echec validation archive
    exit /b 1
)
```

### Cas 2 : Vérifier Archive Téléchargée

```batch
@echo off
REM Vérifier intégrité archive après téléchargement

AssetEngine.exe validate downloaded_assets.asset

if %errorlevel% neq 0 (
    echo [ERROR] Archive corrompue, re-telechargement requis
    exit /b 1
)

echo [OK] Integrite archive verifiee, extraction...
AssetEngine.exe extractall downloaded_assets.asset game_data

echo [SUCCESS] Donnees jeu extraites avec succes
```

### Cas 3 : Mettre à Jour Assets

```batch
@echo off
REM Ajouter nouveaux assets à archive existante

AssetEngine.exe add game.asset new_level.json
AssetEngine.exe add game.asset new_texture.png
AssetEngine.exe add game.asset new_sound.wav

AssetEngine.exe validate game.asset

if %errorlevel% equ 0 (
    echo [SUCCESS] Assets mis a jour et valides
)
```

### Cas 4 : Nettoyer Archive

```batch
@echo off
REM Supprimer assets obsolètes et récupérer espace

echo [INFO] Taille archive avant nettoyage:
for %%A in (game.asset) do echo   %%~zA octets

AssetEngine.exe remove game.asset old_texture.png
AssetEngine.exe remove game.asset deprecated_sound.wav
AssetEngine.exe compact game.asset

echo [INFO] Taille archive apres nettoyage:
for %%A in (game.asset) do echo   %%~zA octets

echo [SUCCESS] Archive nettoyee et compactee
```

### Cas 5 : Script Build Automatisé

```batch
@echo off
REM Pipeline complet build jeu

echo [BUILD] Etape 1/4: Compilation assets...
call compile_assets.bat

echo [BUILD] Etape 2/4: Packaging assets...
AssetEngine.exe create build\game.asset ^
    build\compiled\*.bin ^
    build\textures\*.png ^
    build\sounds\*.wav

echo [BUILD] Etape 3/4: Validation package...
AssetEngine.exe validate build\game.asset
if %errorlevel% neq 0 goto build_failed

echo [BUILD] Etape 4/4: Copie vers distribution...
copy build\game.asset dist\game.asset

echo [SUCCESS] Build termine avec succes
goto end

:build_failed
echo [ERROR] Echec build - erreur validation archive
exit /b 1

:end
```

---

## 🛠️ Personnalisation

### Changer Chemin Exécutable

Modifier cette ligne dans tous les scripts :

```batch
set EXE=ide\AssetEngine\x64\Release\AssetEngine.exe
```

Options :
- **Debug**: `ide\AssetEngine\x64\Debug\AssetEngine.exe`
- **Release**: `ide\AssetEngine\x64\Release\AssetEngine.exe`
- **Custom**: `chemin\vers\AssetEngine.exe`

### Désactiver Cleanup

Commenter la section cleanup en fin de script :

```batch
REM ============================================================================
REM CLEANUP (optionnel - commenter pour garder fichiers)
REM ============================================================================
REM echo [CLEANUP] Nettoyage...
REM if exist my_archive.asset del my_archive.asset
REM if exist my_assets rmdir /s /q my_assets
```

### Ajouter Gestion Erreurs

```batch
%EXE% create my_archive.asset file1.txt

if %errorlevel% neq 0 (
    echo [ERROR] Echec creation archive
    echo [DEBUG] Verifier que file1.txt existe
    exit /b 1
)

echo [OK] Archive creee avec succes
```

---

## 🐛 Dépannage

### Erreur : "AssetEngine.exe n'est pas reconnu"

**Solution** : Vérifier chemin exécutable

```batch
if not exist %EXE% (
    echo [ERROR] AssetEngine.exe introuvable
    echo [CHEMIN] Attendu: %EXE%
    pause
    exit /b 1
)
```

### Erreur : "Archive validation failed"

**Causes possibles** :
1. Archive corrompue (transfert incomplet)
2. Fichier modifié après création
3. Erreur disque

**Solution** : Recréer archive depuis sources

### Erreur : "Failed to open archive"

**Causes possibles** :
1. Archive utilisée par autre processus
2. Permissions insuffisantes
3. Fichier inexistant

**Solution** :
- Fermer tous programmes utilisant l'archive
- Vérifier permissions (clic droit > Propriétés > Sécurité)
- Vérifier existence avec `dir /b *.asset`

---

## 📊 Benchmarks Performance

### Taille Archive vs Temps

| Fichiers | Taille Totale | Temps Create | Temps Validate | Temps ExtractAll |
|----------|--------------|--------------|----------------|------------------|
| 10 | ~10 KB | < 1s | < 1s | < 1s |
| 100 | ~1 MB | < 5s | < 2s | < 3s |
| 1000 | ~100 MB | < 30s | < 10s | < 15s |
| 10000 | ~1 GB | < 5min | < 1min | < 2min |

*Mesuré sur système standard (SSD, Windows 11, Intel i5)*

---

## 🆕 NOUVELLE FEATURE : Création Archive depuis Dossier

### 📦 04_example_directory.bat

**Fonctionnalité** : Utilisation de `std::filesystem` (STL C++17) pour collecter automatiquement tous les fichiers d'un dossier.

### Usage Simple

```batch
REM Au lieu de lister tous les fichiers manuellement :
AssetEngine.exe create game.asset file1.txt file2.txt file3.txt ...

REM Maintenant :
AssetEngine.exe create game.asset assets/
```

### Cas d'Usage

#### 1️⃣ Dossier seul
```batch
AssetEngine.exe create game.asset my_assets/
# [INFO] Collected 15 files from directory: my_assets/
# [OK] Archive created: game.asset (15 files)
```

#### 2️⃣ Mix Dossier + Fichiers
```batch
AssetEngine.exe create game.asset textures/ sounds/ config.txt readme.txt
# [INFO] Collected 50 files from directory: textures/
# [INFO] Collected 20 files from directory: sounds/
# [OK] Archive created: game.asset (72 files)
```

#### 3️⃣ Plusieurs Dossiers
```batch
AssetEngine.exe create game.asset levels/ scripts/ data/
# [INFO] Collected 10 files from directory: levels/
# [INFO] Collected 5 files from directory: scripts/
# [INFO] Collected 8 files from directory: data/
# [OK] Archive created: game.asset (23 files)
```

### Avantages

✅ **Gain de temps** : Pas besoin de lister manuellement tous les fichiers
✅ **Moins d'erreurs** : Aucun risque d'oublier un fichier
✅ **Flexible** : Mix dossiers + fichiers individuels
✅ **STL pure** : `std::filesystem` (C++17, pas de lib externe)

### Code Complet (04_example_directory.bat)

Le fichier démontre :
- Création archive depuis dossier simple
- Validation intégrité
- Extraction complète
- Mix dossier + fichiers standalone

**Durée** : ~2 minutes avec pauses interactives

---

## 📖 Documentation de Référence

- **API Complète** : `/FinalFR/3-Documentation_API.md`
- **Architecture** : `/FinalFR/2-Reference_Architecture_Complete.md`
- **README Exemples** : `EXAMPLES_README.md` (racine projet)

---

## ✅ Validation

Tous les scripts exemples ont été testés et validés :

- ✅ `01_example_basic.bat` - RÉUSSI
- ✅ `02_example_complete.bat` - RÉUSSI
- ✅ `03_example_advanced.bat` - RÉUSSI
- ✅ `04_example_directory.bat` - RÉUSSI ⭐ NOUVEAU
- ✅ `test_cdc_validation.bat` - RÉUSSI (7/7 tests)
- ✅ `test_advanced.bat` - RÉUSSI (10/10 tests)

---

**Dernière Mise à Jour** : 2025-10-30
**Version** : 1.0
**Projet** : Asset Engine - GTech 3 Année 3
