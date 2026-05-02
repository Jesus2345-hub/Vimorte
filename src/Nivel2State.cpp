#include "Nivel2State.hpp"
#include "PauseState.hpp"
#include "Game.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

Nivel2State::Nivel2State(sf::RenderWindow* window, Game* game)
    : State(window, game),
      m_cercaBar(false),
      m_cercaRuleta(false),
      m_cercaSlotMachine(false),
      m_cercaBlackjack(false),
      m_cercaPuertaSalida(false),
      m_cercaVendedor(false),
      m_dinero(10),
      m_tieneLlave(false),
      m_fontLoaded(false)
{
    m_msjActual.texto = "";
    m_msjActual.tiempoRestante = 0.0f;
    m_msjActual.color = sf::Color::Yellow;
    
    // Cargar jugador
    m_player.loadAssets();
    m_player.setPosition(400.f, 600.f);
    m_player.setSpeed(300.0f);
    
    // Tutorial
    if (game->tienePartidaActiva()) {
        const auto& items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
        auto it = std::find(items.begin(), items.end(), "TutorialNivel2Visto");
        if (it == items.end()) {
            m_mostrarTutorial = true;
            game->getSaveManager().addItemRecolectado("TutorialNivel2Visto");
        }
    }
    
    // Cargar fondo
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel2/background.jpg")) {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x),
                                    static_cast<float>(textureSize.y));
    } else {
        m_worldSize = sf::Vector2f(2000.f, 1500.f);
        m_background = nullptr;
    }
    
    // Cámara fija 1280x720
    sf::Vector2u windowSize = window->getSize();
    m_camera = sf::View(
        sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
        sf::Vector2f(1280.f, 720.f)
    );
    m_lastWindowSize = windowSize;
    
    // Áreas de interacción
    m_barArea = sf::FloatRect(sf::Vector2f(100.f, 100.f), sf::Vector2f(200.f, 150.f));
    m_ruletaArea = sf::FloatRect(sf::Vector2f(600.f, 200.f), sf::Vector2f(300.f, 200.f));
    m_slotMachineArea = sf::FloatRect(sf::Vector2f(1000.f, 100.f), sf::Vector2f(150.f, 150.f));
    m_blackjackArea = sf::FloatRect(sf::Vector2f(600.f, 500.f), sf::Vector2f(200.f, 150.f));
    m_vendedorArea = sf::FloatRect(sf::Vector2f(1400.f, 600.f), sf::Vector2f(150.f, 150.f));
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(1800.f, 1200.f), sf::Vector2f(120.f, 180.f));
    
    configurarColisiones();
    
    // ========== CONFIGURAR MINIJUEGOS ==========
    float minijuegoW = windowSize.x * 0.8f;
    float minijuegoH = windowSize.y * 0.8f;
    float minijuegoX = (windowSize.x - minijuegoW) / 2.f;
    float minijuegoY = (windowSize.y - minijuegoH) / 2.f;
    
    // Ruleta
    m_ruletaMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
    m_ruletaMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    m_ruletaMinigame.setDineroJugador(&m_dinero);
    
    // Bartender
    m_bartenderMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
    m_bartenderMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    m_bartenderMinigame.setDineroJugador(&m_dinero);
    
    // Fuente
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (m_fontLoaded) {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setCharacterSize(18);
        m_textoInteraccion->setFillColor(sf::Color::White);
        
        m_textoDinero = std::make_unique<sf::Text>(m_font);
        m_textoDinero->setCharacterSize(24);
        m_textoDinero->setFillColor(sf::Color::Yellow);
        m_textoDinero->setString("$" + std::to_string(m_dinero));
        
        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(20);
        m_textoMensaje->setFillColor(sf::Color::Yellow);
    }
    
    // Guardado automático
    if (game->tienePartidaActiva()) {
        game->getSaveManager().setNivelActual(2, 2);
        game->guardarPartidaActual();
    }
    
    game->setIsInLevel(true);
}


void Nivel2State::configurarColisiones() {
    m_mapaFisico.clear();
    
    // Paredes básicas (ajustar según el fondo)
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 0.f), sf::Vector2f(m_worldSize.x, 50.f));       // Norte
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 0.f), sf::Vector2f(50.f, m_worldSize.y));       // Oeste
    m_mapaFisico.emplace_back(sf::Vector2f(m_worldSize.x - 50.f, 0.f), sf::Vector2f(50.f, m_worldSize.y)); // Este
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, m_worldSize.y - 50.f), sf::Vector2f(m_worldSize.x, 50.f)); // Sur
    
    // Barras del bar
    m_mapaFisico.emplace_back(sf::Vector2f(50.f, 250.f), sf::Vector2f(350.f, 30.f));
    
    // Mesas
    m_mapaFisico.emplace_back(sf::Vector2f(550.f, 350.f), sf::Vector2f(300.f, 200.f));
    m_mapaFisico.emplace_back(sf::Vector2f(550.f, 650.f), sf::Vector2f(250.f, 150.f));
}

