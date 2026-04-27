#include "VitalSigns_sara.hpp"
#include <random>
#include <algorithm>
#include <chrono>
#include <iostream>

// ============================================================
// Constructor
// ============================================================
VitalSigns::VitalSigns()
    : m_heartRate(30.f), m_bloodPressure(50.f), m_oxygen(70.f),
      m_opportunitiesLeft(999), m_gameOver(false), m_stabilized(false),
      m_fluctuationTimer(0.f), m_fluctuationInterval(2.f), m_messageTimer(0.f),
      m_marginLeft(20.f), m_marginBottom(100.f)
{
    initUI();
}

// ============================================================
// Inicialización de la interfaz (textos y barras)
// ============================================================
void VitalSigns::initUI() {
    // Cargar fuente (asegúrate de que la ruta sea correcta)
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "ERROR: No se pudo cargar la fuente para VitalSigns" << std::endl;
        return;
    }

    // Título principal
    m_titleText = std::make_unique<sf::Text>(m_font, "SIGNOS VITALES - ANDRES", 15);
    m_titleText->setFillColor(sf::Color::White);

    // Textos de cada signo (se actualizarán más adelante)
    m_heartText = std::make_unique<sf::Text>(m_font, "", 16);
    m_bpText = std::make_unique<sf::Text>(m_font, "", 16);
    m_oxygenText = std::make_unique<sf::Text>(m_font, "", 16);
    m_opportunitiesText = std::make_unique<sf::Text>(m_font, "", 16);
    m_messageText = std::make_unique<sf::Text>(m_font, "", 14);
    m_messageText->setFillColor(sf::Color::Yellow);

    // Configuración visual de las barras verticales
    m_heartBar.setFillColor(sf::Color::Red);
    m_heartBar.setOutlineThickness(1.f);
    m_heartBar.setOutlineColor(sf::Color::White);
    m_bpBar.setFillColor(sf::Color(255, 100, 0));
    m_bpBar.setOutlineThickness(1.f);
    m_bpBar.setOutlineColor(sf::Color::White);
    m_oxygenBar.setFillColor(sf::Color::Cyan);
    m_oxygenBar.setOutlineThickness(1.f);
    m_oxygenBar.setOutlineColor(sf::Color::White);

    // Inicializar textos y barras con los valores actuales
    updateTexts();
    updateBars();
}

// ============================================================
// Actualizar los strings de los textos (formato amigable)
// ============================================================
void VitalSigns::updateTexts() {
    if (m_heartText) m_heartText->setString("Corazon: " + std::to_string((int)m_heartRate) + " bpm");
    if (m_bpText) m_bpText->setString("Presion: " + std::to_string((int)m_bloodPressure) + " mmHg");
    if (m_oxygenText) m_oxygenText->setString("Oxigeno: " + std::to_string((int)m_oxygen) + " %");
    // Ocultamos el texto de oportunidades (ya no es relevante)
    if (m_opportunitiesText) m_opportunitiesText->setString("");
}

// ============================================================
// Redimensionar las barras verticales según el valor actual
// ============================================================
void VitalSigns::updateBars() {
    float heartHeight = (m_heartRate / 150.f) * m_barMaxHeight;
    float bpHeight = (m_bloodPressure / 200.f) * m_barMaxHeight;
    float oxygenHeight = (m_oxygen / 100.f) * m_barMaxHeight;
    m_heartBar.setSize(sf::Vector2f(m_barWidth, heartHeight));
    m_bpBar.setSize(sf::Vector2f(m_barWidth, bpHeight));
    m_oxygenBar.setSize(sf::Vector2f(m_barWidth, oxygenHeight));
}

// ============================================================
// Comprobación de rangos normales
// ============================================================
bool VitalSigns::isHeartNormal() const { return m_heartRate >= 60 && m_heartRate <= 100; }
bool VitalSigns::isBPNormal() const    { return m_bloodPressure >= 90 && m_bloodPressure <= 120; }
bool VitalSigns::isOxygenNormal() const { return m_oxygen >= 95 && m_oxygen <= 100; }

// ============================================================
// Mostrar mensaje temporal en la interfaz
// ============================================================
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

