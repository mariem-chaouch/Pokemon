#pragma once

#include "Pokemon.h"
#include <vector>

using namespace std;

/**
 * C'est une classe abstraite pour gérer une collection de Pokémon.
 */
class Pokemon_Vector {

protected:
    vector<Pokemon*> Pokemons;

    Pokemon* findById(int id);
    Pokemon* findByName(const string& name);

public:
    Pokemon_Vector() = default;
    virtual ~Pokemon_Vector();

    // Cette classe detient les pointeurs de Pokemons : une copie superficielle
    // ferait detruire les memes objets deux fois.
    Pokemon_Vector(const Pokemon_Vector&) = delete;
    Pokemon_Vector& operator=(const Pokemon_Vector&) = delete;
    Pokemon_Vector(Pokemon_Vector&&) = delete;
    Pokemon_Vector& operator=(Pokemon_Vector&&) = delete;

    virtual void addPokemon(Pokemon* pokemon);
    virtual void removePokemon(Pokemon* pokemon);

    size_t size() const;
    /** Vue en lecture seule des Pokemon de la collection, utile pour l'interface. */
    const vector<Pokemon*>& getPokemons() const;
    virtual void display() const = 0;
};
