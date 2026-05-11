#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

class Abuelita {
public:
    enum class Estado {
        NORMAL,
        SONRIENDO
    };
    
private:
    sf::Texture m_normalTexture;
    sf::Texture m_sonriendoTexture;
    std::unique_ptr<sf::Sprite> m_sprite;
    Estado m_estado;
    sf::Vector2f m_position;
    sf::Vector2f m_escala;
    
public:
    Abuelita();
    void setPosition(float x, float y);
    void setScale(float scaleX, float scaleY);
    void sonreir();
    bool estaNormal() const { return m_estado == Estado::NORMAL; }
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void ponerNormal();
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const;
};