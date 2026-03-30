#include "Game.hpp"
#include "MenuState.hpp" 
#include "State.hpp" 
#include <optional>
#include <iostream>

Game::Game() 
{
    // Creamos la ventana con la resolución estándar
    window = std::make_unique<sf::RenderWindow>(sf::VideoMode({1280, 720}), "Vimorte");
    
    // Limitamos los FPS para que no consuma recursos de más y el delta sea estable
    window->setFramerateLimit(60);

    // Iniciamos con el MenuState
    states.push(std::make_unique<MenuState>(window.get(), this));
}

void Game::changeState(std::unique_ptr<State> state) 
{
    if (!states.empty()) states.pop();
    states.push(std::move(state));
}

void Game::run() 
{
    sf::Clock clock;
    while (window->isOpen()) {
        float deltaTime = clock.restart().asSeconds();

        while (const std::optional<sf::Event> event = window->pollEvent()) {
            if (event->is<sf::Event::Closed>()) window->close();
        }

        if (!states.empty()) states.top()->update(deltaTime);

        // 1. Limpiamos con el color de fondo
        window->clear(sf::Color::Cyan); 
        sf::Texture::bind(nullptr);
        
        // ========== DIBUJAR TODOS LOS ESTADOS ==========
        // Como no podemos copiar la pila directamente, usamos un vector temporal
        std::vector<State*> estadosParaDibujar;
        std::stack<std::unique_ptr<State>> temp;
        
        // Extraer punteros sin mover los unique_ptr
        while (!states.empty()) {
            estadosParaDibujar.push_back(states.top().get());
            temp.push(std::move(states.top()));
            states.pop();
        }
        
        // Restaurar la pila original
        while (!temp.empty()) {
            states.push(std::move(temp.top()));
            temp.pop();
        }
        
        // Dibujar desde el fondo (primero en el vector) hasta la cima (último)
        for (int i = estadosParaDibujar.size() - 1; i >= 0; --i) {
            window->setView(window->getDefaultView());
            estadosParaDibujar[i]->draw();
        }
        // ========== FIN DIBUJAR TODOS ==========

        // 2. 🚨 EL EXORCISMO 🚨
        window->setView(window->getDefaultView());
        sf::Texture::bind(nullptr); 

        window->display(); 
    }
}

void Game::popState() 
{
    if (!states.empty()) {
        states.pop();
    }
}

void Game::pushState(std::unique_ptr<State> state) 
{
    states.push(std::move(state));
}

void Game::clearStates() 
{
    while (!states.empty()) {
        states.pop();
    }
}

void Game::returnToMenu() 
{
    // Restaurar vista por defecto
    window->setView(window->getDefaultView());
    // Vaciar la pila
    while (!states.empty()) {
        states.pop();
    }
    
    // Agregar el menú
    states.push(std::make_unique<MenuState>(window.get(), this));
}