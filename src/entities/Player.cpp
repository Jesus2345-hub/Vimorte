#include "entities/Player.hpp"
#include <iostream>
#include <cmath>

Player::Player() : m_speed(300.0f), m_position(600, 300), m_sprite(nullptr) {
}

void Player::loadTexture(const std::string& path) {
    if (!m_texture.loadFromFile(path)) {
        std::cerr << "Error: No se pudo cargar textura del jugador: " << path << std::endl;
        return;
    }
    m_sprite = std::make_unique<sf::Sprite>(m_texture);
    
    float anchoDeseado = 100.0f;
    float altoDeseado = 150.0f;
    
    sf::Vector2u texturaSize = m_texture.getSize();
    float escalaX = anchoDeseado / texturaSize.x;
    float escalaY = altoDeseado / texturaSize.y;
    
    m_sprite->setScale(sf::Vector2f(escalaX, escalaY));
    
    sf::FloatRect bounds = m_sprite->getLocalBounds();
    m_sprite->setOrigin(sf::Vector2f(bounds.size.x / 2, bounds.size.y / 2));
    m_sprite->setPosition(m_position);
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
    if (m_sprite) {
        m_sprite->setPosition(m_position);
    }
}

void Player::draw(sf::RenderWindow& window) {
    if (m_sprite) {
        window.draw(*m_sprite);
    }
}

void Player::move(sf::Vector2f direction, float dt) {
    m_position += direction * m_speed * dt;
    if (m_sprite) {
        m_sprite->setPosition(m_position);
    }
}

sf::Vector2f Player::getPosition() const {
    return m_position;
}

sf::FloatRect Player::getHurtbox() const {
    if (m_sprite) {
        return m_sprite->getGlobalBounds();
    }
    return sf::FloatRect();
}

sf::FloatRect Player::getBounds() const {
    return getHurtbox();
}

sf::FloatRect Player::getHitbox() const {
    if (m_sprite) {
        sf::FloatRect bounds = m_sprite->getGlobalBounds();
        
        // La hitbox son solo los pies (60% del ancho y 20px de alto)
        float anchoHitbox = bounds.size.x * 0.6f; 
        float altoHitbox = 20.0f; 
        
        return sf::FloatRect(
            { bounds.position.x + (bounds.size.x - anchoHitbox) / 2.0f, 
              bounds.position.y + bounds.size.y - altoHitbox },
            { anchoHitbox, altoHitbox }
        );
    }
    return sf::FloatRect();
}