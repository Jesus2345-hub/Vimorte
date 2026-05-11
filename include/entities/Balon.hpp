#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

class Balon {
private:
    sf::Texture m_texture;
    std::unique_ptr<sf::Sprite> m_sprite;
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;
    float m_friction;
    sf::Vector2f m_escala;
    
public:
    Balon();
    void setPosition(float x, float y);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void empujar(const sf::Vector2f& direccion, float fuerza);
    sf::FloatRect getBounds() const;
    float getRadius() const;
    sf::Vector2f getPosition() const;
};