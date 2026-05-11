#include "MinigameBaloncesto.hpp"
#include <iostream>
#include <cmath>

//saber si volvi
MinigameBaloncesto::MinigameBaloncesto()
    : m_isActive(false), m_gameWon(false), m_score(0), m_scoreToWin(5),
      m_attempts(0), m_maxAttempts(7), m_fase(Fase::INTENSIDAD),
      m_pelotaVolando(false), m_mostrandoEnceste(false), m_tiempoEnceste(0.f),
      m_faseEnceste(0),
      m_indicadorVel(400.f), m_indicadorPos(0.f), m_indicadorSubiendo(true),
      m_intensidadFijada(false), m_intensidad(0.f),
      m_crosshairSpeed(150.f), m_messageTimer(0.f)
{
    m_background.setFillColor(sf::Color(0, 0, 0, 200));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(100, 100, 100));
    
    // Barra de intensidad
    m_barraFondo.setSize(sf::Vector2f(40.f, 300.f));
    m_barraFondo.setFillColor(sf::Color(50, 50, 50));
    m_barraFondo.setOutlineThickness(2.f);
    m_barraFondo.setOutlineColor(sf::Color::White);
    
    m_barraVerde.setSize(sf::Vector2f(40.f, 60.f));
    m_barraVerde.setFillColor(sf::Color::Green);
    
    m_barraIndicador.setSize(sf::Vector2f(50.f, 6.f));
    m_barraIndicador.setFillColor(sf::Color::Yellow);
    
    // Crosshair
    m_crosshair.setRadius(20.f);
    m_crosshair.setFillColor(sf::Color(255, 0, 0, 120));
    m_crosshair.setOutlineThickness(2.f);
    m_crosshair.setOutlineColor(sf::Color::Red);
    m_crosshair.setOrigin(sf::Vector2f(20.f, 20.f));
}

void MinigameBaloncesto::setPosition(const sf::Vector2f& pos) {
    m_position = pos;
    m_background.setPosition(pos);
}

void MinigameBaloncesto::setSize(const sf::Vector2f& size) {
    m_size = size;
    m_background.setSize(size);
    
    float centroX = m_position.x + m_size.x / 2.f;
    float centroY = m_position.y + m_size.y / 2.f;
    
    // Fondo del minijuego (tablero)
    if (m_bgTexture.loadFromFile("assets/images/niveles/nivel7/baloncesto_bg.png")) {
        m_bgSprite = std::make_unique<sf::Sprite>(m_bgTexture);
        float escalaTablero = 1.5f;
        m_bgSprite->setScale(sf::Vector2f(escalaTablero, escalaTablero));
        sf::FloatRect bounds = m_bgSprite->getLocalBounds();
        m_bgSprite->setOrigin(sf::Vector2f(bounds.size.x/2.f, bounds.size.y/2.f));
        m_bgSprite->setPosition(sf::Vector2f(centroX + 120, centroY));
    }
    
    if (m_aroVacioTexture.loadFromFile("assets/images/niveles/nivel7/aro_vacio.png")) {
        m_aroSprite = std::make_unique<sf::Sprite>(m_aroVacioTexture);
        float escalaAro = 1.0f;
        m_aroSprite->setScale(sf::Vector2f(escalaAro, escalaAro));
        sf::FloatRect bounds = m_aroSprite->getLocalBounds();
        m_aroSprite->setOrigin(sf::Vector2f(bounds.size.x/2.f, bounds.size.y/2.f));
        // El aro centrado donde está el aro en el tablero
        m_aroSprite->setPosition(sf::Vector2f(centroX + 100, centroY+ 150));
    }
    m_aroEncestandoTexture.loadFromFile("assets/images/niveles/nivel7/aro_encestando.png");
    m_aroEncestadoTexture.loadFromFile("assets/images/niveles/nivel7/aro_encestado.png");

    // Zona de tiro (rectángulo visible para debug)
    m_zonaTiro = sf::FloatRect(
        sf::Vector2f(centroX, centroY-65.f),
        sf::Vector2f(250.f, 200.f)
    );

    // Pelota (181x182)
    if (m_pelotaTexture.loadFromFile("assets/images/niveles/nivel7/pelota_basket.png")) {
        m_pelotaSprite = std::make_unique<sf::Sprite>(m_pelotaTexture);
        float escalaPelota = 1.0f;
        m_pelotaSprite->setScale(sf::Vector2f(escalaPelota, escalaPelota));
        sf::FloatRect bounds = m_pelotaSprite->getLocalBounds();
        m_pelotaSprite->setOrigin(sf::Vector2f(bounds.size.x/2.f, bounds.size.y/2.f));
        m_pelotaSprite->setPosition(sf::Vector2f(centroX, m_position.y + m_size.y - 100.f));
    }

    // Barra de intensidad (pegada al borde izquierdo)
    float barraX = m_position.x + 220.f;
    float barraY = centroY - 110.f;
    m_barraFondo.setPosition(sf::Vector2f(barraX, barraY));
    m_barraVerde.setPosition(sf::Vector2f(barraX, barraY + 120.f));  // Verde en el centro

    // Textos
    if (m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        m_titleText = std::make_unique<sf::Text>(m_font, "TIROS LIBRES", 28);
        m_titleText->setFillColor(sf::Color::Yellow);
        
        m_scoreText = std::make_unique<sf::Text>(m_font, "Canastas: 0/" + std::to_string(m_scoreToWin), 20);
        m_scoreText->setFillColor(sf::Color::White);
        
        m_attemptsText = std::make_unique<sf::Text>(m_font, "Tiros: 0/" + std::to_string(m_maxAttempts), 20);
        m_attemptsText->setFillColor(sf::Color(200, 200, 200));
        
        m_closeText = std::make_unique<sf::Text>(m_font, "ESPACIO: Tirar | ESC: Salir", 14);
        m_closeText->setFillColor(sf::Color(150, 150, 150));
        
        m_messageText = std::make_unique<sf::Text>(m_font, "", 24);
        m_messageText->setStyle(sf::Text::Bold);
    }
}

