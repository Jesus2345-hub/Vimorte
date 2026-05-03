#include "Nivel2State.hpp"
#include "PauseState.hpp"
#include "Game.hpp"
#include "CoordenadasDebug.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

// ============================================================
// CONSTRUCTOR - Inicializa todo el nivel 2 (Casino Vimorte)
// ============================================================
Nivel2State::Nivel2State(sf::RenderWindow* window, Game* game)
    : State(window, game)
    , m_dinero(10)           // Dinero inicial: $10
    , m_tieneLlave(false)    // No tiene la llave al empezar
    , m_fontLoaded(false)    // Fuente no cargada aún
{
    // ----- MENSAJE TEMPORAL (para mostrar feedback al jugador) -----
    m_msjActual.texto = "";
    m_msjActual.tiempoRestante = 0.0f;
    m_msjActual.color = sf::Color::Yellow;
    
    // ----- CONFIGURAR JUGADOR -----
    m_player.loadAssets();
    m_player.setPosition(818.f, 335.f);  // Posición inicial en el casino
    m_player.setSpeed(300.0f);
    
    // ----- TUTORIAL (solo se muestra la primera vez que entra) -----
    if (game->tienePartidaActiva()) {
        const auto& items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
        auto it = std::find(items.begin(), items.end(), "TutorialNivel2Visto");
        if (it == items.end()) {
            m_mostrarTutorial = true;
            game->getSaveManager().addItemRecolectado("TutorialNivel2Visto");
        }
    }
    
    // ----- CARGAR FONDO DEL NIVEL -----
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel2/background.jpg")) {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x),
                                    static_cast<float>(textureSize.y));
    } else {
        // Fondo de respaldo si no carga la imagen
        m_worldSize = sf::Vector2f(2000.f, 1500.f);
        m_background = nullptr;
    }
    
    // ----- CÁMARA FIJA 1280x720 -----
    sf::Vector2u windowSize = window->getSize();
    m_camera = sf::View(
        sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
        sf::Vector2f(1280.f, 720.f)
    );
    m_lastWindowSize = windowSize;
    
    // ----- ÁREAS DE INTERACCIÓN (ZONAS DONDE EL JUGADOR PUEDE INTERACTUAR) -----
    m_barArea      = sf::FloatRect(sf::Vector2f(187.f, 392.f), sf::Vector2f(278.f, 58.f));   // Zona del bartender
    m_ruletaArea   = sf::FloatRect(sf::Vector2f(230.f, 470.f), sf::Vector2f(400.f, 490.f));  // Zona de la ruleta
    m_vendedorArea = sf::FloatRect(sf::Vector2f(1055.f, 0.f), sf::Vector2f(305.f, 428.f)); // Zona del vendedor de llaves
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(740.f, 0.f), sf::Vector2f(158.f, 330.f)); // Puerta de salida
    
    // ----- CONFIGURAR COLISIONES DEL MAPA -----
    configurarColisiones();
    
    // ----- CONFIGURAR MINIJUEGOS (RULETA Y BARTENDER) -----
    float minijuegoW = windowSize.x * 0.8f;   // 80% del ancho de la ventana
    float minijuegoH = windowSize.y * 0.8f;   // 80% del alto de la ventana
    float minijuegoX = (windowSize.x - minijuegoW) / 2.f;  // Centrado horizontal
    float minijuegoY = (windowSize.y - minijuegoH) / 2.f;  // Centrado vertical
    
    // Ruleta
    m_ruletaMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
    m_ruletaMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    m_ruletaMinigame.setDineroJugador(&m_dinero);  // Conectar el dinero del jugador
    
    // Bartender
    m_bartenderMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
    m_bartenderMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    m_bartenderMinigame.setDineroJugador(&m_dinero);  // Conectar el dinero del jugador
    
    // ----- CARGAR FUENTE PARA TEXTOS -----
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (m_fontLoaded) {
        // Texto de interacción (aparece abajo cuando estás cerca de algo)
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setCharacterSize(18);
        m_textoInteraccion->setFillColor(sf::Color::White);
        
        // Texto del dinero (esquina superior izquierda)
        m_textoDinero = std::make_unique<sf::Text>(m_font);
        m_textoDinero->setCharacterSize(24);
        m_textoDinero->setFillColor(sf::Color::Yellow);
        m_textoDinero->setString("$" + std::to_string(m_dinero));
        
        // Texto para mensajes temporales (feedback)
        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(20);
        m_textoMensaje->setFillColor(sf::Color::Yellow);
    }
    
    // ----- GUARDADO AUTOMÁTICO -----
    if (game->tienePartidaActiva()) {
        game->getSaveManager().setNivelActual(2, 2);
        game->guardarPartidaActual();
    }
    
    game->setIsInLevel(true);
}

