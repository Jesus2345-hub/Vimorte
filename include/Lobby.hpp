#pragma once
#include "State.hpp"
#include "Game.hpp"
#include <SFML/Graphics.hpp>
#include "entities/Player.hpp"
#include "Obstaculo.hpp" // El header de Sarai
#include <memory>
#include <vector>

class LobbyState : public State 
{
private:
    Player m_player;
    
    // Fondo del lobby
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;

    // Lógica de colisiones de Sarai
    std::vector<Obstaculo> m_mapaFisico;
    void configurarColisiones();
    
public:
    LobbyState(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    ~LobbyState() override = default;
};