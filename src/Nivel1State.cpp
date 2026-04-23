#include "Nivel1State.hpp"
#include "PauseState.hpp"
#include "MuerteCentinelaState.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

Nivel1State::Nivel1State(sf::RenderWindow* window, Game* game) 
    : State(window, game), 
      m_background(nullptr), 
      m_cercaMesaPool(false), 
      m_textoInteraccion(nullptr),
      m_mostrarPuertaSalida(true), 
      m_cercaPuertaSalida(false), 
      m_escapeConsumed(false), 
      m_mostrarTutorial(false), 
      m_cercaMesaColorMix(false), 
      m_mostrarTutorialPorTecla(false), 
      m_esperandoSegundaE(false),
      m_msjActual()
{
    m_msjActual.texto = "";
    m_msjActual.tiempoRestante = 0.0f;
    m_msjActual.color = sf::Color::Yellow;

    m_player.loadAssets();
    m_player.setPosition(1150.f, 300.f);
    m_player.setSpeed(300.0f);
    
    // Verificar si es la primera vez para mostrar el tutorial
    if (game->tienePartidaActiva()) 
    {
        const auto& items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
        auto it = std::find(items.begin(), items.end(), "TutorialVisto");
        
        if (it == items.end()) {
            m_mostrarTutorial = true;
            game->getSaveManager().addItemRecolectado("TutorialVisto");
            std::cout << "Primer ingreso: Mostrando tutorial" << std::endl;
        }
    }
    
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel1/background.jpg")) {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x), 
                                    static_cast<float>(textureSize.y));
        std::cout << "Nivel 1 cargado. Tamaño: " << m_worldSize.x << "x" << m_worldSize.y << std::endl;
    } else {
        std::cerr << "Error: No se pudo cargar background.jpg" << std::endl;
        m_worldSize = sf::Vector2f(1754.f, 1587.f);
    }
    
    sf::Vector2u windowSize = window->getSize();
    m_camera = sf::View(sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f), 
                        sf::Vector2f(static_cast<float>(windowSize.x), 
                                     static_cast<float>(windowSize.y)));
    
    // ========== ÁREAS DE INTERACCIÓN ==========
    m_pizarraArea = sf::FloatRect(sf::Vector2f(200.f, 700.f), sf::Vector2f(180.f, 150.f));
    m_mesaColorMixArea = sf::FloatRect(sf::Vector2f(40.f, 280.f), sf::Vector2f(100.f, 120.f));
    m_mesaPoolArea = sf::FloatRect(sf::Vector2f(910.f, 900.f), sf::Vector2f(240.f, 120.f));
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(1550.f, 1350.f), sf::Vector2f(120.f, 180.f));
    
    configurarColisiones();
    
    m_poolMinigame.setSize(sf::Vector2f(800.f, 500.f));
    m_poolMinigame.setPosition(sf::Vector2f(
        (windowSize.x - 1000.f) / 2.f,
        (windowSize.y - 500.f - 70.f) / 2.f - 15.f
    ));
    m_colorMixMinigame.initUI();
    
    m_quizMinigame.setSize(sf::Vector2f(900.f, 600.f));
    m_quizMinigame.setPosition(sf::Vector2f(
        (windowSize.x - 900.f) / 2.f,
        (windowSize.y - 600.f) / 2.f
    ));
    
    // Cargar fuente y crear textos
    if (m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setString("Presiona R para jugar al pool");
        m_textoInteraccion->setCharacterSize(20);
        m_textoInteraccion->setFillColor(sf::Color::White);
        m_textoInteraccion->setPosition(sf::Vector2f(windowSize.x / 2.f, windowSize.y - 70.f));
        
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        
        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(24);
        m_textoMensaje->setFillColor(sf::Color::Yellow);
    }
    
    m_colorMixMinigame.setSize(sf::Vector2f(900.f, 600.f));
    m_colorMixMinigame.setPosition(sf::Vector2f((windowSize.x - 900.f) / 2.f, (windowSize.y - 600.f) / 2.f));
    
    // ========== GUARDADO AUTOMÁTICO ==========
    if (game->tienePartidaActiva()) {
        game->getSaveManager().setNivelActual(1, 1);
        game->guardarPartidaActual();
        std::cout << "Partida guardada automáticamente en Nivel 1" << std::endl;
    }
    
    std::cout << "Nivel1State inicializado correctamente" << std::endl;
}

