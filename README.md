# TP Pokémon

Projet C++20 autour de la gestion de Pokémon. Il propose des classes pour représenter un Pokémon, gérer des collections et charger un Pokédex depuis un fichier CSV. Une fenêtre SFML sert actuellement de point d’entrée graphique.

## Fonctionnalités

- Représentation d’un Pokémon : identifiant, nom, points de vie, attaque, défense et génération.
- Attaque entre deux Pokémon avec prise en compte de la défense.
- Collection polymorphe de Pokémon via `Pokemon_Vector`.
- Chargement du Pokédex depuis `Data/pokedex.csv`.
- Instance unique de `Pokedex` (pattern Singleton).
- Fenêtre graphique SFML de démonstration.

## Prérequis

- Un compilateur C++ compatible C++20.
- [CMake](https://cmake.org/) 4.3 ou plus récent (selon `CMakeLists.txt`).
- [vcpkg](https://github.com/microsoft/vcpkg) pour installer SFML 2.6.2.

## Installation des dépendances

Depuis la racine du projet, avec vcpkg installé :

```powershell
vcpkg install
```

## Compiler et exécuter

Configurez CMake en indiquant le toolchain vcpkg :

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="<chemin-vers-vcpkg>/scripts/buildsystems/vcpkg.cmake"
cmake --build build
.\build\Tp_pokemon.exe
```

Dans CLion, ouvrez simplement le dossier du projet et configurez le profil CMake avec le même toolchain vcpkg.

> **Note :** le `CMakeLists.txt` référence encore `Pokemon_party` et `Pokemon_attack`, dont les fichiers ne sont pas présents dans le dépôt. Il faudra les ajouter ou retirer ces références avant une compilation complète. La liaison explicite de SFML dans CMake reste également à finaliser.

## Organisation

```text
Inc/       Fichiers d’en-tête des classes métier
src/       Implémentations C++ et point d’entrée
Data/      Données CSV et ressources graphiques
CMakeLists.txt
vcpkg.json Dépendance SFML
```

## Données

`Data/pokedex.csv` est lu par la classe `Pokedex`. Le fichier doit conserver son en-tête et les colonnes utilisées par l’application, notamment l’identifiant, le nom, les PV, l’attaque, la défense et la génération.

## État actuel

Le point d’entrée (`src/main.cpp`) ouvre actuellement une fenêtre SFML avec un cercle vert. Les classes métier peuvent ensuite servir à construire l’interface du Pokédex et les combats.
