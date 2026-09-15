#include "../Inc/Pokemon_vector.h"
#include <iostream>

using namespace std;

Pokemon_Vector::Pokemon_Vector()
{
}

Pokemon_Vector::~Pokemon_Vector()
{
    for (Pokemon* pokemon : pokemons)
    {
        delete pokemon;
    }

    pokemons.clear();
}