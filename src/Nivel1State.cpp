#include "Nivel1State.hpp"
#include "PauseState.hpp"
#include "MuerteCentinelaState.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

#include "Nivel1State.hpp"
#include "PauseState.hpp"
#include "MuerteCentinelaState.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

Nivel1State::Nivel1State(sf::RenderWindow *window, Game *game)
    : State(window, game),
      m_background(nullptr),
      m_cercaMesaPool(false),
      m_textoInteraccion(nullptr),
      m_mostrarPuertaSalida(false),
      m_cercaPuertaSalida(false),
      m_escapeConsumed(false),
      m_mostrarTutorial(false),
      m_cercaMesaColorMix(false),
      m_mostrarTutorialPorTecla(false),
      m_esperandoSegundaE(false),
      m_msjActual(),
      m_fontLoaded(false),
      m_poolCompletado(false),
      m_quizCompletado(false),
      m_colorMixCompletado(false),
      m_todosPuzzlesCompletados(false),
      m_poolEstabaActivo(false),
      m_quizEstabaActivo(false),
      m_colorMixEstabaActivo(false),
      m_debugMode(false)
{
    m_msjActual.texto = "";
    m_msjActual.tiempoRestante = 0.0f;
    m_msjActual.color = sf::Color::Yellow;

    m_player.loadAssets();
    m_player.setPosition(1150.f, 300.f);
    m_player.setSpeed(300.0f);

    if (game->tienePartidaActiva())
    {
        const auto &items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
        auto it = std::find(items.begin(), items.end(), "TutorialVisto");

        if (it == items.end())
        {
            m_mostrarTutorial = true;
            game->getSaveManager().addItemRecolectado("TutorialVisto");
        }
    }

    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel1/background.jpg"))
    {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x),
                                   static_cast<float>(textureSize.y));
    }
    else
    {
        std::cerr << "Error: No se pudo cargar background.jpg" << std::endl;
        m_worldSize = sf::Vector2f(1754.f, 1587.f);
    }

    sf::Vector2u windowSize = window->getSize();
    float fixedWidth = 1280.f;
    float fixedHeight = 720.f;
    m_camera = sf::View(
        sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
        sf::Vector2f(fixedWidth, fixedHeight));
    m_lastWindowSize = windowSize;

    // Áreas de interacción
    m_pizarraArea = sf::FloatRect(sf::Vector2f(200.f, 700.f), sf::Vector2f(180.f, 150.f));
    m_mesaColorMixArea = sf::FloatRect(sf::Vector2f(40.f, 280.f), sf::Vector2f(100.f, 120.f));
    m_mesaPoolArea = sf::FloatRect(sf::Vector2f(910.f, 900.f), sf::Vector2f(240.f, 120.f));
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(727.f, 250.f), sf::Vector2f(95.f, 50.f));

    configurarColisiones();

    float poolW = windowSize.x * 0.78f;
    float poolH = windowSize.y * 0.83f;
    m_poolMinigame.setSize(sf::Vector2f(poolW, poolH));
    m_poolMinigame.setPosition(sf::Vector2f(
        (windowSize.x - poolW) / 2.f,
        (windowSize.y - poolH) / 2.f));

    float quizW = windowSize.x * 0.7f;
    float quizH = windowSize.y * 0.83f;
    m_quizMinigame.setSize(sf::Vector2f(quizW, quizH));
    m_quizMinigame.setPosition(sf::Vector2f(
        (windowSize.x - quizW) / 2.f,
        (windowSize.y - quizH) / 2.f));

    float colorW = windowSize.x * 0.7f;
    float colorH = windowSize.y * 0.83f;
    m_colorMixMinigame.setSize(sf::Vector2f(colorW, colorH));
    m_colorMixMinigame.setPosition(sf::Vector2f(
        (windowSize.x - colorW) / 2.f,
        (windowSize.y - colorH) / 2.f));
    m_colorMixMinigame.initUI();

    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (m_fontLoaded)
    {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setCharacterSize(20);
        m_textoInteraccion->setFillColor(sf::Color::White);

        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(24);
        m_textoMensaje->setFillColor(sf::Color::Yellow);
    }
    else
    {
        m_textoInteraccion = nullptr;
        m_textoMensaje = nullptr;
    }

    if (game->tienePartidaActiva())
    {
        game->getSaveManager().setNivelActual(1, 1);
        game->guardarPartidaActual();
    }

    game->setIsInLevel(true);
}

