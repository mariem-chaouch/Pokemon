#include "Etat.h"

#include <algorithm>
#include <cmath>

std::size_t dernierDebutDePage(std::size_t nombrePokemon);

namespace {
constexpr std::size_t TEAM_SIZE = 6;
constexpr std::size_t CANDIDATS_VISIBLES = 12;

const sf::FloatRect valider(940.f, 676.f, 260.f, 54.f);
const sf::FloatRect lancer(490.f, 342.f, 300.f, 76.f);
const sf::FloatRect listeCandidats(35.f, 160.f, 665.f, 455.f);
const sf::FloatRect pisteDefilement(698.f, 170.f, 12.f, 430.f);
const sf::FloatRect actionCapture(490.f, 490.f, 300.f, 62.f);
const sf::FloatRect retourCapture(490.f, 575.f, 300.f, 52.f);
const sf::FloatRect actionExploration1(280.f, 485.f, 300.f, 72.f);
const sf::FloatRect actionExploration2(700.f, 485.f, 300.f, 72.f);
const sf::FloatRect ouvrirCollection(1010.f, 48.f, 220.f, 48.f);
const sf::FloatRect retourCollection(990.f, 48.f, 220.f, 48.f);
const sf::FloatRect pageCollectionPrecedente(470.f, 675.f, 100.f, 42.f);
const sf::FloatRect pageCollectionSuivante(590.f, 675.f, 100.f, 42.f);
const sf::FloatRect actionCombat(490.f, 595.f, 300.f, 62.f);
const sf::FloatRect quitterCombat(940.f, 48.f, 230.f, 50.f);
const sf::FloatRect retourAccueil(440.f, 455.f, 400.f, 68.f);

std::array<sf::FloatRect, TEAM_SIZE> creerZonesEquipe() {
    std::array<sf::FloatRect, TEAM_SIZE> zones{};
    for (std::size_t i = 0; i < TEAM_SIZE; ++i)
        zones[i] = {760.f + static_cast<float>(i % 2) * 230.f,
                    170.f + static_cast<float>(i / 2) * 150.f, 205.f, 120.f};
    return zones;
}

// Centralise la conversion des coordonnees souris en test de zone cliquable.
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

// La texture contient 13 segments ; le segment choisi depend des PV restants.
void afficherJauge(sf::RenderWindow& window, const sf::Texture& texture, sf::Vector2f position,
                   float pourcentage, float echelle = 0.55f) {
    const int index = std::clamp(static_cast<int>((1.f - pourcentage) * 12.f), 0, 12);
    sf::Sprite jauge(texture, sf::IntRect(index * 30, 0, 30, 80));
    jauge.setPosition(position);
    jauge.setScale(echelle, echelle);
    window.draw(jauge);
}

// Dessine une carte reutilisable pour la selection, la collection et les equipes.
void carte(ContexteJeu& contexte, const sf::FloatRect& zone, const Pokemon* pokemon,
           bool selectionnee, int position = 0) {
    sf::RectangleShape fond({zone.width, zone.height});
    fond.setPosition(zone.left, zone.top);
    fond.setFillColor(pokemon ? sf::Color(245, 249, 255) : sf::Color(35, 48, 73));
    fond.setOutlineThickness(selectionnee ? 4.f : 2.f);
    fond.setOutlineColor(selectionnee ? sf::Color(255, 190, 55) :
                         (pokemon ? sf::Color(130, 185, 220) : sf::Color(125, 150, 190)));
    contexte.window.draw(fond);

    if (!pokemon) {
        texte(contexte.window, contexte.font, "Emplacement " + std::to_string(position), 18,
              {zone.left + 18.f, zone.top + 20.f}, sf::Color(175, 190, 215));
        texte(contexte.window, contexte.font, "Cliquez pour placer", 15,
              {zone.left + 18.f, zone.top + 55.f}, sf::Color(130, 150, 180));
        return;
    }
    if (const sf::Texture* texture = contexte.texturePokemon(pokemon->getId())) {
        sf::Sprite sprite(*texture);
        const sf::Vector2u taille = texture->getSize();
        const float echelle = std::min((zone.height - 18.f) / taille.x, (zone.height - 18.f) / taille.y);
        sprite.setScale(echelle, echelle);
        sprite.setPosition(zone.left + (zone.width - taille.x * echelle) / 2.f,
                           zone.top + 5.f + (zone.height - 18.f - taille.y * echelle) / 2.f);
        contexte.window.draw(sprite);
    }
    if (position > 0) {
        sf::CircleShape numero(17.f);
        numero.setPosition(zone.left + 7.f, zone.top + 7.f);
        numero.setFillColor(sf::Color(255, 190, 55));
        contexte.window.draw(numero);
        sf::Text ordre(std::to_string(position), contexte.font, 19);
        ordre.setFillColor(sf::Color(25, 54, 83));
        ordre.setPosition(zone.left + 17.f - ordre.getLocalBounds().width / 2.f,
                          zone.top + 4.f);
        contexte.window.draw(ordre);
    }
    sf::RectangleShape bandeNom({zone.width, 30.f});
    bandeNom.setPosition(zone.left, zone.top + zone.height - 30.f);
    bandeNom.setFillColor(sf::Color(25, 54, 83, 238));
    contexte.window.draw(bandeNom);
    sf::Text nom(pokemon->getName(), contexte.font, 17);
    const auto limites = nom.getLocalBounds();
    nom.setPosition(zone.left + (zone.width - limites.width) / 2.f - limites.left,
                    zone.top + zone.height - 27.f - limites.top);
    nom.setFillColor(sf::Color::White);
    contexte.window.draw(nom);
}

// Le survol est visuel uniquement : les clics sont geres par chaque etat.
void bouton(ContexteJeu& contexte, const sf::FloatRect& zone,
            const std::string& libelle, bool actif) {
    sf::RectangleShape fond({zone.width, zone.height});
    fond.setPosition(zone.left, zone.top);
    const sf::Vector2i souris = sf::Mouse::getPosition(contexte.window);
    const bool survole = actif && contient(zone, souris);
    fond.setFillColor(!actif ? sf::Color(67, 83, 103) :
                      survole ? sf::Color(52, 172, 195) : sf::Color(34, 126, 164));
    fond.setOutlineThickness(2.f);
    fond.setOutlineColor(actif ? sf::Color(124, 218, 229) : sf::Color(105, 123, 143));
    contexte.window.draw(fond);
    sf::Text label(libelle, contexte.font, 20);
    const auto limites = label.getLocalBounds();
    label.setPosition(zone.left + (zone.width - limites.width) / 2.f - limites.left,
                      zone.top + (zone.height - limites.height) / 2.f - limites.top);
    contexte.window.draw(label);
}

double degats(const Pokemon& attaquant, const Pokemon& defenseur) {
    return std::max(1.0, attaquant.getAttack() - defenseur.getDefense() * 0.35);
}

bool estClicGauche(const sf::Event& evenement) {
    return evenement.type == sf::Event::MouseButtonPressed &&
           evenement.mouseButton.button == sf::Mouse::Left;
}
} // namespace

