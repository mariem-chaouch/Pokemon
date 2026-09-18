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
    virtual ~Pokemon_Vector();

    virtual void addPokemon(Pokemon* pokemon);
    virtual void removePokemon(Pokemon* pokemon);

    virtual void display() const = 0;
};