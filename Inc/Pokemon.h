#pragma once

#include <string>
using std::string;
/**
 * Cette classe représente un Pokémon.
 **/
class Pokemon {
private:
    int id;
    string name;
    double hitPoint;
    double hitPointMax;
    double attack;
    double defense;
    int generation;
    static int countpok;
public:
    //constructeur
    Pokemon(int id,string name,double hitPoint,double hitPointMax,double attack,double defense,int generation=0);
    //constructeur de copie
    Pokemon(const Pokemon& p);
    //destructeur
    ~Pokemon();

    Pokemon* clone() const;
    
    //getters
    int getId()const;
    string getName()const;
    double getHitPoint()const;
    double getHitPointMax()const;
    double getAttack()const;
    double getDefense()const;
    int getGeneration()const;

    //setters
    void setId(int i);
    void setName(string n);
    void setHitPoint(double h);
    void setHitPointMax(double hm);
    void setAttack(double a);
    void setDefense(double d);
    void setGeneration(int g);

    //methodes
    void attackPokemon(Pokemon& p);
    void displayInfo() const;



};
