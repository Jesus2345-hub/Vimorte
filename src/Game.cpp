#include "Game.hpp"
#include "MenuState.hpp" 
#include "State.hpp"
#include "Nivel1State.hpp"
#include <optional>
#include <iostream>
#include <vector>

Game::Game() 
{
    window = std::make_unique<sf::RenderWindow>(sf::VideoMode({1280, 720}), "Vimorte");
    window->setFramerateLimit(60);
    states.push(std::make_unique<MenuState>(window.get(), this));
}

Game::~Game() = default;

void Game::run() 
{
    sf::Clock clock;
    while (window->isOpen()) {
        float deltaTime = clock.restart().asSeconds();

        while (const std::optional<sf::Event> event = window->pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window->close();
            }
            
            // Pasar evento al estado actual si es Nivel1State
            if (!states.empty()) {
                if (auto* nivel1 = dynamic_cast<Nivel1State*>(states.top().get())) {
                    nivel1->handleEvent(*event);
                }
            }
        }

        if (!states.empty()) {
            states.top()->update(deltaTime);
        }

        window->clear(sf::Color::Black); 
        
        if (!states.empty()) {
            // DIBUJAR TODOS LOS ESTADOS (Para transparencia en Pausa)
            std::vector<State*> paraDibujar;
            std::stack<std::unique_ptr<State>> temp;

            while (!states.empty()) {
                paraDibujar.push_back(states.top().get());
                temp.push(std::move(states.top()));
                states.pop();
            }
            while (!temp.empty()) {
                states.push(std::move(temp.top()));
                temp.pop();
            }
            for (int i = paraDibujar.size() - 1; i >= 0; --i) {
                window->setView(window->getDefaultView());
                paraDibujar[i]->draw();
            }
        }
        window->display(); 
    }
}

void Game::changeState(std::unique_ptr<State> state) {
    if (!states.empty()) states.pop();
    states.push(std::move(state));
}

void Game::pushState(std::unique_ptr<State> state) { 
    states.push(std::move(state)); 
}

void Game::popState() { 
    if (!states.empty()) states.pop(); 
}

void Game::returnToMenu() {
    while (states.size() > 1) states.pop();
    changeState(std::make_unique<MenuState>(window.get(), this));
}