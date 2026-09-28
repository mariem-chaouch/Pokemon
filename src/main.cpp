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
#include <tuple>

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

void carte(sf::RenderWindow& window, const sf::Font& font,
           std::map<int, sf::Texture>& texturesPokemon, const sf::FloatRect& zone,
           const Pokemon* pokemon, bool selectionnee, int position = 0) {
    sf::RectangleShape fond({zone.width, zone.height});
    fond.setPosition(zone.left, zone.top);
    fond.setFillColor(pokemon ? sf::Color(245, 249, 255) : sf::Color(35, 48, 73));
    fond.setOutlineThickness(selectionnee ? 4.f : 2.f);
    fond.setOutlineColor(selectionnee ? sf::Color(255, 190, 55) : (pokemon ? sf::Color(130, 185, 220) : sf::Color(125, 150, 190)));
    window.draw(fond);

    if (!pokemon) {
        texte(window, font, "Emplacement " + std::to_string(position), 18, {zone.left + 18.f, zone.top + 20.f},
              sf::Color(175, 190, 215));
        texte(window, font, "Cliquez pour placer", 15, {zone.left + 18.f, zone.top + 55.f}, sf::Color(130, 150, 180));
        return;
    }
    if (const sf::Texture* texture = texturePokemon(pokemon->getId(), texturesPokemon)) {
        sf::Sprite sprite(*texture);
        const sf::Vector2u taille = texture->getSize();
        const float echelle = std::min((zone.height - 18.f) / taille.x, (zone.height - 18.f) / taille.y);
        sprite.setScale(echelle, echelle);
        sprite.setPosition(zone.left + (zone.width - taille.x * echelle) / 2.f,
                           zone.top + 5.f + (zone.height - 18.f - taille.y * echelle) / 2.f);
        window.draw(sprite);
    }
    sf::RectangleShape bandeNom({zone.width, 30.f});
    bandeNom.setPosition(zone.left, zone.top + zone.height - 30.f);
    bandeNom.setFillColor(sf::Color(25, 54, 83, 238));
    window.draw(bandeNom);
    sf::Text nom(pokemon->getName(), font, 17);
    const auto limites = nom.getLocalBounds();
    nom.setPosition(zone.left + (zone.width - limites.width) / 2.f - limites.left,
                    zone.top + zone.height - 27.f - limites.top);
    nom.setFillColor(sf::Color::White);
    window.draw(nom);
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
    const sf::FloatRect lancer(490.f, 535.f, 300.f, 76.f);
    const sf::FloatRect actionCapture(490.f, 490.f, 300.f, 62.f);
    const sf::FloatRect retourCapture(490.f, 575.f, 300.f, 52.f);
    const sf::FloatRect actionExploration1(280.f, 485.f, 300.f, 72.f);
    const sf::FloatRect actionExploration2(700.f, 485.f, 300.f, 72.f);
    const sf::FloatRect actionCombat(490.f, 595.f, 300.f, 62.f);
    const sf::FloatRect quitterCombat(940.f, 48.f, 230.f, 50.f);
    const sf::FloatRect retourAccueil(440.f, 455.f, 400.f, 68.f);
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
                const bool clavier = event.type == sf::Event::KeyPressed &&
                                     (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space);
                const bool clic = event.type == sf::Event::MouseButtonPressed &&
                                  event.mouseButton.button == sf::Mouse::Left &&
                                  contient(lancer, {event.mouseButton.x, event.mouseButton.y});
                if (clavier || clic) moteur.transition(IdEtat::SelectionEquipe);
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
            } else if (moteur.id() == IdEtat::Exploration &&
                       (event.type == sf::Event::KeyPressed ||
                        (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left))) {
                const bool clic = event.type == sf::Event::MouseButtonPressed;
                const sf::Vector2i souris = clic ? sf::Vector2i(event.mouseButton.x, event.mouseButton.y) : sf::Vector2i(-1, -1);
                if ((!clic && event.key.code == sf::Keyboard::C) || (clic && contient(actionExploration1, souris))) {
                    captureTerminee = false;
                    captureReussie = false;
                    messageCapture = "Un Pokemon sauvage apparait !";
                    moteur.transition(IdEtat::RencontreCapture);
                }
                if ((!clic && event.key.code == sf::Keyboard::A) || (clic && contient(actionExploration2, souris))) {
                    const auto& catalogue = pokedex->getPokemons();
                    const Pokemon* joueur = pokemonAttack->getPokemons().front();
                    const Pokemon* adversaire = catalogue.size() > CANDIDATE_COUNT + 1 ? catalogue[CANDIDATE_COUNT + 1] : joueur;
                    pvJoueur = joueur->getHitPointMax();
                    pvAdversaire = adversaire->getHitPointMax();
                    messageCombat = "Le combat commence !";
                    moteur.transition(IdEtat::CombatArene);
                }
            } else if (moteur.id() == IdEtat::RencontreCapture &&
                       (event.type == sf::Event::KeyPressed ||
                        (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left))) {
                const bool clic = event.type == sf::Event::MouseButtonPressed;
                const sf::Vector2i souris = clic ? sf::Vector2i(event.mouseButton.x, event.mouseButton.y) : sf::Vector2i(-1, -1);
                if (((!clic && event.key.code == sf::Keyboard::C) || (clic && contient(actionCapture, souris))) && !captureTerminee) {
                    const auto& catalogue = pokedex->getPokemons();
                    captureTerminee = true;
                    captureReussie = std::uniform_int_distribution<int>(1, 100)(aleatoire) <= 65;
                    if (captureReussie && catalogue.size() > CANDIDATE_COUNT) {
                        party.addPokemon(catalogue[CANDIDATE_COUNT]->clone());
                        messageCapture = "Capture reussie ! Le Pokemon rejoint votre Party.";
                    } else {
                        messageCapture = "La Pokeball se brise : le Pokemon s'echappe.";
                    }
                } else if ((!clic && event.key.code == sf::Keyboard::Escape) || (clic && contient(retourCapture, souris))) {
                    moteur.transition(IdEtat::Exploration);
                }
            } else if (moteur.id() == IdEtat::CombatArene &&
                       (event.type == sf::Event::KeyPressed ||
                        (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left))) {
                const bool clic = event.type == sf::Event::MouseButtonPressed;
                const sf::Vector2i souris = clic ? sf::Vector2i(event.mouseButton.x, event.mouseButton.y) : sf::Vector2i(-1, -1);
                if ((!clic && event.key.code == sf::Keyboard::Space) || (clic && contient(actionCombat, souris))) {
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
                } else if ((!clic && event.key.code == sf::Keyboard::Escape) || (clic && contient(quitterCombat, souris))) {
                    moteur.transition(IdEtat::Exploration);
                }
            } else if (moteur.id() == IdEtat::GameOver && action) {
                const bool clic = event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left &&
                                  contient(retourAccueil, {event.mouseButton.x, event.mouseButton.y});
                const bool clavier = event.type == sf::Event::KeyPressed &&
                                     (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space);
                if (clic || clavier) moteur.transition(IdEtat::Accueil);
            }
        }

        window.clear(sf::Color(19, 31, 52));
        if (moteur.id() == IdEtat::Accueil) {
            // Ecran d'accueil dessine avec les sprites du Pokedex pour rester autonome.
            sf::RectangleShape ciel({1280.f, 760.f});
            ciel.setFillColor(sf::Color(105, 190, 231));
            window.draw(ciel);
            sf::CircleShape soleil(72.f);
            soleil.setPosition(1000.f, 105.f);
            soleil.setFillColor(sf::Color(255, 226, 126, 210));
            window.draw(soleil);
            sf::CircleShape collineGauche(300.f), collineDroite(340.f);
            collineGauche.setPosition(-100.f, 455.f);
            collineDroite.setPosition(790.f, 445.f);
            collineGauche.setFillColor(sf::Color(75, 174, 111));
            collineDroite.setFillColor(sf::Color(55, 151, 104));
            window.draw(collineGauche);
            window.draw(collineDroite);
            sf::RectangleShape sol({1280.f, 170.f});
            sol.setPosition(0.f, 590.f);
            sol.setFillColor(sf::Color(70, 164, 105));
            window.draw(sol);

            // Voile discret pour garder le titre et le bouton lisibles.
            sf::RectangleShape panneau({600.f, 465.f});
            panneau.setPosition(340.f, 35.f);
            panneau.setFillColor(sf::Color(24, 79, 133, 105));
            panneau.setOutlineThickness(3.f);
            panneau.setOutlineColor(sf::Color(255, 255, 255, 130));
            window.draw(panneau);
            texte(window, font, "POKEMON GO", 58, {390.f, 76.f}, sf::Color(255, 213, 61));
            texte(window, font, "L'aventure commence ici", 25, {475.f, 164.f}, sf::Color::White);

            for (const auto& [id, position, echelle] :
                 std::array<std::tuple<int, sf::Vector2f, float>, 4>{{
                     {1, {40.f, 360.f}, 2.0f}, {25, {130.f, 470.f}, 2.25f},
                     {6, {1020.f, 335.f}, 2.15f}, {25, {1095.f, 475.f}, 1.85f}}}) {
                if (const sf::Texture* texture = texturePokemon(id, texturesPokemon)) {
                    sf::Sprite pokemon(*texture);
                    pokemon.setScale(echelle, echelle);
                    pokemon.setPosition(position);
                    window.draw(pokemon);
                }
            }
            bouton(window, font, lancer, "JOUER", true);
            texte(window, font, "Entree ou Espace pour commencer", 17, {500.f, 635.f}, sf::Color(239, 250, 255));
        } else if (moteur.id() == IdEtat::SelectionEquipe) {
            sf::RectangleShape bandeau({1280.f, 130.f});
            bandeau.setFillColor(sf::Color(30, 51, 82));
            window.draw(bandeau);
            texte(window, font, "PREPARE TON EQUIPE", 34, {45.f, 28.f}, sf::Color(255, 215, 70));
            texte(window, font, "Choisis un Pokemon a gauche, puis une place libre a droite.", 18, {48.f, 78.f}, sf::Color(195, 215, 240));
            texte(window, font, "TA COLLECTION", 21, {45.f, 137.f}, sf::Color(130, 215, 255));
            texte(window, font, "TON EQUIPE - 6", 21, {760.f, 137.f}, sf::Color(130, 215, 255));
            sf::RectangleShape separateur({3.f, 470.f});
            separateur.setPosition(720.f, 165.f);
            separateur.setFillColor(sf::Color(70, 98, 132));
            window.draw(separateur);
            const auto& pokemons = party.getPokemons();
            for (std::size_t i = 0; i < pokemons.size(); ++i) {
                const sf::FloatRect zone(45.f + static_cast<float>(i % 3) * 220.f, 170.f + static_cast<float>(i / 3) * 115.f, 195.f, 90.f);
                carte(window, font, texturesPokemon, zone, pokemons[i], candidatSelectionne == static_cast<int>(i));
            }
            for (std::size_t i = 0; i < TEAM_SIZE; ++i) {
                const Pokemon* pokemon = equipe[i] >= 0 ? party.getPokemons()[equipe[i]] : nullptr;
                carte(window, font, texturesPokemon, slots[i], pokemon, false, static_cast<int>(i + 1));
            }
            const bool complete = std::all_of(equipe.begin(), equipe.end(), [](int i) { return i >= 0; });
            bouton(window, font, valider, "VALIDER L'EQUIPE", complete);
            texte(window, font, complete ? "Equipe complete : vous pouvez valider." : "Selectionnez 6 Pokemon.", 17, {760.f, 620.f}, complete ? sf::Color(150, 240, 170) : sf::Color(220, 190, 130));
        } else if (moteur.id() == IdEtat::Exploration) {
            sf::RectangleShape ciel({1280.f, 760.f}); ciel.setFillColor(sf::Color(104, 183, 218)); window.draw(ciel);
            sf::CircleShape colline(260.f); colline.setPosition(425.f, 360.f); colline.setFillColor(sf::Color(76, 166, 107)); window.draw(colline);
            sf::RectangleShape sol({1280.f, 180.f}); sol.setPosition(0.f, 580.f); sol.setFillColor(sf::Color(63, 147, 91)); window.draw(sol);
            texte(window, font, "EXPLORATION", 40, {490.f, 45.f}, sf::Color(255, 215, 70));
            const Pokemon* compagnon = pokemonAttack->getPokemons().front();
            if (const sf::Texture* texture = texturePokemon(compagnon->getId(), texturesPokemon)) {
                sf::Sprite sprite(*texture);
                sprite.setScale(3.5f, 3.5f);
                sprite.setPosition(545.f, 280.f);
                window.draw(sprite);
            }
            sf::RectangleShape info({500.f, 74.f}); info.setPosition(390.f, 190.f); info.setFillColor(sf::Color(22, 65, 105, 220)); window.draw(info);
            texte(window, font, compagnon->getName() + " t'accompagne", 24, {500.f, 204.f}, sf::Color::White);
            texte(window, font, std::to_string(pokemonAttack->size()) + " Pokemon dans ton equipe", 18, {525.f, 238.f}, sf::Color(198, 228, 246));
            bouton(window, font, actionExploration1, "RENCONTRER UN POKEMON", true);
            bouton(window, font, actionExploration2, "DEFIER L'ARENE", true);
            texte(window, font, "C : rencontre     A : arene", 17, {520.f, 590.f}, sf::Color(240, 250, 255));
        } else if (moteur.id() == IdEtat::RencontreCapture) {
            sf::RectangleShape fond({1280.f, 760.f}); fond.setFillColor(sf::Color(32, 54, 83)); window.draw(fond);
            texte(window, font, "RENCONTRE SAUVAGE", 38, {420.f, 72.f}, sf::Color(255, 215, 70));
            texte(window, font, "Un Pokemon apparait dans les hautes herbes...", 20, {415.f, 132.f}, sf::Color(195, 215, 240));
            const auto& catalogue = pokedex->getPokemons();
            const Pokemon* sauvage = catalogue.size() > CANDIDATE_COUNT ? catalogue[CANDIDATE_COUNT] : nullptr;
            carte(window, font, texturesPokemon, {430.f, 220.f, 420.f, 190.f}, sauvage, false);
            if (captureTerminee) {
                texte(window, font, messageCapture, 21, {320.f, 440.f},
                      captureReussie ? sf::Color(150, 240, 170) : sf::Color(255, 145, 130));
                bouton(window, font, retourCapture, "CONTINUER L'EXPLORATION", true);
            } else {
                texte(window, font, messageCapture, 20, {425.f, 435.f}, sf::Color(190, 210, 235));
                bouton(window, font, actionCapture, "LANCER UNE POKEBALL", true);
                texte(window, font, "C : lancer     Echap : fuir", 17, {515.f, 580.f}, sf::Color(190, 210, 235));
            }
        } else if (moteur.id() == IdEtat::CombatArene) {
            sf::RectangleShape fond({1280.f, 760.f}); fond.setFillColor(sf::Color(39, 42, 74)); window.draw(fond);
            sf::RectangleShape haut({1280.f, 130.f}); haut.setFillColor(sf::Color(28, 31, 59)); window.draw(haut);
            texte(window, font, "COMBAT D'ARENE", 38, {430.f, 38.f}, sf::Color(255, 215, 70));
            bouton(window, font, quitterCombat, "FUIR LE COMBAT", false);
            const Pokemon* joueur = pokemonAttack->getPokemons().front();
            const auto& catalogue = pokedex->getPokemons();
            const Pokemon* adversaire = catalogue.size() > CANDIDATE_COUNT + 1 ? catalogue[CANDIDATE_COUNT + 1] : joueur;
            carte(window, font, texturesPokemon, {130.f, 235.f, 290.f, 150.f}, joueur, false);
            carte(window, font, texturesPokemon, {860.f, 235.f, 290.f, 150.f}, adversaire, false);
            sf::Sprite versus(textureVs);
            versus.setPosition(586.f, 255.f);
            versus.setScale(1.25f, 1.25f);
            window.draw(versus);
            texte(window, font, "VOS PV : " + std::to_string(static_cast<int>(pvJoueur)) + "/" + std::to_string(static_cast<int>(joueur->getHitPointMax())), 20, {130.f, 420.f});
            afficherJauge(window, textureJauge, {340.f, 405.f}, static_cast<float>(pvJoueur / joueur->getHitPointMax()), 0.8f);
            texte(window, font, "PV ADVERSE : " + std::to_string(static_cast<int>(pvAdversaire)) + "/" + std::to_string(static_cast<int>(adversaire->getHitPointMax())), 20, {860.f, 420.f});
            afficherJauge(window, textureJauge, {1110.f, 405.f}, static_cast<float>(pvAdversaire / adversaire->getHitPointMax()), 0.8f);
            texte(window, font, messageCombat, 18, {235.f, 515.f}, sf::Color(255, 225, 150));
            bouton(window, font, actionCombat, "ATTAQUER", true);
            texte(window, font, "Espace : attaquer     Echap : fuir", 17, {475.f, 675.f}, sf::Color(190, 210, 235));
        } else {
            sf::RectangleShape fond({1280.f, 760.f}); fond.setFillColor(victoire ? sf::Color(36, 83, 73) : sf::Color(63, 39, 57)); window.draw(fond);
            texte(window, font, victoire ? "VICTOIRE !" : "FIN DE PARTIE", 52, {475.f, 220.f},
                  victoire ? sf::Color(150, 240, 170) : sf::Color(235, 82, 76));
            texte(window, font, victoire ? "L'adversaire n'a plus de points de vie." : "Votre Pokemon n'a plus de points de vie.", 23, {370.f, 330.f});
            bouton(window, font, retourAccueil, "RETOUR A L'ACCUEIL", true);
            texte(window, font, "Entree ou Espace pour continuer", 17, {485.f, 545.f}, sf::Color(190, 210, 235));
        }
        window.display();
    }
    return 0;
}