MoteurJeu::MoteurJeu() : etat_(std::make_unique<EtatAccueil>()) {}

void MoteurJeu::traiterEvenement(ContexteJeu& contexte, const sf::Event& evenement) {
    if (auto nouvelEtat = etat_->traiterEvenement(contexte, evenement)) {
        etat_ = std::move(nouvelEtat);
    }
}


void MoteurJeu::dessiner(ContexteJeu& contexte) const {
    etat_->dessiner(contexte);
}

std::unique_ptr<Etat> EtatAccueil::traiterEvenement(ContexteJeu&, const sf::Event& evenement) {
    if (estClicGauche(evenement) && contient(lancer, {evenement.mouseButton.x, evenement.mouseButton.y})) {
        return std::make_unique<EtatSelectionEquipe>();
    }
    return nullptr;
}

void EtatAccueil::dessiner(ContexteJeu& contexte) const {
    auto& window = contexte.window;
    auto& font = contexte.font;
    const sf::Vector2u tailleFond = contexte.textureFondAccueil.getSize();
    sf::Sprite fond(contexte.textureFondAccueil);
    const float echelle = std::max(1280.f / static_cast<float>(tailleFond.x),
                                   760.f / static_cast<float>(tailleFond.y));
    fond.setScale(echelle, echelle);
    fond.setPosition((1280.f - tailleFond.x * echelle) / 2.f,
                     (760.f - tailleFond.y * echelle) / 2.f);
    window.draw(fond);

    sf::Text titre("POKEMON GO", font, 62);
    const sf::FloatRect limites = titre.getLocalBounds();
    titre.setOrigin(limites.left + limites.width / 2.f, limites.top + limites.height / 2.f);
    titre.setPosition(640.f, 74.f);
    titre.setFillColor(sf::Color(255, 215, 55));
    titre.setOutlineColor(sf::Color(20, 46, 80, 230));
    titre.setOutlineThickness(4.f);
    window.draw(titre);
    bouton(contexte, lancer, "JOUER", true);
}

