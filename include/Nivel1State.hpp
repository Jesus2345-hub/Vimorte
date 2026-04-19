#pragma once
#include "State.hpp"
#include "Game.hpp"
#include <SFML/Graphics.hpp>
#include "entities/Player.hpp"
#include "Obstaculo.hpp"
#include "MinigamePool.hpp"
#include <memory>
#include <vector>

class Nivel1State : public State 
{
private:
    Player m_player;
    
    // Fondo del nivel 1
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    
    // Dimensiones del mundo
    sf::Vector2f m_worldSize;
    
    // Sistema de colisiones
    std::vector<Obstaculo> m_mapaFisico;
    void configurarColisiones();
    
    // Cámara para el desplazamiento
    sf::View m_camera;
    
    // Minijuego de pool
    MinigamePool m_poolMinigame;
    bool m_cercaMesaPool;
    
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    
public:
    Nivel1State(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event);
    ~Nivel1State() override = default;
};