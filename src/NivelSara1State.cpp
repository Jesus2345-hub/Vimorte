#include "NivelSara1State.hpp"
#include "PauseState.hpp"
#include "Game.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

NivelSara1State::NivelSara1State(sf::RenderWindow* window, Game* game)
    : State(window, game), 
    m_cercaPuertaSalida(false),
    m_fontLoaded(false) ,
    m_skipPauseThisFrame(false) , 
    m_mostrarTutorial(false), 
    m_mostrarTutorialPorTecla(false), 
    m_escapeConsumed(false) ,
    m_vitalSignsAndres()
{
    m_vitalSignsAndrea.setAnchorRight(true);  
    m_vitalSignsAndres.setAnchorRight(false);
    
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
    m_player.loadAssets();
    m_player.setPosition(1212.f, 753.f);
    m_player.setSpeed(300.0f);
    // Cargar fondo 
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel_sara/background.png")) {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x),
                                   static_cast<float>(textureSize.y));
        std::cout << "NivelSara1m cargado. Tamaño: " << m_worldSize.x << "x" << m_worldSize.y << std::endl;
    } else {
        std::cerr << "Error: No se pudo cargar el fondo del nivel Sara" << std::endl;
        m_worldSize = sf::Vector2f(1920.f, 1080.f);
        m_background = nullptr;
    }

    // Configurar cámara 
    sf::Vector2u windowSize = window->getSize();
    float fixedWidth = 1280.f;
    float fixedHeight = 720.f;
    m_camera = sf::View(
        sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
        sf::Vector2f(fixedWidth, fixedHeight)
    );
    m_lastWindowSize = windowSize;
    
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(1550.f, 1350.f), sf::Vector2f(120.f, 180.f));
    configurarColisiones();
   

    // Configurar área de dardos 
    m_dartsArea = sf::FloatRect(sf::Vector2f(30.f, 40.f ), sf::Vector2f( 150.f,  150.f));   
    m_dartsTriggerRadius = 150.f;

    // Configurar minijuego de dardos
    sf::Vector2u winSize = window->getSize();
    float dartsW = winSize.x * 0.8f;
    float dartsH = winSize.y * 0.93f;
    m_dartsMinigame.setSize(sf::Vector2f(dartsW, dartsH));
    m_dartsMinigame.setPosition(sf::Vector2f(
        (winSize.x - dartsW) / 2.f,
        (winSize.y - dartsH) / 2.f
    ));
    m_dartsMinigame.setVitalSigns(&m_vitalSignsAndres);
    
    // Configurar signos de Andrea (oscilan pero no se modifican con dardos)
    
    float panelWidth = 200.f;             
    float marginRight = 100.f;             
    float leftPos = winSize.x - panelWidth - marginRight;

    m_vitalSignsAndrea.setLeftMargin(leftPos);
    m_vitalSignsAndrea.setBottomMargin(100.f);   
    m_vitalSignsAndrea.setTitle("      SIGNOS VITALES\n         -ANDREA-");
    m_vitalSignsAndrea.setTitleOffsetX(-50.f);

    // Fuente
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");

    if (m_fontLoaded) {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setCharacterSize(20);
        m_textoInteraccion->setFillColor(sf::Color::White);
        
        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(24);
        m_textoMensaje->setFillColor(sf::Color::Yellow);
        //
        m_textoCoordenadas = std::make_unique<sf::Text>(m_font);
        m_textoCoordenadas->setCharacterSize(18); // Tamaño más pequeño para que no estorbe
        m_textoCoordenadas->setFillColor(sf::Color::Cyan); // Color llamativo
        m_textoCoordenadas->setPosition(sf::Vector2f(10.f, 10.f)); // Lo ponemos en la esquina superior izquierda
        ////
    }

    // Guardado automático
    if (game->tienePartidaActiva()) {
        game->getSaveManager().setNivelActual(4, 4);
        game->guardarPartidaActual();
        std::cout << "Partida guardada en NivelSara1m" << std::endl;
    }
     // Zona de teletransporte (coordenadas absolutas del mundo)
    m_teleportZone = sf::FloatRect(sf::Vector2f(70.f, 320.f), sf::Vector2f(60.f, 60.f));
    m_teleportDestination = sf::Vector2f(1460.f, 325.f);
     // Segundo teletransporte (coordenadas absolutas del mundo)
    m_teleportZone2 = sf::FloatRect(sf::Vector2f(1200.f, 120.f), sf::Vector2f(60.f, 60.f));
    m_teleportDestination2 = sf::Vector2f(450.f, 120.f);
    actualizarUIPosiciones();
    std::cout << "NivelSara1State inicializado correctamente" << std::endl;
    game->setIsInLevel(true);
}
// ============================================================================
// VERIFICAR TELETRANSPORTE DESPUÉS DE GANAR
// ============================================================================
void NivelSara1State::verificarTeletransportePostJuego() {
    // Solo si el paciente está estabilizado
    if (!m_vitalSignsAndres.isStabilized()) return;
    
    // Verificar si el jugador está cerca del área de dardos
    bool cercaDartsTeletransporte = m_player.getHurtbox().findIntersection(m_dartsArea).has_value();
    
    static bool eTeletransportePresionado = false;
    
    if (cercaDartsTeletransporte && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
        if (!eTeletransportePresionado) {
            eTeletransportePresionado = true;
            
            // Teletransportar a otra zona del mapa (ajusta estas coordenadas)
            sf::Vector2f destinoTeletransporte(100.f, 400.f);  
            m_player.setPosition(destinoTeletransporte.x, destinoTeletransporte.y);
            
            // Mensaje de confirmación
            mostrarMensaje("Teletransportado... Ahora ve a ayudar a Andrea", 2.f, sf::Color::Green);
            
            std::cout << "Jugador teletransportado a: " << destinoTeletransporte.x << ", " << destinoTeletransporte.y << std::endl;
        }
    } else {
        eTeletransportePresionado = false;
    }
}
void NivelSara1State::configurarColisiones() {
    m_mapaFisico.clear();
    // Paredes exteriores
    m_mapaFisico.emplace_back(sf::Vector2f(30.f, 12.f), sf::Vector2f(m_worldSize.x - 60.f, 130.f));
    m_mapaFisico.emplace_back(sf::Vector2f(18.f, 12.f), sf::Vector2f(20.f, m_worldSize.y - 24.f));
    m_mapaFisico.emplace_back(sf::Vector2f(m_worldSize.x - 50.f, 12.f), sf::Vector2f(20.f, m_worldSize.y - 24.f));

     m_mapaFisico.emplace_back(sf::Vector2f(233.f, 186.f), sf::Vector2f(15.f, 303.f));
     m_mapaFisico.emplace_back(sf::Vector2f(248.f, 186.f), sf::Vector2f(71.f, 24.f));
     m_mapaFisico.emplace_back(sf::Vector2f(118.f, 461.f), sf::Vector2f(114.f, 27.f));
     m_mapaFisico.emplace_back(sf::Vector2f(118.f, 461.f), sf::Vector2f(24.f, 175.f));
     m_mapaFisico.emplace_back(sf::Vector2f(307.f, 451.f), sf::Vector2f(113.f, 34.f));
     m_mapaFisico.emplace_back(sf::Vector2f(307.f, 251.f), sf::Vector2f(15.f, 196.f));
     m_mapaFisico.emplace_back(sf::Vector2f(329.f, 251.f), sf::Vector2f(80.f, 12.f));
     m_mapaFisico.emplace_back(sf::Vector2f(417.f, 303.f), sf::Vector2f(15.f, 145.f));
     m_mapaFisico.emplace_back(sf::Vector2f(436.f, 303.f), sf::Vector2f(144.f, 25.f));
     m_mapaFisico.emplace_back(sf::Vector2f(225.f, 543.f), sf::Vector2f(15.f, 293.f));
     m_mapaFisico.emplace_back(sf::Vector2f(241.f, 543.f), sf::Vector2f(174.f, 31.f));
     m_mapaFisico.emplace_back(sf::Vector2f(331.f, 543.f), sf::Vector2f(15.f, 200.f));
     m_mapaFisico.emplace_back(sf::Vector2f(241.f, 807.f), sf::Vector2f(291.f, 27.f));
     m_mapaFisico.emplace_back(sf::Vector2f(523.f, 370.f), sf::Vector2f(9.f, 439.f));
     m_mapaFisico.emplace_back(sf::Vector2f(645.f, 253.f), sf::Vector2f(15.f, 194.f));
     m_mapaFisico.emplace_back(sf::Vector2f(630.f, 143.f), sf::Vector2f(78.f, 103.f));
     m_mapaFisico.emplace_back(sf::Vector2f(727.f, 143.f), sf::Vector2f(376.f, 372.f));
     m_mapaFisico.emplace_back(sf::Vector2f(420.f, 624.f), sf::Vector2f(98.f, 119.f));
     m_mapaFisico.emplace_back(sf::Vector2f(590.f, 413.f), sf::Vector2f(15.f, 145.f));
     m_mapaFisico.emplace_back(sf::Vector2f(513.f, 249.f), sf::Vector2f(149.f, 12.f));
     m_mapaFisico.emplace_back(sf::Vector2f(587.f, 625.f), sf::Vector2f(272.f, 113.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1165.f, 212.f), sf::Vector2f(245.f, 69.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1110.f, 212.f), sf::Vector2f(15.f, 24.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1173.f, 323.f), sf::Vector2f(115.f, 248.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1289.f, 458.f), sf::Vector2f(198.f, 220.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1404.f, 650.f), sf::Vector2f(15.f, 800.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1238.f, 650.f), sf::Vector2f(162.f, 24.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1075.f, 670.f), sf::Vector2f(98.f, 300.f));
     m_mapaFisico.emplace_back(sf::Vector2f(965.f, 588.f), sf::Vector2f(272.f, 24.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1173.f, 869.f), sf::Vector2f(231.f, 24.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1257.f, 762.f), sf::Vector2f(55.f, 103.f));
     m_mapaFisico.emplace_back(sf::Vector2f(960.f, 590.f), sf::Vector2f(15.f, 306.f));
     m_mapaFisico.emplace_back(sf::Vector2f(793.f, 798.f), sf::Vector2f(166.f, 100.f));
     m_mapaFisico.emplace_back(sf::Vector2f(793.f, 738.f), sf::Vector2f(68.f, 62.f));
     m_mapaFisico.emplace_back(sf::Vector2f(18.f, 940.f), sf::Vector2f(1062.f, 62.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1237.f, 624.f), sf::Vector2f(5.f, 5.f));
     m_mapaFisico.emplace_back(sf::Vector2f(119.f, 884.f), sf::Vector2f(586.f, 15.f));
     m_mapaFisico.emplace_back(sf::Vector2f(691.f, 797.f), sf::Vector2f(12.f, 92.f));
     m_mapaFisico.emplace_back(sf::Vector2f(586.f, 738.f), sf::Vector2f(15.f, 60.f));
     m_mapaFisico.emplace_back(sf::Vector2f(586.f, 800.f), sf::Vector2f(111.f, 24.f));
     m_mapaFisico.emplace_back(sf::Vector2f(120.f, 707.f), sf::Vector2f(15.f, 171.f));
     m_mapaFisico.emplace_back(sf::Vector2f(746.f, 586.f), sf::Vector2f(12.f, 12.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1400.f, 295.f), sf::Vector2f(15.f, 113.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1417.f, 380.f), sf::Vector2f(133.f, 25.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1541.f, 424.f), sf::Vector2f(13.f, 220.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1500.f, 647.f), sf::Vector2f(13.f, 22.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1535.f, 143.f), sf::Vector2f(100.f, 266.f));
     m_mapaFisico.emplace_back(sf::Vector2f(1490.f, 150.f), sf::Vector2f(50.f, 102.f));
     m_mapaFisico.emplace_back(sf::Vector2f(167.f, 288.f), sf::Vector2f(15.f, 122.f));
     m_mapaFisico.emplace_back(sf::Vector2f(47.f, 288.f), sf::Vector2f(182.f, 24.f));
     m_mapaFisico.emplace_back(sf::Vector2f(39.f, 142.f), sf::Vector2f(75.f, 114.f));

}