std::size_t maximumDebutListe(std::size_t nombrePokemon) {
    return nombrePokemon > CANDIDATS_VISIBLES ? nombrePokemon - CANDIDATS_VISIBLES : 0;
}

sf::FloatRect curseurDefilement(std::size_t nombrePokemon, std::size_t debut) {
    const float hauteur = nombrePokemon == 0 ? pisteDefilement.height :
        std::max(28.f, pisteDefilement.height * std::min<std::size_t>(CANDIDATS_VISIBLES, nombrePokemon) /
                                    static_cast<float>(nombrePokemon));
    const std::size_t maximum = maximumDebutListe(nombrePokemon);
    const float progression = maximum == 0 ? 0.f : static_cast<float>(debut) / static_cast<float>(maximum);
    return {pisteDefilement.left, pisteDefilement.top + progression * (pisteDefilement.height - hauteur),
            pisteDefilement.width, hauteur};
}

void placerListeDepuisCurseur(ContexteJeu& contexte, float positionY) {
    const std::size_t nombrePokemon = contexte.pokedex.getPokemons().size();
    const sf::FloatRect curseur = curseurDefilement(nombrePokemon, contexte.debutListe);
    const float espace = pisteDefilement.height - curseur.height;
    const float progression = espace <= 0.f ? 0.f :
        std::clamp((positionY - pisteDefilement.top - curseur.height / 2.f) / espace, 0.f, 1.f);
    contexte.debutListe = static_cast<std::size_t>(progression * maximumDebutListe(nombrePokemon));
}

std::size_t dernierDebutDePage(std::size_t nombrePokemon) {
    // Retourne l'index de depart de la derniere page complete ou partielle.
    return nombrePokemon == 0 ? 0 : ((nombrePokemon - 1) / CANDIDATS_VISIBLES) * CANDIDATS_VISIBLES;
}