// ============================================================
// Actualización principal (fluctuación periódica)
// ============================================================
void VitalSigns::update(float dt) {
    if (m_gameOver || m_stabilized) return;
    updateMessage(dt);

    m_fluctuationTimer += dt;
    if (m_fluctuationTimer >= m_fluctuationInterval) {
        m_fluctuationTimer = 0.f;

        static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<float> dist(-2.f, 2.f);

        m_heartRate = std::clamp(m_heartRate + dist(rng), 0.f, 150.f);
        m_bloodPressure = std::clamp(m_bloodPressure + dist(rng), 0.f, 200.f);
        m_oxygen = std::clamp(m_oxygen + dist(rng), 0.f, 100.f);

        updateTexts();
        updateBars();
        // Verificar muerte o estabilización
        if (m_heartRate <= 0 || m_bloodPressure <= 0 || m_oxygen <= 0) {
            m_gameOver = true;
            showMessage(" GAME OVER - El paciente ha muerto", sf::Color::Red);
        }
        
    }
}

// ============================================================
// Aplicar efecto de un dardo (puntos positivos o negativos)
// ============================================================
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

// ============================================================
// Dibujar toda la interfaz (siempre visible, esquina inferior izquierda)
// ============================================================
void VitalSigns::draw(sf::RenderWindow& window) {
    // Fijar vista por defecto
    sf::View defaultView = window.getDefaultView();
    window.setView(defaultView);

    sf::Vector2u winSize = window.getSize();
    float bottomY = static_cast<float>(winSize.y) - m_marginBottom;
    float baseY = bottomY - m_barMaxHeight;
    float barX = m_marginLeft;

    // Posicionar las barras
    m_heartBar.setPosition(sf::Vector2f(barX, baseY + (m_barMaxHeight - m_heartBar.getSize().y)));
    m_bpBar.setPosition(sf::Vector2f(barX + m_barWidth + m_barSpacing,
                                     baseY + (m_barMaxHeight - m_bpBar.getSize().y)));
    m_oxygenBar.setPosition(sf::Vector2f(barX + 2*(m_barWidth + m_barSpacing),
                                         baseY + (m_barMaxHeight - m_oxygenBar.getSize().y)));

    // ===== TEXTOS EN VERTICAL (UNO DEBAJO DEL OTRO) =====
    float textX = barX;  // misma X para todos
    float startY = baseY + m_barMaxHeight + 5.f;  // posición Y inicial
    float lineHeight = 30.f;  // espacio entre líneas

    if (m_heartText) 
        m_heartText->setPosition(sf::Vector2f(textX, startY));
    if (m_bpText) 
        m_bpText->setPosition(sf::Vector2f(textX, startY + lineHeight));
    if (m_oxygenText) 
        m_oxygenText->setPosition(sf::Vector2f(textX, startY + lineHeight * 2));

    // Oportunidades justo debajo
    if (m_opportunitiesText) 
        m_opportunitiesText->setPosition(sf::Vector2f(textX, startY + lineHeight * 3 + 5.f));

    // Mensaje temporal
    if (m_messageText && m_messageTimer > 0.f)
        m_messageText->setPosition(sf::Vector2f(textX, startY + lineHeight * 4 + 10.f));

    // Título arriba de las barras
    if (m_titleText) 
        m_titleText->setPosition(sf::Vector2f(barX, baseY - 25.f));

    // Dibujar todo
    if (m_titleText) window.draw(*m_titleText);
    if (m_heartText) window.draw(*m_heartText);
    if (m_bpText) window.draw(*m_bpText);
    if (m_oxygenText) window.draw(*m_oxygenText);
    if (m_opportunitiesText) window.draw(*m_opportunitiesText);
    if (m_messageText && m_messageTimer > 0.f) window.draw(*m_messageText);
    window.draw(m_heartBar);
    window.draw(m_bpBar);
    window.draw(m_oxygenBar);
}
// ============================================================
// Consultas de estado
// ============================================================
bool VitalSigns::isGameOver() const { return m_gameOver; }
bool VitalSigns::isStabilized() const { return m_stabilized; }

// ============================================================
// Reiniciar completamente el sistema
// ============================================================
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