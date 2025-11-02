# Documentation — Format d'Archive & Processus de Lecture/Écriture

## Présentation

**Asset Engine** est un système d'archivage de fichiers développé en C++ pur (C++17), conçu pour empaqueter et gérer efficacement des ressources numériques (textures, sons, configurations, données de jeu, etc.) dans un format binaire propriétaire `.asset`.

Le projet implémente un **format d'archive autonome** avec les fonctionnalités suivantes :
- ✅ Création d'archives depuis fichiers individuels ou dossiers complets
- ✅ Gestion CRUD complète (Create, Read, Update, Delete)
- ✅ Vérification d'intégrité via checksums CRC32
- ✅ Chiffrement symétrique XOR avec clé privée
- ✅ Système de soft delete et compactage pour optimisation de l'espace disque
- ✅ Interface CLI complète pour manipulation en ligne de commande
- ✅ Architecture extensible basée sur le polymorphisme (Stream abstrait)

**Technologies utilisées** : C++17 standard (STL uniquement), `std::filesystem`, Visual Studio 2022

**Format de fichier** : `.asset` (format binaire propriétaire avec magic number `"ASET"`)

---

## Structure générale du format

  ```

  ┌─────────────────────────────────────────────┐
  │           ARCHIVE FILE (.asset)             │
  ├─────────────────────────────────────────────┤
  │  ARCHIVE HEADER                             │
  │  - Magic "ASET" (4 bytes)                   │
  │  - Version (uint32_t)                       │
  │  - Flags (uint32_t)                         │
  │  - FileCount (uint32_t)                     │
  ├─────────────────────────────────────────────┤
  │  FILE TABLE (map en mémoire)                │
  │  - map<string, FileHeader>                  │
  ├─────────────────────────────────────────────┤
  │  FILE 1                                     │
  │  ┌───────────────────────────────────────┐  │
  │  │ FILE HEADER                           │  │
  │  │ - Filename[256] (char array)          │  │
  │  │ - Offset (uint64_t)                   │  │
  │  │ - Size (uint64_t)                     │  │
  │  │ - CRC32 (uint32_t)                    │  │
  │  │ - Flags (uint32_t)                    │  │
  │  ├───────────────────────────────────────┤  │
  │  │ FILE DATA                             │  │
  │  └───────────────────────────────────────┘  │
  ├─────────────────────────────────────────────┤
  │  FILE 2                                     │
  │  ...                                        │
  ├─────────────────────────────────────────────┤
  │  FILE N                                     │
  └─────────────────────────────────────────────┘

  ```

## 1. Composants principaux du système

### **Stream** (classe abstraite)
- Interface de base commune à tous les types de flux.
- Sert à unifier la gestion entre fichiers physiques et données en mémoire.
- Méthodes principales :
  - `Read(buffer, size)` → lecture binaire
  - `Write(buffer, size)` → écriture binaire
  - `Seek(offset)` → déplacement du curseur

---

### **File** (hérite de Stream)
Gère les opérations sur les fichiers physiques du disque.
Elle centralise :
- L'ouverture (`Open`, `OpenRead`, `OpenWrite`)
- La lecture/écriture (`Read`, `Write`)
- Le chiffrement/déchiffrement symétrique

#### Système de chiffrement
- Type : **symétrique** (XOR)
- Clé privée définie par l'utilisateur via :
  ```cpp
  file.SetKey("MaClePrivee");
  file.EnableEncryption(true);
  ```
- Fonctionnement :
  ```cpp
  data[i] = data[i] ^ key[i % keyLength];
  ```
  → le même algorithme sert à chiffrer et à déchiffrer.

---

### **Memory** (hérite de Stream)
- Gère un flux en mémoire vive (`UINT8* buffer`).
- Permet d'accéder, modifier ou manipuler des données sans accès disque.
- Utile pour la décompression, la sérialisation, ou le streaming GPU.

---

## 2. Processus de lecture d'une archive

