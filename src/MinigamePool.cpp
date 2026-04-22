#include "MinigamePool.hpp"
#include <iostream>
#include <algorithm>

MinigamePool::MinigamePool() 
    : m_isActive(false), m_isDragging(false), m_friction(0.98f), 
      m_minVelocity(10.f), m_ballsPocketed(0), m_ballsToWin(3), 
      m_gameWon(false), m_showCue(false), m_cueBall(nullptr)
{
    // Configurar fondo
    m_background.setFillColor(sf::Color(0, 0, 0, 200));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(100, 100, 100));
    
    // Mesa de pool
    m_table.setFillColor(sf::Color(30, 100, 30));
    m_table.setOutlineThickness(5.f);
    m_table.setOutlineColor(sf::Color(139, 69, 19));
    
    // Taco
    m_cueStick.setFillColor(sf::Color(210, 180, 140));
    m_cueStick.setSize(sf::Vector2f(5.f, 100.f));
    m_cueStick.setOrigin(sf::Vector2f(2.5f, 0.f));
}

void MinigamePool::setPosition(const sf::Vector2f& pos) {
    m_position = pos;
    m_background.setPosition(pos);
}

void MinigamePool::setSize(const sf::Vector2f& size) {
    m_size = size;
    m_background.setSize(size);
    
    // Mesa de pool tamaño (un poco más pequeña que el fondo)
    sf::Vector2f tableSize(size.x - 80.f, size.y - 100.f);
    m_table.setSize(tableSize);
    
    //Mesa de Pool / posicion
    m_table.setPosition(sf::Vector2f(
        m_position.x + 220.f,
        m_position.y + 100.f
    ));
    
    // Configurar textos
    if (m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        
        // TÍTULO - CENTRADO
        m_titleText = std::make_unique<sf::Text>(m_font);
        m_titleText->setString("MINIJUEGO DE POOL");
        m_titleText->setCharacterSize(24);
        m_titleText->setFillColor(sf::Color::White);
        // Centrar el origen del texto
        sf::FloatRect titleBounds = m_titleText->getLocalBounds();
        m_titleText->setOrigin(sf::Vector2f(titleBounds.size.x / 2.f, titleBounds.size.y / 2.f));
        
        // Posicionar del titulo
        m_titleText->setPosition(sf::Vector2f(
            m_position.x + size.x / 2.f,
            m_position.y + 65.f
        ));
        
        // TEXTO DE PUNTUACIÓN - ESQUINA SUPERIOR DERECHA
        m_scoreText = std::make_unique<sf::Text>(m_font);
        m_scoreText->setCharacterSize(20);
        m_scoreText->setFillColor(sf::Color::Yellow);
        
        //Posicion del score
        m_scoreText->setPosition(sf::Vector2f(
            m_position.x + size.x - 150.f,
            m_position.y + 65.f
        ));
        
        // TEXTO DE INSTRUCCIONES - ESQUINA INFERIOR IZQUIERDA
        m_instructionText = std::make_unique<sf::Text>(m_font);
        m_instructionText->setString("Click y arrastra para golpear la bola blanca");
        m_instructionText->setCharacterSize(14);
        m_instructionText->setFillColor(sf::Color(200, 200, 200));
        //Posicion de las instrcuciones
        m_instructionText->setPosition(sf::Vector2f(
            m_position.x + 260.f,
            m_position.y + size.y + 20.f 
        ));
        
        // TEXTO PARA CERRAR - ESQUINA INFERIOR DERECHA
        m_closeText = std::make_unique<sf::Text>(m_font);
        m_closeText->setString("Presiona ESC para cerrar");
        m_closeText->setCharacterSize(14);
        m_closeText->setFillColor(sf::Color(150, 150, 150));
        // Alinear a la derecha
        sf::FloatRect closeBounds = m_closeText->getLocalBounds();
        m_closeText->setOrigin(sf::Vector2f(closeBounds.size.x, 0.f));
        //Posicion del texto de cerrar
        m_closeText->setPosition(sf::Vector2f(
            m_position.x + size.x - 20.f,
            m_position.y + size.y + 20.f
        ));
    }
    
    initPockets();
    initBalls();
}
void MinigamePool::initPockets() {
    m_pockets.clear();
    sf::Vector2f tablePos = m_table.getPosition();
    sf::Vector2f tableSize = m_table.getSize();
    float pocketRadius = 35.f;  // AUMENTADO de 20 a 35 (más grandes)
    
    // 6 trincheras
    sf::Vector2f positions[] = {
        {tablePos.x, tablePos.y},
        {tablePos.x + tableSize.x, tablePos.y},
        {tablePos.x, tablePos.y + tableSize.y},
        {tablePos.x + tableSize.x, tablePos.y + tableSize.y},
        {tablePos.x + tableSize.x / 2.f, tablePos.y},
        {tablePos.x + tableSize.x / 2.f, tablePos.y + tableSize.y}
    };
    
    for (const auto& pos : positions) {
        sf::CircleShape pocket(pocketRadius);
        pocket.setPosition(pos - sf::Vector2f(pocketRadius, pocketRadius));
        pocket.setFillColor(sf::Color::Black);
        pocket.setOutlineThickness(3.f);
        pocket.setOutlineColor(sf::Color(80, 80, 80));
        m_pockets.push_back(pocket);
    }
}