void Nivel1State::handleEvent(const sf::Event &event)
{
    // Manejar teclas globales (Escape, M, F3)
    if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Escape)
        {
            if (m_mostrarTutorial || m_mostrarTutorialPorTecla)
            {
                m_mostrarTutorial = false;
                m_mostrarTutorialPorTecla = false;
                return;
            }
        }

        if (keyPressed->code == sf::Keyboard::Key::M)
        {
            if (game->tienePartidaActiva())
            {
                const auto &items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
                auto it = std::find(items.begin(), items.end(), "TutorialVisto");
                if (it != items.end())
                {
                    m_mostrarTutorialPorTecla = true;
                }
                else
                {
                    m_mostrarTutorial = true;
                }
            }
            else
            {
                m_mostrarTutorialPorTecla = true;
            }
        }
        
        if (keyPressed->code == sf::Keyboard::Key::F3)
        {
            m_debugMode = !m_debugMode;
            CoordenadasDebug::getInstance().setVisible(m_debugMode);
        }
    }

    // Manejar eventos de minijuegos
    if (m_poolMinigame.isActive())
    {
        m_poolMinigame.handleEvent(event, *window);
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyEvent->code == sf::Keyboard::Key::Escape)
            {
                m_poolMinigame.deactivate();
                m_escapeConsumed = true;
                return;
            }
        }
    }
    else if (m_quizMinigame.isActive())
    {
        m_quizMinigame.handleEvent(event, *window);
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyEvent->code == sf::Keyboard::Key::Escape)
            {
                m_quizMinigame.deactivate();
                m_escapeConsumed = true;
                return;
            }
        }
    }
    else if (m_colorMixMinigame.isActive())
    {
        m_colorMixMinigame.handleEvent(event, *window);
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyEvent->code == sf::Keyboard::Key::Escape)
            {
                m_colorMixMinigame.deactivate();
                m_escapeConsumed = true;
                return;
            }
        }
    }

    Inventory *inv = m_player.getInventory();
    if (inv)
    {
        inv->handleEvent(event, *window);
    }
}

void Nivel1State::verificarEntradaCentinela()
{
    LevelNode *currentNode = game->getLevelTree().getCurrentNode();
    if (currentNode && currentNode->hasCentinela())
    {
    }
}

void Nivel1State::verificarSalidaNivel()
{
    // Actualizar cercanía a la puerta SIEMPRE
    m_cercaPuertaSalida = m_player.getHurtbox().findIntersection(m_puertaSalidaArea).has_value();

    // Solo permitir salir si la puerta está activa
    if (!m_todosPuzzlesCompletados) return;

    static bool ePresionado = false;
    if (m_cercaPuertaSalida && !m_poolMinigame.isActive() && !m_quizMinigame.isActive() && !m_colorMixMinigame.isActive())
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
        {
            if (!ePresionado)
            {
                ePresionado = true;
                std::cout << "¡Puzzles completados! Saliendo del nivel..." << std::endl;
                game->avanzarNivel();
            }
        }
        else
        {
            ePresionado = false;
        }
    }

}

