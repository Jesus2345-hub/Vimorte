#include "MinigameDarts_sara.hpp"
#include <iostream>
#include <algorithm>

MinigameDarts::MinigameDarts()
    : m_isActive(false), m_gameWon(false), m_score(0), m_scoreToWin(0),  
      m_throwsLeft(5), m_isDragging(false), m_showTrajectory(false),
      m_messageTimer(0.f), m_gen(m_rd())
{
    // Fondo oscuro
    m_background.setFillColor(sf::Color(0, 0, 0, 200));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(100, 100, 100));

    // Cargar fuente
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente en MinigameDarts" << std::endl;
        m_fontLoaded = false;
    } else {
        m_fontLoaded = true;
    }

    // Cargar textura
    if (!m_dartboardTexture.loadFromFile("assets/images/niveles/nivel_sara/dardos.png")) {
        std::cerr << "Error cargando textura de la diana" << std::endl;
        m_dartboardSprite = nullptr;   // o simplemente no lo uses
    } else {
        m_dartboardSprite = std::make_unique<sf::Sprite>(m_dartboardTexture);
    }

    initScoringRings();
    initUI();
}

void MinigameDarts::initScoringRings() {
    // Nuevos puntajes para hacer más desafiante la estabilización
    m_scoringRings = {
        {18.f, 20, sf::Color::Red},           // Bullseye interno (antes 50)
        {36.f, 12, sf::Color(255, 100, 0)},   // Bullseye externo (antes 25)
        {72.f, 6, sf::Color(0, 100, 200)},    // Anillo medio (antes 10)
        {108.f, 3, sf::Color(0, 200, 100)},   // Anillo exterior (antes 5)
        {150.f, 1, sf::Color(200, 200, 200)}  // Borde (igual)
    };
}

void MinigameDarts::initUI() {
    // Configurar título
    if (!m_fontLoaded) return;
    m_titleText = std::make_unique<sf::Text>(m_font);
    m_titleText->setString("DARTS CHALLENGE");
    m_titleText->setCharacterSize(28);
    m_titleText->setFillColor(sf::Color::Yellow);
    sf::FloatRect titleBounds = m_titleText->getLocalBounds();
    m_titleText->setOrigin(sf::Vector2f(titleBounds.size.x / 2.f, 0.f));

    // Puntuación
    m_scoreText = std::make_unique<sf::Text>(m_font);
    m_scoreText->setCharacterSize(24);
    m_scoreText->setFillColor(sf::Color::White);

    // Tiros restantes
    m_throwsText = std::make_unique<sf::Text>(m_font);
    m_throwsText->setCharacterSize(18);
    m_throwsText->setFillColor(sf::Color(200, 200, 200));

    // Instrucciones
    m_instructionText = std::make_unique<sf::Text>(m_font);
    m_instructionText->setString("Arrastra hacia atras y suelta para lanzar");
    m_instructionText->setCharacterSize(14);
    m_instructionText->setFillColor(sf::Color(200, 200, 200));

    // Texto cerrar
    m_closeText = std::make_unique<sf::Text>(m_font);
    m_closeText->setString("Presiona ESC para salir");
    m_closeText->setCharacterSize(14);
    m_closeText->setFillColor(sf::Color(150, 150, 150));

    // Mensaje temporal
    m_messageText = std::make_unique<sf::Text>(m_font);
    m_messageText->setCharacterSize(30);
    m_messageText->setStyle(sf::Text::Bold);
}

void MinigameDarts::setPosition(const sf::Vector2f& pos) {
    m_position = pos;
    m_background.setPosition(pos);
}

void MinigameDarts::setSize(const sf::Vector2f& size) {
    m_size = size;
    m_background.setSize(size);
    m_targetCenter = sf::Vector2f(
        m_position.x + m_size.x / 2.f,
        m_position.y + m_size.y / 2.f
    );

    if (m_dartboardSprite && m_dartboardTexture.getSize().x > 0) {
    float targetRadius = 150.f;
    sf::FloatRect texBounds = m_dartboardSprite->getLocalBounds();
    float scaleX = (targetRadius * 2.f) / texBounds.size.x;
    float scaleY = (targetRadius * 2.f) / texBounds.size.y;
    m_dartboardSprite->setScale(sf::Vector2f(scaleX, scaleY));
    m_dartboardSprite->setOrigin(sf::Vector2f(texBounds.size.x / 2.f, texBounds.size.y / 2.f));
    m_dartboardSprite->setPosition(m_targetCenter);
    }

    m_dartLine.setSize(sf::Vector2f(4.f, 100.f));
    m_dartLine.setOrigin(sf::Vector2f(2.f, 100.f));
    m_dartLine.setFillColor(sf::Color(210, 180, 140));
}

