#pragma once
#include "State.hpp"
#include "Game.hpp"
#include <SFML/Graphics.hpp>
#include "entities/Player.hpp"
#include "Obstaculo.hpp"
#include "MinigamePool.hpp"
#include "Inventory.hpp"
#include <memory>
#include <vector>

class Nivel1State : public State 
{
private:
    Player m_player;
    
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    sf::Vector2f m_worldSize;
    
    std::vector<Obstaculo> m_mapaFisico;
    void configurarColisiones();
    
    sf::View m_camera;
    
    MinigamePool m_poolMinigame;
    bool m_cercaMesaPool;
    
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    
    std::vector<sf::CircleShape> m_worldItems;
    std::vector<bool> m_itemsCollected;
    
public:
    Nivel1State(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
    ~Nivel1State() override = default;
};