void NivelSara1State::verificarSalidaNivel() {
    m_cercaPuertaSalida = m_player.getHurtbox().findIntersection(m_puertaSalidaArea).has_value();

    static bool ePresionado = false;
    if (m_cercaPuertaSalida && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
        if (!ePresionado) {
            ePresionado = true;
            std::cout << "Saliendo del nivel Sara..." << std::endl;
            game->avanzarNivel();
        }
    } else {
        ePresionado = false;
    }
}

void NivelSara1State::handleEvent(const sf::Event& event) {
    // Manejar teclas globales (Escape y M) tutorial
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
    // si el minijuego está activo, darle prioridad
    if (m_dartsMinigame.isActive()) {
        m_dartsMinigame.handleEvent(event, *window);
        if (event.is<sf::Event::KeyPressed>()) {
            const auto& keyEvent = event.getIf<sf::Event::KeyPressed>();
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                m_dartsMinigame.deactivate();
                m_skipPauseThisFrame = true;
                std::cout << "Minijuego de dardos cerrado" << std::endl;
            }
        }
        return;
    }
}

void NivelSara1State::update(float dt) {
    //pantalla
    sf::Vector2u currentSize = window->getSize();
    if (currentSize != m_lastWindowSize) {
        m_lastWindowSize = currentSize;
        actualizarUIPosiciones();
        window->setView(window->getDefaultView());
    }

    // Teletransporte si colisiona con la zona
    if (m_player.getHurtbox().findIntersection(m_teleportZone).has_value()) {
        m_player.setPosition(m_teleportDestination.x, m_teleportDestination.y);
    }

    // Segundo teletransporte
    if (m_player.getHurtbox().findIntersection(m_teleportZone2).has_value()) {
        m_player.setPosition(m_teleportDestination2.x, m_teleportDestination2.y);
    }

    // Actualizar mensaje temporal
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f) {
        m_msjActual.tiempoRestante -= dt;
        if (m_msjActual.tiempoRestante <= 0.0f) {
            m_textoMensaje->setString("");
        }
    }

    //ACTUALIZAR SIGNOS VITALES (siempre, incluso durante minijuego)
    m_vitalSignsAndres.update(dt);
    m_vitalSignsAndrea.update(dt);
   
    sf::Vector2f posAnterior = m_player.getPosition();
    sf::Vector2u windowSize = window->getSize();

    // Actualizar minijuego si está activo
    if (m_dartsMinigame.isActive()) {
        m_dartsMinigame.update(dt);
        m_player.update(dt);
        return; 
    }

    // Colisión con el rectángulo del área de dardos
    sf::FloatRect playerBounds = m_player.getHurtbox();
    m_cercaDarts = playerBounds.findIntersection(m_dartsArea).has_value();
    static bool rPresionado = false;

    if (m_cercaDarts && !m_dartsMinigame.isActive()) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
        if (!rPresionado) {
            rPresionado = true;
            // Si el paciente murió, reiniciamos todo
            if (m_vitalSignsAndres.isGameOver()) {
                m_vitalSignsAndres.reset();
            }
            
            if (!m_vitalSignsAndres.isStabilized()) {
                m_dartsMinigame.reset();
                m_dartsMinigame.activate();
            } else {
                mostrarMensaje("Paciente estabilizado. Ya puedes ir a Salvar a la\n   Dr. Andrea por la puerta gris", 2.f, sf::Color::Yellow);
            }
        }
    } else {
        rPresionado = false;
    }
    }

    // Movimiento (WASD / flechas)
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

        // ========== CÁMARA FIJA ==========
    sf::Vector2f playerPos = m_player.getPosition();
    sf::Vector2f cameraPos = playerPos;

    float halfWidth = 1280.f / 2.f;
    float halfHeight = 720.f / 2.f;

    if (halfWidth * 2.f >= m_worldSize.x) {
        cameraPos.x = m_worldSize.x / 2.f;
    } else {
        if (cameraPos.x < halfWidth) cameraPos.x = halfWidth;
        if (cameraPos.x > m_worldSize.x - halfWidth) cameraPos.x = m_worldSize.x - halfWidth;
    }

    if (halfHeight * 2.f >= m_worldSize.y) {
        cameraPos.y = m_worldSize.y / 2.f;
    } else {
        if (cameraPos.y < halfHeight) cameraPos.y = halfHeight;
        if (cameraPos.y > m_worldSize.y - halfHeight) cameraPos.y = m_worldSize.y - halfHeight;
    }

    m_camera.setCenter(cameraPos);

    verificarTeletransportePostJuego();
    verificarSalidaNivel();
    verificarSalidaNivel();

    // Pausa: solo si el tutorial no está activo
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

    // Resetear el flag de consumido cuando se suelta Escape
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        m_escapeConsumed = false;
    }

    // Si acabamos de cerrar el minijuego, ignoramos la pulsación de Escape este frame
    if (m_skipPauseThisFrame) {
        m_skipPauseThisFrame = false;
    }