void MinigamePool::initBalls() {
    m_balls.clear();
    sf::Vector2f tablePos = m_table.getPosition();
    sf::Vector2f tableSize = m_table.getSize();
    float ballRadius = 18.f;
    
    // Bola blanca
    Ball cueBall;
    cueBall.shape.setRadius(ballRadius);
    cueBall.shape.setFillColor(sf::Color::White);
    cueBall.shape.setOutlineThickness(2.f);
    cueBall.shape.setOutlineColor(sf::Color::Black);
    cueBall.shape.setOrigin(sf::Vector2f(ballRadius, ballRadius));
    cueBall.shape.setPosition(sf::Vector2f(tablePos.x + tableSize.x * 0.25f, 
                                            tablePos.y + tableSize.y * 0.5f));
    cueBall.velocity = sf::Vector2f(0.f, 0.f);
    cueBall.pocketed = false;
    cueBall.isCue = true;
    m_balls.push_back(cueBall);
    m_cueBall = &m_balls.back();
    
    // 3 bolas objetivo (más separadas para facilitar)
    sf::Color targetColors[] = {sf::Color::Red, sf::Color(255, 100, 0), sf::Color(200, 0, 200)};
    sf::Vector2f positions[] = {
        {tablePos.x + tableSize.x * 0.5f, tablePos.y + tableSize.y * 0.35f},
        {tablePos.x + tableSize.x * 0.6f, tablePos.y + tableSize.y * 0.5f},
        {tablePos.x + tableSize.x * 0.5f, tablePos.y + tableSize.y * 0.65f}
    };
    
    for (int i = 0; i < 3; ++i) {
        Ball ball;
        ball.shape.setRadius(ballRadius);
        ball.shape.setFillColor(targetColors[i]);
        ball.shape.setOutlineThickness(2.f);
        ball.shape.setOutlineColor(sf::Color::Black);
        ball.shape.setOrigin(sf::Vector2f(ballRadius, ballRadius));
        ball.shape.setPosition(positions[i]);
        ball.velocity = sf::Vector2f(0.f, 0.f);
        ball.pocketed = false;
        ball.isCue = false;
        m_balls.push_back(ball);
    }
}

void MinigamePool::activate() {
    m_isActive = true;
    m_gameWon = false;
    reset();
}

void MinigamePool::deactivate() {
    m_isActive = false;
}

void MinigamePool::reset() {
    initBalls();
    m_ballsPocketed = 0;
    m_gameWon = false;
    m_showCue = false;
    m_isDragging = false;
    if (m_titleText) {
        m_titleText->setString("MINIJUEGO DE POOL");
        m_titleText->setFillColor(sf::Color::White);
    }
}