void Nivel1State::update(float dt)
{
	 verificarEstadoMinijuegos(); 
    // Actualizar mensaje temporal
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f)
    {
        m_msjActual.tiempoRestante -= dt;
        if (m_msjActual.tiempoRestante <= 0.0f)
        {
            m_textoMensaje->setString("");
        }
    }

    sf::Vector2f posAnterior = m_player.getPosition();

    // ========== ÁREA DE LA MESA DE POOL ==========
    m_cercaMesaPool = m_player.getBounds().findIntersection(m_mesaPoolArea).has_value();

    static bool rPresionado = false;
    if (m_cercaMesaPool && !m_poolMinigame.isActive())
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
        {
            if (!rPresionado)
            {
                rPresionado = true;
                m_poolMinigame.activate();
                std::cout << "Minijuego de pool activado!" << std::endl;
            }
        }
        else
        {
            rPresionado = false;
        }
    }

    if (m_poolMinigame.isActive())
    {
        m_poolMinigame.update(dt);
        m_player.update(dt);

        // Cámara fija
        sf::Vector2f playerPos = m_player.getPosition();
        sf::Vector2f cameraPos = playerPos;
        float halfWidth = 1280.f / 2.f;
        float halfHeight = 720.f / 2.f;
        if (halfWidth * 2.f >= m_worldSize.x)
        {
            cameraPos.x = m_worldSize.x / 2.f;
        }
        else
        {
            if (cameraPos.x < halfWidth)
                cameraPos.x = halfWidth;
            if (cameraPos.x > m_worldSize.x - halfWidth)
                cameraPos.x = m_worldSize.x - halfWidth;
        }
        if (halfHeight * 2.f >= m_worldSize.y)
        {
            cameraPos.y = m_worldSize.y / 2.f;
        }
        else
        {
            if (cameraPos.y < halfHeight)
                cameraPos.y = halfHeight;
            if (cameraPos.y > m_worldSize.y - halfHeight)
                cameraPos.y = m_worldSize.y - halfHeight;
        }
        m_camera.setCenter(cameraPos);

        return;
    }

    // ========== ÁREA DE LA PIZARRA ==========
    m_cercaPizarra = m_player.getHurtbox().findIntersection(m_pizarraArea).has_value();

    static bool rQuizPresionado = false;
    if (m_cercaPizarra && !m_quizMinigame.isActive() && !m_poolMinigame.isActive())
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
        {
            if (!rQuizPresionado)
            {
                rQuizPresionado = true;
                m_quizMinigame.activate();
                std::cout << "Minijuego de preguntas activado!" << std::endl;
            }
        }
        else
        {
            rQuizPresionado = false;
        }
    }

    if (m_quizMinigame.isActive())
    {
        m_quizMinigame.update(dt);
        m_player.update(dt);

        // Cámara fija
        sf::Vector2f playerPos = m_player.getPosition();
        sf::Vector2f cameraPos = playerPos;
        float halfWidth = 1280.f / 2.f;
        float halfHeight = 720.f / 2.f;
        if (halfWidth * 2.f >= m_worldSize.x)
        {
            cameraPos.x = m_worldSize.x / 2.f;
        }
        else
        {
            if (cameraPos.x < halfWidth)
                cameraPos.x = halfWidth;
            if (cameraPos.x > m_worldSize.x - halfWidth)
                cameraPos.x = m_worldSize.x - halfWidth;
        }
        if (halfHeight * 2.f >= m_worldSize.y)
        {
            cameraPos.y = m_worldSize.y / 2.f;
        }
        else
        {
            if (cameraPos.y < halfHeight)
                cameraPos.y = halfHeight;
            if (cameraPos.y > m_worldSize.y - halfHeight)
                cameraPos.y = m_worldSize.y - halfHeight;
        }
        m_camera.setCenter(cameraPos);

        return;
    }

    // ========== ÁREA DE LA MESA DE COLORES ==========
    m_cercaMesaColorMix = m_player.getHurtbox().findIntersection(m_mesaColorMixArea).has_value();

    static bool rColorPresionado = false;
    if (m_cercaMesaColorMix && !m_colorMixMinigame.isActive() &&
        !m_poolMinigame.isActive() && !m_quizMinigame.isActive())
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
        {
            if (!rColorPresionado)
            {
                rColorPresionado = true;
                m_colorMixMinigame.activate();
                std::cout << "Minijuego de mezcla de colores activado!" << std::endl;
            }
        }
        else
        {
            rColorPresionado = false;
        }
    }

    if (m_colorMixMinigame.isActive())
    {
        m_colorMixMinigame.update(dt);
        m_player.update(dt);

        // Cámara fija
        sf::Vector2f playerPos = m_player.getPosition();
        sf::Vector2f cameraPos = playerPos;
        float halfWidth = 1280.f / 2.f;
        float halfHeight = 720.f / 2.f;
        if (halfWidth * 2.f >= m_worldSize.x)
        {
            cameraPos.x = m_worldSize.x / 2.f;
        }
        else
        {
            if (cameraPos.x < halfWidth)
                cameraPos.x = halfWidth;
            if (cameraPos.x > m_worldSize.x - halfWidth)
                cameraPos.x = m_worldSize.x - halfWidth;
        }
        if (halfHeight * 2.f >= m_worldSize.y)
        {
            cameraPos.y = m_worldSize.y / 2.f;
        }
        else
        {
            if (cameraPos.y < halfHeight)
                cameraPos.y = halfHeight;
            if (cameraPos.y > m_worldSize.y - halfHeight)
                cameraPos.y = m_worldSize.y - halfHeight;
        }
        m_camera.setCenter(cameraPos);

        return;
    }

    // ========== MOVIMIENTO ==========
    Inventory *inv = m_player.getInventory();
    if (!inv || !inv->isOpen())
    {
        sf::Vector2f movimiento(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            movimiento.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            movimiento.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            movimiento.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            movimiento.x += 1.f;

        if (movimiento.x != 0.f || movimiento.y != 0.f)
        {
            float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
            movimiento /= length;
        }

        m_player.move(movimiento, dt);
    }

    m_player.update(dt);

    // ========== COLISIONES ==========
    for (const auto &obj : m_mapaFisico)
    {
        if (m_player.getHurtbox().findIntersection(obj.getBounds()).has_value())
        {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }

    // ========== CÁMARA ==========
    sf::Vector2f playerPos = m_player.getPosition();
    sf::Vector2f cameraPos = playerPos;

    // Tamaño FIJO de la cámara (1280x720)
    float halfWidth = 1280.f / 2.f;
    float halfHeight = 720.f / 2.f;

    if (halfWidth * 2.f >= m_worldSize.x)
    {
        cameraPos.x = m_worldSize.x / 2.f;
    }
    else
    {
        if (cameraPos.x < halfWidth)
            cameraPos.x = halfWidth;
        if (cameraPos.x > m_worldSize.x - halfWidth)
            cameraPos.x = m_worldSize.x - halfWidth;
    }

    if (halfHeight * 2.f >= m_worldSize.y)
    {
        cameraPos.y = m_worldSize.y / 2.f;
    }
    else
    {
        if (cameraPos.y < halfHeight)
            cameraPos.y = halfHeight;
        if (cameraPos.y > m_worldSize.y - halfHeight)
            cameraPos.y = m_worldSize.y - halfHeight;
    }

    m_camera.setCenter(cameraPos);

        // Actualizar cercanía a la puerta SIEMPRE (independientemente de puzzles)
    m_cercaPuertaSalida = m_player.getHurtbox().findIntersection(m_puertaSalidaArea).has_value();
    
    verificarSalidaNivel();
    verificarEntradaCentinela();
    actualizarPuertaSalida(); 
    
     // Actualizar coordenadas debug SOLO si está en modo debug
    if (m_debugMode) {
        CoordenadasDebug::getInstance().actualizar(window, m_camera, m_player.getPosition());
    }

    // ========== PAUSA ==========
    if (!m_mostrarTutorial && !m_mostrarTutorialPorTecla)
    {
        static bool escapeProcesado_ = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
        {
            if (!escapeProcesado_)
            {
                escapeProcesado_ = true;
                game->pushState(std::make_unique<PauseState>(window, game));
            }
        }
        else
        {
            escapeProcesado_ = false;
        }
    }

    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
    {
        m_escapeConsumed = false;
    }
}

void Nivel1State::draw()
{
    if (!window)
        return;

    // ===== FASE 1: DIBUJAR MUNDO CON CÁMARA =====
    window->setView(m_camera);

    if (m_background)
    {
        window->draw(*m_background);
    }
    else
    {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(50, 30, 30));
        window->draw(fallback);
    }

    m_player.draw(*window);
    m_player.drawHurtbox(*window);

    // ===== DEBUG: Solo dibujar si m_debugMode está activo =====
    if (m_debugMode) {
        // Dibujar colisiones (ROJO)
        for (const auto &obj : m_mapaFisico)
        {
            sf::RectangleShape colision;
            colision.setPosition(sf::Vector2f(obj.getBounds().position.x, obj.getBounds().position.y));
            colision.setSize(sf::Vector2f(obj.getBounds().size.x, obj.getBounds().size.y));
            colision.setFillColor(sf::Color(255, 0, 0, 100));
            colision.setOutlineThickness(2.f);
            colision.setOutlineColor(sf::Color::Red);
            window->draw(colision);
        }

        // Dibujar área de PIZARRA (AZUL)
        sf::RectangleShape pizarraDebug(sf::Vector2f(m_pizarraArea.size.x, m_pizarraArea.size.y));
        pizarraDebug.setPosition(sf::Vector2f(m_pizarraArea.position.x, m_pizarraArea.position.y));
        pizarraDebug.setFillColor(sf::Color(0, 0, 255, 100));
        pizarraDebug.setOutlineThickness(3.f);
        pizarraDebug.setOutlineColor(sf::Color::Blue);
        window->draw(pizarraDebug);

        // Dibujar área de MESA DE POOL (VERDE)
        sf::RectangleShape mesaDebug(sf::Vector2f(m_mesaPoolArea.size.x, m_mesaPoolArea.size.y));
        mesaDebug.setPosition(sf::Vector2f(m_mesaPoolArea.position.x, m_mesaPoolArea.position.y));
        mesaDebug.setFillColor(sf::Color(0, 255, 0, 100));
        mesaDebug.setOutlineThickness(3.f);
        mesaDebug.setOutlineColor(sf::Color::Green);
        window->draw(mesaDebug);

        // Dibujar área de MESA DE COLORES (AMARILLO)
        sf::RectangleShape colorMixDebug(sf::Vector2f(m_mesaColorMixArea.size.x, m_mesaColorMixArea.size.y));
        colorMixDebug.setPosition(sf::Vector2f(m_mesaColorMixArea.position.x, m_mesaColorMixArea.position.y));
        colorMixDebug.setFillColor(sf::Color(255, 255, 0, 100));
        colorMixDebug.setOutlineThickness(3.f);
        colorMixDebug.setOutlineColor(sf::Color::Yellow);
        window->draw(colorMixDebug);

                // Dibujar PUERTA DE SALIDA (CIAN) - Solo en debug y solo si puzzles completados
        if (m_mostrarPuertaSalida && m_todosPuzzlesCompletados)
        {
            sf::RectangleShape salidaDebug(sf::Vector2f(m_puertaSalidaArea.size.x, m_puertaSalidaArea.size.y));
            salidaDebug.setPosition(sf::Vector2f(m_puertaSalidaArea.position.x, m_puertaSalidaArea.position.y));
            salidaDebug.setFillColor(sf::Color(0, 255, 255, 100));
            salidaDebug.setOutlineThickness(3.f);
            salidaDebug.setOutlineColor(sf::Color::Cyan);
            window->draw(salidaDebug);
        }
        else if (m_mostrarPuertaSalida && !m_todosPuzzlesCompletados)
        {
            // Dibujar en gris si no está activa aún (opcional, para debug)
            sf::RectangleShape salidaDebug(sf::Vector2f(m_puertaSalidaArea.size.x, m_puertaSalidaArea.size.y));
            salidaDebug.setPosition(sf::Vector2f(m_puertaSalidaArea.position.x, m_puertaSalidaArea.position.y));
            salidaDebug.setFillColor(sf::Color(100, 100, 100, 100));
            salidaDebug.setOutlineThickness(3.f);
            salidaDebug.setOutlineColor(sf::Color(100, 100, 100));
            window->draw(salidaDebug);
        }
    }

    // ===== FASE 2: DIBUJAR UI =====
    window->setView(window->getDefaultView());

    // Textos de interacción
    if (m_fontLoaded && m_textoInteraccion)
    {
        float winW = static_cast<float>(window->getSize().x);
        float winH = static_cast<float>(window->getSize().y);

        if (m_cercaMesaColorMix && !m_colorMixMinigame.isActive() &&
            !m_poolMinigame.isActive() && !m_quizMinigame.isActive())
        {
            m_textoInteraccion->setString("Presiona R para jugar a mezclar colores");
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 70.f));
            window->draw(*m_textoInteraccion);
        }

        if (m_cercaMesaPool && !m_poolMinigame.isActive())
        {
            m_textoInteraccion->setString("Presiona R para jugar al pool");
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 70.f));
            window->draw(*m_textoInteraccion);
        }

        if (m_cercaPizarra && !m_quizMinigame.isActive() && !m_poolMinigame.isActive())
        {
            m_textoInteraccion->setString("Presiona R para la leccion de matematicas");
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 70.f));
            window->draw(*m_textoInteraccion);
        }

               if (m_cercaPuertaSalida && !m_poolMinigame.isActive() && !m_quizMinigame.isActive() && !m_colorMixMinigame.isActive())
        {
            if (m_todosPuzzlesCompletados) {
                m_textoInteraccion->setString("Presiona E para avanzar al siguiente nivel");
            } else {
                m_textoInteraccion->setString("Completa todos los puzzles para poder salir");
            }
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 70.f));
            window->draw(*m_textoInteraccion);
        }
    }

    // Mensaje temporal
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f && !m_textoMensaje->getString().isEmpty())
    {
        sf::Vector2u winSize = window->getSize();
        m_textoMensaje->setPosition(sf::Vector2f(winSize.x / 2.f, winSize.y / 3.f));
        window->draw(*m_textoMensaje);
    }

    // Minijuegos
    if (m_colorMixMinigame.isActive())
    {
        m_colorMixMinigame.draw(*window);
    }
    if (m_quizMinigame.isActive())
    {
        m_quizMinigame.draw(*window);
    }
    if (m_poolMinigame.isActive())
    {
        m_poolMinigame.draw(*window);
    }

    // Tutorial
    if (m_mostrarTutorial || m_mostrarTutorialPorTecla)
    {
        sf::RectangleShape overlay(sf::Vector2f(window->getSize().x, window->getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);

        if (m_fontLoaded)
        {
            sf::Text tutorialText(m_font);
            tutorialText.setString(
                "DESPIERTA... ESTAS EN VIMORTE\n\n"
                "No hay salida simple. Para escapar de esta habitacion\n"
                "deberas superar los acertijos que esconde cada rincon.\n\n"
                "Observa bien: algunos caminos solo se abriran\n"
                "cuando demuestres tu habilidad.\n\n"
                "Empieza por la mesa de pool...\n\n"
                "[ESC] Cerrar | [M] ayuda");
            tutorialText.setCharacterSize(20);
            tutorialText.setFillColor(sf::Color::White);
            sf::FloatRect textBounds = tutorialText.getLocalBounds();
            tutorialText.setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            tutorialText.setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y / 2.f));
            window->draw(tutorialText);
        }
    }

    // Inventario
    Inventory *inv = m_player.getInventory();
    if (inv)
    {
        inv->draw(*window);
    }
    
    // Dibujar coordenadas debug SOLO si está en modo debug
    if (m_debugMode) {
        CoordenadasDebug::getInstance().dibujar(*window);
    }
    
    window->setView(window->getDefaultView());
    CoordenadasDebug::getInstance().dibujar(*window);
}

