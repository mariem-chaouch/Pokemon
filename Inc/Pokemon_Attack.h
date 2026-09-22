#pragma once

#include "Pokemon_Party.h"
#include "Pokemon_vector.h"

#include <cstddef>

/**
 * Équipe destinée au combat. Elle contient au maximum six Pokémon extraits
 * d'une Pokemon_Party.
 */
class Pokemon_Attack : public Pokemon_Vector {
private:
    static constexpr std::size_t MAX_POKEMON = 6;

public:
    /** Transfère les six premiers Pokémon disponibles de la Party. */
    void creerDepuisParty(Pokemon_Party& party);

    /** Transfère tous les Pokémon de combat dans la Party. */
    void reintegrerDansParty(Pokemon_Party& party);

    /** Transfère dans l'équipe le Pokémon demandé de la Party. */
    bool ajouterDepuisParty(Pokemon_Party& party, int id);

    /** Ajout direct, refusé dès que l'équipe contient six Pokémon. */
    void addPokemon(Pokemon* pokemon) override;

    void display() const override;
};
