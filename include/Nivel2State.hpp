#ifndef NIVEL2STATE_HPP
#define NIVEL2STATE_HPP

#include "State.hpp"
#include "entities/Player.hpp"
#include "MinigameRuleta.hpp"
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

private:
    // Jugador
    Player m_player;
    
    // Fondo
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    sf::Vector2f m_worldSize;
    
    // Cámara
    sf::View m_camera;
    sf::Vector2u m_lastWindowSize;
    
    // Colisiones
    std::vector<sf::FloatRect> m_mapaFisico;
    void configurarColisiones();
    
    // Áreas de interacción
    sf::FloatRect m_barArea;        // Bartender
    sf::FloatRect m_ruletaArea;     // Ruleta
    sf::FloatRect m_slotMachineArea; // Tragaperras
    sf::FloatRect m_blackjackArea;  // Blackjack
    sf::FloatRect m_puertaSalidaArea;
    sf::FloatRect m_vendedorArea;   // Donde comprar la llave
    
    // Estados de cercanía
    bool m_cercaBar = false;
    bool m_cercaRuleta = false;
    bool m_cercaSlotMachine = false;
    bool m_cercaBlackjack = false;
    bool m_cercaPuertaSalida = false;
    bool m_cercaVendedor = false;
    
    // Dinero y llave
    int m_dinero = 0;
    bool m_tieneLlave = false;
    
    // Minijuegos
    MinigameRuleta m_ruletaMinigame;
    
    // Fuente y textos
    sf::Font m_font;
    bool m_fontLoaded = false;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    std::unique_ptr<sf::Text> m_textoDinero;
    std::unique_ptr<sf::Text> m_textoMensaje;
    
    // Tutorial
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
};

#endif