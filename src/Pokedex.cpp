#include "../Inc/Pokedex.h"

#include <fstream>
#include <iostream>
#include <sstream>


Pokedex* Pokedex::pinstance = {nullptr};
/**
 * Constructeur de la classe
 * @param fileName Nom du fichier ou sont stockés les Pokémons, attendu au format CSV
 */
Pokedex::Pokedex(const string& fichier_csv) : Pokemon_Vector() {
    std::cout << "*** Constructeur du Pokedex ***" << std::endl;

    std::ifstream file(fichier_csv);
    if(!file.is_open()){
        std::cerr<<"File "<<fichier_csv<<" not found "<<std::endl;
        return;
    }

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
    //    std::cout << line << std::endl;
        std::stringstream inputstringstream(line);
        std::string cell;
        std::vector<std::string> lineData;

        while(std::getline(inputstringstream,cell,',')){
            lineData.push_back(cell);
        }
        int id = std::stoi(lineData.at(0));
        double attackValue = std::stod(lineData.at(6));
        double hitPoint = std::stod(lineData.at(5));
        double defenseValue = std::stod(lineData.at(7));
        int generation = std::stoi(lineData.at(11));

        Pokemons.push_back(new Pokemon(id,lineData.at(1),hitPoint,hitPoint,attackValue,
                                             defenseValue,generation));
    }
}
/**
 * Cree une unique instance de cette classe
 * @param nom_de_fichier designe le nom du fichier où sont stockés les Pokémons, fichier attendu au format CSV.
 * @return une reference sur l'instance si elle existait deja, et en cree une sinon.
 */
Pokedex* Pokedex::getInstance(const string& fichier_csv) {
    if (pinstance == nullptr) {
        pinstance = new Pokedex(fichier_csv);
    }
    return pinstance;
}


void Pokedex::display() const  {
    std::cout << "Pokedex : " << std::endl;
    for ( Pokemon* pokemon : Pokemons) {
        pokemon->displayInfo();
    }
}




