#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include <stack>
#include "GameSaveManager.hpp"
#include "LevelTree.hpp"
#include "Config.hpp"

class State;

class Game {
private:
    std::unique_ptr<sf::RenderWindow> window;
    std::stack<std::unique_ptr<State>> states;
    
    // Sistema de guardado
    GameSaveManager saveManager;
    
    // Árbol de niveles
    LevelTree levelTree;
    
    // Configuración de audio
    float volGeneral = 100.f;
    float volMusica = 50.f;
    float volEfectos = 80.f;
    
    // Música del juego
    sf::Music m_currentMusic;
    std::string m_currentMusicPath;
    
    void cargarConfiguracionAudio();
    void guardarConfiguracionAudio();

public:
    Game();
    ~Game();
    void run();
    
    // Gestión de estados
    void pushState(std::unique_ptr<State> state);
    void popState();
    void changeState(std::unique_ptr<State> state);
    void returnToMenu();
    
    // Sistema de guardado
    GameSaveManager& getSaveManager() { return saveManager; }
    void guardarPartidaActual();
    void cargarPartidaYContinuar(int slotId);
    bool tienePartidaActiva() const { return saveManager.getCurrentSlotId() >= 0; }
    
    // Árbol de niveles
    LevelTree& getLevelTree() { return levelTree; }
    void avanzarNivel();
    void entrarCentinela();
    void volverDeCentinela();
    
    // Configuración de audio
    void setVolGeneral(float v);
    void setVolMusica(float v);
    void setVolEfectos(float v);
    
    float getVolGeneral() const { return volGeneral; }
    float getVolMusica() const { return volMusica; }
    float getVolEfectos() const { return volEfectos; }
    float getRealMusica() const { return (volMusica * volGeneral) / 100.f; }
    float getRealEfectos() const { return (volEfectos * volGeneral) / 100.f; }
    
    // Gestión de música
    void cambiarMusica(const std::string& rutaMusica);
    void detenerMusica();
    void actualizarVolumenMusica();
    
    // Añadir en la sección pública de Game.hpp:
    void reintentarCentinela();
    
    Config& getConfig() { return Config::getInstance(); }
    void setPantallaCompleta(bool fullscreen);
    void aplicarConfiguracionPantalla();
    void applyCurrentView();
    // En la sección pública de Game.hpp, añadir:
std::stack<std::unique_ptr<State>>& getStates() { return states; }
};

#endif