#include "Nivel1State.hpp"
#include "PauseState.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

Nivel1State::Nivel1State(sf::RenderWindow* window, Game* game) 
    : State(window, game), m_background(nullptr), m_cercaMesaPool(false), m_textoInteraccion(nullptr)
{
    // 1. CARGAR JUGADOR - Posición en Vimorte
    m_player.loadAssets();
    m_player.setPosition(1150.f, 300.f);
    m_player.setSpeed(300.0f);
    
    // 2. CARGAR FONDO DEL NIVEL 1
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel1/background.jpg")) {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x), 
                                    static_cast<float>(textureSize.y));
        std::cout << "✅ Nivel 1 cargado. Tamaño: " << m_worldSize.x << "x" << m_worldSize.y << std::endl;
    } else {
        std::cerr << "❌ Error: No se pudo cargar background.jpg" << std::endl;
        m_worldSize = sf::Vector2f(1754.f, 1587.f);
    }
    
    // 3. CONFIGURAR CÁMARA (usar window->getSize() directamente)
    sf::Vector2u windowSize = window->getSize();
    m_camera = sf::View(sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f), 
                        sf::Vector2f(static_cast<float>(windowSize.x), 
                                     static_cast<float>(windowSize.y)));
    
    // 4. CONFIGURAR COLISIONES
    configurarColisiones();
    
    // 5. CONFIGURAR MINIJUEGO DE POOL (usar el mismo windowSize)
    m_poolMinigame.setSize(sf::Vector2f(1000.f, 700.f));
    m_poolMinigame.setPosition(sf::Vector2f(
        (windowSize.x - 1000.f) / 2.f,
        (windowSize.y - 700.f) / 2.f
    ));
    
    // 6. CONFIGURAR TEXTO DE INTERACCIÓN
    if (m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setString("Presiona R para jugar al pool");
        m_textoInteraccion->setCharacterSize(20);
        m_textoInteraccion->setFillColor(sf::Color::White);
        m_textoInteraccion->setPosition(sf::Vector2f(windowSize.x / 2.f, windowSize.y - 70.f));
        
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
    }
    
    std::cout << "✅ Nivel1State inicializado correctamente" << std::endl;
}

void Nivel1State::update(float dt) 
{
    sf::Vector2f posAnterior = m_player.getPosition();
    sf::Vector2u windowSize = window->getSize();
    
    // Verificar si está cerca de la mesa de pool
    sf::FloatRect mesaPoolArea(sf::Vector2f(1100.f, 1100.f), sf::Vector2f(500.f, 300.f));
    m_cercaMesaPool = m_player.getBounds().findIntersection(mesaPoolArea).has_value();
    
    // Activar minijuego con R
    static bool rPresionado = false;
    if (m_cercaMesaPool && !m_poolMinigame.isActive()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            if (!rPresionado) {
                rPresionado = true;
                m_poolMinigame.activate();
                std::cout << "🎱 Minijuego de pool activado!" << std::endl;
            }
        } else {
            rPresionado = false;
        }
    }
    
    // Si el minijuego está activo, no procesar movimiento del jugador
    if (m_poolMinigame.isActive()) {
        m_poolMinigame.update(dt);
        m_player.update(dt);
        
        // Actualizar cámara
        sf::Vector2f playerPos = m_player.getPosition();
        sf::Vector2f cameraPos = playerPos;
        float halfWidth = static_cast<float>(windowSize.x) / 2.f;
        float halfHeight = static_cast<float>(windowSize.y) / 2.f;
        cameraPos.x = std::clamp(cameraPos.x, halfWidth, m_worldSize.x - halfWidth);
        cameraPos.y = std::clamp(cameraPos.y, halfHeight, m_worldSize.y - halfHeight);
        m_camera.setCenter(cameraPos);
        
        return;
    }
    
    // MOVIMIENTO DEL JUGADOR (solo si minijuego no está activo)
    sf::Vector2f movimiento(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || 
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) movimiento.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || 
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) movimiento.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || 
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) movimiento.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || 
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) movimiento.x += 1.f;
    
    if (movimiento.x != 0.f || movimiento.y != 0.f) {
        float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
        movimiento /= length;
    }
    
    m_player.move(movimiento, dt);
    m_player.update(dt);
    
    // Verificación de colisiones
    for (const auto& obj : m_mapaFisico) {
        if (m_player.getBounds().findIntersection(obj.getBounds())) {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }
    
    // Actualizar cámara
    sf::Vector2f playerPos = m_player.getPosition();
    sf::Vector2f cameraPos = playerPos;
    float halfWidth = static_cast<float>(windowSize.x) / 2.f;
    float halfHeight = static_cast<float>(windowSize.y) / 2.f;
    cameraPos.x = std::clamp(cameraPos.x, halfWidth, m_worldSize.x - halfWidth);
    cameraPos.y = std::clamp(cameraPos.y, halfHeight, m_worldSize.y - halfHeight);
    m_camera.setCenter(cameraPos);
    
    // PAUSA
    static bool escapeProcesado = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        if (!escapeProcesado) {
            escapeProcesado = true;
            game->pushState(std::make_unique<PauseState>(window, game));
        }
    } else {
        escapeProcesado = false;
    }
}

