# Banc logiciel de validation des communications ECU / CAN

**C++17 · CAN classique simulé · CMake · CTest · tests automatisés**

Projet pédagogique de validation logicielle d'un échange de trames CAN entre un bus virtuel et un calculateur électronique simulé (ECU). Il vérifie la structure des trames, les identifiants, le DLC, le décodage des signaux, les plages de valeurs et le comportement logiciel de repli de l'ECU.

> **Périmètre de sécurité :** ce dépôt est un simulateur logiciel autonome. Il n'utilise aucune interface CAN, aucun adaptateur OBD-II, aucun calculateur physique et aucun véhicule. Les scénarios de défaut sont injectés uniquement dans la file logicielle du bus virtuel. Ne pas connecter ce programme à un véhicule ou à un réseau embarqué réel.

## 1. Objectifs

- Modéliser un bus CAN classique virtuel et un ECU récepteur.
- Générer des trames nominales pour trois signaux de démonstration.
- Contrôler les identifiants CAN 11 bits, les trames étendues, le DLC et les identifiants non répertoriés.
- Décoder les valeurs et contrôler leurs plages configurées.
- Journaliser les trames acceptées, rejetées et ignorées.
- Faire passer l'ECU simulé en `SAFE_STATE` lors d'une erreur de validation classée comme critique, puis vérifier qu'il ignore les messages suivants.
- Exécuter une suite de tests automatisés avec CTest, sans dépendance externe.

## 2. Architecture

```text
+-------------------+        +--------------------+        +-------------------+
| FrameFactory      | -----> | VirtualCanBus      | -----> | SimulatedEcu      |
| trames nominales  |        | file FIFO en RAM   |        | état + télémétrie |
+-------------------+        +--------------------+        +---------+---------+
                                                                      |
                                                                      v
                                                           +---------------------+
                                                           | FrameValidator      |
                                                           | ID / DLC / signaux  |
                                                           +----------+----------+
                                                                      |
                                                                      v
                                                           +---------------------+
                                                           | Event log + stats   |
                                                           | SAFE_STATE simulé  |
                                                           +---------------------+
```

### Modules

| Module | Responsabilité |
|---|---|
| `CanFrame` | Structure d'une trame CAN classique simulée : ID, DLC, 8 octets, horodatage. |
| `FrameFactory` | Création des trames nominales des signaux. |
| `VirtualCanBus` | File FIFO en mémoire et distribution aux abonnés. |
| `FrameValidator` | Contrôle structurel, identification du signal, décodage et vérification des limites. |
| `SimulatedEcu` | Mise à jour de la télémétrie, statistiques, journal et état de repli. |
| `ecu_can_tests` | Tests unitaires et tests d'intégration logiciels. |

## 3. Profil de messages simulé

Les identifiants et codages ci-dessous constituent **un protocole de démonstration défini pour ce projet**. Ils ne représentent pas le dictionnaire CAN d'un constructeur ou d'un véhicule réel.

| ID | Signal | DLC | Codage | Plage acceptée |
|---|---|---:|---|---|
| `0x100` | Vitesse véhicule | 2 | Entier non signé big-endian, résolution 0,1 km/h | 0 à 250 km/h |
| `0x101` | Température liquide de refroidissement | 1 | Valeur brute - 40 | -40 à 150 °C |
| `0x102` | Position pédale accélérateur | 1 | Pourcentage entier | 0 à 100 % |

La configuration est centralisée dans `include/validation/frame_validator.hpp` et `src/frame_validator.cpp` pour faciliter l'ajout de signaux supplémentaires.

## 4. Prérequis

- CMake 3.16 ou supérieur.
- Compilateur compatible C++17 : GCC, Clang ou MSVC récent.
- Aucun package tiers n'est nécessaire.

Vérifier les outils :

```bash
cmake --version
g++ --version
```

## 5. Compilation

Depuis la racine du dépôt :

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Exécuter la démonstration

Linux / macOS :

```bash
./build/ecu_can_bench --demo
```

Windows (générateur Visual Studio) :

```powershell
.\build\Release\ecu_can_bench.exe --demo
```

