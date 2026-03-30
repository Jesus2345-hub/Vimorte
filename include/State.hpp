#pragma once
#include <SFML/Graphics.hpp>

class Game; 

class State 
{
    protected:
        sf::RenderWindow* window;
        Game* game; 

    public:
        
        State(sf::RenderWindow* window, Game* game) 
            : window(window), game(game) {}

        virtual ~State() = default; 
        virtual void update(float dt) = 0;
        virtual void draw() = 0;
};