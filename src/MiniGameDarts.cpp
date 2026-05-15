#include "MinigameDarts.hpp"
#include <iostream>
#include <algorithm>


// CONSTRUCTOR

MinigameDarts::MinigameDarts()
    : m_isActive(false), m_gameWon(false), m_score(0), m_throwsLeft(5),
      m_hasThrown(false), m_messageTimer(0.f), m_gen(m_rd())
{
    
    // Cargar textura de FONDO DE BOSQUE
    
    if (!m_backgroundTexture.loadFromFile("assets/images/niveles/nivel4/bosque.jpg")) {
        std::cerr << "Error cargando textura de fondo bosque.png" << std::endl;
        m_backgroundForest = nullptr;
    } else {
        m_backgroundForest = std::make_unique<sf::Sprite>(m_backgroundTexture);
    }

    
    // Configurar fondo oscuro (se usará como respaldo si no hay textura)
    
    m_background.setFillColor(sf::Color(0, 0, 0, 200));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(100, 100, 100));

    
    // // Configurar fondo oscuro del minijuego
    
    // m_background.setFillColor(sf::Color(0, 0, 0, 200));
    // m_background.setOutlineThickness(3.f);
    // m_background.setOutlineColor(sf::Color(100, 100, 100));

    
    // Cargar la fuente para los textos
    
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente en MinigameDarts" << std::endl;
        m_fontLoaded = false;
    } else {
        m_fontLoaded = true;
    }

    
    // Cargar la textura de la diana
    
    if (!m_dartboardTexture.loadFromFile("assets/images/niveles/nivel4/dardos.png")) {
        std::cerr << "Error cargando textura de la diana" << std::endl;
        m_dartboardSprite = nullptr;
    } else {
        m_dartboardSprite = std::make_unique<sf::Sprite>(m_dartboardTexture);
    }

    
    // Inicializar anillos de puntuación y elementos de la interfaz
    
    initScoringRings();
    initUI();

    
    // Configurar el círculo de puntería (blanco con borde rojo)
    
    m_aimingCircle.setRadius(12.f);
    m_aimingCircle.setFillColor(sf::Color(255, 255, 255, 180));
    m_aimingCircle.setOutlineThickness(2.f);
    m_aimingCircle.setOutlineColor(sf::Color::Red);
    m_aimingCircle.setOrigin(sf::Vector2f(12.f, 12.f));   

    
    // Variables de movimiento del círculo
    
    m_aimAngle = 0.f;
    m_aimSpeed = 3.5f;           
    m_noiseOffsetX = 0.f;
    m_noiseOffsetY = 100.f;
}


// DEFINICIÓN DE LOS ANILLOS DE PUNTUACIÓN

void MinigameDarts::initScoringRings() {
    // Los anillos están ordenados del centro hacia afuera.
    m_scoringRings = {
        {0.12f, 20, sf::Color::Red},           
        {0.24f, 12, sf::Color(255, 100, 0)},   
        {0.48f, 6, sf::Color(0, 100, 200)},    
        {0.72f, 3, sf::Color(0, 200, 100)},    
        {1.00f, 1, sf::Color(200, 200, 200)}   
    };
}


// INICIALIZACIÓN DE LA INTERFAZ DE USUARIO (TEXTOS)

void MinigameDarts::initUI() {
    if (!m_fontLoaded) return;

    // Título principal
    m_titleText = std::make_unique<sf::Text>(m_font);
    m_titleText->setString("DARTS CHALLENGE");
    m_titleText->setCharacterSize(28);
    m_titleText->setOutlineThickness(1.5f);      
    m_titleText->setOutlineColor(sf::Color::Black);
    m_titleText->setFillColor(sf::Color::Yellow);

    // Vidas restantes
    m_throwsText = std::make_unique<sf::Text>(m_font);
    m_throwsText->setCharacterSize(18);
    m_throwsText->setOutlineThickness(1.5f);     
    m_throwsText->setOutlineColor(sf::Color::Black);
    m_throwsText->setFillColor(sf::Color(200, 200, 200));

    // Instrucción
    m_instructionText = std::make_unique<sf::Text>(m_font);
    m_instructionText->setString("Haz clic cuando el circulo este sobre el centro");
    m_instructionText->setCharacterSize(20);
    m_instructionText->setOutlineThickness(1.5f); 
    m_instructionText->setOutlineColor(sf::Color::Black);
    m_instructionText->setFillColor(sf::Color(200, 200, 200));

    // Texto "Presiona F para salir"
    m_closeText = std::make_unique<sf::Text>(m_font);
    m_closeText->setString("Presiona F para salir");
    m_closeText->setCharacterSize(16);
    m_closeText->setOutlineThickness(1.5f);      
    m_closeText->setOutlineColor(sf::Color::Black);
    m_closeText->setFillColor(sf::Color::White);  

    // Mensajes temporales
    m_messageText = std::make_unique<sf::Text>(m_font);
    m_messageText->setCharacterSize(30);
    m_messageText->setStyle(sf::Text::Bold);
    m_messageText->setOutlineThickness(1.5f);    
    m_messageText->setOutlineColor(sf::Color::Black);
}

