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
    

    sf::FloatRect m_teleportZone2;
    sf::Vector2f m_teleportDestination2;
    sf::FloatRect m_teleportZone;
    sf::Vector2f m_teleportDestination;

    //
    std::unique_ptr<sf::Text> m_textoCoordenadas;
    //

    VitalSigns m_vitalSignsAndres;
    VitalSigns m_vitalSignsAndrea; 

    void verificarTeletransportePostJuego();

    float m_dartsTriggerRadius = 80.f;
    // Tutorial
    bool m_mostrarTutorial;
    bool m_mostrarTutorialPorTecla;
    bool m_escapeConsumed;
    bool m_fontLoaded; 
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

    //minijuego de dardos
    MinigameDarts m_dartsMinigame;

    //minijuego Memoria
    MiniGameTetris m_tetris;

    // Área de interacción con el tablón de dardos
    sf::FloatRect m_dartsArea;
    bool m_cercaDarts;
    bool m_skipPauseThisFrame;
    void configurarInteracciones();

    // Área de interacción con el TETRIS 
    sf::FloatRect m_tetrisArea;
    bool m_cercaTetris;

    // Áreas de interacción
    sf::FloatRect m_puertaSalidaArea;
    bool m_cercaPuertaSalida;
    void verificarSalidaNivel();

    // UI y textos
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    std::unique_ptr<sf::Text> m_textoMensaje;
    struct MensajeTemporal {
        std::string texto;
        float tiempoRestante;
        sf::Color color;
    } m_msjActual;

    void actualizarUIPosiciones();

    void mostrarMensaje(const std::string& texto, float duracion, sf::Color color);
    sf::Vector2u m_lastWindowSize;
};