void MinigamePool::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_isActive || m_gameWon) return;
    
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    
    if (event.is<sf::Event::MouseButtonPressed>()) {
        const auto& mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            sf::Vector2f cuePos = m_cueBall->shape.getPosition();
            float distance = std::sqrt(std::pow(mousePos.x - cuePos.x, 2) + 
                                       std::pow(mousePos.y - cuePos.y, 2));
            
            if (distance < 30.f && !m_cueBall->pocketed) {
                m_isDragging = true;
                m_dragStart = mousePos;
                m_dragEnd = mousePos;
                m_showCue = true;
            }
        }
    }
    else if (event.is<sf::Event::MouseMoved>()) {
        if (m_isDragging) {
            const auto& mouseEvent = event.getIf<sf::Event::MouseMoved>();
            m_dragEnd = sf::Vector2f(static_cast<float>(mouseEvent->position.x), 
                                      static_cast<float>(mouseEvent->position.y));
        }
    }
    else if (event.is<sf::Event::MouseButtonReleased>()) {
        const auto& mouseEvent = event.getIf<sf::Event::MouseButtonReleased>();
        if (mouseEvent->button == sf::Mouse::Button::Left && m_isDragging) {
            m_isDragging = false;
            m_showCue = false;
            
            sf::Vector2f power = calculateCuePower();
            if (std::abs(power.x) > 5.f || std::abs(power.y) > 5.f) {
                shootCueBall(power);
            }
        }
    }
}

void MinigamePool::update(float dt) {
    if (!m_isActive || m_gameWon) return;
    
    applyPhysics(dt);
    checkPocketCollisions();
    checkBallCollisions();
    checkTableCollisions();
    
    // Actualizar texto
    if (m_scoreText) {
        m_scoreText->setString("Bolas: " + std::to_string(m_ballsPocketed) + "/" + std::to_string(m_ballsToWin));
    }
    
    // Verificar victoria
    if (m_ballsPocketed >= m_ballsToWin) {
        m_gameWon = true;
        if (m_titleText) {
            m_titleText->setString("¡VICTORIA! Presiona ESC");
            m_titleText->setFillColor(sf::Color::Green);
        }
    }
    
    // Verificar si bola blanca entronada
    if (m_cueBall->pocketed && !m_gameWon) {
        if (m_titleText) {
            m_titleText->setString("¡FALLO! Presiona ESC para reintentar");
            m_titleText->setFillColor(sf::Color::Red);
        }
    }
}

void MinigamePool::draw(sf::RenderWindow& window) {
    if (!m_isActive) return;
    
    window.draw(m_background);
    window.draw(m_table);
    
    for (const auto& pocket : m_pockets) {
        window.draw(pocket);
    }
    
    for (const auto& ball : m_balls) {
        if (!ball.pocketed) {
            window.draw(ball.shape);
        }
    }
    
    if (m_showCue && !m_cueBall->pocketed) {
        sf::Vector2f cuePos = m_cueBall->shape.getPosition();
        sf::Vector2f direction = m_dragStart - m_dragEnd;
        float power = std::min(std::sqrt(direction.x * direction.x + direction.y * direction.y), 200.f);
        
        m_cueStick.setSize(sf::Vector2f(5.f, power + 50.f));
        m_cueStick.setOrigin(sf::Vector2f(2.5f, 0.f));
        m_cueStick.setPosition(cuePos + direction * 0.1f);
        
        float angle = std::atan2(direction.y, direction.x) * 180.f / 3.14159f;
        m_cueStick.setRotation(sf::degrees(angle + 90.f));
        
        window.draw(m_cueStick);
    }
    
    if (m_titleText) window.draw(*m_titleText);
    if (m_instructionText) window.draw(*m_instructionText);
    if (m_scoreText) window.draw(*m_scoreText);
    if (m_closeText) window.draw(*m_closeText);
}

sf::Vector2f MinigamePool::calculateCuePower() const {
    return (m_dragStart - m_dragEnd) * 8.f;  // AUMENTADO de 5 a 8
}

