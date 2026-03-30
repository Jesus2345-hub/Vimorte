#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <stack>
#include "State.hpp" 

class Game
{
    private:
        std::unique_ptr<sf::RenderWindow> window;
        std::stack<std::unique_ptr<State>> states;

    public:
        Game();
        int getStateCount() const { return states.size(); }
        std::stack<std::unique_ptr<State>>& getStates() { return states; }
        void pushState(std::unique_ptr<State> state);
        void popState();
        void changeState(std::unique_ptr<State> state); // <--- IMPORTANTE
        void run();
        void clearStates();
        void returnToMenu();
};