// ============================================================
// CONFIGURAR COLISIONES - Define las paredes y obstáculos
// ============================================================
void Nivel2State::configurarColisiones() {
    m_mapaFisico.clear();
    
    // Paredes exteriores (bordes del mapa)
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 0.f), sf::Vector2f(m_worldSize.x, 50.f));                 // Pared norte
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 0.f), sf::Vector2f(50.f, m_worldSize.y));                 // Pared oeste
    m_mapaFisico.emplace_back(sf::Vector2f(m_worldSize.x - 50.f, 0.f), sf::Vector2f(50.f, m_worldSize.y)); // Pared este
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, m_worldSize.y - 50.f), sf::Vector2f(m_worldSize.x, 50.f)); // Pared sur
    
    // Barra del bar (mostrador)
    m_mapaFisico.emplace_back(sf::Vector2f(50.f, 285.f), sf::Vector2f(388.f, 107.f));
    m_mapaFisico.emplace_back(sf::Vector2f(50.f, 0.f), sf::Vector2f(297.f, 285.f));
    m_mapaFisico.emplace_back(sf::Vector2f(347.f, 0.f), sf::Vector2f(1003.f, 240.f));
    
    // Paredes internas del casino
    m_mapaFisico.emplace_back(sf::Vector2f(615.f, 0.f), sf::Vector2f(735.f, 300.f));
    m_mapaFisico.emplace_back(sf::Vector2f(1084.f, 0.f), sf::Vector2f(189.f, 373.f));
    
    // Mesas y obstáculos
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 0.f), sf::Vector2f(188.f, 658.f));
    m_mapaFisico.emplace_back(sf::Vector2f(317.f, 450.f), sf::Vector2f(273.f, 466.f));  // Mesa de ruleta
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 691.f), sf::Vector2f(184.f, 809.f));
    m_mapaFisico.emplace_back(sf::Vector2f(594.f, 915.f), sf::Vector2f(291.f, 585.f));
    m_mapaFisico.emplace_back(sf::Vector2f(955.f, 890.f), sf::Vector2f(445.f, 610.f));
    
    // Muebles adicionales
    m_mapaFisico.emplace_back(sf::Vector2f(1247.f, 416.f), sf::Vector2f(153.f, 264.f));
    m_mapaFisico.emplace_back(sf::Vector2f(1135.f, 488.f), sf::Vector2f(265.f, 127.f));
}

// ============================================================
// MANEJAR EVENTOS - Teclas, clicks, etc.
// ============================================================
void Nivel2State::handleEvent(const sf::Event& event) {
    // ----- TECLAS GLOBALES -----
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        // ESC: Cerrar tutorial o salir de minijuegos
        if (keyPressed->code == sf::Keyboard::Key::Escape) {
            if (m_mostrarTutorial || m_mostrarTutorialPorTecla) {
                m_mostrarTutorial = false;
                m_mostrarTutorialPorTecla = false;
                m_escapeConsumed = true;
                return;
            }
        }
        
        // M: Mostrar tutorial manualmente
        if (keyPressed->code == sf::Keyboard::Key::M) {
            if (game->tienePartidaActiva()) {
                const auto& items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
                auto it = std::find(items.begin(), items.end(), "TutorialNivel2Visto");
                m_mostrarTutorialPorTecla = (it != items.end());
                if (!m_mostrarTutorialPorTecla) m_mostrarTutorial = true;
            } else {
                m_mostrarTutorialPorTecla = true;
            }
        }
        
        // F3: Activar/desactivar coordenadas de debug
        if (keyPressed->code == sf::Keyboard::Key::F3) {
            CoordenadasDebug::getInstance().toggleVisible();
        }
    }
    
    // ----- EVENTOS DEL MINIJUEGO BARTENDER -----
    if (m_bartenderMinigame.isActive()) {
        m_bartenderMinigame.handleEvent(event, *window);
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                m_bartenderMinigame.deactivate();
                m_escapeConsumed = true;
                return;
            }
        }
        return;
    }
    
    // ----- EVENTOS DEL MINIJUEGO RULETA -----
    if (m_ruletaMinigame.isActive()) {
        m_ruletaMinigame.handleEvent(event, *window);
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                m_ruletaMinigame.deactivate();
                m_escapeConsumed = true;
                return;
            }
        }
        return;
    }
    
    // ----- EVENTOS DEL INVENTARIO -----
    Inventory* inv = m_player.getInventory();
    if (inv) inv->handleEvent(event, *window);
}

