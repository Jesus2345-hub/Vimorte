#include "entities/Abuelita.hpp"
#include <iostream>

Abuelita::Abuelita() 
    : m_estado(Estado::NORMAL),
      m_position(0.f, 0.f),
      m_escala(0.15f, 0.15f)  // ← AJUSTA ESTE VALOR
{
    if (!m_normalTexture.loadFromFile("assets/images/niveles/nivel6/abuelita/normal.png")) {
        std::cerr << "❌ Error cargando normal.png de la abuelita" << std::endl;
    }
    
    if (!m_sonriendoTexture.loadFromFile("assets/images/niveles/nivel6/abuelita/sonriendo.png")) {
        std::cerr << "❌ Error cargando sonriendo.png de la abuelita" << std::endl;
    }
    
    m_sprite = std::make_unique<sf::Sprite>(m_normalTexture);
    m_sprite->setScale(m_escala);
    
    sf::FloatRect bounds = m_sprite->getLocalBounds();
    m_sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    
    std::cout << "👵 Abuelita creada (sin dientes)" << std::endl;
}

void Abuelita::setPosition(float x, float y) {
    m_position = sf::Vector2f(x, y);
    m_sprite->setPosition(m_position);
}

void Abuelita::setScale(float scaleX, float scaleY) {
    m_escala = sf::Vector2f(scaleX, scaleY);
    m_sprite->setScale(m_escala);
}

void Abuelita::sonreir() {
    if (m_estado == Estado::SONRIENDO) return;
    
    m_estado = Estado::SONRIENDO;
    
    // Crear un NUEVO sprite con la textura de sonriendo
    m_sprite = std::make_unique<sf::Sprite>(m_sonriendoTexture);
    m_sprite->setScale(m_escala);
    m_sprite->setPosition(m_position);
    
    // Centrar origen de nuevo
    sf::FloatRect bounds = m_sprite->getLocalBounds();
    m_sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    
}

void Abuelita::update(float dt) {
    // Sin animación por ahora
}

void Abuelita::draw(sf::RenderWindow& window) {
    if (m_sprite) {
        window.draw(*m_sprite);
    }
}

sf::FloatRect Abuelita::getBounds() const {
    if (m_sprite) {
        return m_sprite->getGlobalBounds();
    }
    return sf::FloatRect();
}

sf::Vector2f Abuelita::getPosition() const {
    return m_position;
}

void Abuelita::ponerNormal() {
    if (m_estado == Estado::NORMAL) return;
    
    m_estado = Estado::NORMAL;
    m_sprite = std::make_unique<sf::Sprite>(m_normalTexture);
    m_sprite->setScale(m_escala);
    m_sprite->setPosition(m_position);
    
    sf::FloatRect bounds = m_sprite->getLocalBounds();
    m_sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
}