```
┌──────────────┐
│  Ouvrir      │
│  l'archive   │
└──────┬───────┘
       │
       v
┌──────────────────────────────┐
│ Lire ARCHIVE HEADER          │
│ - Vérifier "ASET"            │
│ - Lire version / flags       │
│ - Si ENCRYPTED → activer clé │
│ - Lire FileCount             │
└──────┬───────────────────────┘
       │
       v
┌──────────────────────────────┐
│ Lire la table des fichiers   │
│ - map<nom, FileHeader>       │
│ - Positionner le curseur     │
└──────┬───────────────────────┘
       │
       v
┌──────────────────────────────┐
│ Pour chaque fichier :        │
│ - Lire FILE HEADER           │
│ - Vérifier taille & CRC      │
│ - Lire FILE DATA             │
│ - Si flag ENCRYPTED → XOR    │
│ - Si flag COMPRESSED → décomp│
│ - Créer un flux Memory/File  │
└──────────────────────────────┘
```

---

## 3. Processus d'écriture d'une archive

```
  ┌──────────────────────────────┐
  │  Collecter les fichiers      │
  │  - Liste args CLI            │
  │  - Scan dossiers (filesystem)│
  │  - Validation existence      │
  └──────┬───────────────────────┘
         │
         v
  ┌──────────────────────────────┐
  │ Créer l'archive (Stream)     │
  │ - Ouvrir fichier .asset      │
  │ - Mode écriture binaire      │
  └──────┬───────────────────────┘
         │
         v
  ┌──────────────────────────────┐
  │ Écrire ARCHIVE HEADER        │
  │ - Magic "ASET" (4 bytes)     │
  │ - Version (uint32_t)         │
  │ - Flags (uint32_t)           │
  │   • ARCHIVE_ENCRYPTED si clé │
  │ - FileCount (uint32_t)       │
  └──────┬───────────────────────┘
         │
         v
  ┌──────────────────────────────┐
  │ Pour chaque fichier :        │
  │                              │
  │ ┌────────────────────────┐   │
  │ │ 1. Lire données source │   │
  │ │    - Stream::Read()    │   │
  │ └──────┬─────────────────┘   │
  │        v                     │
  │ ┌────────────────────────┐   │
  │ │ 2. Calculer CRC32      │   │
  │ │    - Sur plaintext     │   │
  │ └──────┬─────────────────┘   │
  │        v                     │
  │ ┌────────────────────────┐   │
  │ │ 3. Compresser (opt.)   │   │
  │ │    - Si flag COMPRESS  │   │
  │ └──────┬─────────────────┘   │
  │        v                     │
  │ ┌────────────────────────┐   │
  │ │ 4. Chiffrer (opt.)     │   │
  │ │    - XOR si clé fournie│   │
  │ │    - Flag FILE_ENCRYPT │   │
  │ └──────┬─────────────────┘   │
  │        v                     │
  │ ┌────────────────────────┐   │
  │ │ 5. Écrire FILE HEADER  │   │
  │ │    - Filename[256]     │   │
  │ │    - Offset (uint64_t) │   │
  │ │    - Size (uint64_t)   │   │
  │ │    - CRC32 (uint32_t)  │   │
  │ │    - Flags (uint32_t)  │   │
  │ └──────┬─────────────────┘   │
  │        v                     │
  │ ┌────────────────────────┐   │
  │ │ 6. Écrire FILE DATA    │   │
  │ │    - Données traitées  │   │
  │ │    - Stream::Write()   │   │
  │ └────────────────────────┘   │
  │                              │
  └──────┬───────────────────────┘
         │
         v
  ┌──────────────────────────────┐
  │ Construire File Table        │
  │ - map<string, FileHeader>    │
  │ - Index rapide nom → offset  │
  └──────┬───────────────────────┘
         │
         v
  ┌──────────────────────────────┐
  │ Fermer l'archive             │
  │ - Flush buffer               │
  │ - Stream::Close()            │
  └──────────────────────────────┘
```