std::unique_ptr<Etat> EtatSelectionEquipe::traiterEvenement(ContexteJeu& c, const sf::Event& e) {
    const std::size_t taille = c.pokedex.getPokemons().size();
    if (e.type == sf::Event::MouseWheelScrolled &&
        listeCandidats.contains(e.mouseWheelScroll.x, e.mouseWheelScroll.y)) {
        const int pas = std::max(1, static_cast<int>(std::abs(e.mouseWheelScroll.delta)) * 3);
        if (e.mouseWheelScroll.delta < 0)
            c.debutListe = std::min(maximumDebutListe(taille), c.debutListe + static_cast<std::size_t>(pas));
        else
            c.debutListe = c.debutListe > static_cast<std::size_t>(pas) ? c.debutListe - pas : 0;
        return nullptr;
    }
    if (e.type == sf::Event::MouseMoved && c.defilementListeActif) {
        placerListeDepuisCurseur(c, static_cast<float>(e.mouseMove.y));
        return nullptr;
    }
    if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left) {
        c.defilementListeActif = false;
        return nullptr;
    }
    if (!estClicGauche(e)) return nullptr;

    const sf::Vector2i souris(e.mouseButton.x, e.mouseButton.y);
    if (contient(pisteDefilement, souris)) {
        if (!contient(curseurDefilement(taille, c.debutListe), souris))
            placerListeDepuisCurseur(c, static_cast<float>(souris.y));
        c.defilementListeActif = true;
        return nullptr;
    }
    const auto& pokemons = c.pokedex.getPokemons();
    const std::size_t visibles = c.debutListe < pokemons.size()
        ? std::min(CANDIDATS_VISIBLES, pokemons.size() - c.debutListe) : 0;
    for (std::size_t i = 0; i < visibles; ++i) {
        const sf::FloatRect zone(45.f + static_cast<float>(i % 3) * 220.f,
                                 170.f + static_cast<float>(i / 3) * 115.f, 195.f, 90.f);
        if (contient(zone, souris)) c.candidatSelectionne = static_cast<int>(c.debutListe + i);
    }
    // Un candidat selectionne est place dans une case ; le re-selectionner
    // depuis une autre case permute les deux membres de l'equipe.
    const auto zonesEquipe = creerZonesEquipe();
    for (std::size_t slot = 0; slot < TEAM_SIZE; ++slot) {
        if (!contient(zonesEquipe[slot], souris)) continue;
        if (c.candidatSelectionne >= 0) {
            int ancienSlot = -1;
            for (std::size_t i = 0; i < TEAM_SIZE; ++i)
                if (c.equipe[i] == c.candidatSelectionne) ancienSlot = static_cast<int>(i);
            if (ancienSlot >= 0) std::swap(c.equipe[slot], c.equipe[ancienSlot]);
            else c.equipe[slot] = c.candidatSelectionne;
            c.candidatSelectionne = -1;
        } else if (c.equipe[slot] >= 0) c.candidatSelectionne = c.equipe[slot];
    }

    // La validation ne devient disponible que lorsque les six cases sont remplies.
    const bool complete = std::all_of(c.equipe.begin(), c.equipe.end(), [](int i) { return i >= 0; });
    if (complete && contient(valider, souris)) {
        c.pokemonAttack = std::make_unique<Pokemon_Attack>();
        for (int i : c.equipe) {
            const Pokemon* pokemon = c.pokedex.getPokemons()[i];
            c.pokemonAttack->addPokemon(pokemon->clone());
            const bool dejaPossede = std::any_of(c.party.getPokemons().begin(), c.party.getPokemons().end(),
                [pokemon](const Pokemon* possede) { return possede->getId() == pokemon->getId(); });
            if (!dejaPossede) c.party.addPokemon(pokemon->clone());
        }
        return std::make_unique<EtatExploration>();
    }
    return nullptr;
}

void EtatSelectionEquipe::dessiner(ContexteJeu& c) const {
    auto& window = c.window;
    auto& font = c.font;
    sf::RectangleShape bandeau({1280.f, 130.f});
    bandeau.setFillColor(sf::Color(30, 51, 82));
    window.draw(bandeau);
    texte(window, font, "PREPARE TON EQUIPE", 34, {45.f, 24.f}, sf::Color(255, 215, 70));
    texte(window, font, "Choisis un Pokemon a gauche, puis une place libre a droite.", 18,
          {48.f, 78.f}, sf::Color(195, 215, 240));
    texte(window, font, "POKEDEX - TRI ALPHABETIQUE", 21, {45.f, 137.f}, sf::Color(130, 215, 255));
    texte(window, font, "TON EQUIPE - 6", 21, {760.f, 137.f}, sf::Color(130, 215, 255));
    const std::size_t taille = c.pokedex.getPokemons().size();
    const std::string navigation = "" + std::to_string(c.debutListe + (taille ? 1 : 0)) + "-" +
        std::to_string(std::min(c.debutListe + CANDIDATS_VISIBLES, taille)) + " / " +
        std::to_string(taille);
    texte(window, font, navigation, 14, {530.f, 143.f}, sf::Color(195, 215, 240));
    sf::RectangleShape separateur({3.f, 470.f});
    separateur.setPosition(720.f, 165.f);
    separateur.setFillColor(sf::Color(70, 98, 132));
    window.draw(separateur);

    const auto& pokemons = c.pokedex.getPokemons();
    const std::size_t visibles = c.debutListe < pokemons.size()
        ? std::min(CANDIDATS_VISIBLES, pokemons.size() - c.debutListe) : 0;
    for (std::size_t visible = 0; visible < visibles; ++visible) {
        const std::size_t index = c.debutListe + visible;
        const sf::FloatRect zone(45.f + static_cast<float>(visible % 3) * 220.f,
                                 170.f + static_cast<float>(visible / 3) * 115.f, 195.f, 90.f);
        carte(c, zone, pokemons[index], c.candidatSelectionne == static_cast<int>(index));
    }
    sf::RectangleShape piste({pisteDefilement.width, pisteDefilement.height});
    piste.setPosition(pisteDefilement.left, pisteDefilement.top);
    piste.setFillColor(sf::Color(46, 66, 91));
    window.draw(piste);
    const sf::FloatRect zoneCurseur = curseurDefilement(taille, c.debutListe);
    sf::RectangleShape curseur({zoneCurseur.width, zoneCurseur.height});
    curseur.setPosition(zoneCurseur.left, zoneCurseur.top);
    curseur.setFillColor(sf::Color(52, 172, 195));
    curseur.setOutlineThickness(1.f);
    curseur.setOutlineColor(sf::Color(150, 225, 235));
    window.draw(curseur);
    const auto zonesEquipe = creerZonesEquipe();
    for (std::size_t i = 0; i < TEAM_SIZE; ++i) {
        const Pokemon* pokemon = c.equipe[i] >= 0 ? pokemons[c.equipe[i]] : nullptr;
        carte(c, zonesEquipe[i], pokemon, false, static_cast<int>(i + 1));
    }
    const bool complete = std::all_of(c.equipe.begin(), c.equipe.end(), [](int i) { return i >= 0; });
    bouton(c, valider, "VALIDER L'EQUIPE", complete);
    texte(window, font, complete ? "Equipe complete : vous pouvez valider." : "Selectionnez 6 Pokemon.",
          17, {760.f, 620.f}, complete ? sf::Color(150, 240, 170) : sf::Color(220, 190, 130));
}

