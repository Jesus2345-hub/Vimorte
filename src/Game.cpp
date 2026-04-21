#include "Game.hpp"
#include "MenuState.hpp" 
#include "State.hpp"
#include "Nivel1State.hpp"
#include "Lobby.hpp"
#include <optional>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

Game::Game() 
{
    window = std::make_unique<sf::RenderWindow>(sf::VideoMode({1280, 720}), "Vimorte");
    window->setFramerateLimit(60);
    
    // Cargar configuración de audio guardada
    cargarConfiguracionAudio();
    
    states.push(std::make_unique<MenuState>(window.get(), this));
}

Game::~Game() {
    guardarConfiguracionAudio();
    detenerMusica();
}

void Game::cargarConfiguracionAudio() {
    std::ifstream file("saves/config.csv");
    if (!file.is_open()) return;
    
    std::string linea;
    std::getline(file, linea); // Saltar cabecera
    
    if (std::getline(file, linea)) {
        std::stringstream ss(linea);
        std::string token;
        
        std::getline(ss, token, ','); volGeneral = std::stof(token);
        std::getline(ss, token, ','); volMusica = std::stof(token);
        std::getline(ss, token, ','); volEfectos = std::stof(token);
        
        std::cout << "✅ Configuración de audio cargada" << std::endl;
    }
}

void Game::guardarConfiguracionAudio() {
    #ifdef _WIN32
        system("mkdir saves 2> nul");
    #else
        system("mkdir -p saves");
    #endif
    
    std::ofstream file("saves/config.csv");
    if (!file.is_open()) return;
    
    file << "vol_general,vol_musica,vol_efectos\n";
    file << volGeneral << "," << volMusica << "," << volEfectos << "\n";
    
    std::cout << "✅ Configuración de audio guardada" << std::endl;
}

void Game::cambiarMusica(const std::string& rutaMusica) {
    // Si es la misma música que ya está sonando, no hacer nada
    if (m_currentMusicPath == rutaMusica && m_currentMusic.getStatus() == sf::Music::Status::Playing) {
        return;
    }
    
    // Detener música actual
    m_currentMusic.stop();
    
    // Cargar nueva música
    if (m_currentMusic.openFromFile(rutaMusica)) {
        m_currentMusicPath = rutaMusica;
        m_currentMusic.setLooping(true);
        m_currentMusic.setVolume(getRealMusica());
        m_currentMusic.play();
        std::cout << "🎵 Música cambiada: " << rutaMusica << std::endl;
    } else {
        std::cerr << "❌ Error: No se pudo cargar la música: " << rutaMusica << std::endl;
    }
}

void Game::detenerMusica() {
    m_currentMusic.stop();
    m_currentMusicPath = "";
}

void Game::setVolGeneral(float v) { 
    if (v < 0.f) v = 0.f;
    if (v > 100.f) v = 100.f;
    volGeneral = v;
    actualizarVolumenMusica();
    guardarConfiguracionAudio();
}

void Game::setVolMusica(float v) { 
    if (v < 0.f) v = 0.f;
    if (v > 100.f) v = 100.f;
    volMusica = v;
    actualizarVolumenMusica();
    guardarConfiguracionAudio();
}

void Game::setVolEfectos(float v) { 
    if (v < 0.f) v = 0.f;
    if (v > 100.f) v = 100.f;
    volEfectos = v;
    guardarConfiguracionAudio();
}

void Game::actualizarVolumenMusica() {
    m_currentMusic.setVolume(getRealMusica());
}

void Game::guardarPartidaActual() {
    if (tienePartidaActiva()) {
        saveManager.guardarProgresoActual();
        std::cout << "💾 Partida guardada en slot " << saveManager.getCurrentSlotId() << std::endl;
    }
}

void Game::cargarPartidaYContinuar(int slotId) {
    if (saveManager.cargarPartida(slotId)) {
        int nivelId = saveManager.getCurrentProgress().nivelActualId;
        std::cout << "📂 Cargando partida del slot " << slotId << " - Nivel " << nivelId << std::endl;
        
        // Detener la música del menú antes de cambiar de estado
        detenerMusica();
        
        // Cargar el nivel correspondiente
        if (nivelId == 1) {
            changeState(std::make_unique<Nivel1State>(window.get(), this));
        } else {
            changeState(std::make_unique<LobbyState>(window.get(), this));
        }
    }
}

void Game::run() 
{
    sf::Clock clock;
    while (window->isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        
        // Actualizar tiempo de juego si hay partida activa
        if (tienePartidaActiva()) {
            saveManager.addTiempoJugado(deltaTime);
        }

        while (const std::optional<sf::Event> event = window->pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window->close();
            }
            
            if (!states.empty()) {
                states.top()->handleEvent(*event);
            }
        }

        if (!states.empty()) {
            states.top()->update(deltaTime);
        }

        window->clear(sf::Color::Black); 
        
        if (!states.empty()) {
            std::vector<State*> paraDibujar;
            std::stack<std::unique_ptr<State>> temp;

            while (!states.empty()) {
                paraDibujar.push_back(states.top().get());
                temp.push(std::move(states.top()));
                states.pop();
            }
            while (!temp.empty()) {
                states.push(std::move(temp.top()));
                temp.pop();
            }
            for (int i = paraDibujar.size() - 1; i >= 0; --i) {
                window->setView(window->getDefaultView());
                paraDibujar[i]->draw();
            }
        }
        window->display(); 
    }
}

void Game::changeState(std::unique_ptr<State> state) {
    if (!states.empty()) states.pop();
    states.push(std::move(state));
}

void Game::pushState(std::unique_ptr<State> state) { 
    states.push(std::move(state)); 
}

void Game::popState() { 
    if (!states.empty()) states.pop(); 
}

void Game::returnToMenu() {
    // Detener música actual al volver al menú
    detenerMusica();
    
    while (states.size() > 1) states.pop();
    changeState(std::make_unique<MenuState>(window.get(), this));
}