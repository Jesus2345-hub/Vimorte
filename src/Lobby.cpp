#include "Lobby.hpp"
#include <iostream>

LobbyState::LobbyState(sf::RenderWindow* window, Game* game) 
    : State(window, game) 
{
    // Configuramos el rombo
    jugadorPrototipo.setRadius(40.f);
    jugadorPrototipo.setPointCount(4); 
    
    // Intentamos ponerle color (aunque salga negro, dejamos el código listo)
    jugadorPrototipo.setFillColor(sf::Color::White);
    
    // Posición inicial
    jugadorPrototipo.setPosition({ 600.f, 300.f });
}

void LobbyState::update(float dt) 
{
    sf::Vector2f movimiento(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movimiento.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movimiento.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movimiento.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movimiento.x += 1.f;

    // Movimiento simple
    jugadorPrototipo.move(movimiento * 300.f * dt);
}

void LobbyState::draw()
{
    if (window) {
        // Dibujo simple
        window->draw(jugadorPrototipo);
    }
}