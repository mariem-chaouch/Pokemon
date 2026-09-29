#pragma once

#include "Pokedex.h"
#include "Pokemon_Attack.h"
#include "Pokemon_Party.h"

#include <SFML/Graphics.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <random>
#include <string>

/** Donnees partagees entre les ecrans : ressources graphiques et progression du jeu. */
struct ContexteJeu {
    Pokedex& pokedex;
    Pokemon_Party& party;
    sf::RenderWindow& window;
    sf::Font& font;
    sf::Texture& textureFondAccueil;
    sf::Texture& textureJauge;
    sf::Texture& textureVs;
    std::function<const sf::Texture*(int)> texturePokemon;

    std::array<int, 6> equipe{};
    std::array<int, 6> equipeAdverse{};
    std::array<double, 6> pvEquipeJoueur{};
    std::array<double, 6> pvEquipeAdverse{};
    std::size_t combattantJoueur = 0;
    std::size_t combattantAdverse = 0;
    int candidatSelectionne = -1;
    std::size_t debutListe = 0;
    bool defilementListeActif = false;
    std::size_t debutCollection = 0;
    std::size_t indiceSauvage = 0;
    std::unique_ptr<Pokemon_Attack> pokemonAttack;
    bool captureTerminee = false;
    bool captureReussie = false;
    bool victoire = false;
    std::string messageCapture;
    std::string messageCombat;
    std::mt19937 aleatoire{std::random_device{}()};
};

class Etat {
public:
    virtual ~Etat() = default;
    /** Traite un evenement SFML. Retourne un nouvel etat si l'ecran doit changer. */
    virtual std::unique_ptr<Etat> traiterEvenement(ContexteJeu& contexte, const sf::Event& evenement) = 0;
    /** Dessine l'ecran courant sans modifier la progression du jeu. */
    virtual void dessiner(ContexteJeu& contexte) const = 0;
};

class EtatAccueil final : public Etat {
public:
    std::unique_ptr<Etat> traiterEvenement(ContexteJeu&, const sf::Event&) override;
    void dessiner(ContexteJeu&) const override;
};

class EtatSelectionEquipe final : public Etat {
public:
    std::unique_ptr<Etat> traiterEvenement(ContexteJeu&, const sf::Event&) override;
    void dessiner(ContexteJeu&) const override;
};

class EtatExploration final : public Etat {
public:
    std::unique_ptr<Etat> traiterEvenement(ContexteJeu&, const sf::Event&) override;
    void dessiner(ContexteJeu&) const override;
};

class EtatRencontreCapture final : public Etat {
public:
    std::unique_ptr<Etat> traiterEvenement(ContexteJeu&, const sf::Event&) override;
    void dessiner(ContexteJeu&) const override;
};

class EtatCollection final : public Etat {
public:
    std::unique_ptr<Etat> traiterEvenement(ContexteJeu&, const sf::Event&) override;
    void dessiner(ContexteJeu&) const override;
};

class EtatCombatArene final : public Etat {
public:
    std::unique_ptr<Etat> traiterEvenement(ContexteJeu&, const sf::Event&) override;
    void dessiner(ContexteJeu&) const override;
};

class EtatGameOver final : public Etat {
public:
    std::unique_ptr<Etat> traiterEvenement(ContexteJeu&, const sf::Event&) override;
    void dessiner(ContexteJeu&) const override;
};

class MoteurJeu {
private:
    std::unique_ptr<Etat> etat_;
public:
    MoteurJeu();
    /** Transmet l'evenement a l'ecran courant et applique une transition eventuelle. */
    void traiterEvenement(ContexteJeu& contexte, const sf::Event& evenement);
    /** Dessine l'ecran courant. */
    void dessiner(ContexteJeu& contexte) const;
    

};