void MinigameBaloncesto::activate() {
    m_isActive = true;
    reset();
}

void MinigameBaloncesto::deactivate() {
    m_isActive = false;
}

void MinigameBaloncesto::reset() {
    m_score = 0;
    m_attempts = 0;
    m_gameWon = false;
    m_fase = Fase::INTENSIDAD;
    m_indicadorPos = 0.f;
    m_indicadorSubiendo = true;
    m_intensidadFijada = false;
    m_pelotaVolando = false;
    m_mostrandoEnceste = false;
    m_faseEnceste = 0;
    m_messageTimer = 0.f;
    
    if (m_scoreText) m_scoreText->setString("Canastas: 0/" + std::to_string(m_scoreToWin));
    if (m_attemptsText) m_attemptsText->setString("Tiros: 0/" + std::to_string(m_maxAttempts));
    if (m_aroSprite) m_aroSprite->setTexture(m_aroVacioTexture);
    if (m_pelotaSprite) m_pelotaSprite->setPosition(sf::Vector2f(m_position.x + m_size.x/2.f, m_position.y + m_size.y - 100.f));
}

void MinigameBaloncesto::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_isActive || m_gameWon) return;
    
    if (event.is<sf::Event::KeyPressed>()) {
        const auto& keyEvent = event.getIf<sf::Event::KeyPressed>();
        
        if (keyEvent->code == sf::Keyboard::Key::Space || keyEvent->code == sf::Keyboard::Key::Enter) {
            if (m_fase == Fase::INTENSIDAD && !m_intensidadFijada) {
                // Fijar intensidad
                m_intensidadFijada = true;
                // Calcular qué tan lejos está del centro verde (posición 120-180)
                float centroVerde = 150.f;
                m_intensidad = 1.f - std::abs(m_indicadorPos - centroVerde) / 150.f;
                if (m_intensidad < 0.f) m_intensidad = 0.f;
                
                m_fase = Fase::APUNTAR;
                // Crosshair empieza en el centro
                m_crosshairPos = sf::Vector2f(m_position.x + m_size.x/2.f, m_position.y + m_size.y/2.f - 110.f);
                
            }
            else if (m_fase == Fase::APUNTAR) {
                iniciarLanzamiento();
                m_fase = Fase::VOLANDO;
            }
        }
    }
}

