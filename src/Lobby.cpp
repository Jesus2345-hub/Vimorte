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
    if (m_backgroundTexture.loadFromFile("assets/images/lobby/background.png")) {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2f scale(
            1280.0f / m_backgroundTexture.getSize().x,
            720.0f / m_backgroundTexture.getSize().y
        );
        m_background->setScale(scale);
    } else {
        std::cerr << "Error: No se pudo cargar fondo del lobby" << std::endl;
    }

    // 3. CONFIGURAR COLISIONES
    configurarColisiones();
}

void LobbyState::update(float dt) 
{
    sf::Vector2f posAnterior = m_player.getPosition();

    // MOVIMIENTO
    sf::Vector2f movimiento(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) movimiento.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) movimiento.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) movimiento.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) movimiento.x += 1.f;
    
    if (movimiento.x != 0 || movimiento.y != 0) {
        float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
        movimiento /= length;
    }
    
    m_player.move(movimiento, dt);
    m_player.update(dt);
    
    // Verificación de colisiones usando la Hitbox de los pies
    for (const auto& obj : m_mapaFisico) 
    {
        if (m_player.getHitbox().findIntersection(obj.getBounds())) 
        {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break; 
        }
    }

    // PAUSA
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
    
    if (m_background) {
        window->draw(*m_background);
    } else {
        sf::RectangleShape fallback(sf::Vector2f(1280, 720));
        fallback.setFillColor(sf::Color(30, 30, 50));
        window->draw(fallback);
    }
    
    m_player.draw(*window);
}

void LobbyState::configurarColisiones() 
{
    m_mapaFisico.clear();
    m_mapaFisico.emplace_back(0.f, 0.f, 900.f, 220.f);
    m_mapaFisico.emplace_back(900.f, 0.f, 1280.f, 5.f);
    m_mapaFisico.emplace_back(1180.f, 0.f, 100.f, 220.f);
    m_mapaFisico.emplace_back(0.f, 220.f, 30.f, 410.f);
    m_mapaFisico.emplace_back(1000.f, 530.f, 630.f, 300.f);
    m_mapaFisico.emplace_back(820.f, 655.f, 50.f, 50.f);
    m_mapaFisico.emplace_back(590.f, 410.f, 120.f, 100.f);
    m_mapaFisico.emplace_back(300.f, 650.f, 150.f, 80.f);
    m_mapaFisico.emplace_back(-5.f, 0.f, 5.f, 720.f); 
    m_mapaFisico.emplace_back(1280.f, 0.f, 5.f, 720.f);
    m_mapaFisico.emplace_back(0.f, 715.f, 1280.f, 5.f);
}