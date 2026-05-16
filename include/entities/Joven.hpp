#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

class Joven {
public:
    enum class Estado {
        DORMIDO,
        DESPIERTO
    };
    
private:



    sf::Texture m_sleepTexture;
    sf::Texture m_awakeTexture;
    std::unique_ptr<sf::Sprite> m_sprite;
    Estado m_estado;
    sf::Vector2f m_position;
    sf::Vector2f m_escala;
    
public:
    Joven();
    void setPosition(float x, float y);
    void setScale(float scaleX, float scaleY);
    void despertar();
    bool estaDormido() const { return m_estado == Estado::DORMIDO; }
    void update(float dt);
    void draw(sf::RenderWindow& window);
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const;
};
