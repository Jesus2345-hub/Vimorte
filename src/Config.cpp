#include "Config.hpp"

// Inicialización de miembros estáticos
float Config::s_volGeneral = 50.f;
float Config::s_volMusica = 50.f;
float Config::s_volEfectos = 50.f;
bool Config::s_pantallaCompleta = false;

sf::Vector2u Config::s_resolucionActual(1280, 720);

void Config::cargar() {
    // Igual que antes, pero usando s_ en lugar de m_
    std::ifstream file("saves/config.csv");
    if (!file.is_open()) return;
    
    std::string linea;
    std::getline(file, linea);
    if (std::getline(file, linea)) {
        std::stringstream ss(linea);
        std::string token;
        std::getline(ss, token, ','); s_volGeneral = std::stof(token);
        std::getline(ss, token, ','); s_volMusica = std::stof(token);
        std::getline(ss, token, ','); s_volEfectos = std::stof(token);
        if (std::getline(ss, token, ',')) s_pantallaCompleta = (token == "true");
        if (std::getline(ss, token, ',')) {
            s_resolucionActual.x = std::stoul(token);
            std::getline(ss, token, ',');
            s_resolucionActual.y = std::stoul(token);
        }
    }
}

void Config::guardar() {
    // Igual que antes
    #ifdef _WIN32
        system("mkdir saves 2> nul");
    #else
        system("mkdir -p saves");
    #endif
    
    std::ofstream file("saves/config.csv");
    if (!file.is_open()) return;
    
    file << "vol_general,vol_musica,vol_efectos,pantalla_completa,res_x,res_y\n";
    file << s_volGeneral << "," << s_volMusica << "," << s_volEfectos << "," 
         << (s_pantallaCompleta ? "true" : "false") << ","
         << s_resolucionActual.x << "," << s_resolucionActual.y << "\n";
}

void Config::alternarPantalla(sf::RenderWindow* window) {
    // Igual que antes
    if (!window) return;
    
    s_pantallaCompleta = !s_pantallaCompleta;
    
    if (s_pantallaCompleta) {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        s_resolucionActual = sf::Vector2u(desktop.size.x, desktop.size.y);
        window->create(sf::VideoMode({s_resolucionActual.x, s_resolucionActual.y}), 
                      "Vimorte", sf::State::Fullscreen);
    } else {
        s_resolucionActual = sf::Vector2u(1280, 720);
        window->create(sf::VideoMode({1280, 720}), "Vimorte", sf::State::Windowed);
    }
    
    window->setFramerateLimit(60);
    guardar();
}