void Nivel1State::draw()
{
    if (!window) return;
    
    window->setView(m_camera);
    
    // Dibujar fondo
    if (m_background) {
        window->draw(*m_background);
    } else {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(50, 30, 30));
        window->draw(fallback);
    }
    
    // Dibujar jugador
    m_player.draw(*window);
    
    // ===== DEBUG: DIBUJAR COLISIONES =====
    for (const auto& obj : m_mapaFisico) {
        sf::RectangleShape colision;
        colision.setPosition(sf::Vector2f(obj.getBounds().position.x, obj.getBounds().position.y));
        colision.setSize(sf::Vector2f(obj.getBounds().size.x, obj.getBounds().size.y));
        colision.setFillColor(sf::Color(255, 0, 0, 100));
        colision.setOutlineThickness(2.f);
        colision.setOutlineColor(sf::Color::Red);
        window->draw(colision);
    }
    
    // Dibujar área de la mesa de pool (DEBUG)
    sf::RectangleShape mesaDebug(sf::Vector2f(351.f, 182.f));
    mesaDebug.setPosition(sf::Vector2f(1184.f, 1174.f));
    mesaDebug.setFillColor(sf::Color(0, 255, 0, 100));
    mesaDebug.setOutlineThickness(3.f);
    mesaDebug.setOutlineColor(sf::Color::Green);
    window->draw(mesaDebug);
    
    // Dibujar texto de interacción
    if (m_cercaMesaPool && !m_poolMinigame.isActive() && m_textoInteraccion) {
        window->setView(window->getDefaultView());
        window->draw(*m_textoInteraccion);
        window->setView(m_camera);
    }
    
    // Dibujar minijuego
    if (m_poolMinigame.isActive()) {
        window->setView(window->getDefaultView());
        m_poolMinigame.draw(*window);
    }
}

void Nivel1State::configurarColisiones() 
{
    m_mapaFisico.clear();
    
    // ===== BORDES DEL MUNDO =====
    m_mapaFisico.emplace_back(-10.f, 0.f, 10.f, m_worldSize.y);           // Izquierda
    m_mapaFisico.emplace_back(m_worldSize.x, 0.f, 10.f, m_worldSize.y);   // Derecha
    m_mapaFisico.emplace_back(0.f, -10.f, m_worldSize.x, 10.f);           // Arriba
    m_mapaFisico.emplace_back(0.f, m_worldSize.y, m_worldSize.x, 10.f);   // Abajo
    
    // ===== MURO HORIZONTAL CENTRAL (con hueco para pasar) =====
    // Parte izquierda del muro central
    m_mapaFisico.emplace_back(0.f, 571.f, 600.f, 48.f);
    // Parte derecha del muro central (deja hueco de 600 a 800)
    m_mapaFisico.emplace_back(800.f, 571.f, 954.f, 48.f);
    
    // ===== MURO VERTICAL IZQUIERDO (con hueco para pasar) =====
    // Parte superior del muro vertical izquierdo
    m_mapaFisico.emplace_back(763.f, 0.f, 53.f, 400.f);
    // Parte inferior del muro vertical izquierdo (deja hueco de 400 a 550)
    m_mapaFisico.emplace_back(763.f, 550.f, 53.f, 69.f);
    
    // ===== MURO VERTICAL INFERIOR (con hueco para pasar) =====
    // Parte superior del muro vertical inferior
    m_mapaFisico.emplace_back(763.f, 920.f, 53.f, 200.f);
    // Parte inferior del muro vertical inferior (deja hueco de 1120 a 1300)
    m_mapaFisico.emplace_back(763.f, 1300.f, 53.f, 287.f);

    m_mapaFisico.emplace_back(1184.f, 1174.f, 351.f, 182.f);  // MEsa de Pool

    
    std::cout << "✅ Colisiones configuradas: " << m_mapaFisico.size() << " paredes con huecos" << std::endl;
}

void Nivel1State::handleEvent(const sf::Event& event) {
    if (m_poolMinigame.isActive()) {
        m_poolMinigame.handleEvent(event, *window);
        
        // Cerrar minijuego con ESC
        if (event.is<sf::Event::KeyPressed>()) {
            const auto& keyEvent = event.getIf<sf::Event::KeyPressed>();
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                m_poolMinigame.deactivate();
                std::cout << "🎱 Minijuego de pool cerrado" << std::endl;
            }
        }
    }
}