std::unique_ptr<Etat> EtatExploration::traiterEvenement(ContexteJeu& c, const sf::Event& e) {
    if (!estClicGauche(e)) return nullptr;
    const sf::Vector2i souris(e.mouseButton.x, e.mouseButton.y);
    if (contient(actionExploration1, souris)) {
        c.captureTerminee = false;
        c.captureReussie = false;
        const auto& catalogue = c.pokedex.getPokemons();
        if (!catalogue.empty())
            c.indiceSauvage = std::uniform_int_distribution<std::size_t>(0, catalogue.size() - 1)(c.aleatoire);
        c.messageCapture = "Un Pokemon sauvage apparait !";
        return std::make_unique<EtatRencontreCapture>();
    }
    if (contient(actionExploration2, souris)) {
        const auto& catalogue = c.pokedex.getPokemons();
        if (catalogue.empty() || c.pokemonAttack->getPokemons().size() < TEAM_SIZE) return nullptr;
        c.combattantJoueur = c.combattantAdverse = 0;
        for (std::size_t i = 0; i < TEAM_SIZE; ++i) {
            const Pokemon* joueur = c.pokemonAttack->getPokemons()[i];
            c.pvEquipeJoueur[i] = joueur->getHitPointMax();
            c.equipeAdverse[i] = static_cast<int>(std::uniform_int_distribution<std::size_t>(0, catalogue.size() - 1)(c.aleatoire));
            c.pvEquipeAdverse[i] = catalogue[c.equipeAdverse[i]]->getHitPointMax();
        }
        c.messageCombat = "Le combat commence !";
        return std::make_unique<EtatCombatArene>();
    }
    if (contient(ouvrirCollection, souris)) return std::make_unique<EtatCollection>();
    return nullptr;
}

void EtatExploration::dessiner(ContexteJeu& c) const {
    auto& window = c.window;
    auto& font = c.font;
    sf::RectangleShape ciel({1280.f, 760.f}); ciel.setFillColor(sf::Color(104, 183, 218)); window.draw(ciel);
    sf::RectangleShape bandeau({1280.f, 112.f}); bandeau.setFillColor(sf::Color(24, 50, 79, 235)); window.draw(bandeau);
    sf::CircleShape colline(260.f); colline.setPosition(425.f, 360.f); colline.setFillColor(sf::Color(76, 166, 107)); window.draw(colline);
    sf::RectangleShape sol({1280.f, 180.f}); sol.setPosition(0.f, 580.f); sol.setFillColor(sf::Color(63, 147, 91)); window.draw(sol);
    texte(window, font, "EXPLORATION", 40, {490.f, 45.f}, sf::Color(255, 215, 70));
    const Pokemon* compagnon = c.pokemonAttack->getPokemons().front();
    if (const sf::Texture* texture = c.texturePokemon(compagnon->getId())) {
        sf::Sprite sprite(*texture);
        sprite.setScale(3.5f, 3.5f);
        sprite.setPosition(545.f, 280.f);
        window.draw(sprite);
    }
    sf::RectangleShape info({500.f, 74.f}); info.setPosition(390.f, 190.f); info.setFillColor(sf::Color(22, 65, 105, 220)); window.draw(info);
    texte(window, font, compagnon->getName() + " t'accompagne", 24, {500.f, 204.f}, sf::Color::White);
    texte(window, font, std::to_string(c.pokemonAttack->size()) + " Pokemon dans ton equipe", 18, {525.f, 238.f}, sf::Color(198, 228, 246));
    bouton(c, actionExploration1, "RENCONTRER UN POKEMON", true);
    bouton(c, actionExploration2, "DEFIER L'ARENE", true);
    bouton(c, ouvrirCollection, "MA COLLECTION", true);
    texte(window, font, "Clique sur une action pour continuer", 17, {485.f, 590.f}, sf::Color(240, 250, 255));
}