void Nivel1State::handleEvent(const sf::Event& event) 
{
    // Manejar teclas globales (Escape y M)
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Escape) {
            if (m_mostrarTutorial || m_mostrarTutorialPorTecla) {
                m_mostrarTutorial = false;
                m_mostrarTutorialPorTecla = false;
                m_escapeConsumed = true;
                return;
            }
        }
        
        if (keyPressed->code == sf::Keyboard::Key::M) {
            std::cout << "M presionada - Activando tutorial" << std::endl;
            if (game->tienePartidaActiva()) {
                const auto& items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
                auto it = std::find(items.begin(), items.end(), "TutorialVisto");
                if (it != items.end()) {
                    m_mostrarTutorialPorTecla = true;
                } else {
                    m_mostrarTutorial = true;
                }
            } else {
                m_mostrarTutorialPorTecla = true;
            }
        }
    }

    // Manejar eventos de minijuegos
    if (m_poolMinigame.isActive()) {
        m_poolMinigame.handleEvent(event, *window);
        if (event.is<sf::Event::KeyPressed>()) {
            const auto& keyEvent = event.getIf<sf::Event::KeyPressed>();
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                m_poolMinigame.deactivate();
                std::cout << "Minijuego de pool cerrado" << std::endl;
                m_escapeConsumed = true;
                return;
            }
        }
    }
    else if (m_quizMinigame.isActive()) {
        m_quizMinigame.handleEvent(event, *window);
        if (event.is<sf::Event::KeyPressed>()) {
            const auto& keyEvent = event.getIf<sf::Event::KeyPressed>();
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                m_quizMinigame.deactivate();
                m_escapeConsumed = true;
                return;
            }
        }
    }
    else if (m_colorMixMinigame.isActive()) {
        m_colorMixMinigame.handleEvent(event, *window);
        if (event.is<sf::Event::KeyPressed>()) {
            const auto& keyEvent = event.getIf<sf::Event::KeyPressed>();
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                m_colorMixMinigame.deactivate();
                std::cout << "Minijuego de colores cerrado" << std::endl;
                m_escapeConsumed = true;
                return;
            }
        }
    }
    
    // Manejar eventos del inventario
    Inventory* inv = m_player.getInventory();
    if (inv) {
        inv->handleEvent(event, *window);
    }
}

void Nivel1State::verificarEntradaCentinela() {
    LevelNode* currentNode = game->getLevelTree().getCurrentNode();
    if (currentNode && currentNode->hasCentinela()) {
    }
}

void Nivel1State::verificarSalidaNivel() 
{
    m_cercaPuertaSalida = m_player.getHurtbox().findIntersection(m_puertaSalidaArea).has_value();
    
    static bool ePresionado = false;
    if (m_cercaPuertaSalida && !m_poolMinigame.isActive() && !m_quizMinigame.isActive()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
            if (!ePresionado) {
                ePresionado = true;
                std::cout << "Saliendo del nivel..." << std::endl;
                game->avanzarNivel();
            }
        } else {
            ePresionado = false;
        }
    }
}