void Nivel1State::configurarColisiones()
{
    m_mapaFisico.clear();

    m_mapaFisico.emplace_back(30.f, 12.f, 1700.f, 130.f);
    m_mapaFisico.emplace_back(18.f, 12.f, 20.f, 1700.f);
    m_mapaFisico.emplace_back(570.f, 670.f, 20.f, 217.f);
    m_mapaFisico.emplace_back(570.f, 965.f, 20.f, 217.f);
    m_mapaFisico.emplace_back(570.f, 12.f, 22.f, 410.f);
    
    m_mapaFisico.emplace_back(677.f, 0.f, 53.f, 300.f);
       m_mapaFisico.emplace_back(820.f, 0.f, 61.f, 300.f);
          m_mapaFisico.emplace_back(730.f, 0.f, 90.f, 245.f);
    
    m_mapaFisico.emplace_back(0.f, 420.f, 315.f, 95.f);
    m_mapaFisico.emplace_back(400.f, 420.f, 520.f, 95.f);
	m_mapaFisico.emplace_back(1030.f, 420.f, 300.f, 95.f);
	
	m_mapaFisico.emplace_back(1254.f, 0.f, 100.f, 1400.f);
	m_mapaFisico.emplace_back(0.f, 1155.f, 1400.f, 95.f);
	
	m_mapaFisico.emplace_back(400.f, 420.f, 520.f, 95.f);
	
	m_mapaFisico.emplace_back(0.f, 675.f, 580.f, 160.f); 
	m_mapaFisico.emplace_back(0.f, 230.f, 120.f, 160.f);

	m_mapaFisico.emplace_back(180.f, 290.f, 91.f, 180.f);
	
	m_mapaFisico.emplace_back(60.f, 855.f, 140.f, 145.f);
	m_mapaFisico.emplace_back(0.f, 1030.f, 127.f, 145.f);
	m_mapaFisico.emplace_back(400.f, 965.f, 171.f, 153.f);
	
	m_mapaFisico.emplace_back(735.f, 670.f, 60.f, 220.f);
	m_mapaFisico.emplace_back(735.f, 965.f, 30.f, 220.f);
	
m_mapaFisico.emplace_back(915.f, 675.f, 380.f, 145.f);
m_mapaFisico.emplace_back(315.f, 466.f, 35.f, 100.f);

    std::cout << "Colisiones configuradas: " << m_mapaFisico.size() << " paredes" << std::endl;
}