// ============================================================
// ACTUALIZAR - Se llama cada frame
// ============================================================
void Nivel2State::update(float dt) {
    // ----- ACTUALIZAR TAMAÑO DE MINIJUEGOS AL CAMBIAR VENTANA (F11) -----
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

    // ----- ACTUALIZAR MENSAJE TEMPORAL -----
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f) {
        m_msjActual.tiempoRestante -= dt;
        if (m_msjActual.tiempoRestante <= 0.0f) m_textoMensaje->setString("");
    }
    
    // ----- ACTUALIZAR TEXTO DE DINERO -----
    if (m_textoDinero) {
        std::string textoDinero = "$" + std::to_string(m_dinero);
        if (m_tieneLlave) textoDinero += " [LLAVE]";
        m_textoDinero->setString(textoDinero);
    }
    
    sf::Vector2f posAnterior = m_player.getPosition();  // Guardar posición por si hay colisión
    
    // ----- VERIFICAR CERCANÍA A ZONAS DE INTERACCIÓN -----
    m_cercaRuleta   = m_player.getHurtbox().findIntersection(m_ruletaArea).has_value();
    m_cercaBar      = m_player.getHurtbox().findIntersection(m_barArea).has_value();
    m_cercaVendedor = m_player.getHurtbox().findIntersection(m_vendedorArea).has_value();
    
    // ========== BARTENDER ACTIVO ==========
    if (m_bartenderMinigame.isActive()) {
        m_bartenderMinigame.update(dt);
        m_player.update(dt);
        sf::Vector2f pos = m_player.getPosition();
        pos.x = std::clamp(pos.x, 640.f, m_worldSize.x - 640.f);
        pos.y = std::clamp(pos.y, 360.f, m_worldSize.y - 360.f);
        m_camera.setCenter(pos);
        CoordenadasDebug::getInstance().actualizar(window, m_camera, m_player.getPosition());
        return;
    }
    
    // ========== RULETA ACTIVA ==========
    if (m_ruletaMinigame.isActive()) {
        m_ruletaMinigame.update(dt);
        m_player.update(dt);
        sf::Vector2f pos = m_player.getPosition();
        pos.x = std::clamp(pos.x, 640.f, m_worldSize.x - 640.f);
        pos.y = std::clamp(pos.y, 360.f, m_worldSize.y - 360.f);
        m_camera.setCenter(pos);
        CoordenadasDebug::getInstance().actualizar(window, m_camera, m_player.getPosition());
        return;
    }
    
    // ----- ACTIVAR RULETA (TECLA R CERCA DE LA MESA) -----
    static bool rRuletaPresionado = false;
    if (m_cercaRuleta && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
        if (!rRuletaPresionado) { rRuletaPresionado = true; m_ruletaMinigame.activate(); }
    } else { rRuletaPresionado = false; }
    
    // ----- ACTIVAR BARTENDER (TECLA R CERCA DE LA BARRA) -----
    static bool rBartenderPresionado = false;
    if (m_cercaBar && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
        if (!rBartenderPresionado) { rBartenderPresionado = true; m_bartenderMinigame.activate(); }
    } else { rBartenderPresionado = false; }
    
    // ----- COMPRAR LLAVE (TECLA E CERCA DEL VENDEDOR) -----
    static bool ePresionadoVendedor = false;
    if (m_cercaVendedor && !m_tieneLlave && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
        if (!ePresionadoVendedor) {
            ePresionadoVendedor = true;
            if (m_dinero >= 100) {
                // Tiene suficiente dinero: comprar llave
                m_dinero -= 100;
                m_tieneLlave = true;
                
                // Añadir la llave al inventario del jugador
				Inventory* inv = m_player.getInventory();
				if (inv) {
   					 Item llave("Llave Casino", sf::Color(255, 215, 0));
    				inv->addItem(llave);
				}
                mostrarMensaje("Has comprado la llave! Ve a la puerta de salida.", 3.0f, sf::Color::Green);
            } else {
                // No tiene suficiente dinero
                mostrarMensaje("Necesitas $100. Tienes $" + std::to_string(m_dinero), 2.0f, sf::Color::Red);
            }
        }
    } else { ePresionadoVendedor = false; }
    
    // ----- MOVIMIENTO DEL JUGADOR (WASD Y FLECHAS) -----
    sf::Vector2f movimiento(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    movimiento.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  movimiento.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  movimiento.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) movimiento.x += 1.f;
    
    // Normalizar vector de movimiento (para que no vaya más rápido en diagonal)
    if (movimiento.x != 0.f || movimiento.y != 0.f) {
        float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
        movimiento /= length;
    }
    
    m_player.move(movimiento, dt);
    m_player.update(dt);
    
    // ----- COLISIONES CON PAREDES Y OBSTÁCULOS -----
    for (const auto& obj : m_mapaFisico) {
        if (m_player.getHurtbox().findIntersection(obj).has_value()) {
            m_player.setPosition(posAnterior.x, posAnterior.y);  // Retroceder si hay colisión
            break;
        }
    }
    
    // ----- ACTUALIZAR CÁMARA (SEGUIR AL JUGADOR) -----
    sf::Vector2f playerPos = m_player.getPosition();
    playerPos.x = std::clamp(playerPos.x, 640.f, m_worldSize.x - 640.f);
    playerPos.y = std::clamp(playerPos.y, 360.f, m_worldSize.y - 360.f);
    m_camera.setCenter(playerPos);
    
    // ----- VERIFICAR SALIDA DEL NIVEL -----
    verificarSalidaNivel();
    
    // ----- PAUSA (TECLA ESC) -----
    if (!m_mostrarTutorial && !m_mostrarTutorialPorTecla && !m_escapeConsumed) {
        static bool escapeProcesado = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
            if (!escapeProcesado) { escapeProcesado = true; game->pushState(std::make_unique<PauseState>(window, game)); }
        } else { escapeProcesado = false; }
    }
    
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) m_escapeConsumed = false;
    
    // ----- ACTUALIZAR COORDENADAS DEBUG -----
    CoordenadasDebug::getInstance().actualizar(window, m_camera, m_player.getPosition());
}