---

## 4. Exemple de flux global (simplifié)

| Étape | Fonction appelée | Action principale |
|-------|------------------|-------------------|
| 1 | `Archive::Create("data.asset", files)` | Crée l'archive et écrit le header `"ASET"` |
| 2 | `Archive::SetKey("Secret")` | Active le chiffrement symétrique |
| 3 | `Archive::AddFile("file.txt")` | Lit, calcule CRC32, chiffre (XOR) et écrit les données |
| 4 | `Archive::Save()` | Ferme le flux et flush les buffers |
| 5 | `Archive::Load("data.asset")` | Lit le header et détecte `ARCHIVE_ENCRYPTED` |
| 6 | `Archive::SetKey("Secret")` | Active la clé pour déchiffrement |
| 7 | `Archive::Extract("file.txt", "out.txt")` | Déchiffre les données à la volée |
| 8 | Résultat | Les données extraites sont identiques à celles d'origine |

---

## 5. Format des headers

### ARCHIVE HEADER (16 bytes)

| Champ | Type | Taille (bytes) | Exemple | Description |
|:------|:-----|:---------------|:--------|:------------|
| `magic` | char[4] | 4 | `'A' 'S' 'E' 'T'` | Signature du format |
| `version` | uint32_t | 4 | `1` | Version du format |
| `flags` | uint32_t | 4 | `0x00000001` | `ARCHIVE_ENCRYPTED` = 0x1 |
| `fileCount` | uint32_t | 4 | `5` | Nombre de fichiers dans l'archive |

### FILE HEADER (280 bytes)

| Champ | Type | Taille (bytes) | Exemple | Description |
|:------|:-----|:---------------|:--------|:------------|
| `filename` | char[256] | 256 | `"texture.png\0..."` | Nom du fichier (fixed-size) |
| `offset` | uint64_t | 8 | `1024` | Position absolue dans l'archive |
| `size` | uint64_t | 8 | `2048` | Taille des données |
| `crc32` | uint32_t | 4 | `0xABCD1234` | Checksum (calculé sur plaintext) |
| `flags` | uint32_t | 4 | `0x00000003` | `FILE_ACTIVE \| FILE_ENCRYPTED` |

**Flags disponibles** :
- `FILE_ACTIVE = 0x1` : Fichier actif
- `FILE_ENCRYPTED = 0x2` : Fichier chiffré
- `FILE_COMPRESSED = 0x4` : Fichier compressé
- `FILE_DELETED = 0x8` : Fichier supprimé (soft delete)

---

## 6. En résumé

- Le format d'archive est **structuré, auto-descriptif** et **extensible**.
- Le header `"ASET"` permet d'identifier et de configurer automatiquement la lecture.
- Le chiffrement symétrique XOR est :
  - Léger
  - Réversible
  - Suffisant pour empêcher la lecture brute des données
- Le système `Stream` permet à tout moment de remplacer `File` par `Memory` sans changer l'interface.


## 7. Utilisation du terminal

L'outil AssetEngine.exe permet de manipuler des archives directement depuis la console grâce à un système de commandes.
Chaque action est passée en argument au programme, suivie des paramètres requis.

### Syntaxe générale
```bash
AssetEngine.exe <command> [archive] [parameters...]
```

### Commandes disponibles

