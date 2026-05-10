#pragma once
#include "State.hpp"
#include "Game.hpp"
#include <SFML/Graphics.hpp>
#include "entities/Player.hpp"
#include "entities/Balon.hpp"
#include "Obstaculo.hpp"
#include "Inventory.hpp"
#include <memory>
#include <vector>

class Nivel7State : public State 
{
private:
    Player m_player;
    
    // Fondo del nivel 7
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    sf::Vector2f m_worldSize;
    
    // Sistema de colisiones
    std::vector<Obstaculo> m_mapaFisico;
    void configurarColisiones();
    
    // Cámara
    sf::View m_camera;
    sf::Vector2u m_lastWindowSize;
    
    // Tutorial
    bool m_mostrarTutorial;
    bool m_mostrarTutorialPorTecla;
    bool m_fontLoaded;
    
    // Fuente y textos
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    std::unique_ptr<sf::Text> m_textoMensaje;
    struct MensajeTemporal {
        std::string texto;
        float tiempoRestante;
        sf::Color color;
    } m_msjActual;
    void mostrarMensaje(const std::string& texto, float duracion = 2.0f, sf::Color color = sf::Color::Yellow);
    
    // Área de salida (ascensor)
    sf::FloatRect m_puertaSalidaArea;
    bool m_cercaPuertaSalida;

    // Balón de fútbol
    Balon m_balon;
    sf::FloatRect m_balonArea;
    sf::FloatRect m_arcoArea;
    bool m_balonEnArco;
    
    void verificarSalidaNivel();
    
public:
    Nivel7State(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
    ~Nivel7State() override = default;
};