std::unique_ptr<Etat> EtatCollection::traiterEvenement(ContexteJeu& c, const sf::Event& e) {
    if (!estClicGauche(e)) return nullptr;
    const sf::Vector2i souris(e.mouseButton.x, e.mouseButton.y);
    const std::size_t maximum = dernierDebutDePage(c.party.getPokemons().size());
    if (contient(pageCollectionPrecedente, souris)) {
        c.debutListe = c.debutListe > CANDIDATS_VISIBLES ? c.debutListe - CANDIDATS_VISIBLES : 0;
    } else if (contient(pageCollectionSuivante, souris)) {
        c.debutListe = std::min(maximum, c.debutListe + CANDIDATS_VISIBLES);
    } else if (contient(retourCollection, souris)) {
        return std::make_unique<EtatExploration>();
    }
    return nullptr;
}

void EtatCollection::dessiner(ContexteJeu& c) const {
    auto& window = c.window;
    sf::RectangleShape fond({1280.f, 760.f});
    fond.setFillColor(sf::Color(18, 38, 64));
    window.draw(fond);
    texte(window, c.font, "MA COLLECTION", 36, {45.f, 35.f}, sf::Color(255, 215, 70));
    bouton(c, retourCollection, "RETOUR", true);
    const auto& pokemons = c.pokedex.getPokemons();
    const std::size_t visibles = c.debutListe < pokemons.size()
        ? std::min(CANDIDATS_VISIBLES, pokemons.size() - c.debutListe) : 0;
    for (std::size_t i = 0; i < visibles; ++i) {
        const sf::FloatRect zone(45.f + static_cast<float>(i % 4) * 300.f,
                                 130.f + static_cast<float>(i / 4) * 230.f, 260.f, 190.f);
        carte(c, zone, pokemons[c.debutListe + i], false);
    }
    const std::size_t maximum = dernierDebutDePage(pokemons.size());
    bouton(c, pageCollectionPrecedente, "PREC.", c.debutListe > 0);
    bouton(c, pageCollectionSuivante, "SUIV.", c.debutListe < maximum);
    texte(window, c.font, "Pokemons possedes : " + std::to_string(pokemons.size()), 18,
          {45.f, 680.f}, sf::Color(195, 215, 240));
}

std::unique_ptr<Etat> EtatRencontreCapture::traiterEvenement(ContexteJeu& c, const sf::Event& e) {
    if (!estClicGauche(e)) return nullptr;
    const sf::Vector2i souris(e.mouseButton.x, e.mouseButton.y);
    if (contient(actionCapture, souris) && !c.captureTerminee) {
        c.captureTerminee = true;
        c.captureReussie = std::uniform_int_distribution<int>(1, 100)(c.aleatoire) <= 65;
        const auto& catalogue = c.pokedex.getPokemons();
        if (c.captureReussie && !catalogue.empty()) {
            c.party.addPokemon(catalogue[c.indiceSauvage]->clone());
            c.messageCapture = "Capture reussie ! Le Pokemon a ete ajoute a votre collection.";
        } else {
            c.messageCapture = "La Pokeball se brise : le Pokemon s'echappe.";
        }
        return nullptr;
    }
    if (contient(retourCapture, souris)) {
        return std::make_unique<EtatExploration>();
    }
    return nullptr;
}

