#pragma once
#include "State.hpp"
#include <SFML/Graphics.hpp>

class LobbyState : public State 
{
    private:
        sf::CircleShape jugadorPrototipo; // Volvemos al CircleShape

    public:
        LobbyState(sf::RenderWindow* window, Game* game);
        void update(float dt) override;
        void draw() override;
        ~LobbyState() override = default;
};