// POSICIÓN DEL MINIJUEGO EN LA PANTALLA

void MinigameDarts::setPosition(const sf::Vector2f& pos) {
    m_position = pos;
    m_background.setPosition(pos);
}


// TAMAÑO DEL MINIJUEGO (SE ESCALAN LA DIANA Y EL FONDO)

void MinigameDarts::setSize(const sf::Vector2f& size) {
    m_size = size;
    m_background.setSize(size);
    m_targetCenter = sf::Vector2f(
        m_position.x + m_size.x / 2.f,
        m_position.y + m_size.y / 2.f
    );

    
    // Escalar el fondo de bosque para que ocupe todo el minijuego
    
    if (m_backgroundForest && m_backgroundTexture.getSize().x > 0) {
        sf::FloatRect texBounds = m_backgroundForest->getLocalBounds();
        float scaleX = m_size.x / texBounds.size.x;
        float scaleY = m_size.y / texBounds.size.y;
        m_backgroundForest->setScale(sf::Vector2f(scaleX, scaleY));
        m_backgroundForest->setPosition(m_position);
    }

    
    // Escalar la diana
    
    if (m_dartboardSprite && m_dartboardTexture.getSize().x > 0) {
        m_currentDartboardRadius = std::min(m_size.x, m_size.y) * 0.35f;
        
        sf::FloatRect texBounds = m_dartboardSprite->getLocalBounds();
        float scaleX = (m_currentDartboardRadius * 2.f) / texBounds.size.x;
        float scaleY = (m_currentDartboardRadius * 2.f) / texBounds.size.y;
        m_dartboardSprite->setScale(sf::Vector2f(scaleX, scaleY));
        m_dartboardSprite->setOrigin(sf::Vector2f(texBounds.size.x / 2.f, texBounds.size.y / 2.f));
        m_dartboardSprite->setPosition(m_targetCenter);
    }
}


// ACTIVAR / DESACTIVAR MINIJUEGO

void MinigameDarts::activate() {
    m_isActive = true;
    reset();
}

void MinigameDarts::deactivate() {
    m_isActive = false;
}


// REINICIAR EL MINIJUEGO (VALORES POR DEFECTO)

void MinigameDarts::reset() {
    m_gameWon = false;
    m_score = 0;
    m_throwsLeft = 5;          
    m_hasThrown = false;
    m_messageTimer = 0.f;
    m_aimAngle = 0.f;

    if (m_throwsText) m_throwsText->setString("Vidas: " + std::to_string(m_throwsLeft));
    if (m_titleText) m_titleText->setString("DARTS CHALLENGE");
    if (m_messageText) m_messageText->setString("");
}


// MANEJAR EVENTOS DEL MINIJUEGO 

