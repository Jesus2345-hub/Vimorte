#include "Game.hpp"
#include "MenuState.hpp" 
#include "State.hpp" 
#include <optional>

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
        if (!states.empty()) {
    window->setView(window->getDefaultView());
    states.top()->draw();
}
        // 2. 🚨 EL EXORCISMO 🚨
        // Resetamos la cámara y forzamos a SFML a usar texturas nulas
        window->setView(window->getDefaultView());
        
        // Esto le dice a la tarjeta de video: "Olvida cualquier imagen que tuvieras cargada"
        sf::Texture::bind(nullptr); 

        // 3. Dibujamos
        if (!states.empty()) {
            states.top()->draw();
        }

        window->display(); 
    }
}