// ============================================================
// DIBUJAR - Renderiza todo el nivel
// ============================================================
void Nivel2State::draw() {
    if (!window) return;
    
    // ===== FASE 1: MUNDO DEL JUEGO (VISTA DE CÁMARA) =====
    window->setView(m_camera);
    
    // Dibujar fondo del casino
    if (m_background) window->draw(*m_background);
    else {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(50, 30, 30));
        window->draw(fallback);
    }
    
    // Dibujar jugador
    m_player.draw(*window);
    
    // ===== FASE 2: INTERFAZ DE USUARIO (VISTA POR DEFECTO) =====
    window->setView(window->getDefaultView());
    
    // Coordenadas de debug (F3 para activar)
    CoordenadasDebug::getInstance().dibujar(*window);
    
    // Texto del dinero (esquina superior izquierda)
    if (m_textoDinero) {
        m_textoDinero->setPosition(sf::Vector2f(20.f, 20.f));
        window->draw(*m_textoDinero);
    }
    
    // Textos de interacción (aparecen abajo cuando estás cerca de algo)
    if (m_fontLoaded && m_textoInteraccion) {
        float winW = static_cast<float>(window->getSize().x);
        float winH = static_cast<float>(window->getSize().y);
        
        auto drawText = [&](const std::string& texto) {
            m_textoInteraccion->setString(texto);
            sf::FloatRect bounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 90.f));
            window->draw(*m_textoInteraccion);
        };
        
        // Texto para la ruleta
        if (m_cercaRuleta && !m_ruletaMinigame.isActive() && !m_bartenderMinigame.isActive())
            drawText("Presiona R para jugar a la ruleta");
        
        // Texto para el bartender
        if (m_cercaBar && !m_bartenderMinigame.isActive() && !m_ruletaMinigame.isActive())
            drawText("Presiona R para trabajar de bartender");
        
        // Texto para el vendedor de llaves
        if (m_cercaVendedor && !m_tieneLlave)
            drawText("Presiona E para comprar la llave ($100) | Dinero: $" + std::to_string(m_dinero));
        if (m_cercaVendedor && m_tieneLlave)
            drawText("Ya tienes la llave. Dirigete a la puerta de salida.");
        
        // Texto para la puerta de salida
        if (m_cercaPuertaSalida)
            drawText(m_tieneLlave ? "Presiona E para usar la llave y salir" : "Necesitas comprar la llave ($100) en la tienda.");
    }
    
    // Mensaje temporal (feedback de acciones)
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f && !m_textoMensaje->getString().isEmpty()) {
        sf::Vector2u winSize = window->getSize();
        sf::FloatRect bounds = m_textoMensaje->getLocalBounds();
        m_textoMensaje->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_textoMensaje->setPosition(sf::Vector2f(winSize.x / 2.f, winSize.y / 3.f));
        window->draw(*m_textoMensaje);
    }
    
    // Dibujar minijuegos (si están activos)
    if (m_bartenderMinigame.isActive()) m_bartenderMinigame.draw(*window);
    if (m_ruletaMinigame.isActive())   m_ruletaMinigame.draw(*window);
    
    // Dibujar inventario del jugador
    Inventory* inv = m_player.getInventory();
    if (inv) inv->draw(*window);
    
    // ===== TUTORIAL (OVERLAY NEGRO CON TEXTO) =====
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
                "- Trabaja de bartender ($1 por bebida)\n"
                "- Juega a la ruleta (apuesta y multiplica)\n\n"
                "[ESC] Cerrar | [M] ayuda | [F3] Coordenadas"
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

