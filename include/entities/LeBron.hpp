#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

class LeBron {
private:
    sf::Texture m_texture;
    std::unique_ptr<sf::Sprite> m_sprite;
    sf::Vector2f m_position;
    sf::Vector2f m_escala;
    
public:
    LeBron();
    void setPosition(float x, float y);
    void setScale(float scaleX, float scaleY);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const;
};