void MinigameDarts::activate() {
    m_isActive = true;
    reset();
}

void MinigameDarts::deactivate() {
    m_isActive = false;
    return;
}

void MinigameDarts::reset() {
    m_gameWon = false;
    m_score = 0;
    m_throwsLeft = 5;
    m_isDragging = false;
    m_showTrajectory = false;
    m_messageTimer = 0.f;
    if (m_scoreText) m_scoreText->setString("Puntuacion: 0");   
    if (m_throwsText) m_throwsText->setString("Tiros: " + std::to_string(m_throwsLeft));
    if (m_titleText) m_titleText->setString("DARTS CHALLENGE");
    if (m_messageText) m_messageText->setString("");
}

void MinigameDarts::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_isActive || m_gameWon) return;

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), window.getDefaultView());
    if (event.is<sf::Event::MouseButtonPressed>()) {
        const auto& mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            if (m_background.getGlobalBounds().contains(mousePos)) {
                m_isDragging = true;
                m_dragStart = mousePos;
                m_dragEnd = mousePos;
                m_showTrajectory = true;
            }
        }
    }
    else if (event.is<sf::Event::MouseMoved>()) {
        if (m_isDragging) {
            m_dragEnd = mousePos;
        }
    }
    else if (event.is<sf::Event::MouseButtonReleased>()) {
        const auto& mouseEvent = event.getIf<sf::Event::MouseButtonReleased>();
        if (mouseEvent->button == sf::Mouse::Button::Left && m_isDragging) {
            m_isDragging = false;
            m_showTrajectory = false;

            // Calcular fuerza de arrastre
            sf::Vector2f dragVector = m_dragStart - m_dragEnd;
            float power = std::min(std::sqrt(dragVector.x * dragVector.x + dragVector.y * dragVector.y), 200.f);
            if (power < 15.f) {
                showTemporaryMessage("Muy debil", sf::Color::Red, 0.8f);
                return;
            }

            // Comprobar si el lanzamiento es fallo (se suelta fuera del círculo)
            sf::Vector2f deltaRelease = m_dragEnd - m_targetCenter;
            float releaseDist = std::sqrt(deltaRelease.x * deltaRelease.x + deltaRelease.y * deltaRelease.y);
            const float dartboardRadius = 150.f;

            int points = 0;
            if (releaseDist > dartboardRadius) {
                // Fallo: fuera de la diana
                points = 0;
                if (m_vitalSigns) {
                    m_vitalSigns->applyEffect(-10);
                    showTemporaryMessage("Fallaste... -10", sf::Color::Red, 0.8f);
                }
            } else {
                // Acierto: calcular impacto con desviación
                sf::Vector2f direction = m_dragEnd - m_targetCenter;
                float lenDir = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                if (lenDir > 0.01f) direction /= lenDir;

                float accuracy = std::min(power / 200.f, 1.f);
                std::uniform_real_distribution<float> distAngle(-0.2f * (1.f - accuracy), 0.2f * (1.f - accuracy));
                float angleOffset = distAngle(m_gen);
                float newAngle = std::atan2(direction.y, direction.x) + angleOffset;
                sf::Vector2f newDir(std::cos(newAngle), std::sin(newAngle));
                float impactDistance = std::min(power * 0.8f, dartboardRadius);
                sf::Vector2f hitPoint = m_targetCenter + newDir * impactDistance;
                points = calculateScore(hitPoint);

                if (m_vitalSigns && points != 0) {
                    m_vitalSigns->applyEffect(points);
                }
            }

            // Acumular puntuación solo si acertó
            if (points > 0) {
                m_score += points;
            }

            // Reducir tiros (una sola vez)
            if (m_throwsLeft > 0) {
                m_throwsLeft--;
            }

            // Verificar fin del juego
            if (m_throwsLeft == 0 && !m_gameWon) {
            m_gameWon = true;
            if (m_titleText) m_titleText->setString("GAME OVER");
            if (m_vitalSigns && !m_vitalSigns->isStabilized()) {
                showTemporaryMessage("Sin mas tiros. No se pudo estabilizar.", sf::Color::Red, 2.f);
                }
            }

                        // Actualizar UI
                        if (m_scoreText) m_scoreText->setString("Puntuacion: " + std::to_string(m_score));
                        if (m_throwsText) m_throwsText->setString("Tiros: " + std::to_string(m_throwsLeft));
                    }
                }

}

