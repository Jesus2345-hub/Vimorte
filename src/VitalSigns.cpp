#include "VitalSigns.hpp"
#include <random>
#include <algorithm>
#include <chrono>
#include <iostream>


// Constructor

VitalSigns::VitalSigns()
    : m_heartRate(30.f), m_bloodPressure(50.f), m_oxygen(70.f),
      m_opportunitiesLeft(999), m_gameOver(false), m_stabilized(false),
      m_fluctuationTimer(0.f), m_fluctuationInterval(2.f), m_messageTimer(0.f),
      m_marginLeft(20.f), m_marginBottom(100.f), m_anchorRight(false)
{
    initUI();
    m_showBackground = false;
}


// Inicialización de la interfaz (textos y barras)

void VitalSigns::initUI() {
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "ERROR: No se pudo cargar la fuente para VitalSigns" << std::endl;
        return;
    }

    // Configurar fondo del panel - MÁS VISIBLE
    m_backgroundPanel.setFillColor(sf::Color(15, 15, 35, 240));  // Azul oscuro casi opaco
    m_backgroundPanel.setOutlineThickness(3.f);
    m_backgroundPanel.setOutlineColor(sf::Color(255, 200, 80));  // Borde dorado

    // Escala inicial (se actualizará en draw)
    m_scaleFactor = 1.0f;

    // Crear textos con tamaños que luego se escalarán
    m_titleText = std::make_unique<sf::Text>(m_font, " SIGNOS VITALES\n    -ANDRES-", 15);
    m_titleText->setOutlineThickness(1.f);
    m_titleText->setOutlineColor(sf::Color::Black);
    
    m_heartText = std::make_unique<sf::Text>(m_font, "", 16);
    m_heartText->setOutlineThickness(1.f);
    m_heartText->setOutlineColor(sf::Color::Black);
    
    m_bpText = std::make_unique<sf::Text>(m_font, "", 16);
    m_bpText->setOutlineThickness(1.f);
    m_bpText->setOutlineColor(sf::Color::Black);
    
    m_oxygenText = std::make_unique<sf::Text>(m_font, "", 16);
    m_oxygenText->setOutlineThickness(1.f);
    m_oxygenText->setOutlineColor(sf::Color::Black);
    
    m_opportunitiesText = std::make_unique<sf::Text>(m_font, "", 16);
    m_messageText = std::make_unique<sf::Text>(m_font, "", 14);
    m_messageText->setFillColor(sf::Color::Yellow);
    m_messageText->setOutlineThickness(1.f);
    m_messageText->setOutlineColor(sf::Color::Black);

    m_heartBar.setFillColor(sf::Color::Red);
    m_heartBar.setOutlineThickness(2.f);
    m_heartBar.setOutlineColor(sf::Color::White);
    m_bpBar.setFillColor(sf::Color(255, 100, 0));
    m_bpBar.setOutlineThickness(2.f);
    m_bpBar.setOutlineColor(sf::Color::White);
    m_oxygenBar.setFillColor(sf::Color::Cyan);
    m_oxygenBar.setOutlineThickness(2.f);
    m_oxygenBar.setOutlineColor(sf::Color::White);
}


// Actualizar los strings de los textos (formato amigable)

void VitalSigns::updateTexts() {
    if (m_heartText) {
        m_heartText->setString("Corazon: " + std::to_string((int)m_heartRate) + " bpm");
        m_heartText->setFillColor(sf::Color::White);
        m_heartText->setOutlineThickness(1.f);
        m_heartText->setOutlineColor(sf::Color::Black);
    }
    if (m_bpText) {
        m_bpText->setString("Presion: " + std::to_string((int)m_bloodPressure) + " mmHg");
        m_bpText->setFillColor(sf::Color::White);
        m_bpText->setOutlineThickness(1.f);
        m_bpText->setOutlineColor(sf::Color::Black);
    }
    if (m_oxygenText) {
        m_oxygenText->setString("Oxigeno: " + std::to_string((int)m_oxygen) + " %");
        m_oxygenText->setFillColor(sf::Color::White);
        m_oxygenText->setOutlineThickness(1.f);
        m_oxygenText->setOutlineColor(sf::Color::Black);
    }
    if (m_opportunitiesText) m_opportunitiesText->setString("");
}


// Redimensionar las barras verticales según el valor actual

void VitalSigns::updateBars() {
    float heartHeight = (m_heartRate / 150.f) * m_barMaxHeight;
    float bpHeight = (m_bloodPressure / 200.f) * m_barMaxHeight;
    float oxygenHeight = (m_oxygen / 100.f) * m_barMaxHeight;
    m_heartBar.setSize(sf::Vector2f(m_barWidth, heartHeight));
    m_bpBar.setSize(sf::Vector2f(m_barWidth, bpHeight));
    m_oxygenBar.setSize(sf::Vector2f(m_barWidth, oxygenHeight));
}


// Comprobación de rangos normales

bool VitalSigns::isHeartNormal() const { return m_heartRate >= 60 && m_heartRate <= 100; }
bool VitalSigns::isBPNormal() const    { return m_bloodPressure >= 90 && m_bloodPressure <= 120; }
bool VitalSigns::isOxygenNormal() const { return m_oxygen >= 95 && m_oxygen <= 100; }