void Nivel1State::update(float dt) 
{
    // Actualizar mensaje temporal
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f) {
        m_msjActual.tiempoRestante -= dt;
        if (m_msjActual.tiempoRestante <= 0.0f) {
            m_textoMensaje->setString("");
        }
    }
    
    sf::Vector2f posAnterior = m_player.getPosition();
    sf::Vector2u windowSize = window->getSize();
    
    // ========== ÁREA DE LA MESA DE POOL ==========
    m_cercaMesaPool = m_player.getBounds().findIntersection(m_mesaPoolArea).has_value();

    static bool rPresionado = false;
    if (m_cercaMesaPool && !m_poolMinigame.isActive())
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            if (!rPresionado) {
                rPresionado = true;
                m_poolMinigame.activate();
                std::cout << "Minijuego de pool activado!" << std::endl;
            }
        } else {
            rPresionado = false;
        }
    }
    
    if (m_poolMinigame.isActive()) {
        m_poolMinigame.update(dt);
        m_player.update(dt);
        
        sf::Vector2f playerPos = m_player.getPosition();
        sf::Vector2f cameraPos = playerPos;
        float halfWidth = static_cast<float>(windowSize.x) / 2.f;
        float halfHeight = static_cast<float>(windowSize.y) / 2.f;
        cameraPos.x = std::clamp(cameraPos.x, halfWidth, m_worldSize.x - halfWidth);
        cameraPos.y = std::clamp(cameraPos.y, halfHeight, m_worldSize.y - halfHeight);
        m_camera.setCenter(cameraPos);
        
        return;
    }

    // ========== ÁREA DE LA PIZARRA ==========
    m_cercaPizarra = m_player.getHurtbox().findIntersection(m_pizarraArea).has_value();

    static bool rQuizPresionado = false;
    if (m_cercaPizarra && !m_quizMinigame.isActive() && !m_poolMinigame.isActive()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            if (!rQuizPresionado) {
                rQuizPresionado = true;
                m_quizMinigame.activate();
                std::cout << "Minijuego de preguntas activado!" << std::endl;
            }
        } else {
            rQuizPresionado = false;
        }
    }

    if (m_quizMinigame.isActive()) {
        m_quizMinigame.update(dt);
        m_player.update(dt);
    
        sf::Vector2f playerPos = m_player.getPosition();
        sf::Vector2f cameraPos = playerPos;
        float halfWidth = static_cast<float>(windowSize.x) / 2.f;
        float halfHeight = static_cast<float>(windowSize.y) / 2.f;
        cameraPos.x = std::clamp(cameraPos.x, halfWidth, m_worldSize.x - halfWidth);
        cameraPos.y = std::clamp(cameraPos.y, halfHeight, m_worldSize.y - halfHeight);
        m_camera.setCenter(cameraPos);
    
        return;
    }

    // ========== ÁREA DE LA MESA DE COLORES ==========
    m_cercaMesaColorMix = m_player.getHurtbox().findIntersection(m_mesaColorMixArea).has_value();
    
    static bool rColorPresionado = false;
    if (m_cercaMesaColorMix && !m_colorMixMinigame.isActive() && 
        !m_poolMinigame.isActive() && !m_quizMinigame.isActive()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            if (!rColorPresionado) {
                rColorPresionado = true;
                m_colorMixMinigame.activate();
                std::cout << "Minijuego de mezcla de colores activado!" << std::endl;
            }
        } else {
            rColorPresionado = false;
        }
    }
    
    if (m_colorMixMinigame.isActive()) {
        m_colorMixMinigame.update(dt);
        m_player.update(dt);
        
        sf::Vector2f playerPos = m_player.getPosition();
        sf::Vector2f cameraPos = playerPos;
        float halfWidth = static_cast<float>(windowSize.x) / 2.f;
        float halfHeight = static_cast<float>(windowSize.y) / 2.f;
        cameraPos.x = std::clamp(cameraPos.x, halfWidth, m_worldSize.x - halfWidth);
        cameraPos.y = std::clamp(cameraPos.y, halfHeight, m_worldSize.y - halfHeight);
        m_camera.setCenter(cameraPos);
        
        return;
    }

    // ========== MOVIMIENTO ==========
    Inventory* inv = m_player.getInventory();
    if (!inv || !inv->isOpen()) {
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
    }
    
    m_player.update(dt);
    
    // ========== COLISIONES ==========
    for (const auto& obj : m_mapaFisico) {
        if (m_player.getHurtbox().findIntersection(obj.getBounds()).has_value()) {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }
    
    // ========== CÁMARA ==========
    sf::Vector2f playerPos = m_player.getPosition();
    sf::Vector2f cameraPos = playerPos;
    float halfWidth = static_cast<float>(windowSize.x) / 2.f;
    float halfHeight = static_cast<float>(windowSize.y) / 2.f;
    cameraPos.x = std::clamp(cameraPos.x, halfWidth, m_worldSize.x - halfWidth);
    cameraPos.y = std::clamp(cameraPos.y, halfHeight, m_worldSize.y - halfHeight);
    m_camera.setCenter(cameraPos);
    
    verificarSalidaNivel();
    verificarEntradaCentinela();
    
    // ========== PAUSA ==========
    if (!m_mostrarTutorial && !m_mostrarTutorialPorTecla && !m_escapeConsumed) {
        static bool escapeProcesado_ = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
            if (!escapeProcesado_) {
                escapeProcesado_ = true;
                game->pushState(std::make_unique<PauseState>(window, game));
            }
        } else {
            escapeProcesado_ = false;
        }
    }

    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        m_escapeConsumed = false;
    }
}