void EtatRencontreCapture::dessiner(ContexteJeu& c) const {
    auto& window = c.window;
    auto& font = c.font;
    sf::RectangleShape fond({1280.f, 760.f}); fond.setFillColor(sf::Color(18, 38, 64)); window.draw(fond);
    sf::RectangleShape bandeau({1280.f, 155.f}); bandeau.setFillColor(sf::Color(27, 53, 84)); window.draw(bandeau);
    texte(window, font, "RENCONTRE SAUVAGE", 38, {420.f, 72.f}, sf::Color(255, 215, 70));
    texte(window, font, "Un Pokemon apparait dans les hautes herbes...", 20, {415.f, 132.f}, sf::Color(195, 215, 240));
    const auto& catalogue = c.pokedex.getPokemons();
    const Pokemon* sauvage = catalogue.empty() ? nullptr : catalogue[c.indiceSauvage];
    carte(c, {430.f, 220.f, 420.f, 190.f}, sauvage, false);
    if (c.captureTerminee) {
        texte(window, font, c.messageCapture, 21, {320.f, 440.f},
              c.captureReussie ? sf::Color(150, 240, 170) : sf::Color(255, 145, 130));
        bouton(c, retourCapture, "CONTINUER L'EXPLORATION", true);
    } else {
        texte(window, font, c.messageCapture, 20, {425.f, 435.f}, sf::Color(190, 210, 235));
        bouton(c, actionCapture, "LANCER UNE POKEBALL", true);
        texte(window, font, "Clique sur le bouton pour lancer la Pokeball", 17, {430.f, 580.f}, sf::Color(190, 210, 235));
    }
}

std::unique_ptr<Etat> EtatCombatArene::traiterEvenement(ContexteJeu& c, const sf::Event& e) {
    if (!estClicGauche(e)) return nullptr;
    const sf::Vector2i souris(e.mouseButton.x, e.mouseButton.y);
    if (contient(actionCombat, souris)) {
        const Pokemon* joueur = c.pokemonAttack->getPokemons()[c.combattantJoueur];
        const auto& catalogue = c.pokedex.getPokemons();
        const Pokemon* adversaire = catalogue[c.equipeAdverse[c.combattantAdverse]];
        const double degatsJoueur = degats(*joueur, *adversaire);
        double& pvAdversaire = c.pvEquipeAdverse[c.combattantAdverse];
        pvAdversaire = std::max(0.0, pvAdversaire - degatsJoueur);
        c.messageCombat = joueur->getName() + " inflige " + std::to_string(static_cast<int>(degatsJoueur)) + " degats a " + adversaire->getName() + ".";
        if (pvAdversaire <= 0.0) {
            c.messageCombat += " " + adversaire->getName() + " est K.O.";
            ++c.combattantAdverse;
            if (c.combattantAdverse == TEAM_SIZE) {
                c.victoire = true;
                return std::make_unique<EtatGameOver>();
            }
            const Pokemon* prochain = catalogue[c.equipeAdverse[c.combattantAdverse]];
            c.messageCombat += " " + prochain->getName() + " entre en combat !";
            return nullptr;
        }
        const double degatsEnnemi = degats(*adversaire, *joueur);
        double& pvJoueur = c.pvEquipeJoueur[c.combattantJoueur];
        pvJoueur = std::max(0.0, pvJoueur - degatsEnnemi);
        c.messageCombat += " " + adversaire->getName() + " contre-attaque : " +
                           std::to_string(static_cast<int>(degatsEnnemi)) + " degats.";
        if (pvJoueur <= 0.0) {
            c.messageCombat += " " + joueur->getName() + " est K.O.";
            ++c.combattantJoueur;
            if (c.combattantJoueur == TEAM_SIZE) {
                c.victoire = false;
                return std::make_unique<EtatGameOver>();
            }
            const Pokemon* prochain = c.pokemonAttack->getPokemons()[c.combattantJoueur];
            c.messageCombat += " " + prochain->getName() + " entre en combat !";
        }
    } else if (contient(quitterCombat, souris)) {
        return std::make_unique<EtatExploration>();
    }
    return nullptr;
}

