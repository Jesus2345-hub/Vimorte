#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <cmath>
#include <memory>

class MinigamePool {
private:
    // Dimensiones del minijuego
    sf::Vector2f m_size;
    sf::Vector2f m_position;
    
    // Fondo y UI
    sf::RectangleShape m_background;
    sf::RectangleShape m_table;
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_titleText;
    std::unique_ptr<sf::Text> m_instructionText;
    std::unique_ptr<sf::Text> m_scoreText;
    std::unique_ptr<sf::Text> m_closeText;
    
    // Bolas
    struct Ball {
        sf::CircleShape shape;
        sf::Vector2f velocity;
        bool pocketed;
        bool isCue;
    };
    
    std::vector<Ball> m_balls;
    Ball* m_cueBall;
    
    // Trincheras (hoyos)
    std::vector<sf::CircleShape> m_pockets;
    
    // Estado del juego
    bool m_isActive;
    bool m_isDragging;
    sf::Vector2f m_dragStart;
    sf::Vector2f m_dragEnd;
    int m_ballsPocketed;
    int m_ballsToWin;
    bool m_gameWon;
    
    // Física
    float m_friction;
    float m_minVelocity;
    
    // Taco visual
    sf::RectangleShape m_cueStick;
    bool m_showCue;
    
public:
    MinigamePool();
    
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
    
private:
    void initBalls();
    void initPockets();
    void applyPhysics(float dt);
    void checkPocketCollisions();
    void checkBallCollisions();
    void checkTableCollisions();
    sf::Vector2f calculateCuePower() const;
    void shootCueBall(const sf::Vector2f& power);
};