#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include "../Animation.hpp"
#include "../Inventory.hpp"

class Player {
private:
    sf::Texture m_idleTexture;
    std::unique_ptr<sf::Sprite> m_sprite;
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

    // Inventario
    std::unique_ptr<Inventory> m_inventory;
    
public:
    Player();
    void loadAssets();
    void setPosition(float x, float y);
    void setSpeed(float speed);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void move(sf::Vector2f direction, float dt);
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const;

    
    sf::FloatRect getHurtbox() const;
    void drawHurtbox(sf::RenderWindow& window) const;
    
    // Acceso al inventario
    Inventory* getInventory() { return m_inventory.get(); }
    void clearInventory() { if (m_inventory) m_inventory->clear(); }

private:
    void updateAnimation(sf::Vector2f direction);
};