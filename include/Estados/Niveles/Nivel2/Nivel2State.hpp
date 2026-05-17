#ifndef NIVEL2STATE_HPP
#define NIVEL2STATE_HPP

#include "Estados/State.hpp"
#include "Entidades/Player.hpp"
#include "Estados/Niveles/Nivel2/MinigameRuleta.hpp"
#include "Estados/Niveles/Nivel2/MinigameBartender.hpp"
#include "Estados/Niveles/Nivel3/EfectoExplosion.hpp"
#include "Estados/Niveles/Nivel3/EfectoGrieta.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class Nivel2State : public State {
public:
    Nivel2State(sf::RenderWindow* window, Game* game);
    ~Nivel2State() override = default;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw() override;
    void actualizarPosicionRuleta();

private:

    Player m_player;
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    sf::Vector2f m_worldSize;
    sf::View m_camera;
    sf::Vector2u m_lastWindowSize;
    
    std::vector<sf::FloatRect> m_mapaFisico;
    void configurarColisiones();
    
    sf::FloatRect m_barArea;
    sf::FloatRect m_ruletaArea;
    sf::FloatRect m_puertaSalidaArea;
    sf::FloatRect m_vendedorArea;
    
    bool m_cercaBar = false;
    bool m_cercaRuleta = false;
    bool m_cercaPuertaSalida = false;
    bool m_cercaVendedor = false;
        bool m_cercaPiedra;    // Indica si el jugador esta cerca de la piedra explosiva
    bool m_cercaGrieta;    // Indica si el jugador esta cerca de la grieta abierta
        bool m_cercaCristal;     // Indica si el jugador esta cerca del cristal

    int m_dinero = 0;
    bool m_tieneLlave = false;
        bool m_tienePalo;
    bool m_paloLanzado;
        bool m_debeIrACentinela;     // Bandera para ir al centinela en el momento seguro
    float m_tiempoAntesCentinela; // Pequeña pausa antes de ir al centinela
    
    MinigameRuleta m_ruletaMinigame;
    MinigameBartender m_bartenderMinigame;
    
    sf::Font m_font;
    bool m_fontLoaded = false;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    std::unique_ptr<sf::Text> m_textoDinero;
    std::unique_ptr<sf::Text> m_textoMensaje;
    
    bool m_mostrarTutorial = false;
    bool m_mostrarTutorialPorTecla = false;
    bool m_escapeConsumed = false;
    
    void verificarSalidaNivel();
    void mostrarMensaje(const std::string& texto, float duracion = 2.0f, sf::Color color = sf::Color::Yellow);
    
    struct MensajeTemporal {
        std::string texto;
        float tiempoRestante = 0.0f;
        sf::Color color = sf::Color::Yellow;
    };
    MensajeTemporal m_msjActual;

        // ===== SISTEMA DE EXPLOSION Y GRIETA =====
    EfectoExplosion m_explosion;
    EfectoGrieta m_grieta;
    bool m_explosionIniciada;
    bool m_grietaAbierta;
    sf::FloatRect m_areaGrieta;
    sf::Vector2f m_posicionExplosion;
    sf::FloatRect m_areaPalo;  // Area de la piedra que el jugador rompe
           // Indica si ya se lanzo la piedra
    sf::FloatRect m_areaCristal;     // Area donde esta el cristal de la llave
};

#endif