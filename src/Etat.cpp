#include "Etat.h"

#include <algorithm>
#include <cmath>
#include <sstream>

std::size_t dernierDebutDePage(std::size_t nombrePokemon);

namespace {
constexpr std::size_t TEAM_SIZE = 6;
constexpr std::size_t CANDIDATS_VISIBLES = 12;

const sf::FloatRect valider(940.f, 676.f, 260.f, 54.f);
const sf::FloatRect lancer(490.f, 342.f, 300.f, 76.f);
const sf::FloatRect listeCandidats(35.f, 160.f, 665.f, 455.f);
const sf::FloatRect pisteDefilement(698.f, 170.f, 12.f, 430.f);
const sf::FloatRect actionCapture(490.f, 490.f, 300.f, 62.f);
const sf::FloatRect nouvelleRencontre(490.f, 575.f, 300.f, 52.f);
const sf::FloatRect retourExplorationRencontre(490.f, 642.f, 300.f, 52.f);
const sf::FloatRect actionExploration1(280.f, 485.f, 300.f, 72.f);
const sf::FloatRect actionExploration2(700.f, 485.f, 300.f, 72.f);
const sf::FloatRect ouvrirCollection(1010.f, 48.f, 220.f, 48.f);
const sf::FloatRect retourCollection(990.f, 48.f, 220.f, 48.f);
const sf::FloatRect pageCollectionPrecedente(470.f, 675.f, 100.f, 42.f);
const sf::FloatRect pageCollectionSuivante(590.f, 675.f, 100.f, 42.f);
const sf::FloatRect actionCombat(490.f, 676.f, 300.f, 58.f);
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

void texteCentre(sf::RenderWindow& window, const sf::Font& font, const std::string& valeur,
                 unsigned taille, const sf::FloatRect& zone, sf::Color couleur = sf::Color::White) {
    sf::Text label(valeur, font, taille);
    const auto limites = label.getLocalBounds();
    label.setPosition(zone.left + (zone.width - limites.width) / 2.f - limites.left,
                      zone.top + (zone.height - limites.height) / 2.f - limites.top);
    label.setFillColor(couleur);
    window.draw(label);
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
        texteCentre(contexte.window, contexte.font, "Emplacement " + std::to_string(position), 18,
                    {zone.left + 4.f, zone.top + 12.f, zone.width - 8.f, 30.f}, sf::Color(175, 190, 215));
        texteCentre(contexte.window, contexte.font, "Cliquez pour placer", 15,
                    {zone.left + 4.f, zone.top + 48.f, zone.width - 8.f, 28.f}, sf::Color(130, 150, 180));
        return;
    }
    if (const sf::Texture* texture = contexte.texturePokemon(pokemon->getId())) {
        sf::Sprite sprite(*texture);
        const sf::Vector2u taille = texture->getSize();
        const float hauteurImage = zone.height - 38.f;
        const float echelle = std::min((zone.width - 12.f) / taille.x, hauteurImage / taille.y);
        sprite.setScale(echelle, echelle);
        sprite.setPosition(zone.left + (zone.width - taille.x * echelle) / 2.f,
                           zone.top + (hauteurImage - taille.y * echelle) / 2.f);
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

std::string couperTexte(const std::string& valeur, const sf::Font& font,
                        unsigned taille, float largeurMax) {
    std::string resultat;
    std::string ligne;
    std::istringstream mots(valeur);
    std::string mot;
    while (mots >> mot) {
        const std::string candidate = ligne.empty() ? mot : ligne + " " + mot;
        sf::Text mesure(candidate, font, taille);
        if (!ligne.empty() && mesure.getLocalBounds().width > largeurMax) {
            if (!resultat.empty()) resultat += '\n';
            resultat += ligne;
            ligne = mot;
        } else {
            ligne = candidate;
        }
    }
    if (!ligne.empty()) {
        if (!resultat.empty()) resultat += '\n';
        resultat += ligne;
    }
    return resultat;
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
    const std::size_t nombrePokemon = contexte.party.getPokemons().size();
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
    const std::size_t taille = c.party.getPokemons().size();
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
    const auto& pokemons = c.party.getPokemons();
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
        std::array<int, TEAM_SIZE> ids{};
        for (std::size_t slot = 0; slot < TEAM_SIZE; ++slot)
            ids[slot] = c.party.getPokemons()[c.equipe[slot]]->getId();
        for (int id : ids) c.pokemonAttack->ajouterDepuisParty(c.party, id);
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
    texte(window, font, "CHOISIS TON EQUIPE DE DEPART", 34, {45.f, 24.f}, sf::Color(255, 215, 70));
    texte(window, font, "Choisis 6 Pokemon communs. Tu en rencontreras d'autres en explorant.", 18,
          {48.f, 78.f}, sf::Color(195, 215, 240));
    texte(window, font, "POKEMON DE DEPART", 21, {45.f, 137.f}, sf::Color(130, 215, 255));
    texte(window, font, "TON EQUIPE - 6", 21, {760.f, 137.f}, sf::Color(130, 215, 255));
    const std::size_t taille = c.party.getPokemons().size();
    const std::string navigation = "" + std::to_string(c.debutListe + (taille ? 1 : 0)) + "-" +
        std::to_string(std::min(c.debutListe + CANDIDATS_VISIBLES, taille)) + " / " +
        std::to_string(taille);
    texte(window, font, navigation, 14, {530.f, 143.f}, sf::Color(195, 215, 240));
    sf::RectangleShape separateur({3.f, 470.f});
    separateur.setPosition(720.f, 165.f);
    separateur.setFillColor(sf::Color(70, 98, 132));
    window.draw(separateur);

    const auto& pokemons = c.party.getPokemons();
    const std::size_t visibles = c.debutListe < pokemons.size()
        ? std::min(CANDIDATS_VISIBLES, pokemons.size() - c.debutListe) : 0;
    for (std::size_t visible = 0; visible < visibles; ++visible) {
        const std::size_t index = c.debutListe + visible;
        const sf::FloatRect zone(45.f + static_cast<float>(visible % 3) * 220.f,
                                 170.f + static_cast<float>(visible / 3) * 115.f, 195.f, 90.f);
        int ordreEquipe = 0;
        for (std::size_t slot = 0; slot < TEAM_SIZE; ++slot)
            if (c.equipe[slot] == static_cast<int>(index)) ordreEquipe = static_cast<int>(slot + 1);
        carte(c, zone, pokemons[index], c.candidatSelectionne == static_cast<int>(index), ordreEquipe);
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
        std::array<std::size_t, POKEMON_DEPART_IDS.size()> adversairesDisponibles{};
        std::size_t nombreAdversaires = 0;
        for (int id : POKEMON_DEPART_IDS) {
            const auto trouve = std::find_if(catalogue.begin(), catalogue.end(), [id](const Pokemon* pokemon) {
                return pokemon->getId() == id;
            });
            if (trouve != catalogue.end())
                adversairesDisponibles[nombreAdversaires++] = static_cast<std::size_t>(trouve - catalogue.begin());
        }
        if (nombreAdversaires == 0) return nullptr;
        std::shuffle(adversairesDisponibles.begin(), adversairesDisponibles.begin() + nombreAdversaires, c.aleatoire);
        c.combattantJoueur = c.combattantAdverse = 0;
        for (std::size_t i = 0; i < TEAM_SIZE; ++i) {
            const Pokemon* joueur = c.pokemonAttack->getPokemons()[i];
            c.pvEquipeJoueur[i] = joueur->getHitPointMax();
            c.equipeAdverse[i] = static_cast<int>(adversairesDisponibles[i % nombreAdversaires]);
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
    const sf::Vector2u tailleFond = c.textureFondExploration.getSize();
    sf::Sprite fond(c.textureFondExploration);
    const float echelle = std::max(1280.f / static_cast<float>(tailleFond.x),
                                   760.f / static_cast<float>(tailleFond.y));
    fond.setScale(echelle, echelle);
    fond.setPosition((1280.f - tailleFond.x * echelle) / 2.f,
                     (760.f - tailleFond.y * echelle) / 2.f);
    window.draw(fond);
    sf::RectangleShape bandeau({1280.f, 112.f});
    bandeau.setFillColor(sf::Color(20, 42, 70, 185));
    window.draw(bandeau);
    texte(window, font, "EXPLORATION", 40, {490.f, 45.f}, sf::Color(255, 215, 70));
    const Pokemon* compagnon = c.pokemonAttack->getPokemons().front();
    if (const sf::Texture* texture = c.texturePokemon(compagnon->getId())) {
        sf::Sprite sprite(*texture);
        const sf::Vector2u taille = texture->getSize();
        const float scale = std::min(2.6f, std::min(220.f / taille.x, 220.f / taille.y));
        sprite.setScale(scale, scale);
        sprite.setPosition(640.f - taille.x * scale / 2.f, 285.f - taille.y * scale / 2.f);
        window.draw(sprite);
    }
    sf::RectangleShape info({500.f, 74.f}); info.setPosition(390.f, 190.f); info.setFillColor(sf::Color(22, 65, 105, 220)); window.draw(info);
    texte(window, font, compagnon->getName() + " t'accompagne", 24, {500.f, 204.f}, sf::Color::White);
    texte(window, font, "Ton equipe : " + std::to_string(c.pokemonAttack->size()) +
          " | Ta collection : " + std::to_string(c.party.size()), 18, {465.f, 238.f}, sf::Color(198, 228, 246));
    sf::RectangleShape panneauRencontre({400.f, 145.f});
    panneauRencontre.setPosition(230.f, 460.f);
    panneauRencontre.setFillColor(sf::Color(22, 65, 88, 225));
    panneauRencontre.setOutlineThickness(2.f);
    panneauRencontre.setOutlineColor(sf::Color(130, 210, 170));
    window.draw(panneauRencontre);
    sf::RectangleShape panneauArene({400.f, 145.f});
    panneauArene.setPosition(650.f, 460.f);
    panneauArene.setFillColor(sf::Color(22, 65, 88, 225));
    panneauArene.setOutlineThickness(2.f);
    panneauArene.setOutlineColor(sf::Color(255, 190, 100));
    window.draw(panneauArene);
    bouton(c, actionExploration1, "RENCONTRE SAUVAGE", true);
    bouton(c, actionExploration2, "COMBAT D'ARENE", true);
    texte(window, font, "Tente de capturer un nouveau Pokemon", 15, {270.f, 563.f}, sf::Color(218, 239, 226));
    texte(window, font, "Affronte les six membres de l'arene", 15, {695.f, 563.f}, sf::Color(250, 226, 195));
    bouton(c, ouvrirCollection, "MA COLLECTION", true);
}

std::unique_ptr<Etat> EtatCollection::traiterEvenement(ContexteJeu& c, const sf::Event& e) {
    if (!estClicGauche(e)) return nullptr;
    const sf::Vector2i souris(e.mouseButton.x, e.mouseButton.y);
    const std::size_t maximum = dernierDebutDePage(c.party.getPokemons().size());
    if (contient(pageCollectionPrecedente, souris)) {
        c.debutCollection = c.debutCollection > CANDIDATS_VISIBLES ? c.debutCollection - CANDIDATS_VISIBLES : 0;
    } else if (contient(pageCollectionSuivante, souris)) {
        c.debutCollection = std::min(maximum, c.debutCollection + CANDIDATS_VISIBLES);
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
    const auto& pokemons = c.party.getPokemons();
    const std::size_t visibles = c.debutCollection < pokemons.size()
        ? std::min(CANDIDATS_VISIBLES, pokemons.size() - c.debutCollection) : 0;
    for (std::size_t i = 0; i < visibles; ++i) {
        const sf::FloatRect zone(45.f + static_cast<float>(i % 4) * 300.f,
                                 130.f + static_cast<float>(i / 4) * 170.f, 260.f, 145.f);
        carte(c, zone, pokemons[c.debutCollection + i], false);
    }
    if (visibles == 0)
        texteCentre(window, c.font, "Ta collection est vide pour le moment.", 22,
                    {180.f, 260.f, 920.f, 60.f}, sf::Color(190, 210, 235));
    const std::size_t maximum = dernierDebutDePage(pokemons.size());
    bouton(c, pageCollectionPrecedente, "PREC.", c.debutCollection > 0);
    bouton(c, pageCollectionSuivante, "SUIV.", c.debutCollection < maximum);
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
            const Pokemon* sauvage = catalogue[c.indiceSauvage];
            const bool dejaPossede = std::any_of(c.party.getPokemons().begin(), c.party.getPokemons().end(),
                [sauvage](const Pokemon* pokemon) { return pokemon->getId() == sauvage->getId(); });
            if (dejaPossede) {
                c.messageCapture = "Capture reussie, mais " + sauvage->getName() + " est deja dans ta collection.";
            } else {
                c.party.addPokemon(sauvage->clone());
                c.debutCollection = dernierDebutDePage(c.party.size());
                c.messageCapture = sauvage->getName() + " rejoint ta collection !";
            }
        } else {
            c.messageCapture = "La Pokeball se brise : le Pokemon s'echappe.";
        }
        return nullptr;
    }
    if (c.captureTerminee && contient(nouvelleRencontre, souris)) {
        c.captureTerminee = false;
        c.captureReussie = false;
        const auto& catalogue = c.pokedex.getPokemons();
        if (!catalogue.empty())
            c.indiceSauvage = std::uniform_int_distribution<std::size_t>(0, catalogue.size() - 1)(c.aleatoire);
        c.messageCapture = "Un nouveau Pokemon sauvage apparait !";
        return nullptr;
    }
    if (c.captureTerminee && contient(retourExplorationRencontre, souris)) {
        return std::make_unique<EtatExploration>();
    }
    return nullptr;
}

void EtatRencontreCapture::dessiner(ContexteJeu& c) const {
    auto& window = c.window;
    auto& font = c.font;
    sf::RectangleShape fond({1280.f, 760.f}); fond.setFillColor(sf::Color(18, 38, 64)); window.draw(fond);
    sf::RectangleShape bandeau({1280.f, 155.f}); bandeau.setFillColor(sf::Color(27, 53, 84)); window.draw(bandeau);
    texteCentre(window, font, "RENCONTRE SAUVAGE", 38, {0.f, 45.f, 1280.f, 62.f}, sf::Color(255, 215, 70));
    texteCentre(window, font, "Un Pokemon apparait dans les hautes herbes...", 20,
                {0.f, 118.f, 1280.f, 30.f}, sf::Color(195, 215, 240));
    const auto& catalogue = c.pokedex.getPokemons();
    const Pokemon* sauvage = catalogue.empty() ? nullptr : catalogue[c.indiceSauvage];
    carte(c, {430.f, 220.f, 420.f, 190.f}, sauvage, false);
    if (sauvage) {
        texteCentre(window, font, "N° " + std::to_string(sauvage->getId()) +
              "  |  Generation " + std::to_string(sauvage->getGeneration()),
              16, {430.f, 410.f, 420.f, 25.f}, sf::Color(130, 215, 255));
        texteCentre(window, font, "PV " + std::to_string(static_cast<int>(sauvage->getHitPoint())) +
              " / " + std::to_string(static_cast<int>(sauvage->getHitPointMax())) +
              "    Attaque " + std::to_string(static_cast<int>(sauvage->getAttack())),
              16, {430.f, 434.f, 420.f, 25.f}, sf::Color(235, 240, 250));
        texteCentre(window, font, "Defense " + std::to_string(static_cast<int>(sauvage->getDefense())),
              16, {430.f, 458.f, 420.f, 25.f}, sf::Color(235, 240, 250));
    }
    if (c.captureTerminee) {
        texteCentre(window, font, c.messageCapture, 19, {100.f, 505.f, 1080.f, 45.f},
              c.captureReussie ? sf::Color(150, 240, 170) : sf::Color(255, 145, 130));
        bouton(c, nouvelleRencontre, "NOUVELLE RENCONTRE", true);
        bouton(c, retourExplorationRencontre, "RETOUR A L'EXPLORATION", true);
    } else {
        texteCentre(window, font, c.messageCapture, 20, {100.f, 185.f, 1080.f, 28.f}, sf::Color(190, 210, 235));
        bouton(c, actionCapture, "LANCER UNE POKEBALL", true);
        texteCentre(window, font, "Clique sur le bouton pour lancer la Pokeball", 17,
                    {100.f, 557.f, 1080.f, 28.f}, sf::Color(190, 210, 235));
    }
}

std::unique_ptr<Etat> EtatCombatArene::traiterEvenement(ContexteJeu& c, const sf::Event& e) {
    if (!estClicGauche(e)) return nullptr;
    const sf::Vector2i souris(e.mouseButton.x, e.mouseButton.y);
    if (contient(actionCombat, souris)) {
        const Pokemon* joueur = c.pokemonAttack->getPokemons()[c.combattantJoueur];
        const auto& catalogue = c.pokedex.getPokemons();
        const Pokemon* adversaire = catalogue[c.equipeAdverse[c.combattantAdverse]];
        double& pvAdversaire = c.pvEquipeAdverse[c.combattantAdverse];
        Pokemon cibleAdverse(*adversaire);
        cibleAdverse.setHitPoint(pvAdversaire);
        joueur->attackPokemon(cibleAdverse);
        const double degatsJoueur = pvAdversaire - cibleAdverse.getHitPoint();
        pvAdversaire = cibleAdverse.getHitPoint();
        c.messageCombat = joueur->getName() + " attaque " + adversaire->getName() + " !\n" +
            adversaire->getName() + " perd " + std::to_string(static_cast<int>(degatsJoueur)) +
            " PV (" + std::to_string(static_cast<int>(pvAdversaire)) + " PV restants).";
        if (pvAdversaire <= 0.0) {
            c.messageCombat += "\n" + adversaire->getName() + " est K.O. !";
            ++c.combattantAdverse;
            if (c.combattantAdverse == TEAM_SIZE) {
                c.victoire = true;
                return std::make_unique<EtatGameOver>();
            }
            const Pokemon* prochain = catalogue[c.equipeAdverse[c.combattantAdverse]];
            c.messageCombat += "\n" + prochain->getName() + " entre en combat !";
            return nullptr;
        }
        double& pvJoueur = c.pvEquipeJoueur[c.combattantJoueur];
        Pokemon cibleJoueur(*joueur);
        cibleJoueur.setHitPoint(pvJoueur);
        adversaire->attackPokemon(cibleJoueur);
        const double degatsEnnemi = pvJoueur - cibleJoueur.getHitPoint();
        pvJoueur = cibleJoueur.getHitPoint();
        c.messageCombat += "\n" + adversaire->getName() + " contre-attaque ! " + joueur->getName() +
            " perd " + std::to_string(static_cast<int>(degatsEnnemi)) + " PV (" +
            std::to_string(static_cast<int>(pvJoueur)) + " PV restants).";
        if (pvJoueur <= 0.0) {
            c.messageCombat += "\n" + joueur->getName() + " est K.O. !";
            ++c.combattantJoueur;
            if (c.combattantJoueur == TEAM_SIZE) {
                c.victoire = false;
                return std::make_unique<EtatGameOver>();
            }
            const Pokemon* prochain = c.pokemonAttack->getPokemons()[c.combattantJoueur];
            c.messageCombat += "\n" + prochain->getName() + " entre en combat !";
        }
    } else if (contient(quitterCombat, souris)) {
        return std::make_unique<EtatExploration>();
    }
    return nullptr;
}

void EtatCombatArene::dessiner(ContexteJeu& c) const {
    auto& window = c.window;
    auto& font = c.font;
    sf::RectangleShape fond({1280.f, 760.f});
    fond.setFillColor(sf::Color(14, 29, 49));
    window.draw(fond);
    sf::RectangleShape haut({1280.f, 100.f});
    haut.setFillColor(sf::Color(23, 47, 74));
    window.draw(haut);
    texteCentre(window, font, "COMBAT D'ARENE", 34, {0.f, 18.f, 1280.f, 45.f}, sf::Color(255, 215, 70));
    texteCentre(window, font, "Choisis ton attaque", 16, {0.f, 61.f, 1280.f, 26.f}, sf::Color(177, 202, 225));
    bouton(c, quitterCombat, "FUIR LE COMBAT", true);
    const auto& catalogue = c.pokedex.getPokemons();
    const Pokemon* actifJoueur = c.pokemonAttack->getPokemons()[c.combattantJoueur];
    const Pokemon* actifRival = catalogue[c.equipeAdverse[c.combattantAdverse]];

    const auto dessinerStatut = [&](const Pokemon* pokemon, double pv, sf::Vector2f position, bool joueur) {
        const sf::Color accent = joueur ? sf::Color(97, 198, 229) : sf::Color(232, 126, 120);
        sf::RectangleShape panneau({360.f, 108.f});
        panneau.setPosition(position);
        panneau.setFillColor(joueur ? sf::Color(24, 48, 70) : sf::Color(55, 38, 48));
        panneau.setOutlineThickness(2.f);
        panneau.setOutlineColor(accent);
        window.draw(panneau);
        texteCentre(window, font, joueur ? "TON POKEMON" : "POKEMON ADVERSE", 13,
                    {position.x + 12.f, position.y + 7.f, 336.f, 19.f}, accent);
        texteCentre(window, font, pokemon->getName(), 23,
                    {position.x + 12.f, position.y + 27.f, 336.f, 31.f}, sf::Color::White);
        texteCentre(window, font, "PV " + std::to_string(static_cast<int>(pv)) + " / " +
                    std::to_string(static_cast<int>(pokemon->getHitPointMax())), 14,
                    {position.x + 12.f, position.y + 61.f, 336.f, 20.f}, sf::Color(218, 231, 240));
        const float ratio = pokemon->getHitPointMax() > 0.0
            ? std::clamp(static_cast<float>(pv / pokemon->getHitPointMax()), 0.f, 1.f) : 0.f;
        sf::RectangleShape fondPv({320.f, 8.f});
        fondPv.setPosition(position.x + 20.f, position.y + 89.f);
        fondPv.setFillColor(sf::Color(8, 18, 30));
        window.draw(fondPv);
        sf::RectangleShape barrePv({320.f * ratio, 8.f});
        barrePv.setPosition(position.x + 20.f, position.y + 89.f);
        barrePv.setFillColor(ratio > 0.5f ? sf::Color(93, 208, 137) : sf::Color(241, 166, 73));
        window.draw(barrePv);
    };
    dessinerStatut(actifJoueur, c.pvEquipeJoueur[c.combattantJoueur], {72.f, 125.f}, true);
    dessinerStatut(actifRival, c.pvEquipeAdverse[c.combattantAdverse], {848.f, 125.f}, false);

    sf::CircleShape lumiere(160.f);
    lumiere.setPosition(480.f, 247.f);
    lumiere.setFillColor(sf::Color(40, 75, 94, 100));
    window.draw(lumiere);
    sf::CircleShape solJoueur(105.f, 48);
    solJoueur.setPosition(310.f, 420.f);
    solJoueur.setScale(1.f, 0.23f);
    solJoueur.setFillColor(sf::Color(41, 91, 100, 150));
    window.draw(solJoueur);
    sf::CircleShape solRival(105.f, 48);
    solRival.setPosition(865.f, 420.f);
    solRival.setScale(1.f, 0.23f);
    solRival.setFillColor(sf::Color(101, 63, 69, 150));
    window.draw(solRival);
    const auto dessinerActif = [&](const Pokemon* pokemon, sf::Vector2f position) {
        if (const sf::Texture* texture = c.texturePokemon(pokemon->getId())) {
            sf::Sprite sprite(*texture);
            const sf::Vector2u taille = texture->getSize();
            const float scale = std::min(175.f / taille.x, 175.f / taille.y);
            sprite.setScale(scale, scale);
            sprite.setPosition(position.x + (175.f - taille.x * scale) / 2.f,
                               position.y + (175.f - taille.y * scale) / 2.f);
            window.draw(sprite);
        }
    };
    dessinerActif(actifJoueur, {275.f, 253.f});
    dessinerActif(actifRival, {830.f, 253.f});
    sf::Sprite versus(c.textureVs);
    versus.setPosition(607.f, 316.f);
    versus.setScale(1.05f, 1.05f);
    window.draw(versus);

    for (std::size_t equipe = 0; equipe < 2; ++equipe) {
        const bool joueur = equipe == 0;
        const std::size_t actif = joueur ? c.combattantJoueur : c.combattantAdverse;
        const float y = joueur ? 485.f : 528.f;
        const sf::Color accent = joueur ? sf::Color(97, 198, 229) : sf::Color(232, 126, 120);
        texteCentre(window, font, joueur ? "EQUIPE" : "ADVERSAIRE", 12,
                    {20.f, y, 135.f, 34.f}, accent);
        for (std::size_t i = 0; i < TEAM_SIZE; ++i) {
            const float x = 165.f + static_cast<float>(i) * 181.f;
            sf::RectangleShape pastille({174.f, 34.f});
            pastille.setPosition(x, y);
            const bool estActif = i == actif;
            const bool ko = joueur ? c.pvEquipeJoueur[i] <= 0.0 : c.pvEquipeAdverse[i] <= 0.0;
            pastille.setFillColor(ko ? sf::Color(35, 43, 53) :
                                  joueur ? sf::Color(27, 54, 75) : sf::Color(59, 42, 50));
            pastille.setOutlineThickness(estActif ? 2.f : 1.f);
            pastille.setOutlineColor(estActif ? sf::Color(255, 202, 89) : accent);
            window.draw(pastille);
            const Pokemon* pokemon = joueur ? c.pokemonAttack->getPokemons()[i]
                                               : catalogue[c.equipeAdverse[i]];
            texteCentre(window, font, std::to_string(i + 1) + "  " + pokemon->getName(), 13,
                        {x + 3.f, y + 2.f, 168.f, 30.f}, ko ? sf::Color(126, 137, 149) : sf::Color::White);
        }
    }

    sf::RectangleShape journal({1100.f, 82.f});
    journal.setPosition(90.f, 577.f);
    journal.setFillColor(sf::Color(10, 25, 45, 235));
    journal.setOutlineThickness(1.f);
    journal.setOutlineColor(sf::Color(92, 145, 180));
    window.draw(journal);
    texteCentre(window, font, couperTexte(c.messageCombat, font, 15, 1050.f), 15,
                {110.f, 581.f, 1060.f, 74.f}, sf::Color(255, 230, 165));
    bouton(c, actionCombat, "ATTAQUER", true);
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
    texte(window, font, c.victoire ? "Toute l'equipe adverse est K.O. !" :
          "Toute votre equipe est K.O. !", 23, {385.f, 320.f});
    sf::Text resumeCombat(couperTexte(c.messageCombat, font, 17, 550.f), font, 17);
    const sf::FloatRect limitesResume = resumeCombat.getLocalBounds();
    resumeCombat.setPosition(640.f - limitesResume.width / 2.f, 365.f);
    resumeCombat.setFillColor(sf::Color(255, 225, 150));
    window.draw(resumeCombat);
    bouton(c, retourAccueil, "RETOUR A L'ACCUEIL", true);
    texte(window, font, "Clique sur le bouton pour revenir", 17, {485.f, 545.f}, sf::Color(190, 210, 235));
}
