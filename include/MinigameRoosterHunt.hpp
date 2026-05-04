#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <random>
#include <memory>

class MinigameRoosterHunt {
private:
    // Dimensiones
    sf::Vector2f m_size;
    sf::Vector2f m_position;
    sf::FloatRect m_bgBounds;
    
    // Fondo
    sf::RectangleShape m_background;
    sf::Texture m_bgTexture;
    std::unique_ptr<sf::Sprite> m_bgSprite;
    
    // Punto de mira (mira del rifle)
    sf::Texture m_crosshairTexture;
    std::unique_ptr<sf::Sprite> m_crosshair;

    // Gallo muerto
    sf::Texture m_deadTexture;

    // Indicadores de gallos
    sf::Texture m_aliveIconTexture;
    sf::Texture m_deadIconTexture;

    std::vector<std::unique_ptr<sf::Sprite>> m_scoreIcons;
    
    struct FlyingRooster {
        std::unique_ptr<sf::Sprite> sprite;
        sf::Vector2f velocity;
        float frameTime;
        int currentFrame;
        bool alive;
        bool dying;
        float speed;
    };
    std::vector<FlyingRooster> m_roosters;
    std::vector<sf::Texture> m_flyTextures;
    
    float m_roosterScale;
    float m_roosterSpeedMin;
    float m_roosterSpeedMax;

    // Estado
    bool m_isActive;
    bool m_gameWon;
    int m_score;
    int m_scoreToWin;
    int m_ammo;
    
    // Random
    std::random_device m_rd;
    std::mt19937 m_gen;
    float m_spawnTimer;
    float m_spawnInterval;
    
    // UI
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_titleText;
    std::unique_ptr<sf::Text> m_ammoText;
    std::unique_ptr<sf::Text> m_closeText;
    std::unique_ptr<sf::Text> m_messageText;
    float m_messageTimer;
    
public:
    MinigameRoosterHunt();
    
    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);
    void activate();
    void deactivate();
    bool isActive() const { return m_isActive; }
    bool isGameWon() const { return m_gameWon; }
    
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void reset();
    void initScoreIcons();
    
private:
    void spawnRooster();
    void updateRoosters(float dt);
    void shoot(const sf::Vector2f& mousePos);
    void showMessage(const std::string& msg, const sf::Color& color);
};