void MinigameBaloncesto::iniciarLanzamiento() {
    m_pelotaVolando = true;
    m_attempts++;
    if (m_attemptsText) m_attemptsText->setString("Tiros: " + std::to_string(m_attempts) + "/" + std::to_string(m_maxAttempts));
    
    sf::Vector2f inicio = m_pelotaSprite->getPosition();
    sf::Vector2f destino = m_crosshairPos;
    sf::Vector2f dir = destino - inicio;
    float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    
    if (dist > 0.f) {
        dir /= dist;
        float potencia = 600.f + m_intensidad * 200.f;
        m_pelotaVel = dir * potencia;
        m_pelotaVel.y -= 400.f + m_intensidad * 200.f;
    }
}

void MinigameBaloncesto::verificarEnceste() {
    sf::Vector2f pelotaPos = m_pelotaSprite->getPosition();
    
    if (m_zonaTiro.contains(pelotaPos) && !m_mostrandoEnceste) {
        m_score++;
        m_mostrandoEnceste = true;
        m_tiempoEnceste = 0.4f;
        m_faseEnceste = 1;
        
        if (m_aroSprite) m_aroSprite->setTexture(m_aroEncestandoTexture);
        showMessage("¡CANASTA!", sf::Color::Green);
        if (m_scoreText) m_scoreText->setString("Canastas: " + std::to_string(m_score) + "/" + std::to_string(m_scoreToWin));
        
        if (m_score >= m_scoreToWin) {
            m_gameWon = true;
            if (m_titleText) { m_titleText->setString("¡VICTORIA!"); m_titleText->setFillColor(sf::Color::Green); }
            showMessage("¡GANASTE! Obtuviste la llave", sf::Color::Green);
        }
    }
    
    if (pelotaPos.y > m_position.y + m_size.y + 50.f) {
        m_pelotaVolando = false;
        m_mostrandoEnceste = false;
        m_faseEnceste = 0;
        m_intensidadFijada = false;
        if (m_aroSprite) m_aroSprite->setTexture(m_aroVacioTexture);
        m_pelotaSprite->setPosition(sf::Vector2f(m_position.x + m_size.x/2.f, m_position.y + m_size.y - 100.f));
        
        if (m_attempts >= m_maxAttempts && m_score < m_scoreToWin) {
            if (m_titleText) { m_titleText->setString("SIN TIROS"); m_titleText->setFillColor(sf::Color::Red); }
        } else {
            m_fase = Fase::INTENSIDAD;
            m_indicadorPos = 0.f;
            m_indicadorSubiendo = true;
        }
    }
}

void MinigameBaloncesto::showMessage(const std::string& msg, const sf::Color& color) {
    if (!m_messageText) return;
    m_messageText->setString(msg);
    m_messageText->setFillColor(color);
    sf::FloatRect bounds = m_messageText->getLocalBounds();
    m_messageText->setOrigin(sf::Vector2f(bounds.size.x/2.f, bounds.size.y/2.f));
    m_messageText->setPosition(sf::Vector2f(m_position.x + m_size.x/2.f, m_position.y + m_size.y - 60.f));
    m_messageTimer = 1.5f;
}