void Nivel2State::handleEvent(const sf::Event& event) {
    // Tutorial
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
                auto it = std::find(items.begin(), items.end(), "TutorialNivel2Visto");
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
    
    // Minijuego de bartender
    if (m_bartenderMinigame.isActive()) {
        m_bartenderMinigame.handleEvent(event, *window);
        if (event.is<sf::Event::KeyPressed>()) {
            const auto& keyEvent = event.getIf<sf::Event::KeyPressed>();
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                m_bartenderMinigame.deactivate();
                m_escapeConsumed = true;
                return;
            }
        }
        return;
    }
    
    // Minijuego de ruleta
    if (m_ruletaMinigame.isActive()) {
        m_ruletaMinigame.handleEvent(event, *window);
        if (event.is<sf::Event::KeyPressed>()) {
            const auto& keyEvent = event.getIf<sf::Event::KeyPressed>();
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                m_ruletaMinigame.deactivate();
                m_escapeConsumed = true;
                return;
            }
        }
        return;
    }
    
    // Inventario
    Inventory* inv = m_player.getInventory();
    if (inv) {
        inv->handleEvent(event, *window);
    }
}

void Nivel2State::verificarSalidaNivel() {
    m_cercaPuertaSalida = m_player.getHurtbox().findIntersection(m_puertaSalidaArea).has_value();
    
    static bool ePresionado = false;
    if (m_cercaPuertaSalida && !m_ruletaMinigame.isActive()) {
        if (m_tieneLlave) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
                if (!ePresionado) {
                    ePresionado = true;
                    std::cout << "Saliendo del nivel 2..." << std::endl;
                    game->avanzarNivel();
                }
            } else {
                ePresionado = false;
            }
        }
    }
}

