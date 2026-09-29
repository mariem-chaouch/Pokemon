# Pokémon

Petit jeu de combat Pokémon en C++20 avec SFML 2.6. Le joueur choisit une
équipe de six Pokémon, explore, tente des captures et peut défier une équipe
adverse à l'arène.

## Prérequis

- CMake 4.3 ou plus récent
- Un compilateur C++20
- [vcpkg](https://github.com/microsoft/vcpkg) pour installer SFML 2.6.2

Le manifeste `vcpkg.json` décrit la dépendance SFML. Configurez CMake en
indiquant le fichier toolchain de vcpkg :

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
```

L'exécutable est créé dans le dossier `build`. La configuration CMake copie le
dossier `Data` à côté de l'exécutable après la compilation. Lancez le jeu depuis
ce dossier ou depuis la racine du projet ; le programme recherche les
ressources dans les emplacements usuels autour du répertoire courant et de
l'exécutable.

## Commandes en jeu

1. Cliquez sur **Jouer**.
2. Dans l'écran de sélection, cliquez sur un Pokémon du Pokédex, puis sur une
   case de l'équipe. Le Pokédex est trié par nom ; utilisez la molette ou faites
   glisser le curseur vertical pour parcourir la liste. Cliquez sur un membre déjà placé puis sur une
   autre case pour changer son emplacement.
3. Validez l'équipe complète pour accéder à l'exploration.
4. Choisissez une rencontre pour tenter une capture, consultez la collection,
   ou défiez l'arène. Avant le combat, les six membres de chaque équipe sont
   affichés de part et d'autre du « VS ».
5. Dans l'arène, cliquez sur **Attaquer** pour faire progresser le combat.

## Organisation du projet

```text
Inc/       Déclarations des classes et des états du jeu
src/       Implémentations C++ et point d'entrée
Data/      Pokédex CSV et ressources graphiques
CMakeLists.txt
vcpkg.json Dépendance SFML
```

`MoteurJeu` transmet les événements et le dessin à l'état courant. Chaque écran
est un état distinct (`EtatAccueil`, `EtatSelectionEquipe`, `EtatExploration`,
`EtatCombatArene`, etc.). `ContexteJeu` regroupe les données et ressources
partagées entre ces états.

`Pokemon_Vector` gère une collection de Pokémon. `Pokedex` charge le catalogue
depuis `Data/pokedex.csv`, tandis que `Pokemon_Party` représente la collection
du joueur et `Pokemon_Attack` son équipe de combat.

## Données et ressources

Le Pokédex CSV doit conserver son en-tête et les colonnes utilisées dans
`src/Pokedex.cpp` : identifiant (colonne 0), nom (1), PV (5), attaque (6),
défense (7) et génération (11). Les images des Pokémon sont recherchées dans
`Data/image_pokedex-20260914/pokemon/<id>.png`. Les autres textures utilisées
par le jeu sont `bg.jpg`, `healthGauge.png` et `versusSmall.png` dans `Data/`.
