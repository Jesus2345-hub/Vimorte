#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include <stack>
#include "Administradores/GameSaveManager.hpp"
#include "Estructuras/LevelTree.hpp"

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
    bool m_isInLevel = false;

    bool m_adminMode = false;

    bool m_pendingMenuReturn = false;
    bool m_pendingStateChange = false;
    std::unique_ptr<State> m_pendingState;
public:
    Game();
    ~Game();
    void run();
    
    void completarNivelActual(int siguienteNivelId);
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

        void irACentinela();
    
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
    
    void setPantallaCompleta(bool fullscreen);
    void aplicarConfiguracionPantalla();
    void applyCurrentView();
    std::stack<std::unique_ptr<State>>& getStates() { return states; }
    void setIsInLevel(bool inLevel) { m_isInLevel = inLevel; }
    bool isInLevel() const { return m_isInLevel; }

    //set para modo admin
    void adminVolverAlNivelAnterior();
    void setAdminMode(bool enabled) { m_adminMode = enabled; }
    bool isAdminMode() const { return m_adminMode; }
};

#endif