#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

class VitalSigns {
public:
    VitalSigns();

    void update(float dt);
    void draw(sf::RenderWindow& window);
    void applyEffect(int points);
    void reset();

    bool isGameOver() const;
    bool isStabilized() const;
    void setPosition(const sf::Vector2f& pos) { 
        m_position = pos; 
        m_useAbsolutePosition = true;
    }
    void useRelativePosition(bool use) { m_useAbsolutePosition = !use; }
private:
    sf::RectangleShape m_backgroundPanel;
    bool m_showBackground = true;
    float m_titleOffsetX = 0.f;
    // Valores
    float m_heartRate;
    float m_bloodPressure;
    float m_oxygen;
    int m_opportunitiesLeft;
    bool m_gameOver;
    bool m_stabilized;

    // Temporizadores
    float m_fluctuationTimer;
    float m_fluctuationInterval;
    float m_messageTimer;
    std::string m_lastMessage;

    // Fuente y textos
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_titleText;
    std::unique_ptr<sf::Text> m_heartText;
    std::unique_ptr<sf::Text> m_bpText;
    std::unique_ptr<sf::Text> m_oxygenText;
    std::unique_ptr<sf::Text> m_opportunitiesText;
    std::unique_ptr<sf::Text> m_messageText;

    // Barras (solo una declaración)
    sf::RectangleShape m_heartBar;
    sf::RectangleShape m_bpBar;
    sf::RectangleShape m_oxygenBar;

    // Constantes para barras verticales
    const float m_barWidth = 25.f;
    const float m_barMaxHeight = 150.f;
    const float m_barSpacing = 35.f;


    float m_marginLeft = 20.f;
    float m_marginBottom = 100.f;

    float m_scaleFactor;
    void updateScale();
    bool m_anchorRight;

sf::Vector2f m_position;      // Posición absoluta
    bool m_useAbsolutePosition;   // Si usa posición absoluta o márgenes

public:

    void setLeftMargin(float margin) { m_marginLeft = margin; }
    void setBottomMargin(float margin) { m_marginBottom = margin; }
    void setTitle(const std::string& title);
    void stabilize();
    void initUI();
    void updateBars();
    void updateTexts();
    bool isHeartNormal() const;
    bool isBPNormal() const;
    bool isOxygenNormal() const;
    void showMessage(const std::string& msg, sf::Color color);
    void updateMessage(float dt);
    void setAnchorRight(bool anchorRight) { m_anchorRight = anchorRight; }
    void setTitleOffsetX(float offset) { m_titleOffsetX = offset; }
};