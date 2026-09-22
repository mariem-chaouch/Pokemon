#include "Pokedex.h"
#include "Pokemon_Attack.h"
#include "Pokemon_Party.h"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <map>
#include <memory>
#include <random>
#include <string>

namespace {
constexpr unsigned WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 760;
constexpr std::size_t TEAM_SIZE = 6, CANDIDATE_COUNT = 12;

enum class IdEtat { Accueil, SelectionEquipe, Exploration, RencontreCapture, CombatArene, GameOver };

/** Etat abstrait du moteur : chaque ecran est une instance distincte. */
class Etat {
public:
    virtual ~Etat() = default;
    virtual IdEtat id() const = 0;
};

template <IdEtat identifiant>
class EtatSimple final : public Etat {
public:
    IdEtat id() const override { return identifiant; }
};

/** Contexte du pattern State : il est le seul a effectuer les transitions. */
class MoteurJeu {
public:
    MoteurJeu() { transition(IdEtat::Accueil); }

    IdEtat id() const { return etat_->id(); }

    void transition(IdEtat nouvelEtat) {
        switch (nouvelEtat) {
            case IdEtat::Accueil: etat_ = std::make_unique<EtatSimple<IdEtat::Accueil>>(); break;
            case IdEtat::SelectionEquipe: etat_ = std::make_unique<EtatSimple<IdEtat::SelectionEquipe>>(); break;
            case IdEtat::Exploration: etat_ = std::make_unique<EtatSimple<IdEtat::Exploration>>(); break;
            case IdEtat::RencontreCapture: etat_ = std::make_unique<EtatSimple<IdEtat::RencontreCapture>>(); break;
            case IdEtat::CombatArene: etat_ = std::make_unique<EtatSimple<IdEtat::CombatArene>>(); break;
            case IdEtat::GameOver: etat_ = std::make_unique<EtatSimple<IdEtat::GameOver>>(); break;
        }
    }

private:
    std::unique_ptr<Etat> etat_;
};

sf::Color couleurPokemon(const Pokemon& pokemon) {
    const int teinte = (pokemon.getId() * 37) % 160;
    return {static_cast<sf::Uint8>(45 + teinte), static_cast<sf::Uint8>(105 + teinte / 3),
            static_cast<sf::Uint8>(210 - teinte / 4)};
}

bool contient(const sf::FloatRect& rect, sf::Vector2i pos) {
    return rect.contains(static_cast<float>(pos.x), static_cast<float>(pos.y));
}

void texte(sf::RenderWindow& window, const sf::Font& font, const std::string& valeur,
           unsigned taille, sf::Vector2f position, sf::Color couleur = sf::Color::White) {
    sf::Text label(valeur, font, taille);
    label.setPosition(position);
    label.setFillColor(couleur);
    window.draw(label);
}

void afficherJauge(sf::RenderWindow& window, const sf::Texture& texture, sf::Vector2f position,
                   float pourcentage, float echelle = 0.55f) {
    // healthGauge est une planche de sprites : chaque jauge mesure 32 x 80 px.
    const int index = std::clamp(static_cast<int>((1.f - pourcentage) * 12.f), 0, 12);
    sf::Sprite jauge(texture, sf::IntRect(index * 30, 0, 30, 80));
    jauge.setPosition(position);
    jauge.setScale(echelle, echelle);
    window.draw(jauge);
}

const sf::Texture* texturePokemon(int id, std::map<int, sf::Texture>& cache);

void carte(sf::RenderWindow& window, const sf::Font& font, const sf::Texture& textureJauge,
           std::map<int, sf::Texture>& texturesPokemon, const sf::FloatRect& zone,
           const Pokemon* pokemon, bool selectionnee, int position = 0) {
    sf::RectangleShape fond({zone.width, zone.height});
    fond.setPosition(zone.left, zone.top);
    fond.setFillColor(pokemon ? couleurPokemon(*pokemon) : sf::Color(35, 48, 73));
    fond.setOutlineThickness(selectionnee ? 4.f : 1.f);
    fond.setOutlineColor(selectionnee ? sf::Color(255, 215, 70) : sf::Color(125, 150, 190));
    window.draw(fond);

    if (!pokemon) {
        texte(window, font, "Emplacement " + std::to_string(position), 18, {zone.left + 18.f, zone.top + 20.f},
              sf::Color(175, 190, 215));
        texte(window, font, "vide", 22, {zone.left + 18.f, zone.top + 52.f}, sf::Color(130, 150, 180));
        return;
    }
    if (const sf::Texture* texture = texturePokemon(pokemon->getId(), texturesPokemon)) {
        sf::Sprite sprite(*texture);
        const sf::Vector2u taille = texture->getSize();
        const float echelle = std::min(52.f / taille.x, 52.f / taille.y);
        sprite.setScale(echelle, echelle);
        sprite.setPosition(zone.left + 8.f, zone.top + 20.f);
        window.draw(sprite);
    }
    texte(window, font, "#" + std::to_string(pokemon->getId()), 16, {zone.left + 65.f, zone.top + 10.f});
    texte(window, font, pokemon->getName(), 18, {zone.left + 65.f, zone.top + 33.f});
    texte(window, font, "PV " + std::to_string(static_cast<int>(pokemon->getHitPoint())) +
          "  ATQ " + std::to_string(static_cast<int>(pokemon->getAttack())), 14,
          {zone.left + 12.f, zone.top + zone.height - 25.f});
    afficherJauge(window, textureJauge, {zone.left + zone.width - 27.f, zone.top + 18.f}, 1.f, 0.5f);
}

void bouton(sf::RenderWindow& window, const sf::Font& font, const sf::FloatRect& zone,
            const std::string& libelle, bool actif) {
    sf::RectangleShape fond({zone.width, zone.height});
    fond.setPosition(zone.left, zone.top);
    fond.setFillColor(actif ? sf::Color(235, 82, 76) : sf::Color(90, 100, 120));
    window.draw(fond);
    sf::Text label(libelle, font, 20);
    const auto limites = label.getLocalBounds();
    label.setPosition(zone.left + (zone.width - limites.width) / 2.f - limites.left,
                      zone.top + (zone.height - limites.height) / 2.f - limites.top);
    window.draw(label);
}

std::string cheminPokedex() {
    std::ifstream fichier("Data/pokedex.csv");
    return fichier.good() ? "Data/pokedex.csv" : "../Data/pokedex.csv";
}

bool chargerPolice(sf::Font& font) {
    return font.loadFromFile("Data/arial.ttf") || font.loadFromFile("../Data/arial.ttf") ||
           font.loadFromFile("C:/Windows/Fonts/arial.ttf");
}

bool chargerTexture(sf::Texture& texture, const std::string& nom) {
    return texture.loadFromFile("Data/" + nom) || texture.loadFromFile("../Data/" + nom);
}

const sf::Texture* texturePokemon(int id, std::map<int, sf::Texture>& cache) {
    const auto dejaCharge = cache.find(id);
    if (dejaCharge != cache.end()) return &dejaCharge->second;

    sf::Texture texture;
    const std::string fichier = "image_pokedex-20260914/pokemon/" + std::to_string(id) + ".png";
    if (!chargerTexture(texture, fichier)) return nullptr;
    return &cache.emplace(id, std::move(texture)).first->second;
}

void initialiserParty(Pokemon_Party& party, const Pokedex& pokedex) {
    const auto& catalogue = pokedex.getPokemons();
    for (std::size_t i = 0; i < std::min(CANDIDATE_COUNT, catalogue.size()); ++i)
        party.addPokemon(catalogue[i]->clone());
}

double degats(const Pokemon& attaquant, const Pokemon& defenseur) {
    return std::max(1.0, attaquant.getAttack() - defenseur.getDefense() * 0.35);
}
} // namespace

