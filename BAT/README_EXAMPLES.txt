================================================================================
Asset Engine - Guide des Fichiers .BAT
================================================================================

Ce projet contient 6 fichiers .BAT organises en 2 categories :

================================================================================
EXEMPLES (Usage Pedagogique)
================================================================================

01_example_basic.bat
    Description : Exemple basique pour debuter (6 etapes + bonus dossier)
    Niveau      : Debutant
    Duree       : ~40 secondes
    Fonctions   : create (fichiers + dossier), list, validate, extract
    Usage       : Double-cliquer ou executer "01_example_basic.bat"

02_example_complete.bat
    Description : Demonstration complete de toutes les fonctionnalites CLI
    Niveau      : Avance
    Duree       : ~2-3 minutes (avec pauses interactives)
    Fonctions   : create DEPUIS DOSSIER, list, validate, extract, extractall,
                  add, remove, rename, compact
    Usage       : Double-cliquer ou executer "02_example_complete.bat"

03_example_advanced.bat
    Description : Fonctionnalites avancees et techniques
    Niveau      : Expert
    Duree       : ~5 minutes (avec pauses interactives)
    Fonctions   : Doublons auto-increment, Rename O(1), Soft Delete, Compact,
                  CRC32, Multi-types, Performance, Chiffrement (API)
    Usage       : Double-cliquer ou executer "03_example_advanced.bat"

04_example_directory.bat
    Description : Focus sur creation archive depuis dossier (std::filesystem)
    Niveau      : Intermediaire
    Duree       : ~2 minutes (avec pauses interactives)
    Fonctions   : create depuis dossier, mix dossier + fichiers, collecte auto
    Usage       : Double-cliquer ou executer "04_example_directory.bat"

================================================================================
TESTS (Validation Automatisee)
================================================================================

test_cdc_validation.bat
    Description : Tests automatises de validation du Cahier Des Charges
    Type        : Tests fonctionnels (7 tests)
    Duree       : ~1 minute (automatise, pas de pause)
    Tests       : Help, Create, List, Validate, Extract, ExtractAll, Pattern
    Usage       : Executer "test_cdc_validation.bat"
    Resultat    : PASS/FAIL pour chaque test

test_advanced.bat
    Description : Suite de tests avances d'integration
    Type        : Tests d'integration (10 tests)
    Duree       : ~2 minutes (automatise, pas de pause)
    Tests       : Create, Add, Duplicates, Remove, Rename, Compact, Validate,
                  Extract, ExtractAll, RemoveAll
    Usage       : Executer "test_advanced.bat"
    Resultat    : PASS/FAIL pour chaque test

================================================================================
Quelle Commande Lancer ?
================================================================================

DEBUTANT - Decouvrir Asset Engine :
    > 01_example_basic.bat

INTERMEDIAIRE - Voir toutes les fonctionnalites :
    > 02_example_complete.bat

INTERMEDIAIRE - Feature creation depuis dossier :
    > 04_example_directory.bat

EXPERT - Comprendre les mecanismes internes :
    > 03_example_advanced.bat

VALIDATION - Verifier que tout fonctionne :
    > test_cdc_validation.bat
    > test_advanced.bat

================================================================================
Structure Fichiers
================================================================================

lyo-3m-switch-s1p3-07/BAT/
├── 01_example_basic.bat           # Example 01 : Basique + bonus dossier
├── 02_example_complete.bat        # Example 02 : Complet (avec dossier)
├── 03_example_advanced.bat        # Example 03 : Avance
├── 04_example_directory.bat       # Example 04 : Focus dossier
├── test_cdc_validation.bat        # Tests CDC (7 tests)
├── test_advanced.bat              # Tests avances (10 tests)
├── README_EXAMPLES.txt            # Ce fichier
└── 4-Guide_Exemples_BAT.md        # Guide detaille (Markdown)

================================================================================
Documentation Complete
================================================================================

Pour une documentation detaillee avec exemples de code, cas d'usage reels,
troubleshooting, et benchmarks :

    - BAT/4-Guide_Exemples_BAT.md (Guide detaille en Francais)

================================================================================
Prerequis
================================================================================

- Windows (CMD.exe ou PowerShell)
- Asset Engine compile (Release ou Debug)
- PowerShell disponible (pour creation fichiers binaires dans exemples)

================================================================================
Support
================================================================================

En cas de probleme :

1. Verifier que AssetEngine.exe existe dans :
   ide\AssetEngine\x64\Release\AssetEngine.exe

2. Verifier les permissions (clic droit > Proprietes > Securite)

3. Consulter la documentation complete dans /Final/ ou /FinalFR/

================================================================================
Version : 1.0
Derniere mise a jour : 2025-10-30
Projet : Asset Engine - GTech 3 Annee 3
================================================================================
