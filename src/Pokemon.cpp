#include "../Inc/Pokemon.h"
#include <algorithm>
#include <iostream>
using namespace std;

int Pokemon ::countpok=0;
//constructeur
Pokemon :: Pokemon(int id,string name,double hitPoint,double hitPointMax,double attack,double defense,int generation){
    this->id=id;
    this->name=name;    
    this->hitPoint=hitPoint;
    this->hitPointMax=hitPointMax;
    this->attack=attack;
    this->defense=defense;
    this->generation=generation;

    countpok++;
}
//constructeur de copie
Pokemon::Pokemon(const Pokemon &p)
    :id(p.id),
    name(p.name),
    hitPoint(p.hitPoint),
    hitPointMax(p.hitPointMax),
    attack(p.attack),
    defense(p.defense),
    generation(p.generation  )
{
    countpok++;
}
//destructeur
Pokemon::~Pokemon() {
    countpok--;
}

Pokemon* Pokemon::clone() const {
    return new Pokemon(*this);
}

//getters
int Pokemon::getId() const {
    return id;
}
string Pokemon::getName() const {
    return name;
}
double Pokemon::getHitPoint() const {
    return hitPoint;
}

double Pokemon::getHitPointMax() const {
    return hitPointMax;
}

double Pokemon::getAttack() const {
    return attack;
}
double Pokemon::getDefense() const {
    return defense;
}
int Pokemon::getGeneration() const {
    return generation;
}
//setters
void Pokemon::setId(int i)
{
    id = i;
}
void Pokemon::setName(string n)
{
    name = n;
}
void Pokemon::setHitPoint(double h) {
    hitPoint=h;
}
void Pokemon::setHitPointMax(double hm) {
    hitPointMax=hm;
}
void Pokemon::setAttack(double a) {
    attack=a;
}
void Pokemon::setDefense(double d) {
    defense=d;
}
void Pokemon::setGeneration(int g) {
    generation=g;
}

//affichage
void Pokemon::displayInfo() const {
    cout<<"Id :"<<id<<endl;
    cout<<"Name:"<<name<<endl;
    cout<<"HitPoint :"<<hitPoint<<endl;
    cout<<"HitPointMax :"<<hitPointMax<<endl;
    cout<<"Attack :"<<attack<<endl;
    cout<<"Defense :"<<defense<<endl;
    cout<<"Generation :"<<generation<<endl;
}
//methode d'attaque
void Pokemon::attackPokemon(Pokemon& p) {
    const double degats = std::max(1.0, getAttack() - p.getDefense() * 0.35);
    p.setHitPoint(std::max(0.0, p.getHitPoint() - degats));
}
