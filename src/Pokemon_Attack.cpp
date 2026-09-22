#include "../Inc/Pokemon_Attack.h"

#include <iostream>
#include <stdexcept>

void Pokemon_Attack::addPokemon(Pokemon* pokemon) {
    if (Pokemons.size() >= MAX_POKEMON) {
        throw std::runtime_error("L'equipe d'attaque est deja complete.");
    }
    Pokemon_Vector::addPokemon(pokemon);
}

void Pokemon_Attack::creerDepuisParty(Pokemon_Party& party) {
    if (!Pokemons.empty()) {
        throw std::runtime_error("L'equipe d'attaque doit etre vide avant sa creation.");
    }

    while (Pokemons.size() < MAX_POKEMON) {
        Pokemon* pokemon = party.extrairePremierPokemon();
        if (pokemon == nullptr) {
            break;
        }
        addPokemon(pokemon);
    }
}

bool Pokemon_Attack::ajouterDepuisParty(Pokemon_Party& party, int id) {
    if (Pokemons.size() >= MAX_POKEMON) {
        return false;
    }

    Pokemon* pokemon = party.extrairePokemon(id);
    if (pokemon == nullptr) {
        return false;
    }

    addPokemon(pokemon);
    return true;
}

void Pokemon_Attack::reintegrerDansParty(Pokemon_Party& party) {
    while (!Pokemons.empty()) {
        Pokemon* pokemon = Pokemons.back();
        removePokemon(pokemon);
        party.addPokemon(pokemon);
    }
}

void Pokemon_Attack::display() const {
    std::cout << "Equipe d'attaque :" << std::endl;
    for (const Pokemon* pokemon : Pokemons) {
        std::cout << pokemon->getId() << " - " << pokemon->getName() << std::endl;
    }
}