void MinigameDarts::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_isActive || m_gameWon) return;

    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left && !m_hasThrown && m_throwsLeft > 0) {
            m_hasThrown = true;

            
            // 1. Obtener posición actual del círculo de puntería
            
            sf::Vector2f aimPos = m_aimingCircle.getPosition();

            
            // 2. Calcular desviación del lanzamiento (proporcional a la distancia al centro)
            //    - Si está en el centro = desviación 0 (tiro perfecto)
            //    - Si está en el borde = desviación máxima (±12 píxeles)
            
            float distToCenter = std::sqrt(
                (aimPos.x - m_targetCenter.x) * (aimPos.x - m_targetCenter.x) +
                (aimPos.y - m_targetCenter.y) * (aimPos.y - m_targetCenter.y)
            );
            float maxDeviation = 12.f;
            float deviationMultiplier = distToCenter / m_currentDartboardRadius;
            float actualDeviation = maxDeviation * deviationMultiplier;
            
            std::uniform_real_distribution<float> dist(-actualDeviation, actualDeviation);
            sf::Vector2f hitPoint = aimPos + sf::Vector2f(dist(m_gen), dist(m_gen));

            
            // 3. Calcular puntuación según el anillo donde cayó el dardo
            
            int points = calculateScore(hitPoint);
            
            // Conversión a puntos de mejora para signos vitales (puntos/3)
            int effectPoints = points / 3;
            if (effectPoints < 1 && points > 0) effectPoints = 1;   
            if (effectPoints > -1 && points < 0) effectPoints = -1; 
            
            
            // 4. Aplicar efecto a los signos vitales (puede ser positivo o negativo)
            
            if (m_vitalSigns) {
                m_vitalSigns->applyEffect(effectPoints);
            }
            
            
            // 5. Mostrar mensaje temporal según el resultado
            
            if (points > 0) {
                showTemporaryMessage("+" + std::to_string(points) + " puntos", sf::Color::Green, 0.8f);
            } else if (points < 0) {
                showTemporaryMessage(std::to_string(points) + " puntos", sf::Color::Red, 0.8f);
            }
            
            
            // 6. Reducir una vida (cada lanzamiento cuesta una oportunidad)
            
            m_throwsLeft--;
            if (m_throwsText) m_throwsText->setString("Vidas: " + std::to_string(m_throwsLeft));
            
            
            // 7. Verificar si se acabaron las vidas (GAME OVER)
            
            if (m_throwsLeft == 0) {
                m_gameWon = true;
                if (m_titleText) m_titleText->setString("GAME OVER");
                
                if (m_vitalSigns && m_vitalSigns->isGameOver()) {
                    showTemporaryMessage("El paciente ha muerto", sf::Color::Red, 2.f);
                } else if (m_vitalSigns && !m_vitalSigns->isStabilized()) {
                    showTemporaryMessage("No lograste estabilizarlo\n F y vuelve a intentarlo", sf::Color::Red, 2.5f);
                } else {
                    showTemporaryMessage("GAME OVER", sf::Color::Red, 2.f);
                }
            }
            
            
            // 8. Verificar victoria por estabilización del paciente
            
            if (m_vitalSigns && m_vitalSigns->isStabilized()) {
                m_gameWon = true;
                if (m_titleText) m_titleText->setString("VICTORIA");
                showTemporaryMessage("PACIENTE ESTABLE", sf::Color::Green, 2.f);
            }
        }
    }
    else if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseReleased->button == sf::Mouse::Button::Left) {
            m_hasThrown = false;   
        }
    }
}


// CÁLCULO DE PUNTUACIÓN SEGÚN EL ANILLO DE IMPACTO

int MinigameDarts::calculateScore(const sf::Vector2f& hitPoint) {
    sf::Vector2f delta = hitPoint - m_targetCenter;
    float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    
    int points = 0;
    
    if (distance <= m_currentDartboardRadius) {
        float normalizedDist = distance / m_currentDartboardRadius;
        
        // Anillo 1: CENTRO (Rojo) = +20 puntos
        if (normalizedDist <= 0.12f) {
            points = 20;
            showTemporaryMessage("CENTRO! +20", sf::Color(255, 215, 0), 0.8f);
        }
        // Anillo 2: NARANJA = -5 puntos (Resta)
        else if (normalizedDist <= 0.24f) {
            points = -5;
            showTemporaryMessage("-5", sf::Color::Red, 0.8f);
        }
        // Anillo 3: AZUL → +5 puntos
        else if (normalizedDist <= 0.48f) {
            points = 5;
            showTemporaryMessage("+5", sf::Color(100, 200, 255), 0.8f);
        }
        // Anillo 4: VERDE = -3 puntos (Resta)
        else if (normalizedDist <= 0.72f) {
            points = -3;
            showTemporaryMessage("-3", sf::Color::Red, 0.8f);
        }
        // Anillo 5: GRIS (Borde) = -1 punto (Resta)
        else {
            points = -1;
            showTemporaryMessage("-1", sf::Color(150, 150, 150), 0.8f);
        }
    } else {
        // FUERA DE LA DIANA: fallo total
        showTemporaryMessage("FALLASTE! -10", sf::Color::Red, 0.8f);
        points = -10;
    }
    return points;
}


