#pragma once
#include "Estados/State.hpp"
#include "Entidades/Player.hpp"
#include "Estados/Niveles/Nivel4/MiniGameDarts.hpp"
#include "Estados/Niveles/Nivel4/MiniGameTetris.hpp"
#include "Estados/Niveles/Nivel4/VitalSigns.hpp"
#include <memory>
#include <vector>

// BLOQUE DE MURO ROMPIBLE PARA EL CENTINELA
struct BloqueMuro {
    sf::FloatRect area;
    int golpesRestantes;
    bool muroRoto;
    bool mostrandoMensaje;
    float mensajeTimer;
};

class Nivel4State : public State {
public:
    Nivel4State(sf::RenderWindow* window, Game* game);
    ~Nivel4State() = default;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw() override;

private:

    // Para manejo de muerte
    float m_muerteTimer;
    bool m_mostrandoMuerte;
    std::string m_mensajeMuerte;
    bool m_gameOverTriggered;

    // LOGICAS PARA COMPLETAR CENTINELA
    BloqueMuro m_bloqueMuro;
    bool m_cercaBloqueMuro;
    bool m_golpeCooldown;  
    float m_golpeTimer;
    void configurarBloqueMuro();
    void verificarInteraccionMuro(float dt);
    void dibujarMuro();

    // HERRAMIENTA PARA RECOGER
    bool m_herramientaRecogida;
    sf::FloatRect m_herramientaArea;
    bool m_cercaHerramienta;
    sf::Texture m_herramientaMapTexture;
    std::unique_ptr<sf::Sprite> m_herramientaMapSprite;
    
    // MUNDO
    std::unique_ptr<sf::Sprite> m_background;
    sf::Texture m_backgroundTexture;
    sf::Vector2f m_worldSize;
    sf::View m_camera;

    // JUGADOR
    Player m_player;

    // FISICA
    std::vector<sf::FloatRect> m_mapaFisico;
    void configurarColisiones();

    // MMINIJUEGOS
    MinigameDarts m_dartsMinigame;
    MiniGameTetris m_tetris;

    // AREAS  DE INTERACCION
    sf::FloatRect m_dartsArea;
    sf::FloatRect m_tetrisArea;
    sf::FloatRect m_ascensorArea;      
    sf::FloatRect m_teleportZone;
    sf::Vector2f m_teleportDestination;
    sf::FloatRect m_teleportZone2;
    sf::Vector2f m_teleportDestination2;

    // ESTADOS DE CERCANIA
    bool m_cercaDarts;
    bool m_cercaTetris;
    bool m_cercaAscensor;

    // PACIENTES
    VitalSigns m_vitalSignsAndres;
    VitalSigns m_vitalSignsAndrea;
    bool m_ambosEstabilizados;
    float m_mensajeVictoriaTimer;
    bool m_mostrandoMensajeVictoria;

    // TUTOTIAL
    bool m_mostrarTutorial;
    bool m_mostrarTutorialPorTecla;
    bool m_escapeConsumed;
    bool m_fontLoaded;

    // UI Y TEXTOS
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    std::unique_ptr<sf::Text> m_textoMensaje;
    std::unique_ptr<sf::Text> m_textoCoordenadas;
    
    struct MensajeTemporal {
        std::string texto;
        float tiempoRestante;
        sf::Color color;
    } m_msjActual;

    // METODOS
    void verificarSalidaNivel();           
    void verificarTeletransportePostJuego();
    void actualizarUIPosiciones();
    void mostrarMensaje(const std::string& texto, float duracion, sf::Color color);
    
    sf::Vector2u m_lastWindowSize;
    bool m_skipPauseThisFrame;
};