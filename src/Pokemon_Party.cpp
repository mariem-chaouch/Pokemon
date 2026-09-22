#include "../Inc/Pokemon_Party.h"
#include <iostream>

using namespace std;

Pokemon* Pokemon_Party::extrairePokemon(int id)
{
    Pokemon* pokemon = findById(id);
    if (pokemon != nullptr)
    {
        removePokemon(pokemon);
    }
    return pokemon;
}

Pokemon* Pokemon_Party::extrairePremierPokemon()
{
    if (Pokemons.empty())
    {
        return nullptr;
    }

    Pokemon* pokemon = Pokemons.front();
    removePokemon(pokemon);
    return pokemon;
}

void Pokemon_Party::display() const
{
    for (Pokemon* pokemon : Pokemons)
    {
        cout << pokemon->getId()<< " - "<< pokemon->getName()<< endl;
    }
}