// Mostrar mensaje temporal en la interfaz

void VitalSigns::showMessage(const std::string& msg, sf::Color color) {
    m_lastMessage = msg;
    m_messageTimer = 3.0f;
    m_messageText->setString(msg);
    m_messageText->setFillColor(color);
}

void VitalSigns::updateMessage(float dt) {
    if (m_messageTimer > 0.f) {
        m_messageTimer -= dt;
        if (m_messageTimer <= 0.f) {
            m_lastMessage.clear();
            m_messageText->setString("");
        }
    }
}


// Actualización principal (fluctuación periódica)

void VitalSigns::update(float dt) {
    if (m_gameOver || m_stabilized) return;
    updateMessage(dt);
    
    // ACTUALIZAR TEXTOS CADA FRAME 
    updateTexts();
    updateBars();

    m_fluctuationTimer += dt;
    if (m_fluctuationTimer >= m_fluctuationInterval) {
        m_fluctuationTimer = 0.f;

        static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<float> dist(-2.f, 2.f);

        m_heartRate = std::clamp(m_heartRate + dist(rng), 0.f, 150.f);
        m_bloodPressure = std::clamp(m_bloodPressure + dist(rng), 0.f, 200.f);
        m_oxygen = std::clamp(m_oxygen + dist(rng), 0.f, 100.f);

        // Verificar muerte
        if (m_heartRate <= 0 || m_bloodPressure <= 0 || m_oxygen <= 0) {
            m_gameOver = true;
            showMessage(" GAME OVER - El paciente ha muerto", sf::Color::Red);
        }
    }
}

// Aplicar efecto de un dardo (puntos positivos o negativos)

void VitalSigns::applyEffect(int points) {
    if (m_gameOver || m_stabilized) return;
    // No se comprueba m_opportunitiesLeft
    m_heartRate = std::clamp(m_heartRate + points, 0.f, 150.f);
    m_bloodPressure = std::clamp(m_bloodPressure + points, 0.f, 200.f);
    m_oxygen = std::clamp(m_oxygen + points, 0.f, 100.f);

    updateTexts();
    updateBars();

    if (points > 0)
        showMessage(" + Efecto positivo +", sf::Color::Green);
    else if (points < 0)
        showMessage(" - Efecto negativo -", sf::Color::Red);

    // Verificar muerte
    if (m_heartRate <= 0 || m_bloodPressure <= 0 || m_oxygen <= 0) {
        m_gameOver = true;
        showMessage(" GAME OVER - El paciente ha muerto", sf::Color::Red);
    }
    // Estabilización automática si los tres están en rango
    if (isHeartNormal() && isBPNormal() && isOxygenNormal()) {
        stabilize();
    }
}


// Dibujar toda la interfaz (siempre visible)

