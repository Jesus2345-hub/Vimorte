#pragma once
#include "State.hpp"
#include "entities/Player.hpp"
#include "MiniGameDarts_sara.hpp"
#include "MiniGameTetris.hpp"
#include "VitalSigns_sara.hpp"
#include <memory>
#include <vector>

class NivelSara1State : public State {
public:
    NivelSara1State(sf::RenderWindow* window, Game* game);
    ~NivelSara1State() = default;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw() override;

private:
    // Mundo
    std::unique_ptr<sf::Sprite> m_background;
    sf::Texture m_backgroundTexture;
    sf::Vector2f m_worldSize;
    sf::View m_camera;

    // Jugador
    Player m_player;

    // Física
    std::vector<sf::FloatRect> m_mapaFisico;
    void configurarColisiones();

    // Minijuegos
    MinigameDarts m_dartsMinigame;
    MiniGameTetris m_tetris;

    // Áreas de interacción
    sf::FloatRect m_dartsArea;
    sf::FloatRect m_tetrisArea;
    sf::FloatRect m_ascensorArea;      // Área para salir del nivel
    sf::FloatRect m_teleportZone;
    sf::Vector2f m_teleportDestination;
    sf::FloatRect m_teleportZone2;
    sf::Vector2f m_teleportDestination2;

    // Estados de cercanía
    bool m_cercaDarts;
    bool m_cercaTetris;
    bool m_cercaAscensor;

    // Pacientes
    VitalSigns m_vitalSignsAndres;
    VitalSigns m_vitalSignsAndrea;
    bool m_ambosEstabilizados;
    float m_mensajeVictoriaTimer;
    bool m_mostrandoMensajeVictoria;

    // Tutorial
    bool m_mostrarTutorial;
    bool m_mostrarTutorialPorTecla;
    bool m_escapeConsumed;
    bool m_fontLoaded;

    // UI y textos
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    std::unique_ptr<sf::Text> m_textoMensaje;
    std::unique_ptr<sf::Text> m_textoCoordenadas;
    
    struct MensajeTemporal {
        std::string texto;
        float tiempoRestante;
        sf::Color color;
    } m_msjActual;

    // Métodos
    void verificarSalidaNivel();           
    void verificarTeletransportePostJuego();
    void actualizarUIPosiciones();
    void mostrarMensaje(const std::string& texto, float duracion, sf::Color color);
    
    sf::Vector2u m_lastWindowSize;
    bool m_skipPauseThisFrame;
};