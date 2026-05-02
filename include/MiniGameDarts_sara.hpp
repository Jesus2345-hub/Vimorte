#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include "VitalSigns_sara.hpp"
#include <cmath>
#include <memory>
#include <random>

class MinigameDarts {
public:
    MinigameDarts();
    
    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);
    void activate();
    void deactivate();
    bool isActive() const { return m_isActive; }
    bool isWon() const { return m_gameWon; }

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void reset();
    void setVitalSigns(VitalSigns* signs) { m_vitalSigns = signs; }

private:

    float m_noiseOffsetX;
    float m_noiseOffsetY;
    // Radio actual en píxeles
    float m_currentDartboardRadius;

    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_backgroundForest;
    VitalSigns* m_vitalSigns = nullptr;
    sf::Texture m_dartboardTexture;
    std::unique_ptr<sf::Sprite> m_dartboardSprite;

    // Estados del juego
    bool m_fontLoaded;
    bool m_isActive;
    bool m_gameWon;
    int m_score;
    int m_throwsLeft;
    bool m_hasThrown;               

    // Geometría
    sf::Vector2f m_position;
    sf::Vector2f m_size;

    // UI
    sf::RectangleShape m_background;
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_titleText;
    std::unique_ptr<sf::Text> m_scoreText;
    std::unique_ptr<sf::Text> m_instructionText;
    std::unique_ptr<sf::Text> m_closeText;
    std::unique_ptr<sf::Text> m_throwsText;
    std::unique_ptr<sf::Text> m_messageText;

    // Animación de mensaje
    float m_messageTimer;
    std::string m_currentMessage;
    sf::Color m_messageColor;

    // Centro de la diana
    sf::Vector2f m_targetCenter;

    // Puntuaciones por anillo
    struct RingScore {
    float normalizedRadius; 
    int score;
    sf::Color color;
};
    std::vector<RingScore> m_scoringRings;

    // -----  sistema de puntería móvil -----
    sf::CircleShape m_aimingCircle;   // círculo que se mueve
    float m_aimAngle;                 // ángulo para el movimiento (radianes)
    float m_aimSpeed;                 // radianes por segundo
    float m_aimAmplitude;             // amplitud del movimiento (píxeles)
    // -------------------------------------------

    // Aleatoriedad para desviación al lanzar
    std::random_device m_rd;
    std::mt19937 m_gen;
    std::uniform_real_distribution<float> m_deviationDist;

    // Métodos auxiliares
    int calculateScore(const sf::Vector2f& hitPoint);
    void showTemporaryMessage(const std::string& msg, const sf::Color& color, float duration);
    void updateMessage(float dt);
    void initUI();
    void initScoringRings();
};