void Nivel1State::jugadorHaMuerto()
{
    LevelNode *currentNode = game->getLevelTree().getCurrentNode();
    if (currentNode && currentNode->type == LevelType::CENTINELA)
    {
        GameProgressData &progress = game->getSaveManager().getCurrentProgress();
        game->getSaveManager().addMuerte();
        if (progress.modoElegido == GameProgressData::ModoJuego::CAMINO_AGRADABLE)
        {
            std::cout << "Muerte en centinela (modo agradable) - por implementar" << std::endl;
        }
        else
        {
            std::cout << "Muerte en centinela (modo consecuencias) - por implementar" << std::endl;
        }
    }
    else
    {
        game->pushState(std::make_unique<PauseState>(window, game));
    }
}

void Nivel1State::mostrarMensaje(const std::string &texto, float duracion, sf::Color color)
{
    if (!m_textoMensaje)
        return;

    m_msjActual.texto = texto;
    m_msjActual.tiempoRestante = duracion;
    m_msjActual.color = color;

    m_textoMensaje->setString(texto);
    m_textoMensaje->setFillColor(color);

    sf::FloatRect bounds = m_textoMensaje->getLocalBounds();
    m_textoMensaje->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));

    std::cout << "MENSAJE: " << texto << std::endl;
}

void Nivel1State::verificarEstadoMinijuegos()
{
    // Pool
    if (!m_poolCompletado && m_poolMinigame.isCompleted()) {
        m_poolCompletado = true;
        mostrarMensaje("¡Has completado el pool! Pista obtenida.", 3.0f, sf::Color::Green);
    }
    
    // Quiz
    if (!m_quizCompletado && m_quizMinigame.isCompleted()) {
        m_quizCompletado = true;
        mostrarMensaje("¡Has completado el quiz! Pista obtenida.", 3.0f, sf::Color::Green);
    }
    
    // ColorMix
    if (!m_colorMixCompletado && m_colorMixMinigame.isCompleted()) {
        m_colorMixCompletado = true;
        mostrarMensaje("¡Has completado la mezcla de colores! Pista obtenida.", 3.0f, sf::Color::Green);
    }
    
    if (m_poolCompletado && m_quizCompletado && m_colorMixCompletado && !m_todosPuzzlesCompletados) {
        m_todosPuzzlesCompletados = true;
        m_mostrarPuertaSalida = true;
        mostrarMensaje("¡Todos los puzzles completados! Dirigete a la salida.", 5.0f, sf::Color::Cyan);
    }
}

void Nivel1State::verificarPuzzlesCompletados()
{
    // Ya no hace falta, se hace en verificarEstadoMinijuegos
}

void Nivel1State::actualizarPuertaSalida()
{
    m_mostrarPuertaSalida = m_todosPuzzlesCompletados;
}