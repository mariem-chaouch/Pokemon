# TP Pokémon

Projet  autour de la gestion de Pokémon. Il propose des classes pour représenter un Pokémon, gérer des collections et charger un Pokédex depuis un fichier CSV. Une fenêtre SFML sert actuellement de point d’entrée graphique.

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
