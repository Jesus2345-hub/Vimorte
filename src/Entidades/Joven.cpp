#include "Entidades/Joven.hpp"
#include <iostream>

Joven::Joven() 
    : m_estado(Estado::DORMIDO),
      m_position(0.f, 0.f),
      m_escala(0.15f, 0.15f)
{
    // Cargar texturas
    if (!m_sleepTexture.loadFromFile("assets/images/niveles/nivel6/joven/sleep.png")) {
        std::cerr << "❌ Error cargando sleep.png del joven" << std::endl;
    }
    
    if (!m_awakeTexture.loadFromFile("assets/images/niveles/nivel6/joven/awake.png")) {
        std::cerr << "❌ Error cargando awake.png del joven" << std::endl;
    }
    
    // Crear sprite con textura dormido
    m_sprite = std::make_unique<sf::Sprite>(m_sleepTexture);
    m_sprite->setScale(m_escala);
    
    // Centrar origen
    sf::FloatRect bounds = m_sprite->getLocalBounds();
    m_sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    
}

void Joven::setPosition(float x, float y) {
    m_position = sf::Vector2f(x, y);
    m_sprite->setPosition(m_position);
}

void Joven::setScale(float scaleX, float scaleY) {
    m_escala = sf::Vector2f(scaleX, scaleY);
    m_sprite->setScale(m_escala);
}

void Joven::despertar() {
    if (m_estado == Estado::DESPIERTO) return;
    
    m_estado = Estado::DESPIERTO;
    m_sprite->setTexture(m_awakeTexture);
    m_sprite->setScale(m_escala);
}

void Joven::update(float dt) {
    // Por ahora no tiene animación
}

void Joven::draw(sf::RenderWindow& window) {
    if (m_sprite) {
        window.draw(*m_sprite);
    }
}

sf::FloatRect Joven::getBounds() const {
    if (m_sprite) {
        return m_sprite->getGlobalBounds();
    }
    return sf::FloatRect();
}

sf::Vector2f Joven::getPosition() const {
    return m_position;
}