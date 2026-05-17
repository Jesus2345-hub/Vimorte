#include "Entidades/Balon.hpp"
#include <iostream>
#include <cmath>

Balon::Balon() 
    : m_position(0.f, 0.f),
      m_velocity(0.f, 0.f),
      m_friction(0.92f),
      m_escala(0.20f, 0.20f)
{
    if (!m_texture.loadFromFile("assets/images/niveles/nivel7/balon.png")) {
        std::cerr << "❌ Error cargando balon.png" << std::endl;
    }
    
    m_sprite = std::make_unique<sf::Sprite>(m_texture);
    m_sprite->setScale(m_escala);
    
    sf::FloatRect bounds = m_sprite->getLocalBounds();
    m_sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    
    std::cout << "⚽ Balón creado" << std::endl;
}

void Balon::setPosition(float x, float y) {
    m_position = sf::Vector2f(x, y);
    m_sprite->setPosition(m_position);
}

void Balon::empujar(const sf::Vector2f& direccion, float fuerza) {
    m_velocity += direccion * fuerza;
    
    // Limitar velocidad máxima (más alta para mejor control)
    float speed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
    if (speed > 350.f) {
        m_velocity = (m_velocity / speed) * 350.f;
    }
}

void Balon::update(float dt) {
    // Fricción más suave para que no se detenga tan rápido
    m_velocity *= m_friction;
    
    // Detener solo si es MUY lento
    if (std::abs(m_velocity.x) < 3.f) m_velocity.x = 0.f;
    if (std::abs(m_velocity.y) < 3.f) m_velocity.y = 0.f;
    
    // Mover
    m_position += m_velocity * dt;
    m_sprite->setPosition(m_position);
    
    // Rotación según velocidad
    float speed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
    if (speed > 5.f) {
        m_sprite->rotate(sf::degrees(m_velocity.x * dt * 0.3f));
    }
}

void Balon::draw(sf::RenderWindow& window) {
    if (m_sprite) {
        window.draw(*m_sprite);
    }
}

sf::FloatRect Balon::getBounds() const {
    if (m_sprite) {
        return m_sprite->getGlobalBounds();
    }
    return sf::FloatRect();
}

sf::Vector2f Balon::getPosition() const {
    return m_position;
}

float Balon::getRadius() const {
    if (m_sprite) {
        sf::FloatRect bounds = m_sprite->getGlobalBounds();
        return std::min(bounds.size.x, bounds.size.y) / 2.f;
    }
    return 20.f;
}