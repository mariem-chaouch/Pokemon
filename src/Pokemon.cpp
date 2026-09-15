#include "../Inc/Pokemon.h"
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
    generation(p.generation)
{
    cout<<"Recopie"<<endl;
    countpok++;
}
//destructeur
Pokemon::~Pokemon() {
    cout<<"Destructeur"<<endl;
    countpok--;
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
    cout<<this->getName()<<"attaque "<<p.getName()<<endl;
    if (attack > defense) {
        double newHitPoint=p.getHitPoint()-(attack-defense);
        p.setHitPoint(newHitPoint);
        if (p.getHitPoint()<=0) {
            cout<<p.getName()<<"est décédé"<<endl;
        }
        else
            cout<<p.getName()<<"a encore :"<<p.getHitPoint()<<"point de vie."<<endl;
    }
    else
    {
        cout << "L'attaque est bloquee par la defense !" << endl;
    }

    Pokemon* Pokemon::clone() const
    {
        return new Pokemon(*this);
    }

}