void Nivel2State::update(float dt) {
    // Actualizar tamaño de los minijuegos si cambia la ventana
    sf::Vector2u currentSize = window->getSize();
    static sf::Vector2u lastSize = currentSize;
    if (currentSize != lastSize) {
        lastSize = currentSize;
        float minijuegoW = currentSize.x * 0.8f;
        float minijuegoH = currentSize.y * 0.8f;
        float minijuegoX = (currentSize.x - minijuegoW) / 2.f;
        float minijuegoY = (currentSize.y - minijuegoH) / 2.f;
        
        m_ruletaMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
        m_ruletaMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
        
        m_bartenderMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
        m_bartenderMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    }

    // Mensaje temporal
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f) {
        m_msjActual.tiempoRestante -= dt;
        if (m_msjActual.tiempoRestante <= 0.0f) {
            m_textoMensaje->setString("");
        }
    }
    
    // Actualizar texto de dinero
    if (m_textoDinero) {
        m_textoDinero->setString("$" + std::to_string(m_dinero));
        if (m_tieneLlave) {
            m_textoDinero->setString(m_textoDinero->getString() + " [LLAVE]");
        }
    }
    
    sf::Vector2f posAnterior = m_player.getPosition();
    
    // Verificar cercanía a áreas
    m_cercaRuleta = m_player.getHurtbox().findIntersection(m_ruletaArea).has_value();
    m_cercaBar = m_player.getHurtbox().findIntersection(m_barArea).has_value();
    m_cercaVendedor = m_player.getHurtbox().findIntersection(m_vendedorArea).has_value();
    
    // ========== BARTENDER ACTIVO ==========
    if (m_bartenderMinigame.isActive()) {
        m_bartenderMinigame.update(dt);
        m_player.update(dt);
        
        sf::Vector2f playerPos = m_player.getPosition();
        sf::Vector2f cameraPos = playerPos;
        float halfWidth = 1280.f / 2.f;
        float halfHeight = 720.f / 2.f;
        cameraPos.x = std::clamp(cameraPos.x, halfWidth, m_worldSize.x - halfWidth);
        cameraPos.y = std::clamp(cameraPos.y, halfHeight, m_worldSize.y - halfHeight);
        m_camera.setCenter(cameraPos);
        return;
    }
    
    // ========== RULETA ACTIVA ==========
    if (m_ruletaMinigame.isActive()) {
        m_ruletaMinigame.update(dt);
        m_player.update(dt);
        
        sf::Vector2f playerPos = m_player.getPosition();
        sf::Vector2f cameraPos = playerPos;
        float halfWidth = 1280.f / 2.f;
        float halfHeight = 720.f / 2.f;
        cameraPos.x = std::clamp(cameraPos.x, halfWidth, m_worldSize.x - halfWidth);
        cameraPos.y = std::clamp(cameraPos.y, halfHeight, m_worldSize.y - halfHeight);
        m_camera.setCenter(cameraPos);
        return;
    }
    
    // Activar ruleta
    static bool rPresionado = false;
    if (m_cercaRuleta) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            if (!rPresionado) {
                rPresionado = true;
                m_ruletaMinigame.activate();
                std::cout << "Minijuego de ruleta activado!" << std::endl;
            }
        } else {
            rPresionado = false;
        }
    }
    
    // Activar bartender
    static bool rBartenderPresionado = false;
    if (m_cercaBar) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            if (!rBartenderPresionado) {
                rBartenderPresionado = true;
                m_bartenderMinigame.activate();
                std::cout << "Bartender activado!" << std::endl;
            }
        } else {
            rBartenderPresionado = false;
        }
    }
    
    // Comprar llave
    static bool ePresionadoVendedor = false;
    if (m_cercaVendedor && !m_tieneLlave) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
            if (!ePresionadoVendedor) {
                ePresionadoVendedor = true;
                if (m_dinero >= 100) {
                    m_dinero -= 100;
                    m_tieneLlave = true;
                    mostrarMensaje("Has comprado la llave! Ve a la puerta de salida.", 3.0f, sf::Color::Green);
                } else {
                    mostrarMensaje("Necesitas $100 para comprar la llave. Tienes $" + std::to_string(m_dinero), 2.0f, sf::Color::Red);
                }
            }
        } else {
            ePresionadoVendedor = false;
        }
    }
    
    // Movimiento
    sf::Vector2f movimiento(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        movimiento.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        movimiento.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        movimiento.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        movimiento.x += 1.f;
    
    if (movimiento.x != 0.f || movimiento.y != 0.f) {
        float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
        movimiento /= length;
    }
    
    m_player.move(movimiento, dt);
    m_player.update(dt);
    
    // Colisiones
    for (const auto& obj : m_mapaFisico) {
        if (m_player.getHurtbox().findIntersection(obj).has_value()) {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }
    
    // Cámara
    sf::Vector2f playerPos = m_player.getPosition();
    sf::Vector2f cameraPos = playerPos;
    float halfWidth = 1280.f / 2.f;
    float halfHeight = 720.f / 2.f;
    cameraPos.x = std::clamp(cameraPos.x, halfWidth, m_worldSize.x - halfWidth);
    cameraPos.y = std::clamp(cameraPos.y, halfHeight, m_worldSize.y - halfHeight);
    m_camera.setCenter(cameraPos);
    
    verificarSalidaNivel();
    
    // Pausa
    if (!m_mostrarTutorial && !m_mostrarTutorialPorTecla && !m_escapeConsumed) {
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
    
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        m_escapeConsumed = false;
    }
}

void Nivel2State::draw() {
    if (!window) return;
    
    // Vista del mundo
    window->setView(m_camera);
    
    if (m_background) {
        window->draw(*m_background);
    } else {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(50, 30, 30));
        window->draw(fallback);
    }
    
    m_player.draw(*window);
    
    // Debug: colisiones
    for (const auto& obj : m_mapaFisico) {
        sf::RectangleShape colision;
        colision.setPosition(sf::Vector2f(obj.position.x, obj.position.y));
        colision.setSize(sf::Vector2f(obj.size.x, obj.size.y));
        colision.setFillColor(sf::Color(255, 0, 0, 80));
        window->draw(colision);
    }
    
    // Debug: áreas
    auto drawArea = [&](const sf::FloatRect& area, const sf::Color& color) {
        sf::RectangleShape shape(sf::Vector2f(area.size.x, area.size.y));
        shape.setPosition(sf::Vector2f(area.position.x, area.position.y));
        shape.setFillColor(sf::Color(color.r, color.g, color.b, 80));
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(color);
        window->draw(shape);
    };
    
    drawArea(m_ruletaArea, sf::Color::Green);
    drawArea(m_barArea, sf::Color::Blue);
    drawArea(m_vendedorArea, sf::Color::Yellow);
    drawArea(m_puertaSalidaArea, m_tieneLlave ? sf::Color::Green : sf::Color::Red);
    
    // Vista UI
    window->setView(window->getDefaultView());
    
    // Texto de dinero
    if (m_textoDinero) {
        m_textoDinero->setPosition(sf::Vector2f(20.f, 20.f));
        window->draw(*m_textoDinero);
    }
    
    // Textos de interacción
    if (m_fontLoaded && m_textoInteraccion) {
        float winW = static_cast<float>(window->getSize().x);
        float winH = static_cast<float>(window->getSize().y);
        
        auto drawInteractionText = [&](const std::string& texto) {
            m_textoInteraccion->setString(texto);
            sf::FloatRect bounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 60.f));
            window->draw(*m_textoInteraccion);
        };
        
        if (m_cercaRuleta && !m_ruletaMinigame.isActive() && !m_bartenderMinigame.isActive()) {
            drawInteractionText("Presiona R para jugar a la ruleta");
        }
        
        if (m_cercaBar && !m_bartenderMinigame.isActive() && !m_ruletaMinigame.isActive()) {
            drawInteractionText("Presiona R para trabajar de bartender");
        }
        
        if (m_cercaVendedor && !m_tieneLlave) {
            drawInteractionText("Presiona E para comprar la llave ($100)");
        }
        
        if (m_cercaPuertaSalida) {
            if (m_tieneLlave) {
                drawInteractionText("Presiona E para usar la llave y salir");
            } else {
                drawInteractionText("Necesitas una llave para salir. Cuestan $100.");
            }
        }
    }
    
    // Mensaje temporal
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f && !m_textoMensaje->getString().isEmpty()) {
        sf::Vector2u winSize = window->getSize();
        sf::FloatRect bounds = m_textoMensaje->getLocalBounds();
        m_textoMensaje->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_textoMensaje->setPosition(sf::Vector2f(winSize.x / 2.f, winSize.y / 3.f));
        window->draw(*m_textoMensaje);
    }
    
    // ========== MINIJUEGOS ==========
    if (m_bartenderMinigame.isActive()) {
        m_bartenderMinigame.draw(*window);
    }
    
    if (m_ruletaMinigame.isActive()) {
        m_ruletaMinigame.draw(*window);
    }
    
    // Inventario
    Inventory* inv = m_player.getInventory();
    if (inv) {
        inv->draw(*window);
    }
    
    // Tutorial
    if (m_mostrarTutorial || m_mostrarTutorialPorTecla) {
        sf::RectangleShape overlay(sf::Vector2f(window->getSize().x, window->getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);
        
        if (m_fontLoaded) {
            sf::Text tutorialText(m_font);
            tutorialText.setString(
                "BIENVENIDO AL CASINO VIMORTE\n\n"
                "Necesitas $100 para comprar la llave y escapar.\n\n"
                "FORMAS DE GANAR DINERO:\n"
                "- Trabaja de bartender ($1 por bebida, seguro)\n"
                "- Juega a la ruleta (apuesta y multiplica tu dinero)\n"
                "- Prueba suerte en el tragaperras\n"
                "- Juega al blackjack\n\n"
                "CUIDADO: Puedes perder todo tu dinero si apuestas mal.\n\n"
                "[ESC] Cerrar | [M] ayuda"
            );
            tutorialText.setCharacterSize(20);
            tutorialText.setFillColor(sf::Color::White);
            sf::FloatRect textBounds = tutorialText.getLocalBounds();
            tutorialText.setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            tutorialText.setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y / 2.f));
            window->draw(tutorialText);
        }
    }
}
void Nivel2State::mostrarMensaje(const std::string& texto, float duracion, sf::Color color) {
    if (!m_textoMensaje) return;
    
    m_msjActual.texto = texto;
    m_msjActual.tiempoRestante = duracion;
    m_msjActual.color = color;
    
    m_textoMensaje->setString(texto);
    m_textoMensaje->setFillColor(color);
    
    std::cout << "MENSAJE: " << texto << std::endl;
}

void Nivel2State::actualizarPosicionRuleta() {
    sf::Vector2u windowSize = window->getSize();
    float ruletaW = windowSize.x * 0.8f;
    float ruletaH = windowSize.y * 0.8f;
    float ruletaX = (windowSize.x - ruletaW) / 2.f;
    float ruletaY = (windowSize.y - ruletaH) / 2.f;
    
    m_ruletaMinigame.setSize(sf::Vector2f(ruletaW, ruletaH));
    m_ruletaMinigame.setPosition(sf::Vector2f(ruletaX, ruletaY));
}