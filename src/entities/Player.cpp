#include "entities/Player.hpp"
#include <iostream>
#include <cmath>

Player::Player() : m_speed(300.0f), m_position(600, 300), m_currentAnimation(nullptr), m_lastDirection(0, 1) {
    m_sprite = nullptr;  // Se creará en loadAssets
}

void Player::loadAssets()
{
    // Cargar textura idle
    if (!m_idleTexture.loadFromFile("assets/images/player/idle.png")) {
        std::cerr << "Error: No se pudo cargar idle.png" << std::endl;
    }
    m_idleTexture.setSmooth(false);

    // Crear sprite con la textura idle
    m_sprite = std::make_unique<sf::Sprite>(m_idleTexture);
    
    // Centrar origen
    sf::FloatRect bounds = m_sprite->getLocalBounds();
    m_sprite->setOrigin(sf::Vector2f(bounds.size.x / 2, bounds.size.y / 2));
    m_sprite->setPosition(m_position);
    
    // Cargar animaciones (4 frames, duración total 0.4 segundos)
    m_walkDown.loadFromFolder("assets/images/player/walk/walk_down", 4, 0.4f);
    m_walkUp.loadFromFolder("assets/images/player/walk/walk_up", 4, 0.4f);
    m_walkRight.loadFromFolder("assets/images/player/walk/walk_right", 4, 0.4f);

    // Diagonales arriba
    m_walkUpRight.loadFromFolder("assets/images/player/walk/walk_up_right", 4, 0.6f);

    // Diagonales abajo
    m_walkDownRight.loadFromFolder("assets/images/player/walk/walk_down_right", 4, 0.4f);
    
    m_currentAnimation = nullptr;
}

void Player::setPosition(float x, float y) {
    m_position = {x, y};
    if (m_sprite) {
        m_sprite->setPosition(m_position);
    }
}

void Player::setSpeed(float speed) {
    m_speed = speed;
}

void Player::update(float dt) {
    if (!m_sprite) return;
    
    if (m_currentAnimation) {
        m_currentAnimation->update(dt);
        m_currentAnimation->applyToSprite(*m_sprite);
    }
    m_sprite->setPosition(m_position);
}

void Player::draw(sf::RenderWindow& window) {
    if (m_sprite) {
        window.draw(*m_sprite);
    }
}

void Player::move(sf::Vector2f direction, float dt) {
    if (direction.x != 0 || direction.y != 0) {
        m_position += direction * m_speed * dt;
        updateAnimation(direction);
    } else {
        // Quieto: mostrar idle
        if (m_currentAnimation) {
            m_currentAnimation = nullptr;
            if (m_sprite) {
                m_sprite->setTexture(m_idleTexture);
            }
        }
    }
    if (m_sprite) {
        m_sprite->setPosition(m_position);
    }
}

void Player::updateAnimation(sf::Vector2f direction) {
    // Guardar dirección para cuando se detenga
    if (direction.x != 0 || direction.y != 0) {
        m_lastDirection = direction;
    }
    
    // Normalizar para detectar dirección pura
    float absX = std::abs(direction.x);
    float absY = std::abs(direction.y);
    
    bool movingRight = direction.x > 0;
    bool movingLeft = direction.x < 0;
    bool movingUp = direction.y < 0;
    bool movingDown = direction.y > 0;
    
    // DIAGONALES HACIA ARRIBA: usar animación lateral (walk_right con espejo)
    if ((movingRight && movingUp) || (movingLeft && movingUp)) {
        // Diagonal arriba-derecha o arriba-izquierda
        m_currentAnimation = &m_walkRight;
        
        if (movingRight) {
            // Mirando derecha
            if (m_sprite) m_sprite->setScale(sf::Vector2f(1, 1));
        } else if (movingLeft) {
            // Mirando izquierda (espejo)
            if (m_sprite) m_sprite->setScale(sf::Vector2f(-1, 1));
        }
    }
    // DIAGONALES HACIA ABAJO: usar animación diagonal específica
    else if (movingRight && movingDown) {
        m_currentAnimation = &m_walkDownRight;
        if (m_sprite) m_sprite->setScale(sf::Vector2f(1, 1));
    }
    else if (movingLeft && movingDown) {
        m_currentAnimation = &m_walkDownRight;
        if (m_sprite) m_sprite->setScale(sf::Vector2f(-1, 1));
    }
    // SOLO DERECHA
    else if (movingRight && !movingUp && !movingDown) {
        m_currentAnimation = &m_walkRight;
        if (m_sprite) m_sprite->setScale(sf::Vector2f(1, 1));
    }
    // SOLO IZQUIERDA
    else if (movingLeft && !movingUp && !movingDown) {
        m_currentAnimation = &m_walkRight;
        if (m_sprite) m_sprite->setScale(sf::Vector2f(-1, 1));
    }
    // SOLO ARRIBA
    else if (movingUp) {
        m_currentAnimation = &m_walkUp;
        if (m_sprite) m_sprite->setScale(sf::Vector2f(1, 1));
    }
    // SOLO ABAJO
    else if (movingDown) {
        m_currentAnimation = &m_walkDown;
        if (m_sprite) m_sprite->setScale(sf::Vector2f(1, 1));
    }
}


sf::FloatRect Player::getBounds() const {
    if (m_sprite) {
        return m_sprite->getGlobalBounds();
    }
    return sf::FloatRect();
}

sf::Vector2f Player::getPosition() const {
    return m_position;
}