void MinigameBaloncesto::update(float dt) {
    if (!m_isActive) return;
    
    if (m_messageTimer > 0.f) {
        m_messageTimer -= dt;
        if (m_messageTimer <= 0.f && m_messageText) m_messageText->setString("");
    }
    
    // Fase INTENSIDAD
    if (m_fase == Fase::INTENSIDAD && !m_intensidadFijada) {
        if (m_indicadorSubiendo) {
            m_indicadorPos += m_indicadorVel * dt;
            if (m_indicadorPos >= 300.f) m_indicadorSubiendo = false;
        } else {
            m_indicadorPos -= m_indicadorVel * dt;
            if (m_indicadorPos <= 0.f) m_indicadorSubiendo = true;
        }
    }
    
    // Fase APUNTAR: crosshair se mueve solo en forma de 8
    if (m_fase == Fase::APUNTAR) {
        static float tiempoCross = 0.f;
        tiempoCross += dt;
        float centroX = m_position.x + m_size.x/2.f;
        float centroY = m_position.y + m_size.y/2.f - 110.f;
        float amplitud = 150.f;
        m_crosshairPos.x = centroX + std::sin(tiempoCross * 2.f) * amplitud;
        m_crosshairPos.y = centroY + std::cos(tiempoCross * 3.f) * amplitud * 0.7f;
    }
    
    // Fase VOLANDO
    if (m_fase == Fase::VOLANDO && m_pelotaVolando) {
        m_pelotaVel.y += 500.f * dt;
        sf::Vector2f pos = m_pelotaSprite->getPosition();
        pos += m_pelotaVel * dt;
        m_pelotaSprite->setPosition(pos);
        verificarEnceste();
    }
    
    // Animación enceste
    if (m_mostrandoEnceste) {
        m_tiempoEnceste -= dt;
        if (m_faseEnceste == 1 && m_tiempoEnceste <= 0.f) {
            m_faseEnceste = 2; m_tiempoEnceste = 0.4f;
            if (m_aroSprite) m_aroSprite->setTexture(m_aroEncestadoTexture);
        }
        else if (m_faseEnceste == 2 && m_tiempoEnceste <= 0.f) {
            m_faseEnceste = 0; m_mostrandoEnceste = false;
            if (m_aroSprite) m_aroSprite->setTexture(m_aroVacioTexture);
        }
    }
}

void MinigameBaloncesto::draw(sf::RenderWindow& window) {
    if (!m_isActive) return;
    
    window.draw(m_background);
    if (m_bgSprite) window.draw(*m_bgSprite);
    
    // DEBUG: Zona de tiro (rectángulo verde)
    sf::RectangleShape zonaDebug(sf::Vector2f(m_zonaTiro.size.x, m_zonaTiro.size.y));
    zonaDebug.setPosition(sf::Vector2f(m_zonaTiro.position.x, m_zonaTiro.position.y));
    zonaDebug.setFillColor(sf::Color(0, 255, 0, 80));
    zonaDebug.setOutlineThickness(2.f);
    zonaDebug.setOutlineColor(sf::Color::Green);
    window.draw(zonaDebug);
    
    // Barra de intensidad
    if (m_fase == Fase::INTENSIDAD && !m_intensidadFijada) {
        window.draw(m_barraFondo);
        window.draw(m_barraVerde);
        float barraY = m_position.y + m_size.y/2.f - 150.f;
        m_barraIndicador.setPosition(sf::Vector2f(m_position.x + 45.f, barraY + m_indicadorPos));
        window.draw(m_barraIndicador);
    }
    
    if (m_aroSprite) window.draw(*m_aroSprite);
    if (m_pelotaSprite) window.draw(*m_pelotaSprite);
    
    // Crosshair
    if (m_fase == Fase::APUNTAR) {
        m_crosshair.setPosition(m_crosshairPos);
        window.draw(m_crosshair);
    }
    
    // Textos
    if (m_titleText) {
        sf::FloatRect tb = m_titleText->getLocalBounds();
        m_titleText->setOrigin(sf::Vector2f(tb.size.x/2.f, 0.f));
        m_titleText->setPosition(sf::Vector2f(m_position.x + m_size.x/2.f, m_position.y + 15.f));
        window.draw(*m_titleText);
    }
    if (m_scoreText) {
        m_scoreText->setPosition(sf::Vector2f(m_position.x + 120.f, m_position.y + 50.f));
        window.draw(*m_scoreText);
    }
    if (m_attemptsText) {
        m_attemptsText->setPosition(sf::Vector2f(m_position.x + m_size.x - 220.f, m_position.y + 15.f));
        window.draw(*m_attemptsText);
    }
    if (m_closeText) {
        m_closeText->setPosition(sf::Vector2f(m_position.x + m_size.x - 280.f, m_position.y + m_size.y - 30.f));
        window.draw(*m_closeText);
    }
    if (m_messageText && m_messageTimer > 0.f) window.draw(*m_messageText);
}