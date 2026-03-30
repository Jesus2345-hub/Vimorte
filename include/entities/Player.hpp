#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>

class Player {
private:
    sf::Texture m_texture;
    std::unique_ptr<sf::Sprite> m_sprite;  // Puntero para evitar constructor por defecto
    sf::Vector2f m_position;
    float m_speed;

public:
    Player();
    void loadTexture(const std::string& path);
    void setPosition(float x, float y);
    void setSpeed(float speed);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void move(sf::Vector2f direction, float dt);
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const;
};