// 1. Obtener la posición del mouse en coordenadas de la PANTALLA (píxeles)
sf::Vector2i mousePosPantalla = sf::Mouse::getPosition(*window);

// 2. ¡IMPORTANTE! Convertir a coordenadas del MUNDO (world) usando la vista actual de la cámara.
// Esto es para que te dé las coordenadas correctas dentro de tu mapa, no las de la ventana.
sf::Vector2f mousePosMundo = window->mapPixelToCoords(mousePosPantalla, m_camera);

// 3. Crear el string de texto con las coordenadas
std::string coordsText = "Mouse (Mundo): X=" + std::to_string((int)mousePosMundo.x) + 
                         " Y=" + std::to_string((int)mousePosMundo.y);

// Añadir también las coordenadas del jugador para referencia
coordsText += "\nJugador (Player): X=" + std::to_string((int)m_player.getPosition().x) + 
              " Y=" + std::to_string((int)m_player.getPosition().y);

// 4. Actualizar el texto del Sprite
if (m_textoCoordenadas) {
    m_textoCoordenadas->setString(coordsText);
}
}

void NivelSara1State::draw() {
    if (!window) return;

    // 1. Vista del mundo (cámara)
    window->setView(m_camera);

    // Fondo
    if (m_background)
        window->draw(*m_background);
    else {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(50, 50, 70));
        window->draw(fallback);
    }
    
    // Mensaje temporal (en coordenadas del mundo)
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f && !m_textoMensaje->getString().isEmpty()) {
        sf::Vector2u windowSize = window->getSize();
        m_textoMensaje->setPosition(sf::Vector2f(windowSize.x / 2.f, windowSize.y / 3.f));
        window->draw(*m_textoMensaje);
    }

    // Tutorial (si está activo)
    if (m_mostrarTutorial || m_mostrarTutorialPorTecla) {
        sf::RectangleShape overlay(sf::Vector2f(window->getSize().x, window->getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);
        
        if (m_fontLoaded) {
            sf::Text tutorialText(m_font);
            tutorialText.setString(
                "BIENVENIDO A LA HABITACION DE SARA\n\n"
                "Debes Salvar al Doctor Andres y la Doctora Andrea\n"
                "Para avanzar y salvar a Andrea, deberas superar su desafio de dardos.\n\n"
                "Acercate al tablon y pulsa R para jugar.\n"
                "Gana la partida y podras salir.\n\n"
                "[ESC] Cerrar | [M] ayuda"
            );
            tutorialText.setCharacterSize(20);
            tutorialText.setFillColor(sf::Color::White);
            sf::FloatRect textBounds = tutorialText.getLocalBounds();
            tutorialText.setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            float posX = window->getSize().x / 2.f;
            float posY = window->getSize().y * 0.6f;
            tutorialText.setPosition(sf::Vector2f(posX, posY));
            window->draw(tutorialText);
        } else {
            std::cerr << "No se puede mostrar el tutorial porque la fuente no esta cargada." << std::endl;
        }
    }

    // // Elementos de depuración (colisiones)
    // window->setView(m_camera);
    // for (const auto& rect : m_mapaFisico) {
    //     sf::RectangleShape bloque;
    //     bloque.setPosition(sf::Vector2f(rect.position.x, rect.position.y));
    //     bloque.setSize(sf::Vector2f(rect.size.x, rect.size.y));
    //     bloque.setFillColor(sf::Color(255, 0, 0, 100));
    //     bloque.setOutlineThickness(2.f);
    //     bloque.setOutlineColor(sf::Color::Red);
    //     window->draw(bloque);
    // }
    // Área de dardos (debug)
    sf::RectangleShape dartsRect(m_dartsArea.size);
    dartsRect.setPosition(m_dartsArea.position);
    dartsRect.setFillColor(sf::Color(0, 255, 0, 120));
    dartsRect.setOutlineThickness(2.f);
    dartsRect.setOutlineColor(sf::Color::Green);
    window->draw(dartsRect);

    // //teletrasnportacion
    // sf::RectangleShape teleRect(m_teleportZone.size);
    // teleRect.setPosition(m_teleportZone.position);
    // teleRect.setFillColor(sf::Color(255, 0, 255, 100)); 
    // window->draw(teleRect);

    // sf::RectangleShape teleRect2(m_teleportZone2.size);
    // teleRect2.setPosition(m_teleportZone2.position);
    // teleRect2.setFillColor(sf::Color(0, 255, 255, 150));
    // teleRect2.setOutlineThickness(2.f);
    // teleRect2.setOutlineColor(sf::Color::Cyan);
    // window->draw(teleRect2);

    // Jugador
    m_player.draw(*window);

    // 2. Vista por defecto para la interfaz
    window->setView(window->getDefaultView());

    // Textos de interacción (salida y dardos)
    if (m_cercaPuertaSalida && m_textoInteraccion && m_fontLoaded) {
        m_textoInteraccion->setString("Presiona E para avanzar al siguiente nivel");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    if (m_cercaDarts && !m_dartsMinigame.isActive() && m_textoInteraccion && m_fontLoaded) {
    if (m_vitalSignsAndres.isStabilized()) {
        m_textoInteraccion->setString("Paciente estable. Ve a la puerta, presiona |E|");
    } else if (m_vitalSignsAndres.isGameOver()) {
        m_textoInteraccion->setString("Paciente muerto. Presiona R para reintentar");
    } else {
        m_textoInteraccion->setString("Presiona R para jugar a los dardos");
    }
    sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
    m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
    m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
    window->draw(*m_textoInteraccion);
    }
    // Mensajes temporales (nuevamente, por si acaso)
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f && !m_textoMensaje->getString().isEmpty()) {
        sf::Vector2u winSize = window->getSize();
        m_textoMensaje->setPosition(sf::Vector2f(winSize.x / 2.f, winSize.y / 3.f));
        window->draw(*m_textoMensaje);
    }

    // Minijuego signos vitales dardos
    if (m_dartsMinigame.isActive()) {
        m_dartsMinigame.draw(*window);
    }

    // Signos vitales 
    m_vitalSignsAndres.draw(*window);
    m_vitalSignsAndrea.draw(*window);

    // Inventario 
    Inventory* inv = m_player.getInventory();
    if (inv) inv->draw(*window);

    // //
    // if (m_textoCoordenadas && m_fontLoaded) {
    // window->draw(*m_textoCoordenadas);
    // ///
    // }
}

void NivelSara1State::mostrarMensaje(const std::string& texto, float duracion, sf::Color color) {
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
void NivelSara1State::actualizarUIPosiciones() {
    if (!window) return;

    sf::Vector2u winSize = window->getSize();
    float winW = static_cast<float>(winSize.x);
    float winH = static_cast<float>(winSize.y);
    
    float scale = winH / 720.f;
    scale = std::clamp(scale, 0.7f, 1.5f);

    // Minijuego
    float dartsW = winW * 0.8f;
    float dartsH = winH * 0.93f;
    m_dartsMinigame.setSize(sf::Vector2f(dartsW, dartsH));
    m_dartsMinigame.setPosition(sf::Vector2f(
        (winW - dartsW) * 0.5f,
        (winH - dartsH) * 0.5f
    ));

    float marginBottom = 100.f * scale;
    
    // Andrés (izquierda)
    m_vitalSignsAndres.setBottomMargin(marginBottom);
    m_vitalSignsAndres.setLeftMargin(20.f * scale);
    
    
    m_vitalSignsAndrea.setBottomMargin(marginBottom);
    m_vitalSignsAndrea.setLeftMargin(scale-20.0);  
}
