#include "Etat.h"

#include <filesystem>
#include <map>

namespace {
constexpr unsigned WINDOW_WIDTH = 1280;
constexpr unsigned WINDOW_HEIGHT = 760;
namespace fs = std::filesystem;
fs::path dossierExecutable;

fs::path resoudreRessource(const fs::path& cheminRelatif) {
    const auto chercherDepuis = [&](fs::path dossier) -> fs::path {
        while (!dossier.empty()) {
            const fs::path candidat = dossier / "Data" / cheminRelatif;
            std::error_code erreur;
            if (fs::is_regular_file(candidat, erreur)) return candidat;
            const fs::path parent = dossier.parent_path();
            if (parent == dossier) break;
            dossier = parent;
        }
        return {};
    };

    std::error_code erreur;
    fs::path courant = chercherDepuis(fs::current_path(erreur));
    if (!courant.empty()) return courant;
    return chercherDepuis(dossierExecutable);
}

std::string cheminPokedex() {
    const fs::path chemin = resoudreRessource("pokedex.csv");
    return chemin.empty() ? "Data/pokedex.csv" : chemin.string();
}

bool chargerPolice(sf::Font& font) {
    const fs::path policeDonnees = resoudreRessource("arial.ttf");
    if (!policeDonnees.empty() && font.loadFromFile(policeDonnees.string())) return true;
    for (const char* policeSysteme : {"C:/Windows/Fonts/arial.ttf", "C:/Windows/Fonts/segoeui.ttf",
                                      "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                                      "/System/Library/Fonts/Supplemental/Arial.ttf"}) {
        std::error_code erreur;
        if (fs::is_regular_file(policeSysteme, erreur) && font.loadFromFile(policeSysteme)) return true;
    }
    return false;
}

bool chargerTexture(sf::Texture& texture, const std::string& nom) {
    const fs::path chemin = resoudreRessource(nom);
    return !chemin.empty() && texture.loadFromFile(chemin.string());
}

const sf::Texture* texturePokemon(int id, std::map<int, sf::Texture>& cache) {
    const auto dejaCharge = cache.find(id);
    if (dejaCharge != cache.end()) return &dejaCharge->second;

    sf::Texture texture;
    const std::string fichier = "image_pokedex-20260914/pokemon/" + std::to_string(id) + ".png";
    if (!chargerTexture(texture, fichier)) return nullptr;
    return &cache.emplace(id, std::move(texture)).first->second;
}

} // namespace

int main(int argc, char* argv[]) {
    std::error_code erreur;
    if (argc > 0) dossierExecutable = fs::absolute(argv[0], erreur).parent_path();

    Pokedex* pokedex = Pokedex::getInstance(cheminPokedex());
    Pokemon_Party party;

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Pokemon Attack - Selection d'equipe");
    window.setFramerateLimit(60);
    sf::Font font;
    if (!chargerPolice(font)) return 1;
    sf::Texture textureFondAccueil, textureJauge, textureVs;
    if (!chargerTexture(textureFondAccueil, "image_pokedex-20260914/bg.jpg") ||
        !chargerTexture(textureJauge, "healthGauge.png") || !chargerTexture(textureVs, "versusSmall.png")) return 1;

    std::map<int, sf::Texture> texturesPokemon;
    ContexteJeu contexte{*pokedex, party, window, font, textureFondAccueil, textureJauge, textureVs,
                         [&texturesPokemon](int id) { return texturePokemon(id, texturesPokemon); }};
    contexte.equipe.fill(-1);
    MoteurJeu moteur;

    while (window.isOpen()) {
        sf::Event evenement{};
        while (window.pollEvent(evenement)) {
            if (evenement.type == sf::Event::Closed) window.close();
            moteur.traiterEvenement(contexte, evenement);
        }
        window.clear(sf::Color(19, 31, 52));
        moteur.dessiner(contexte);
        window.display();
    }
    return 0;
}