La démonstration transmet des trames nominales, un identifiant inconnu, puis une trame de pédale hors plage. L'ECU simulé rejette cette dernière, passe en `SAFE_STATE` et ignore ensuite les trames opérationnelles.

## 6. Tests automatisés

```bash
ctest --test-dir build --output-on-failure
```

Ou lancer directement l'exécutable de tests :

```bash
./build/ecu_can_tests
```

La suite couvre notamment :

- acceptation des trois signaux nominaux ;
- identifiant standard hors limite et identifiant étendu non pris en charge ;
- DLC supérieur à huit octets et DLC inattendu ;
- identifiant non répertorié ;
- vitesse, température et pédale hors plage ;
- validation des paramètres de `FrameFactory` ;
- ordre FIFO du bus virtuel et distribution à plusieurs abonnés ;
- mise à jour de la télémétrie ECU ;
- rejet d'une trame et passage en `SAFE_STATE` ;
- mise en quarantaine logicielle des messages après l'entrée en `SAFE_STATE` ;
- scénario d'intégration bus → ECU.

## 7. Résultats attendus

Dans la démonstration, le comportement attendu est :

1. Les messages valides sont acceptés et mettent à jour la télémétrie.
2. Un identifiant inconnu est journalisé et rejeté sans modifier les valeurs du signal.
3. Une valeur critique hors plage déclenche l'état `SAFE_STATE` simulé.
4. Les trames suivantes ne mettent plus à jour la télémétrie tant que l'ECU reste dans cet état.
5. Les compteurs et les événements permettent de retracer les décisions du logiciel.

Les résultats de test sont reproductibles localement. Ils ne constituent pas une validation automobile, une homologation, une mesure de sûreté fonctionnelle ou une preuve de fonctionnement sur un ECU réel.

## 8. Extensions possibles

Pour faire évoluer le projet tout en restant dans un environnement logiciel contrôlé :

- déplacer le dictionnaire de signaux dans un fichier YAML/JSON ;
- ajouter des tests de propriétés et des tests de fuzzing hors ligne sur les objets `CanFrame` ;
- produire un rapport CSV/JSON des tests et des diagnostics ;
- introduire des tests de charge du bus virtuel et mesurer le temps de traitement logiciel ;
- ajouter un second ECU simulé avec un protocole de messages explicitement documenté ;
- intégrer un pipeline CI (GitHub Actions) pour compiler et exécuter CTest à chaque push.

## 9. Structure du dépôt

```text
ecu-can-validation-bench/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── docs/
│   ├── architecture.md
│   └── test-plan.md
├── include/
│   ├── can/
│   │   ├── frame.hpp
│   │   └── virtual_can_bus.hpp
│   ├── ecu/
│   │   ├── ecu.hpp
│   │   └── frame_factory.hpp
│   └── validation/
│       └── frame_validator.hpp
├── src/
│   ├── ecu.cpp
│   ├── frame_validator.cpp
│   ├── main.cpp
│   └── virtual_can_bus.cpp
└── tests/
    ├── test_ecu.cpp
    ├── test_frame_validator.cpp
    ├── test_framework.hpp
    ├── test_main.cpp
    └── test_virtual_can_bus.cpp
```


## 10. Lancement en une commande

Linux / macOS :

```bash
./scripts/build_and_test.sh
```

Windows PowerShell :

```powershell
.\scripts\build_and_test.ps1
```

Le dépôt contient également un workflow GitHub Actions dans `.github/workflows/ci.yml` : il configure CMake, compile le projet et exécute CTest à chaque push ou pull request.

## 11. Limites techniques à annoncer

- Le bus est une file FIFO idéalisée en mémoire ; l'arbitrage, le bit timing, les ACK, les erreurs de couche physique et la charge électrique du bus ne sont pas modélisés.
- Le profil de signaux et les identifiants CAN sont fictifs et propres à la démonstration.
- Le passage `SAFE_STATE` représente une logique de réaction logicielle simplifiée ; ce n'est ni une stratégie de sûreté automobile validée ni une implémentation de commande véhicule.
- Aucun test n'a été exécuté sur un calculateur, un véhicule ou une interface CAN physique.
