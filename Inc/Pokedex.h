#ifndef TP_POKEMON_POKEDEX_H
#define TP_POKEMON_POKEDEX_H

#include "Pokemon_vector.h"
#include <string>

using std::string;

class Pokedex : public Pokemon_vector {
private:
    static Pokedex* instance;
    Pokedex();

    void loadFromCSV(const string& filename);

public:
    static Pokedex& getInstance();
    Pokemon* find_by_ID(int id);
    Pokemon* find_by_name(string name);
    void display() const override;
}
#endif //TP_POKEMON_POKEDEX_H
