#include "Lobby.hpp"
#include "PauseState.hpp"
#include <iostream>
#include <cmath>

LobbyState::LobbyState(sf::RenderWindow* window, Game* game) 
    : State(window, game), m_background(nullptr)
{
    // 1. CARGAR JUGADOR
    m_player.loadTexture("assets/images/player/idle.png");
    m_player.setPosition(600, 300);
    m_player.setSpeed(300.0f);
    
    // 2. CARGAR FONDO DEL LOBBY
    if (m_backgroundTexture.loadFromFile("assets/images/lobby/background.jpg")) {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2f scale(
            1280.0f / m_backgroundTexture.getSize().x,
            720.0f / m_backgroundTexture.getSize().y
        );
        m_background->setScale(scale);
    } else {
        std::cerr << "Error: No se pudo cargar fondo del lobby" << std::endl;
        // m_background se queda como nullptr, usaremos fallback en draw()
    }
}

void LobbyState::update(float dt) 
{
    // MOVIMIENTO (WASD o flechas)
    sf::Vector2f movimiento(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || 
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
        movimiento.y -= 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || 
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
        movimiento.y += 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || 
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
        movimiento.x -= 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || 
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
        movimiento.x += 1.f;
    }
    
    // Normalizar para movimiento diagonal igual de rápido
    if (movimiento.x != 0 || movimiento.y != 0) {
        float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
        movimiento /= length;
    }
    
    m_player.move(movimiento, dt);
    m_player.update(dt);
    
    // ABRIR MENÚ DE PAUSA CON ESC
    static bool escapeProcesado = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        if (!escapeProcesado) {
            escapeProcesado = true;
            game->pushState(std::make_unique<PauseState>(window, game));
        }
    } else {
        escapeProcesado = false;
    }
}

void LobbyState::draw()
{
    if (!window) return;
    
    // Dibujar fondo
    if (m_background) {
        window->draw(*m_background);
    } else {
        sf::RectangleShape fallback(sf::Vector2f(1280, 720));
        fallback.setFillColor(sf::Color(30, 30, 50));
        window->draw(fallback);
    }
    
    // Dibujar jugador
    m_player.draw(*window);
}
