#pragma once

#include "Pokemon_vector.h"
#include "Pokedex.h"

/**
 * Représente l'équipe du joueur.
 * La taille de l'équipe n'est pas limitée.
 *
 * La collection est propriétaire des Pokémon ajoutés.
 */
class Pokemon_Party : public Pokemon_Vector {

    public:
        Pokemon_Party() = default;
        ~Pokemon_Party() override = default;

        Pokemon* extrairePokemon(int id);
        Pokemon* extrairePremierPokemon();
        void display() const override;
};
