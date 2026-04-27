#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

class Config {
private:
    static Config* instance;
    
    float volGeneral = 50.f;
    float volMusica = 50.f;
    float volEfectos = 50.f;
    bool pantallaCompleta = false;
    sf::Vector2u resolucionActual;
    
    Config() {
        cargar();
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        resolucionActual = sf::Vector2u(desktop.size.x, desktop.size.y);
    }
    
public:
    static Config& getInstance() {
        if (!instance) instance = new Config();
        return *instance;
    }
    
    static void destroy() {
        delete instance;
        instance = nullptr;
    }
    
    float getVolGeneral() const { return volGeneral; }
    float getVolMusica() const { return volMusica; }
    float getVolEfectos() const { return volEfectos; }
    bool isPantallaCompleta() const { return pantallaCompleta; }
    sf::Vector2u getResolucion() const { return resolucionActual; }
    
    void setVolGeneral(float v) { 
        volGeneral = std::clamp(v, 0.f, 100.f); 
        guardar(); 
    }
    
    void setVolMusica(float v) { 
        volMusica = std::clamp(v, 0.f, 100.f); 
        guardar(); 
    }
    
    void setVolEfectos(float v) { 
        volEfectos = std::clamp(v, 0.f, 100.f); 
        guardar(); 
    }
    
    void setPantallaCompleta(bool fullscreen) { 
        pantallaCompleta = fullscreen; 
        guardar();
    }
    
    void cargar() {
        std::ifstream file("saves/config.csv");
        if (!file.is_open()) return;
        
        std::string linea;
        std::getline(file, linea);
        if (std::getline(file, linea)) {
            std::stringstream ss(linea);
            std::string token;
            std::getline(ss, token, ','); volGeneral = std::stof(token);
            std::getline(ss, token, ','); volMusica = std::stof(token);
            std::getline(ss, token, ','); volEfectos = std::stof(token);
            if (std::getline(ss, token, ',')) pantallaCompleta = (token == "true");
            if (std::getline(ss, token, ',')) {
                resolucionActual.x = std::stoul(token);
                std::getline(ss, token, ',');
                resolucionActual.y = std::stoul(token);
            }
        }
    }
    
    void guardar() {
        #ifdef _WIN32
            system("mkdir saves 2> nul");
        #else
            system("mkdir -p saves");
        #endif
        
        std::ofstream file("saves/config.csv");
        if (!file.is_open()) return;
        
        file << "vol_general,vol_musica,vol_efectos,pantalla_completa,res_x,res_y\n";
        file << volGeneral << "," << volMusica << "," << volEfectos << "," 
             << (pantallaCompleta ? "true" : "false") << ","
             << resolucionActual.x << "," << resolucionActual.y << "\n";
    }
    
    void alternarPantalla(sf::RenderWindow* window) {
        if (!window) return;
        
        pantallaCompleta = !pantallaCompleta;
        
        if (pantallaCompleta) {
            sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
            resolucionActual = sf::Vector2u(desktop.size.x, desktop.size.y);
            window->create(sf::VideoMode({resolucionActual.x, resolucionActual.y}), 
                          "Vimorte", sf::State::Fullscreen);
        } else {
            resolucionActual = sf::Vector2u(1280, 720);
            window->create(sf::VideoMode({1280, 720}), "Vimorte", sf::State::Windowed);
        }
        
        window->setFramerateLimit(60);
        guardar();
    }
};

#endif