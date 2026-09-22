#pragma once

#include "Pokemon_vector.h"

#include <string>
using namespace std;
/**
 * Cette classe contient tous les Pokémon disponibles.
 * Elle recupere cette liste depuis un fichier CSV.
 */
class Pokedex : public Pokemon_Vector {
    private:
        static Pokedex* pinstance;
        string fichier_csv;
    protected:
        Pokedex(const string& fichier_csv);
        ~Pokedex();

    public:
        Pokedex(Pokedex& autre_pokedex) = delete;
        Pokedex& operator=(const Pokedex&) = delete;

        static Pokedex* getInstance(const string& fichier_csv );
        Pokemon* extrairePokemon(int id);

        void display() const override;  
       
};