void Nivel1State::draw()
{
    if (!window) return;

    // ===== FASE 1: DIBUJAR MUNDO CON CÁMARA =====
    window->setView(m_camera);
    
    if (m_background) {
        window->draw(*m_background);
    } else {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(50, 30, 30));
        window->draw(fallback);
    }
    
    m_player.draw(*window);
    m_player.drawHurtbox(*window);
    
    // Debug: dibujar colisiones
    for (const auto& obj : m_mapaFisico) {
        sf::RectangleShape colision;
        colision.setPosition(sf::Vector2f(obj.getBounds().position.x, obj.getBounds().position.y));
        colision.setSize(sf::Vector2f(obj.getBounds().size.x, obj.getBounds().size.y));
        colision.setFillColor(sf::Color(255, 0, 0, 100));
        colision.setOutlineThickness(2.f);
        colision.setOutlineColor(sf::Color::Red);
        window->draw(colision);
    }

    // Debug: ÁREAS DE INTERACCIÓN
    sf::RectangleShape pizarraDebug(sf::Vector2f(m_pizarraArea.size.x, m_pizarraArea.size.y));
    pizarraDebug.setPosition(sf::Vector2f(m_pizarraArea.position.x, m_pizarraArea.position.y));
    pizarraDebug.setFillColor(sf::Color(0, 0, 255, 100));
    pizarraDebug.setOutlineThickness(3.f);
    pizarraDebug.setOutlineColor(sf::Color::Blue);
    window->draw(pizarraDebug);
    
    sf::RectangleShape mesaDebug(sf::Vector2f(m_mesaPoolArea.size.x, m_mesaPoolArea.size.y));
    mesaDebug.setPosition(sf::Vector2f(m_mesaPoolArea.position.x, m_mesaPoolArea.position.y));
    mesaDebug.setFillColor(sf::Color(0, 255, 0, 100));
    mesaDebug.setOutlineThickness(3.f);
    mesaDebug.setOutlineColor(sf::Color::Green);
    window->draw(mesaDebug);

    sf::RectangleShape colorMixDebug(sf::Vector2f(m_mesaColorMixArea.size.x, m_mesaColorMixArea.size.y));
    colorMixDebug.setPosition(sf::Vector2f(m_mesaColorMixArea.position.x, m_mesaColorMixArea.position.y));
    colorMixDebug.setFillColor(sf::Color(255, 255, 0, 100));
    colorMixDebug.setOutlineThickness(3.f);
    colorMixDebug.setOutlineColor(sf::Color::Yellow);
    window->draw(colorMixDebug);
    
    if (m_mostrarPuertaSalida) {
        sf::RectangleShape salidaDebug(sf::Vector2f(m_puertaSalidaArea.size.x, m_puertaSalidaArea.size.y));
        salidaDebug.setPosition(sf::Vector2f(m_puertaSalidaArea.position.x, m_puertaSalidaArea.position.y));
        salidaDebug.setFillColor(sf::Color(0, 255, 0, 50));
        salidaDebug.setOutlineThickness(3.f);
        salidaDebug.setOutlineColor(sf::Color::Green);
        window->draw(salidaDebug);
    }
    
    // ===== FASE 2: DIBUJAR UI =====
    window->setView(window->getDefaultView());
    
    // Textos de interacción
    if (m_cercaMesaColorMix && !m_colorMixMinigame.isActive() && 
        !m_poolMinigame.isActive() && !m_quizMinigame.isActive() && m_textoInteraccion) {
        m_textoInteraccion->setString("Presiona R para jugar a mezclar colores");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }
    
    if (m_cercaMesaPool && !m_poolMinigame.isActive() && m_textoInteraccion) {
        m_textoInteraccion->setString("Presiona R para jugar al pool");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    if (m_cercaPizarra && !m_quizMinigame.isActive() && !m_poolMinigame.isActive() && m_textoInteraccion) {
        m_textoInteraccion->setString("Presiona R para la leccion de matematicas");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }
    
    if (m_cercaPuertaSalida && !m_poolMinigame.isActive() && !m_quizMinigame.isActive() && m_textoInteraccion) {
        m_textoInteraccion->setString("Presiona E para avanzar al siguiente nivel");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }
    
    // Mensaje temporal
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f && !m_textoMensaje->getString().isEmpty()) {
        sf::Vector2u windowSize = window->getSize();
        m_textoMensaje->setPosition(sf::Vector2f(windowSize.x / 2.f, windowSize.y / 3.f));
        window->draw(*m_textoMensaje);
    }
    
    // Minijuegos
    if (m_colorMixMinigame.isActive()) {
        m_colorMixMinigame.draw(*window);
    }
    if (m_quizMinigame.isActive()) {
        m_quizMinigame.draw(*window);
    }
    if (m_poolMinigame.isActive()) {
        m_poolMinigame.draw(*window);
    }
    
    // Tutorial
    if (m_mostrarTutorial || m_mostrarTutorialPorTecla) {
        sf::RectangleShape overlay(sf::Vector2f(window->getSize().x, window->getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);
        
        sf::Text tutorialText(m_font);
        tutorialText.setString(
            "DESPIERTA... ESTAS EN VIMORTE\n\n"
            "No hay salida simple. Para escapar de esta habitacion\n"
            "deberas superar los acertijos que esconde cada rincon.\n\n"
            "Observa bien: algunos caminos solo se abriran\n"
            "cuando demuestres tu habilidad.\n\n"
            "Empieza por la mesa de pool...\n\n"
            "[ESC] Cerrar | [M] ayuda"
        );
        tutorialText.setCharacterSize(20);
        tutorialText.setFillColor(sf::Color::White);
        sf::FloatRect textBounds = tutorialText.getLocalBounds();
        tutorialText.setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        tutorialText.setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y / 2.f));
        window->draw(tutorialText);
    }
    
    // Inventario
    Inventory* inv = m_player.getInventory();
    if (inv) {
        inv->draw(*window);
    }
}

