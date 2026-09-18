#include "../Inc/Pokemon_vector.h"
#include <stdexcept>
#include <algorithm>

using namespace std;

Pokemon* Pokemon_Vector::findById(int id)  {
    for (Pokemon*p: Pokemons) {
        if (p->getId() == id) {
            return p; // Found
        }
    }
    return nullptr; // Not found
}
Pokemon* Pokemon_Vector::findByName(const string& name) {
    for (Pokemon*p: Pokemons) {
        if (p->getName() == name) {
            return p; // Found
        }
    }
    return nullptr; // Not found
}
void Pokemon_Vector::addPokemon(Pokemon* pokemon) {
    if (pokemon == nullptr) {
        throw std::invalid_argument("Cannot add a null Pokemon");
    }
    Pokemons.push_back(pokemon);
}
void Pokemon_Vector::removePokemon(Pokemon* pokemon) {
    if (pokemon == nullptr) {
        throw std::invalid_argument("Cannot remove a null Pokemon");
    }
    auto it = std::find(Pokemons.begin(), Pokemons.end(), pokemon);
    if (it != Pokemons.end()) {
        Pokemons.erase(it);
    } else {
        throw std::runtime_error("Pokemon not found in the collection");
    }
}

Pokemon_Vector::~Pokemon_Vector() {
    for (Pokemon* p : Pokemons) {
        delete p; // Assuming ownership of Pokemon pointers
    }
    Pokemons.clear();
}
