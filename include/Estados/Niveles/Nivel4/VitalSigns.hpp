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

    //Game over
    std::string m_patientName = "Paciente";
    bool m_justDied = false;


    sf::RectangleShape m_backgroundPanel;
    bool m_showBackground = true;
    float m_titleOffsetX = 0.f;
    // VALORES
    float m_heartRate;
    float m_bloodPressure;
    float m_oxygen;
    int m_opportunitiesLeft;
    bool m_gameOver;
    bool m_stabilized;

    // TEMPORIZADORES
    float m_fluctuationTimer;
    float m_fluctuationInterval;
    float m_messageTimer;
    std::string m_lastMessage;

    // FUENTE Y TEXTO
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_titleText;
    std::unique_ptr<sf::Text> m_heartText;
    std::unique_ptr<sf::Text> m_bpText;
    std::unique_ptr<sf::Text> m_oxygenText;
    std::unique_ptr<sf::Text> m_opportunitiesText;
    std::unique_ptr<sf::Text> m_messageText;

    // BARRAS
    sf::RectangleShape m_heartBar;
    sf::RectangleShape m_bpBar;
    sf::RectangleShape m_oxygenBar;

    // CONSTASTES PARA BARRAS VERTICALES
    const float m_barWidth = 25.f;
    const float m_barMaxHeight = 150.f;
    const float m_barSpacing = 35.f;


    float m_marginLeft = 20.f;
    float m_marginBottom = 100.f;

    float m_scaleFactor;
    void updateScale();
    bool m_anchorRight;

    sf::Vector2f m_position;      
    bool m_useAbsolutePosition;   

public:

    // Game over
    std::string getPatientName() const { return m_patientName; }
    void setPatientName(const std::string& name) { m_patientName = name; }
    bool justDied() const { return m_justDied; }
    void clearJustDied() { m_justDied = false; }

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