void Nivel1State::configurarColisiones() 
{
    m_mapaFisico.clear();
    
    m_mapaFisico.emplace_back(30.f, 12.f, 1700.f, 130.f);
    m_mapaFisico.emplace_back(18.f, 12.f, 20.f, 1700.f);
    m_mapaFisico.emplace_back(570.f, 690.f, 20.f, 100.f);
    m_mapaFisico.emplace_back(570.f, 12.f, 22.f, 410.f);
   
    std::cout << "Colisiones configuradas: " << m_mapaFisico.size() << " paredes" << std::endl;
}

void Nivel1State::jugadorHaMuerto() {
    LevelNode* currentNode = game->getLevelTree().getCurrentNode();
    if (currentNode && currentNode->type == LevelType::CENTINELA) {
        GameProgressData& progress = game->getSaveManager().getCurrentProgress();
        game->getSaveManager().addMuerte();
        if (progress.modoElegido == GameProgressData::ModoJuego::CAMINO_AGRADABLE) {
            std::cout << "Muerte en centinela (modo agradable) - por implementar" << std::endl;
        } else {
            std::cout << "Muerte en centinela (modo consecuencias) - por implementar" << std::endl;
        }
    } else {
        game->pushState(std::make_unique<PauseState>(window, game));
    }
}

void Nivel1State::mostrarMensaje(const std::string& texto, float duracion, sf::Color color) {
    if (!m_textoMensaje) return;
    
    m_msjActual.texto = texto;
    m_msjActual.tiempoRestante = duracion;
    m_msjActual.color = color;
    
    m_textoMensaje->setString(texto);
    m_textoMensaje->setFillColor(color);
    
    sf::FloatRect bounds = m_textoMensaje->getLocalBounds();
    m_textoMensaje->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    
    std::cout << "MENSAJE: " << texto << std::endl;
}