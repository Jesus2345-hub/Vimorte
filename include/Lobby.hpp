#pragma once
#include "State.hpp"
#include "Game.hpp"
#include <SFML/Graphics.hpp>
#include "entities/Player.hpp"
#include <memory>

class LobbyState : public State 
{
private:
    Player m_player;
    
    // Fondo del lobby (como puntero para poder crearlo después)
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    
public:
    LobbyState(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    ~LobbyState() override = default;
};
