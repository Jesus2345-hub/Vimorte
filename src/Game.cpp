#include "Game.hpp"
#include "MenuState.hpp" 
#include "State.hpp"
#include "Nivel1State.hpp"
#include "Lobby.hpp"
#include "AdminMenuState.hpp"
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
    
    // Construir árbol de niveles
    levelTree.buildTree();
    
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
        // Actualizar la ruta del árbol antes de guardar
        saveManager.getCurrentProgress().rutaActual = levelTree.getCurrentPath();
        saveManager.guardarProgresoActual();
        std::cout << "💾 Partida guardada en slot " << saveManager.getCurrentSlotId() << std::endl;
    }
}

void Game::avanzarNivel() {
    if (levelTree.goToNextLevel()) {
        auto newState = levelTree.createCurrentState(window.get(), this);
        if (newState) {
            changeState(std::move(newState));
            
            // Guardar progreso
            if (tienePartidaActiva()) {
                LevelNode* current = levelTree.getCurrentNode();
                saveManager.setNivelActual(current->levelNumber, current->levelNumber);
                saveManager.getCurrentProgress().rutaActual = levelTree.getCurrentPath();
                guardarPartidaActual();
            }
        }
    }
}

void Game::entrarCentinela() {
    if (levelTree.goToCentinela()) {
        auto newState = levelTree.createCurrentState(window.get(), this);
        if (newState) {
            changeState(std::move(newState));
            
            if (tienePartidaActiva()) {
                saveManager.getCurrentProgress().rutaActual = levelTree.getCurrentPath();
                guardarPartidaActual();
            }
        }
    }
}

void Game::volverDeCentinela() {
    if (levelTree.returnFromCentinela()) {
        auto newState = levelTree.createCurrentState(window.get(), this);
        if (newState) {
            changeState(std::move(newState));
            
            if (tienePartidaActiva()) {
                saveManager.getCurrentProgress().rutaActual = levelTree.getCurrentPath();
                guardarPartidaActual();
            }
        }
    }
}

void Game::cargarPartidaYContinuar(int slotId) {
    if (saveManager.cargarPartida(slotId)) {
        // Restaurar la ruta guardada en el árbol
        std::string rutaGuardada = saveManager.getCurrentProgress().rutaActual;
        if (!rutaGuardada.empty() && rutaGuardada != "principal") {
            levelTree.restorePath(rutaGuardada);
        }
        
        int nivelId = saveManager.getCurrentProgress().nivelActualId;
        std::cout << "📂 Cargando partida del slot " << slotId << " - " 
                  << levelTree.getCurrentNodeInfo() << std::endl;
        
        // Detener la música del menú antes de cambiar de estado
        detenerMusica();
        
        // Cargar el estado correspondiente
        auto newState = levelTree.createCurrentState(window.get(), this);
        if (newState) {
            changeState(std::move(newState));
        } else {
            // Fallback: ir al lobby
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
    while (!states.empty()) states.pop();
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
    
    // Limpiar todos los estados y poner el MenuState
    while (!states.empty()) {
        states.pop();
    }
    states.push(std::make_unique<MenuState>(window.get(), this));
    
    std::cout << "🏠 Volviendo al menú principal" << std::endl;
}

// Añadir al final de Game.cpp:
void Game::reintentarCentinela() {
    GameProgressData& progress = saveManager.getCurrentProgress();
    
    if (progress.modoElegido == GameProgressData::ModoJuego::CAMINO_AGRADABLE &&
        progress.tieneCheckpointCentinela) {
        
        // Restaurar la ruta del checkpoint
        levelTree.restorePath(progress.checkpointRutaArbol);
        
        // Limpiar el checkpoint para no poder usarlo infinitamente (solo un reintento)
        progress.tieneCheckpointCentinela = false;
        progress.checkpointRutaArbol = "";
        progress.checkpointCentinelaId = "";
        
        auto newState = levelTree.createCurrentState(window.get(), this);
        if (newState) {
            changeState(std::move(newState));
            std::cout << "🔄 Reintentando desde el checkpoint antes del centinela" << std::endl;
        }
    } else {
        // En modo CONSECUENCIAS o sin checkpoint, volver al menú principal
        std::cout << "💀 Has fallado. Las consecuencias son permanentes." << std::endl;
        returnToMenu();
    }
}