void VitalSigns::draw(sf::RenderWindow& window) {
    sf::View defaultView = window.getDefaultView();
    window.setView(defaultView);

    sf::Vector2u winSize = window.getSize();
    float winW = static_cast<float>(winSize.x);
    float winH = static_cast<float>(winSize.y);
    
    float baseHeight = 720.f;
    m_scaleFactor = winH / baseHeight;
    m_scaleFactor = std::clamp(m_scaleFactor, 0.7f, 1.5f);

    if (m_titleText) {
        m_titleText->setCharacterSize(static_cast<unsigned int>(15 * m_scaleFactor));
    }
    if (m_heartText) {
        m_heartText->setCharacterSize(static_cast<unsigned int>(16 * m_scaleFactor));
    }
    if (m_bpText) {
        m_bpText->setCharacterSize(static_cast<unsigned int>(16 * m_scaleFactor));
    }
    if (m_oxygenText) {
        m_oxygenText->setCharacterSize(static_cast<unsigned int>(16 * m_scaleFactor));
    }
    if (m_messageText && m_messageTimer > 0.f) {
        m_messageText->setCharacterSize(static_cast<unsigned int>(14 * m_scaleFactor));
    }

    float scaledBarWidth = m_barWidth * m_scaleFactor;
    float scaledBarMaxHeight = m_barMaxHeight * m_scaleFactor;
    float scaledBarSpacing = m_barSpacing * m_scaleFactor;

    float bottomY = static_cast<float>(winSize.y) - (m_marginBottom);
    float baseY = bottomY - scaledBarMaxHeight;
    
    float barX;
    if (m_anchorRight) {
        float panelTotalWidth = scaledBarWidth * 3 + scaledBarSpacing * 2;
        float extraOffset = 45.f;
        barX = winW - panelTotalWidth - m_marginLeft - extraOffset;
    } else {
        barX = m_marginLeft;
    }

    barX = std::max(10.f, barX);
    
    float panelWidth = scaledBarWidth * 3 + scaledBarSpacing * 2 + 20.f;
    float startYText = baseY + scaledBarMaxHeight + (5.f * m_scaleFactor);
    float lineHeight = 30.f * m_scaleFactor;
    float panelHeight = (scaledBarMaxHeight + 10.f) + (lineHeight * 3) + 30.f;
    
    float panelX = barX - 10.f;
    float panelY = baseY - (25.f * m_scaleFactor) - 5.f;
    
    
    // FONDO DEL PANEL - BIEN VISIBLE
    
    m_backgroundPanel.setPosition(sf::Vector2f(panelX, panelY));
    m_backgroundPanel.setSize(sf::Vector2f(panelWidth, panelHeight));
    m_backgroundPanel.setFillColor(sf::Color(15, 15, 35, 230));  // CASI OPACO
    window.draw(m_backgroundPanel);
    
    // Borde dorado para que resalte
    sf::RectangleShape border(sf::Vector2f(panelWidth, panelHeight));
    border.setPosition(sf::Vector2f(panelX, panelY));
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(3.f);
    border.setOutlineColor(sf::Color(255, 200, 80, 255));
    window.draw(border);

    float heartHeight = (m_heartRate / 150.f) * scaledBarMaxHeight;
    float bpHeight = (m_bloodPressure / 200.f) * scaledBarMaxHeight;
    float oxygenHeight = (m_oxygen / 100.f) * scaledBarMaxHeight;
    
    m_heartBar.setSize(sf::Vector2f(scaledBarWidth, heartHeight));
    m_bpBar.setSize(sf::Vector2f(scaledBarWidth, bpHeight));
    m_oxygenBar.setSize(sf::Vector2f(scaledBarWidth, oxygenHeight));

    m_heartBar.setPosition(sf::Vector2f(barX, baseY + (scaledBarMaxHeight - heartHeight)));
    m_bpBar.setPosition(sf::Vector2f(barX + scaledBarWidth + scaledBarSpacing,
                                     baseY + (scaledBarMaxHeight - bpHeight)));
    m_oxygenBar.setPosition(sf::Vector2f(barX + 2 * (scaledBarWidth + scaledBarSpacing),
                                         baseY + (scaledBarMaxHeight - oxygenHeight)));

    float textX = barX;
    float startY = baseY + scaledBarMaxHeight + (5.f * m_scaleFactor);
    float lineHeightText = 30.f * m_scaleFactor;

    if (m_heartText) {
        m_heartText->setPosition(sf::Vector2f(textX, startY));
        m_heartText->setFillColor(sf::Color::White);
        m_heartText->setOutlineThickness(1.f);
        m_heartText->setOutlineColor(sf::Color::Black);
    }
    if (m_bpText) {
        m_bpText->setPosition(sf::Vector2f(textX, startY + lineHeightText));
        m_bpText->setFillColor(sf::Color::White);
        m_bpText->setOutlineThickness(1.f);
        m_bpText->setOutlineColor(sf::Color::Black);
    }
    if (m_oxygenText) {
        m_oxygenText->setPosition(sf::Vector2f(textX, startY + lineHeightText * 2));
        m_oxygenText->setFillColor(sf::Color::White);
        m_oxygenText->setOutlineThickness(1.f);
        m_oxygenText->setOutlineColor(sf::Color::Black);
    }
    if (m_opportunitiesText) {
        m_opportunitiesText->setPosition(sf::Vector2f(textX, startY + lineHeightText * 3 + 5.f));
    }
    if (m_messageText && m_messageTimer > 0.f) {
        m_messageText->setPosition(sf::Vector2f(textX, startY + lineHeightText * 4 + 10.f));
    }
    if (m_titleText) {
        m_titleText->setPosition(sf::Vector2f(barX + m_titleOffsetX, baseY - (25.f * m_scaleFactor)));
        m_titleText->setOutlineThickness(1.f);
        m_titleText->setOutlineColor(sf::Color::Black);
    }

    window.draw(*m_titleText);
    window.draw(*m_heartText);
    window.draw(*m_bpText);
    window.draw(*m_oxygenText);
    if (m_opportunitiesText) window.draw(*m_opportunitiesText);
    if (m_messageText && m_messageTimer > 0.f) window.draw(*m_messageText);
    window.draw(m_heartBar);
    window.draw(m_bpBar);
    window.draw(m_oxygenBar);
}

// Consultas de estado

bool VitalSigns::isGameOver() const { return m_gameOver; }
bool VitalSigns::isStabilized() const { return m_stabilized; }


// Reiniciar completamente el sistema

void VitalSigns::reset() {
    m_heartRate = 30.f;
    m_bloodPressure = 50.f;
    m_oxygen = 70.f;
    m_opportunitiesLeft = 999;
    m_gameOver = false;
    m_stabilized = false;
    m_fluctuationTimer = 0.f;
    m_messageTimer = 0.f;
    m_lastMessage.clear();
    updateTexts();
    updateBars();
    showMessage(" Juego reiniciado", sf::Color::Cyan);
}


void VitalSigns::stabilize() {
    if (!m_gameOver && !m_stabilized) {
        m_stabilized = true;
        showMessage(" PACIENTE ESTABILIZADO - Victoria", sf::Color::Green);
    }
}
void VitalSigns::setTitle(const std::string& title) {
    if (m_titleText) m_titleText->setString(title);
}