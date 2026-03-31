#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include "../Animation.hpp"

class Player {
private:
    sf::Texture m_idleTexture;
    std::unique_ptr<sf::Sprite> m_sprite;  // ← Cambiar a puntero
    sf::Vector2f m_position;
    float m_speed;
    
    // Animaciones
    Animation m_walkDown;
    Animation m_walkUp;
    Animation m_walkRight;
    Animation m_walkUpRight;
    Animation m_walkDownRight;
    
    Animation* m_currentAnimation;
    sf::Vector2f m_lastDirection;
    
public:
    Player();
    void loadAssets();
    void setPosition(float x, float y);
    void setSpeed(float speed);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void move(sf::Vector2f direction, float dt);
    sf::FloatRect getBounds() const; //Ojito que le cambie el nombre a esta función, antes era getHitbox() pero ahora devuelve el bounds del sprite completo
    sf::Vector2f getPosition() const;
    
private:
    void updateAnimation(sf::Vector2f direction);
};