// MOSTRAR MENSAJE TEMPORAL EN PANTALLA

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


// ACTUALIZAR EL TEMPORIZADOR DEL MENSAJE

void MinigameDarts::updateMessage(float dt) {
    if (m_messageTimer > 0.f) {
        m_messageTimer -= dt;
        if (m_messageTimer <= 0.f && m_messageText) {
            m_messageText->setString("");   // Borrar mensaje cuando expira
        }
    }
}


// ACTUALIZACIÓN DEL MINIJUEGO (MOVIMIENTO DEL CÍRCULO)

void MinigameDarts::update(float dt) {
    if (!m_isActive || m_gameWon) return;
    updateMessage(dt);

    m_aimAngle += m_aimSpeed * dt;
    
    float amplitude = m_currentDartboardRadius * 1.3f;   
    float offsetX = std::sin(m_aimAngle) * amplitude;
    float offsetY = std::sin(m_aimAngle * 2.0f) * amplitude * 0.6f;   
    
    float aimX = m_targetCenter.x + offsetX;
    float aimY = m_targetCenter.y + offsetY;
    
    m_aimingCircle.setPosition(sf::Vector2f(aimX, aimY));
}


// DIBUJAR EL MINIJUEGO EN PANTALLA

void MinigameDarts::draw(sf::RenderWindow& window) {
    if (!m_isActive) return;

    // DIBUJAR FONDO DE BOSQUE
    if (m_backgroundForest) {
        window.draw(*m_backgroundForest);
    } else {
        window.draw(m_background);
    }

    // Título
    if (m_titleText) {
        m_titleText->setPosition(sf::Vector2f((m_position.x + m_size.x - 50.0) / 2.f, m_position.y + 55.f));
        window.draw(*m_titleText);
    }

    // Diana (sprite)
    if (m_dartboardSprite) {
        window.draw(*m_dartboardSprite);
    }

    // Círculo de puntería
    window.draw(m_aimingCircle);

    
    // POSICIONAMIENTO RELATIVO (escala con el tamaño del minijuego)
    
    
    float marginX = m_size.x * 0.03f;   
    float marginY = m_size.y * 0.02f;   
    
    // Texto "Presiona F para salir" - esquina superior izquierda
    if (m_closeText) {
        // Usar porcentajes del tamaño del minijuego (independiente de resolución)
        float marginX = m_size.x * 0.03f;   // 3% del ancho del minijuego
        float marginY = m_size.y * 0.02f;   // 2% del alto del minijuego
        
        m_closeText->setPosition(sf::Vector2f(
            m_position.x + marginX,
            m_position.y + marginY
        ));
        m_closeText->setOrigin(sf::Vector2f(0.f, 0.f));
        window.draw(*m_closeText);
    }

    // Texto de vidas restantes - esquina superior izquierda, debajo del texto de salir
    if (m_throwsText) {
        // Posición vertical debajo del texto "Presiona F para salir"
        float marginX = m_size.x * 0.03f;   // Mismo margen izquierdo (3%)
        float offsetY = 30.f;                // Espacio entre textos
        
        // Calcular altura del texto "Presiona F para salir" si existe
        if (m_closeText) {
            sf::FloatRect closeBounds = m_closeText->getLocalBounds();
            offsetY = closeBounds.size.y + 10.f;  // Altura del texto anterior + 10px
        }
        
        m_throwsText->setPosition(sf::Vector2f(
            m_position.x + marginX,
            m_position.y + marginY + offsetY
        ));
        window.draw(*m_throwsText);
    }

        // Instrucciones para el jugador (centrado en la parte inferior)
        if (m_instructionText) {
            sf::FloatRect instrBounds = m_instructionText->getLocalBounds();
            m_instructionText->setOrigin(sf::Vector2f(instrBounds.size.x / 2.f, 0.f));
            m_instructionText->setPosition(sf::Vector2f(
                m_position.x + m_size.x / 2.f, 
                m_position.y + m_size.y - 80.f
            ));
            window.draw(*m_instructionText);
        }

        // Mensaje temporal
        if (m_messageText && !m_messageText->getString().isEmpty()) {
            window.draw(*m_messageText);
        }
    }