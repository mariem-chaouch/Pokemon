
#include "../Inc/Pokedex.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

Pokedex* Pokedex::instance = nullptr;
Pokedex::Pokedex()
{
    loadFromCSV("../Data/pokemon.csv");
}

Pokedex& Pokedex::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Pokedex();
    }

    return *instance;
}
Pokemon* Pokedex::getPokemonClone(int index) const
{
    Pokemon* pokemon = getPokemon(index);

    if (pokemon != nullptr)
    {
        return pokemon->clone();
    }

    return nullptr;
}
void Pokedex::display() const
{
    cout << "===== POKEDEX =====" << endl;

    for (Pokemon* pokemon : pokemons)
    {
        pokemon->displayInfo();
        cout << "-------------------" << endl;
    }
}