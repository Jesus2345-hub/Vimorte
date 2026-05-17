#include "Entidades/LeBron.hpp"
#include <iostream>

LeBron::LeBron() 
    : m_position(0.f, 0.f),
      m_escala(0.15f, 0.15f)
{
    if (!m_texture.loadFromFile("assets/images/niveles/nivel7/lebron.png")) {
        std::cerr << "❌ Error cargando lebron.png" << std::endl;
    }
    
    m_sprite = std::make_unique<sf::Sprite>(m_texture);
    m_sprite->setScale(m_escala);
    
    sf::FloatRect bounds = m_sprite->getLocalBounds();
    m_sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    
}

void LeBron::setPosition(float x, float y) {
    m_position = sf::Vector2f(x, y);
    m_sprite->setPosition(m_position);
}

void LeBron::setScale(float scaleX, float scaleY) {
    m_escala = sf::Vector2f(scaleX, scaleY);
    m_sprite->setScale(m_escala);
}

void LeBron::update(float dt) {
    // Sin animación
}

void LeBron::draw(sf::RenderWindow& window) {
    if (m_sprite) {
        window.draw(*m_sprite);
    }
}

sf::FloatRect LeBron::getBounds() const {
    if (m_sprite) {
        return m_sprite->getGlobalBounds();
    }
    return sf::FloatRect();
}

sf::Vector2f LeBron::getPosition() const {
    return m_position;
}