int main() {
    Pokedex* pokedex = Pokedex::getInstance(cheminPokedex());
    Pokemon_Party party;
    initialiserParty(party, *pokedex);

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Pokemon Attack - Selection d'equipe");
    window.setFramerateLimit(60);
    sf::Font font;
    if (!chargerPolice(font)) return 1;
    sf::Texture textureJauge, textureVs;
    if (!chargerTexture(textureJauge, "healthGauge.png") || !chargerTexture(textureVs, "versusSmall.png")) return 1;
    std::map<int, sf::Texture> texturesPokemon;

    MoteurJeu moteur;
    std::array<int, TEAM_SIZE> equipe{};
    equipe.fill(-1);
    int candidatSelectionne = -1;
    std::unique_ptr<Pokemon_Attack> pokemonAttack;
    bool captureTerminee = false;
    bool captureReussie = false;
    bool victoire = false;
    double pvJoueur = 0.0;
    double pvAdversaire = 0.0;
    std::string messageCapture;
    std::string messageCombat;
    std::mt19937 aleatoire(std::random_device{}());
    const sf::FloatRect valider(940.f, 676.f, 260.f, 54.f);
    std::array<sf::FloatRect, TEAM_SIZE> slots{};
    for (std::size_t i = 0; i < TEAM_SIZE; ++i)
        slots[i] = {760.f + static_cast<float>(i % 2) * 230.f,
                    170.f + static_cast<float>(i / 2) * 150.f, 205.f, 120.f};

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            const bool action = event.type == sf::Event::KeyPressed || event.type == sf::Event::MouseButtonPressed;
            if (moteur.id() == IdEtat::Accueil && action) {
                moteur.transition(IdEtat::SelectionEquipe);
            } else if (moteur.id() == IdEtat::SelectionEquipe && event.type == sf::Event::MouseButtonPressed &&
                       event.mouseButton.button == sf::Mouse::Left) {
                const sf::Vector2i souris(event.mouseButton.x, event.mouseButton.y);
                const auto& pokemons = party.getPokemons();
                for (std::size_t i = 0; i < pokemons.size(); ++i) {
                    const sf::FloatRect zone(45.f + static_cast<float>(i % 3) * 220.f,
                                             170.f + static_cast<float>(i / 3) * 115.f, 195.f, 90.f);
                    if (contient(zone, souris)) candidatSelectionne = static_cast<int>(i);
                }
                for (std::size_t slot = 0; slot < TEAM_SIZE; ++slot) if (contient(slots[slot], souris)) {
                    if (candidatSelectionne >= 0) {
                        int ancienSlot = -1;
                        for (std::size_t i = 0; i < TEAM_SIZE; ++i)
                            if (equipe[i] == candidatSelectionne) ancienSlot = static_cast<int>(i);
                        if (ancienSlot >= 0) std::swap(equipe[slot], equipe[ancienSlot]);
                        else equipe[slot] = candidatSelectionne;
                        candidatSelectionne = -1;
                    } else if (equipe[slot] >= 0) candidatSelectionne = equipe[slot];
                }
                const bool complete = std::all_of(equipe.begin(), equipe.end(), [](int i) { return i >= 0; });
                if (complete && contient(valider, souris)) {
                    pokemonAttack = std::make_unique<Pokemon_Attack>();
                    for (int i : equipe) pokemonAttack->addPokemon(party.getPokemons()[i]->clone());
                    moteur.transition(IdEtat::Exploration);
                }
            } else if (moteur.id() == IdEtat::Exploration && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::C) {
                    captureTerminee = false;
                    captureReussie = false;
                    messageCapture = "Un Pokemon sauvage apparait !";
                    moteur.transition(IdEtat::RencontreCapture);
                }
                if (event.key.code == sf::Keyboard::A) {
                    const auto& catalogue = pokedex->getPokemons();
                    const Pokemon* joueur = pokemonAttack->getPokemons().front();
                    const Pokemon* adversaire = catalogue.size() > CANDIDATE_COUNT + 1 ? catalogue[CANDIDATE_COUNT + 1] : joueur;
                    pvJoueur = joueur->getHitPointMax();
                    pvAdversaire = adversaire->getHitPointMax();
                    messageCombat = "Le combat commence !";
                    moteur.transition(IdEtat::CombatArene);
                }
            } else if (moteur.id() == IdEtat::RencontreCapture && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::C && !captureTerminee) {
                    const auto& catalogue = pokedex->getPokemons();
                    captureTerminee = true;
                    captureReussie = std::uniform_int_distribution<int>(1, 100)(aleatoire) <= 65;
                    if (captureReussie && catalogue.size() > CANDIDATE_COUNT) {
                        party.addPokemon(catalogue[CANDIDATE_COUNT]->clone());
                        messageCapture = "Capture reussie ! Le Pokemon rejoint votre Party.";
                    } else {
                        messageCapture = "La Pokeball se brise : le Pokemon s'echappe.";
                    }
                } else if (event.key.code == sf::Keyboard::Escape) {
                    moteur.transition(IdEtat::Exploration);
                }
            } else if (moteur.id() == IdEtat::CombatArene && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    const Pokemon* joueur = pokemonAttack->getPokemons().front();
                    const auto& catalogue = pokedex->getPokemons();
                    const Pokemon* adversaire = catalogue.size() > CANDIDATE_COUNT + 1 ? catalogue[CANDIDATE_COUNT + 1] : joueur;
                    const double degatsJoueur = degats(*joueur, *adversaire);
                    pvAdversaire = std::max(0.0, pvAdversaire - degatsJoueur);
                    messageCombat = joueur->getName() + " inflige " + std::to_string(static_cast<int>(degatsJoueur)) + " degats.";
                    if (pvAdversaire <= 0.0) {
                        victoire = true;
                        moteur.transition(IdEtat::GameOver);
                    } else {
                        const double degatsEnnemi = degats(*adversaire, *joueur);
                        pvJoueur = std::max(0.0, pvJoueur - degatsEnnemi);
                        messageCombat += "  " + adversaire->getName() + " contre-attaque : " +
                                         std::to_string(static_cast<int>(degatsEnnemi)) + " degats.";
                        if (pvJoueur <= 0.0) {
                            victoire = false;
                            moteur.transition(IdEtat::GameOver);
                        }
                    }
                } else if (event.key.code == sf::Keyboard::Escape) {
                    moteur.transition(IdEtat::Exploration);
                }
            } else if (moteur.id() == IdEtat::GameOver && action) moteur.transition(IdEtat::Accueil);
        }

        window.clear(sf::Color(19, 31, 52));
        if (moteur.id() == IdEtat::Accueil) {
            texte(window, font, "POKEMON ATTACK", 52, {365.f, 220.f}, sf::Color(255, 215, 70));
            texte(window, font, "Constituez votre equipe de six Pokemon", 27, {365.f, 310.f});
            texte(window, font, "Appuyez sur une touche ou cliquez pour commencer", 21, {345.f, 390.f}, sf::Color(190, 210, 235));
        } else if (moteur.id() == IdEtat::SelectionEquipe) {
            texte(window, font, "Choisissez vos Pokemon", 32, {45.f, 45.f}, sf::Color(255, 215, 70));
            texte(window, font, "1. Cliquez un Pokemon de la Party   2. Cliquez sa position dans l'equipe", 18, {45.f, 95.f}, sf::Color(195, 215, 240));
            texte(window, font, "Pokemon Party", 25, {45.f, 135.f});
            texte(window, font, "Pokemon Attack (6 positions)", 25, {760.f, 135.f});
            const auto& pokemons = party.getPokemons();
            for (std::size_t i = 0; i < pokemons.size(); ++i) {
                const sf::FloatRect zone(45.f + static_cast<float>(i % 3) * 220.f, 170.f + static_cast<float>(i / 3) * 115.f, 195.f, 90.f);
                carte(window, font, textureJauge, texturesPokemon, zone, pokemons[i], candidatSelectionne == static_cast<int>(i));
            }
            for (std::size_t i = 0; i < TEAM_SIZE; ++i) {
                const Pokemon* pokemon = equipe[i] >= 0 ? party.getPokemons()[equipe[i]] : nullptr;
                carte(window, font, textureJauge, texturesPokemon, slots[i], pokemon, false, static_cast<int>(i + 1));
            }
            const bool complete = std::all_of(equipe.begin(), equipe.end(), [](int i) { return i >= 0; });
            bouton(window, font, valider, "VALIDER L'EQUIPE", complete);
            texte(window, font, complete ? "Equipe complete : vous pouvez valider." : "Selectionnez 6 Pokemon.", 17, {760.f, 620.f}, complete ? sf::Color(150, 240, 170) : sf::Color(220, 190, 130));
        } else if (moteur.id() == IdEtat::Exploration) {
            texte(window, font, "Exploration", 42, {520.f, 170.f}, sf::Color(255, 215, 70));
            texte(window, font, "Votre Pokemon Attack est pret avec " + std::to_string(pokemonAttack->size()) + " Pokemon.", 24, {360.f, 275.f});
            texte(window, font, "C : rencontre / capture     A : combat dans l'arene", 22, {335.f, 365.f}, sf::Color(190, 210, 235));
        } else if (moteur.id() == IdEtat::RencontreCapture) {
            texte(window, font, "Rencontre / Capture de Pokemon", 38, {365.f, 130.f}, sf::Color(255, 215, 70));
            const auto& catalogue = pokedex->getPokemons();
            const Pokemon* sauvage = catalogue.size() > CANDIDATE_COUNT ? catalogue[CANDIDATE_COUNT] : nullptr;
            carte(window, font, textureJauge, texturesPokemon, {510.f, 230.f, 260.f, 135.f}, sauvage, false);
            if (captureTerminee) {
                texte(window, font, messageCapture, 23, {350.f, 455.f},
                      captureReussie ? sf::Color(150, 240, 170) : sf::Color(255, 145, 130));
                texte(window, font, "ECHAP : retourner a l'exploration", 20, {440.f, 520.f});
            } else {
                texte(window, font, messageCapture, 22, {430.f, 430.f}, sf::Color(190, 210, 235));
                texte(window, font, "C : lancer une Pokeball (65 % de chance)     ECHAP : fuir", 22, {335.f, 485.f});
            }
        } else if (moteur.id() == IdEtat::CombatArene) {
            texte(window, font, "Combat dans l'arene", 42, {440.f, 80.f}, sf::Color(255, 215, 70));
            const Pokemon* joueur = pokemonAttack->getPokemons().front();
            const auto& catalogue = pokedex->getPokemons();
            const Pokemon* adversaire = catalogue.size() > CANDIDATE_COUNT + 1 ? catalogue[CANDIDATE_COUNT + 1] : joueur;
            carte(window, font, textureJauge, texturesPokemon, {130.f, 235.f, 290.f, 150.f}, joueur, false);
            carte(window, font, textureJauge, texturesPokemon, {860.f, 235.f, 290.f, 150.f}, adversaire, false);
            sf::Sprite versus(textureVs);
            versus.setPosition(586.f, 255.f);
            versus.setScale(1.25f, 1.25f);
            window.draw(versus);
            texte(window, font, "Vos PV : " + std::to_string(static_cast<int>(pvJoueur)) + "/" + std::to_string(static_cast<int>(joueur->getHitPointMax())), 20, {130.f, 420.f});
            afficherJauge(window, textureJauge, {340.f, 405.f}, static_cast<float>(pvJoueur / joueur->getHitPointMax()), 0.8f);
            texte(window, font, "PV adversaire : " + std::to_string(static_cast<int>(pvAdversaire)) + "/" + std::to_string(static_cast<int>(adversaire->getHitPointMax())), 20, {860.f, 420.f});
            afficherJauge(window, textureJauge, {1110.f, 405.f}, static_cast<float>(pvAdversaire / adversaire->getHitPointMax()), 0.8f);
            texte(window, font, messageCombat, 18, {235.f, 515.f}, sf::Color(255, 225, 150));
            texte(window, font, "ESPACE : attaquer     ECHAP : fuir", 22, {460.f, 575.f}, sf::Color(190, 210, 235));
        } else {
            texte(window, font, victoire ? "VICTOIRE !" : "GAME OVER", 52, {475.f, 260.f},
                  victoire ? sf::Color(150, 240, 170) : sf::Color(235, 82, 76));
            texte(window, font, victoire ? "L'adversaire n'a plus de points de vie." : "Votre Pokemon n'a plus de points de vie.", 23, {370.f, 360.f});
            texte(window, font, "Cliquez pour revenir a l'accueil", 20, {450.f, 440.f}, sf::Color(190, 210, 235));
        }
        window.display();
    }
    return 0;
}