// ============================================================
// VERIFICAR SALIDA DEL NIVEL - Solo si tiene la llave
// ============================================================
void Nivel2State::verificarSalidaNivel() {
    m_cercaPuertaSalida = m_player.getHurtbox().findIntersection(m_puertaSalidaArea).has_value();
    
    static bool ePresionado = false;
    if (m_cercaPuertaSalida && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
        if (!ePresionado) {
            ePresionado = true;
            
            // Verificar si tiene la llave en el inventario
            Inventory* inv = m_player.getInventory();
            bool tieneLlaveInventario = false;
            if (inv) {
                for (int i = 0; i < 5; i++) {  // Revisar los 5 slots del hotbar
                    Item* item = inv->getItem(i);
                    if (item && item->name == "Llave Casino") {
                        tieneLlaveInventario = true;
                        break;
                    }
                }
            }
            
            if (tieneLlaveInventario) {
                std::cout << "Saliendo del nivel 2..." << std::endl;
                game->avanzarNivel();
            } else {
                mostrarMensaje("Necesitas la llave del casino para salir. Comprala por $100.", 2.0f, sf::Color::Red);
            }
        }
    } else {
        ePresionado = false;
    }
}

// ============================================================
// MOSTRAR MENSAJE TEMPORAL EN PANTALLA
// ============================================================
void Nivel2State::mostrarMensaje(const std::string& texto, float duracion, sf::Color color) {
    if (!m_textoMensaje) return;
    m_msjActual.texto = texto;
    m_msjActual.tiempoRestante = duracion;
    m_msjActual.color = color;
    m_textoMensaje->setString(texto);
    m_textoMensaje->setFillColor(color);
}

// ============================================================
// ACTUALIZAR POSICIÓN DE LA RULETA (AL CAMBIAR TAMAÑO DE VENTANA)
// ============================================================
void Nivel2State::actualizarPosicionRuleta() {
    sf::Vector2u windowSize = window->getSize();
    float w = windowSize.x * 0.8f;
    float h = windowSize.y * 0.8f;
    m_ruletaMinigame.setSize(sf::Vector2f(w, h));
    m_ruletaMinigame.setPosition(sf::Vector2f((windowSize.x - w) / 2.f, (windowSize.y - h) / 2.f));
}