void MinigamePool::shootCueBall(const sf::Vector2f& power) {
    if (!m_cueBall->pocketed) {
        m_cueBall->velocity = power;
    }
}

void MinigamePool::applyPhysics(float dt) {
    for (auto& ball : m_balls) {
        if (!ball.pocketed) {
            ball.velocity *= 0.99f;
            
            if (std::abs(ball.velocity.x) < 5.f) ball.velocity.x = 0;  // Antes 10
            if (std::abs(ball.velocity.y) < 5.f) ball.velocity.y = 0;
            
            sf::Vector2f pos = ball.shape.getPosition();
            pos += ball.velocity * dt;
            ball.shape.setPosition(pos);
        }
    }
}

void MinigamePool::checkPocketCollisions() {
    for (auto& ball : m_balls) {
        if (ball.pocketed) continue;
        
        sf::Vector2f ballPos = ball.shape.getPosition();
        float ballRadius = ball.shape.getRadius();
        
        for (const auto& pocket : m_pockets) {
            sf::Vector2f pocketPos = pocket.getPosition() + sf::Vector2f(pocket.getRadius(), pocket.getRadius());
            float distance = std::sqrt(std::pow(ballPos.x - pocketPos.x, 2) + 
                                       std::pow(ballPos.y - pocketPos.y, 2));
            
            if (distance < pocket.getRadius()) {
                ball.pocketed = true;
                ball.velocity = sf::Vector2f(0.f, 0.f);
                
                if (!ball.isCue) {
                    m_ballsPocketed++;
                }
                break;
            }
        }
    }
}

void MinigamePool::checkBallCollisions() {
    for (size_t i = 0; i < m_balls.size(); ++i) {
        if (m_balls[i].pocketed) continue;
        
        for (size_t j = i + 1; j < m_balls.size(); ++j) {
            if (m_balls[j].pocketed) continue;
            
            sf::Vector2f pos1 = m_balls[i].shape.getPosition();
            sf::Vector2f pos2 = m_balls[j].shape.getPosition();
            sf::Vector2f delta = pos2 - pos1;
            float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            float minDistance = m_balls[i].shape.getRadius() + m_balls[j].shape.getRadius();
            
            if (distance < minDistance) {
                sf::Vector2f normal = delta / distance;
                float overlap = minDistance - distance;
                pos1 -= normal * overlap * 0.5f;
                pos2 += normal * overlap * 0.5f;
                m_balls[i].shape.setPosition(pos1);
                m_balls[j].shape.setPosition(pos2);
                
                sf::Vector2f temp = m_balls[i].velocity;
                m_balls[i].velocity = m_balls[j].velocity * 0.9f;
                m_balls[j].velocity = temp * 0.9f;
            }
        }
    }
}

void MinigamePool::checkTableCollisions() {
    sf::Vector2f tablePos = m_table.getPosition();
    sf::Vector2f tableSize = m_table.getSize();
    float padding = 5.f;
    
    for (auto& ball : m_balls) {
        if (ball.pocketed) continue;
        
        sf::Vector2f pos = ball.shape.getPosition();
        float radius = ball.shape.getRadius();
        
        if (pos.x - radius < tablePos.x + padding) {
            pos.x = tablePos.x + padding + radius;
            ball.velocity.x = -ball.velocity.x * 0.7f;
        }
        if (pos.x + radius > tablePos.x + tableSize.x - padding) {
            pos.x = tablePos.x + tableSize.x - padding - radius;
            ball.velocity.x = -ball.velocity.x * 0.7f;
        }
        if (pos.y - radius < tablePos.y + padding) {
            pos.y = tablePos.y + padding + radius;
            ball.velocity.y = -ball.velocity.y * 0.7f;
        }
        if (pos.y + radius > tablePos.y + tableSize.y - padding) {
            pos.y = tablePos.y + tableSize.y - padding - radius;
            ball.velocity.y = -ball.velocity.y * 0.7f;
        }
        
        ball.shape.setPosition(pos);
    }
}