void EtatCombatArene::dessiner(ContexteJeu& c) const {
    auto& window = c.window;
    auto& font = c.font;
    sf::RectangleShape fond({1280.f, 760.f}); fond.setFillColor(sf::Color(18, 38, 64)); window.draw(fond);
    sf::RectangleShape haut({1280.f, 130.f}); haut.setFillColor(sf::Color(27, 53, 84)); window.draw(haut);
    texte(window, font, "COMBAT D'ARENE", 38, {430.f, 38.f}, sf::Color(255, 215, 70));
    bouton(c, quitterCombat, "FUIR LE COMBAT", false);
    const auto& catalogue = c.pokedex.getPokemons();
    texte(window, font, "TON EQUIPE", 22, {205.f, 145.f}, sf::Color(130, 215, 255));
    texte(window, font, "EQUIPE ADVERSE", 22, {900.f, 145.f}, sf::Color(255, 155, 145));
    for (std::size_t i = 0; i < TEAM_SIZE; ++i) {
        const Pokemon* membre = c.pokemonAttack->getPokemons()[i];
        const float x = 55.f + static_cast<float>(i % 2) * 215.f;
        const float y = 185.f + static_cast<float>(i / 2) * 125.f;
        carte(c, {x, y, 190.f, 105.f}, membre, i == c.combattantJoueur, static_cast<int>(i + 1));
        texte(window, font, "PV " + std::to_string(static_cast<int>(c.pvEquipeJoueur[i])) + "/" +
              std::to_string(static_cast<int>(membre->getHitPointMax())), 14, {x + 8.f, y + 82.f}, sf::Color(30, 54, 78));

        const Pokemon* rival = catalogue[c.equipeAdverse[i]];
        const float xr = 845.f + static_cast<float>(i % 2) * 215.f;
        carte(c, {xr, y, 190.f, 105.f}, rival, i == c.combattantAdverse, static_cast<int>(i + 1));
        texte(window, font, "PV " + std::to_string(static_cast<int>(c.pvEquipeAdverse[i])) + "/" +
              std::to_string(static_cast<int>(rival->getHitPointMax())), 14, {xr + 8.f, y + 82.f}, sf::Color(30, 54, 78));
    }
    sf::Sprite versus(c.textureVs);
    versus.setPosition(594.f, 286.f);
    versus.setScale(1.5f, 1.5f);
    window.draw(versus);
    texte(window, font, c.messageCombat, 16, {90.f, 585.f}, sf::Color(255, 225, 150));
    bouton(c, actionCombat, "ATTAQUER", true);
    texte(window, font, "Combattants actifs : " +
          c.pokemonAttack->getPokemons()[c.combattantJoueur]->getName() + " VS " +
          catalogue[c.equipeAdverse[c.combattantAdverse]]->getName(), 16, {430.f, 680.f}, sf::Color(190, 210, 235));
}

std::unique_ptr<Etat> EtatGameOver::traiterEvenement(ContexteJeu&, const sf::Event& e) {
    if (!estClicGauche(e)) return nullptr;
    if (estClicGauche(e) && contient(retourAccueil, {e.mouseButton.x, e.mouseButton.y})) {
        return std::make_unique<EtatAccueil>();
    }
    return nullptr;
}

void EtatGameOver::dessiner(ContexteJeu& c) const {
    auto& window = c.window;
    auto& font = c.font;
    sf::RectangleShape fond({1280.f, 760.f});
    fond.setFillColor(sf::Color(18, 38, 64));
    window.draw(fond);
    sf::RectangleShape panneau({700.f, 430.f});
    panneau.setPosition(290.f, 165.f);
    panneau.setFillColor(sf::Color(27, 53, 84));
    panneau.setOutlineThickness(3.f);
    panneau.setOutlineColor(sf::Color(255, 215, 70));
    window.draw(panneau);
    texte(window, font, c.victoire ? "VICTOIRE !" : "FIN DE PARTIE", 52,
          {475.f, 220.f}, c.victoire ? sf::Color(150, 240, 170) : sf::Color(235, 82, 76));
    texte(window, font, c.victoire ? "L'adversaire n'a plus de points de vie." :
          "Votre Pokemon n'a plus de points de vie.", 23, {370.f, 330.f});
    bouton(c, retourAccueil, "RETOUR A L'ACCUEIL", true);
    texte(window, font, "Clique sur le bouton pour revenir", 17, {485.f, 545.f}, sf::Color(190, 210, 235));
}