| Commande | Description | Syntaxe | Exemple |
| -------- | ----------- | ------- | ------- |
| `help` / `-help` / `--help` | Affiche l'aide complète avec exemples | `AssetEngine.exe help` | `AssetEngine.exe help` |
| `create` | Crée une archive depuis fichiers/dossiers | `AssetEngine.exe create <archive.asset> <file1> [file2] [dir/]` | `AssetEngine.exe create game.asset textures/ config.json` |
| `add` | Ajoute un fichier à une archive existante | `AssetEngine.exe add <archive.asset> <file>` | `AssetEngine.exe add game.asset new_level.dat` |
| `remove` | Supprime un fichier (soft delete) | `AssetEngine.exe remove <archive.asset> <filename>` | `AssetEngine.exe remove game.asset old_texture.png` |
| `removeall` | Supprime tous les fichiers (vide l'archive) | `AssetEngine.exe removeall <archive.asset>` | `AssetEngine.exe removeall game.asset` |
| `rename` | Renomme un fichier dans l'archive | `AssetEngine.exe rename <archive.asset> <old_name> <new_name>` | `AssetEngine.exe rename game.asset level1.dat tutorial.dat` |
| `list` | Liste tous les fichiers de l'archive | `AssetEngine.exe list <archive.asset>` | `AssetEngine.exe list game.asset` |
| `extract` | Extrait un fichier spécifique | `AssetEngine.exe extract <archive.asset> <filename> <output_file>` | `AssetEngine.exe extract game.asset config.json ./config.json` |
| `extractall` | Extrait tous les fichiers dans un dossier | `AssetEngine.exe extractall <archive.asset> <output_dir>` | `AssetEngine.exe extractall game.asset ./output/` |
| `validate` | Vérifie l'intégrité (CRC32) de l'archive | `AssetEngine.exe validate <archive.asset>` | `AssetEngine.exe validate game.asset` |
| `compact` | Compacte l'archive (purge soft-deleted) | `AssetEngine.exe compact <archive.asset>` | `AssetEngine.exe compact game.asset` |

### Notes importantes

1. **Création depuis dossier** : La commande `create` supporte les dossiers. Tous les fichiers du dossier sont ajoutés automatiquement (non-récursif).
   ```bash
   AssetEngine.exe create game.asset assets/
   ```

2. **Mix fichiers + dossiers** : Vous pouvez combiner fichiers individuels et dossiers :
   ```bash
   AssetEngine.exe create game.asset textures/ sounds/ config.json readme.txt
   ```

3. **Gestion des doublons** : Si vous ajoutez un fichier déjà présent, il sera renommé automatiquement avec un suffixe `(N)` :
   ```bash
   AssetEngine.exe add game.asset texture.png
   # Si texture.png existe déjà → ajouté comme texture(1).png
   ```

4. **Soft Delete** : La commande `remove` marque le fichier comme supprimé (flag `FILE_DELETED`) mais ne libère pas l'espace immédiatement. Utilisez `compact` pour récupérer l'espace disque.

5. **Validation CRC32** : La commande `validate` vérifie le magic number `"ASET"`, le CRC32 de chaque fichier actif, et la cohérence de la file table.

6. **Rename O(1)** : Le rename est instantané car les filenames sont fixed-size (256 bytes). Seul le header est modifié, pas les données.

7. **Archives imbriquées** : Il est possible d'archiver des archives (`.asset` dans `.asset`). Les archives imbriquées sont traitées comme des fichiers binaires standards et conservent leur intégrité lors de l'extraction.
   ```bash
   AssetEngine.exe create archive1.asset config.json
   AssetEngine.exe create archive2.asset data.txt
   AssetEngine.exe add archive1.asset archive2.asset
   # archive1.asset contient maintenant archive2.asset
   ```

### Exemple d'usage complet

```bash
# Créer une archive depuis un dossier
AssetEngine.exe create my_game.asset game_assets/

# Lister le contenu
AssetEngine.exe list my_game.asset

# Ajouter un fichier supplémentaire
AssetEngine.exe add my_game.asset patch_notes.txt

# Renommer un fichier
AssetEngine.exe rename my_game.asset level1.dat tutorial_level.dat

# Supprimer un ancien fichier
AssetEngine.exe remove my_game.asset old_logo.png

# Compacter l'archive (purge)
AssetEngine.exe compact my_game.asset

# Valider l'intégrité
AssetEngine.exe validate my_game.asset

# Extraire tout dans un dossier
AssetEngine.exe extractall my_game.asset ./output/
```