int MinigameDarts::calculateScore(const sf::Vector2f& hitPoint) {
    sf::Vector2f delta = hitPoint - m_targetCenter;
    float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    int points = 0;
    const float maxRadius = 150.f;
    if (distance <= maxRadius) {
        for (const auto& ring : m_scoringRings) {
            if (distance <= ring.radius) {
                points = ring.score;
                break;
            }
        }
        showTemporaryMessage("+" + std::to_string(points), sf::Color::Green, 0.8f);
        // NO actualizar m_score aquí, lo haremos fuera
    } else {
        showTemporaryMessage("¡Fallaste!", sf::Color::Red, 0.8f);
        points = 0;
    }
    return points;
}

void MinigameDarts::showTemporaryMessage(const std::string& msg, const sf::Color& color, float duration) {
    m_currentMessage = msg;
    m_messageColor = color;
    m_messageTimer = duration;
    if (m_messageText) {
        m_messageText->setString(msg);
        m_messageText->setFillColor(color);
        sf::FloatRect bounds = m_messageText->getLocalBounds();
        m_messageText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_messageText->setPosition(sf::Vector2f(m_position.x + m_size.x / 2.f, m_position.y + 120.f));
    }
}

void MinigameDarts::updateMessage(float dt) {
    if (m_messageTimer > 0.f) {
        m_messageTimer -= dt;
        if (m_messageTimer <= 0.f && m_messageText) {
            m_messageText->setString("");
        }
    }
}

void MinigameDarts::update(float dt) {
    if (!m_isActive) return;
    updateMessage(dt);
}

void MinigameDarts::draw(sf::RenderWindow& window) {
    if (!m_isActive) return;

    // Fondo
    window.draw(m_background);

    // Título
    if (m_titleText) {
        m_titleText->setPosition(sf::Vector2f(m_position.x + m_size.x / 2.f, m_position.y + 35.f));
        window.draw(*m_titleText);
    }
    // Dibujar la diana con la imagen
    if (m_dartboardSprite) {
        window.draw(*m_dartboardSprite);
    }
    
    // Línea de tiro (si se está arrastrando)
    if (m_showTrajectory && m_isDragging) {
        sf::Vector2f dragVector = m_dragStart - m_dragEnd;
        float power = std::min(std::sqrt(dragVector.x * dragVector.x + dragVector.y * dragVector.y), 200.f);
        m_dartLine.setSize(sf::Vector2f(4.f, power + 40.f));
        m_dartLine.setPosition(m_dragEnd);
        float angle = std::atan2(dragVector.y, dragVector.x) * 180.f / 3.14159f;
        m_dartLine.setRotation(sf::degrees(angle + 90.f));
        window.draw(m_dartLine);
    }

    // Texto de puntuación y tiros
    if (m_scoreText) {
        m_scoreText->setPosition(sf::Vector2f(m_position.x + 30.f, m_position.y + 30.f));
        window.draw(*m_scoreText);
    }
    if (m_throwsText) {
        m_throwsText->setPosition(sf::Vector2f(m_position.x + m_size.x - 150.f, m_position.y + 30.f));
        window.draw(*m_throwsText);
    }
    if (m_instructionText) {
        sf::FloatRect instrBounds = m_instructionText->getLocalBounds();
        m_instructionText->setOrigin(sf::Vector2f(instrBounds.size.x / 2.f, 0.f));
        m_instructionText->setPosition(sf::Vector2f(m_position.x + m_size.x / 2.f, m_position.y + m_size.y - 40.f));
        window.draw(*m_instructionText);
    }
    if (m_closeText) {
        sf::FloatRect closeBounds = m_closeText->getLocalBounds();
        m_closeText->setOrigin(sf::Vector2f(closeBounds.size.x, 0.f));
        m_closeText->setPosition(sf::Vector2f(m_position.x + m_size.x - 20.f, m_position.y + m_size.y - 40.f));
        window.draw(*m_closeText);
    }
    if (m_messageText && !m_messageText->getString().isEmpty()) {
        window.draw(*m_messageText);
    }
}