#include "Nivel4State.hpp"
#include "PauseState.hpp"
#include "MiniGameTetris.hpp"
#include "GameOverState.hpp"
#include "Game.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

Nivel4State::Nivel4State(sf::RenderWindow* window, Game* game)
    : State(window, game),
    m_cercaDarts(false),
    m_cercaTetris(false),
    m_cercaAscensor(false),
    m_ambosEstabilizados(false),
    m_mensajeVictoriaTimer(0.f),
    m_mostrandoMensajeVictoria(false),
    m_mostrarTutorial(false),
    m_mostrarTutorialPorTecla(false),
    m_escapeConsumed(false),
    m_fontLoaded(false),
    m_skipPauseThisFrame(false),
    m_gameOverTriggered(false)
{
    m_vitalSignsAndrea.setAnchorRight(true);
    m_vitalSignsAndres.setAnchorRight(false);

    // Verificar si es la primera vez para mostrar el tutorial
    if (game->tienePartidaActiva()) {
        const auto& items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
        auto it = std::find(items.begin(), items.end(), "TutorialVisto");
        if (it == items.end()) {
            m_mostrarTutorial = true;
            game->getSaveManager().addItemRecolectado("TutorialVisto");
            std::cout << "Primer ingreso: Mostrando tutorial" << std::endl;
        }
    }

    // Configurar jugador
    m_player.loadAssets();
    m_player.setPosition(1212.f, 753.f);
    m_player.setSpeed(300.0f);

    // Cargar fondo
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel4/background.png")) {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x),
                                   static_cast<float>(textureSize.y));
        std::cout << "Nivel4 cargado. Tamanio: " << m_worldSize.x << "x" << m_worldSize.y << std::endl;
    } else {
        std::cerr << "Error: No se pudo cargar el fondo del nivel 4" << std::endl;
        m_worldSize = sf::Vector2f(1920.f, 1080.f);
        m_background = nullptr;
    }

    // Configurar herramienta recogible
    m_herramientaRecogida = false;
    m_herramientaArea = sf::FloatRect(sf::Vector2f(1540.f, 584.f), sf::Vector2f(40.f, 40.f)); 
    m_cercaHerramienta = false;

    // Cargar sprite de la herramienta en el mapa
    if (m_herramientaMapTexture.loadFromFile("assets/images/niveles/nivel4/herramienta.png")) {
        m_herramientaMapSprite = std::make_unique<sf::Sprite>(m_herramientaMapTexture);
        m_herramientaMapSprite->setScale(sf::Vector2f(0.15f, 0.15f)); 
        sf::FloatRect bounds = m_herramientaMapSprite->getLocalBounds();
        m_herramientaMapSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_herramientaMapSprite->setPosition(sf::Vector2f(
            m_herramientaArea.position.x + m_herramientaArea.size.x / 2.f,
            m_herramientaArea.position.y + m_herramientaArea.size.y / 2.f
        ));
    }

    // Configurar camara
    sf::Vector2u windowSize = window->getSize();
    m_camera = sf::View(
        sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
        sf::Vector2f(1280.f, 720.f)
    );
    m_lastWindowSize = windowSize;

    configurarColisiones();

    // Configurar bloque de muro rompible (centinela)
    configurarBloqueMuro();
    m_cercaBloqueMuro = false;
    m_golpeCooldown = false;
    m_golpeTimer = 0.f;

    // Configurar area de dardos
    m_dartsArea = sf::FloatRect(sf::Vector2f(30.f, 40.f), sf::Vector2f(150.f, 150.f));

    // Configurar minijuego de dardos
    float dartsW = windowSize.x * 0.8f;
    float dartsH = windowSize.y * 0.93f;
    m_dartsMinigame.setSize(sf::Vector2f(dartsW, dartsH));
    m_dartsMinigame.setPosition(sf::Vector2f(
        (windowSize.x - dartsW) / 2.f,
        (windowSize.y - dartsH) / 2.f
    ));
    m_dartsMinigame.setVitalSigns(&m_vitalSignsAndres);

    // Configurar juego de Tetris
    float tetrisW = std::min(windowSize.x * 0.65f, 650.f);
    float tetrisH = std::min(windowSize.y * 0.7f, 550.f);
    m_tetris.setSize(sf::Vector2f(tetrisW, tetrisH));
    m_tetris.setPosition(sf::Vector2f(
        (windowSize.x - tetrisW) / 2.f,
        (windowSize.y - tetrisH) / 2.f
    ));
    m_tetris.setVitalSigns(&m_vitalSignsAndrea);

    // Configurar area de Tetris
    m_tetrisArea = sf::FloatRect(sf::Vector2f(1400.f, 120.f), sf::Vector2f(45.f, 45.f));

    // Configurar signos de Andrea
    float panelWidth = 200.f;
    float marginRight = 100.f;
    float leftPos = windowSize.x - panelWidth - marginRight;

    m_vitalSignsAndrea.setLeftMargin(leftPos);
    m_vitalSignsAndrea.setBottomMargin(100.f);
    m_vitalSignsAndrea.setTitle("      SIGNOS VITALES\n         -ANDREA-");
    m_vitalSignsAndrea.setTitleOffsetX(-50.f);

    // Configurar area del ascensor (SALIDA)
    m_ascensorArea = sf::FloatRect(sf::Vector2f(1212.f, 753.f), sf::Vector2f(80.f, 120.f));

    // Zonas de teletransporte
    m_teleportZone = sf::FloatRect(sf::Vector2f(70.f, 320.f), sf::Vector2f(60.f, 60.f));
    m_teleportDestination = sf::Vector2f(1492.f, 290.f);
    m_teleportZone2 = sf::FloatRect(sf::Vector2f(1200.f, 120.f), sf::Vector2f(60.f, 60.f));
    m_teleportDestination2 = sf::Vector2f(450.f, 120.f);

    // Fuente
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");

    if (m_fontLoaded) {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setCharacterSize(20);
        m_textoInteraccion->setFillColor(sf::Color::White);
        m_textoInteraccion->setOutlineThickness(1.5f);      
        m_textoInteraccion->setOutlineColor(sf::Color::Black);

        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(24);
        m_textoMensaje->setFillColor(sf::Color::Yellow);
        m_textoMensaje->setOutlineThickness(1.5f);     
        m_textoMensaje->setOutlineColor(sf::Color::Black);

        m_textoCoordenadas = std::make_unique<sf::Text>(m_font);
        m_textoCoordenadas->setCharacterSize(18);
        m_textoCoordenadas->setFillColor(sf::Color::Cyan);
        m_textoCoordenadas->setPosition(sf::Vector2f(10.f, 10.f));
    }

    // Mensaje temporal inicial
    m_msjActual.texto = "";
    m_msjActual.tiempoRestante = 0.f;

    // Guardado automatico
    if (game->tienePartidaActiva()) {
        game->getSaveManager().setNivelActual(4, 4);
        game->getSaveManager().guardarProgresoActual();  
        std::cout << "Partida guardada en Nivel4" << std::endl;
    }

    // =====  Asegurar que el arbol apunte al nivel actual =====
    if (game && game->tienePartidaActiva()) {

        game->getLevelTree().jumpToNode("nivel4");
        std::cout << "LevelTree actualizado a nivel4" << std::endl;
    }
    actualizarUIPosiciones();
    std::cout << "Nivel4State inicializado correctamente" << std::endl;
    game->setIsInLevel(true);

    // Establecer nombres de los pacientes
    m_vitalSignsAndres.setPatientName("Andres");
    m_vitalSignsAndrea.setPatientName("Andrea");
}

void Nivel4State::configurarColisiones() {
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
void Nivel4State::configurarBloqueMuro() {

    m_bloqueMuro.area = sf::FloatRect(sf::Vector2f(598.f, 445.f), sf::Vector2f(10.f, 10.f));
    m_bloqueMuro.golpesRestantes = 5;
    m_bloqueMuro.muroRoto = false;
    m_bloqueMuro.mostrandoMensaje = false;
    m_bloqueMuro.mensajeTimer = 0.f;
    
}
void Nivel4State::verificarInteraccionMuro(float dt) {
    // Si el muro ya esta roto, no hacer nada
    if (m_bloqueMuro.muroRoto) return;
    
    // Verificar si el jugador esta cerca del muro
    m_cercaBloqueMuro = m_player.getHurtbox().findIntersection(m_bloqueMuro.area).has_value();
    
    // Actualizar temporizador de cooldown usando dt REAL
    if (m_golpeCooldown) {
        m_golpeTimer -= dt; 
        if (m_golpeTimer <= 0.f) {
            m_golpeCooldown = false;
        }
    }
    
    // Actualizar mensaje temporal usando dt REAL
    if (m_bloqueMuro.mostrandoMensaje) {
        m_bloqueMuro.mensajeTimer -= dt;  
        if (m_bloqueMuro.mensajeTimer <= 0.f) {
            m_bloqueMuro.mostrandoMensaje = false;
        }
    }
    
    // Procesar interacción
    static bool fPresionado = false;
    
    if (m_cercaBloqueMuro && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
        if (!fPresionado && !m_golpeCooldown) {
            fPresionado = true;
            
            // Verificar si tiene la herramienta
            Inventory* inv = m_player.getInventory();
            bool tieneHerramienta = false;
            
            if (inv) {
                for (int i = 0; i < 20; i++) {
                    Item* item = inv->getItem(i);
                    if (item && item->name == "Herramienta") {
                        tieneHerramienta = true;
                        break;
                    }
                }
            }
            
            if (!tieneHerramienta) {
                mostrarMensaje("Necesitas una herramienta para romper este muro", 2.f, sf::Color::Yellow);
                m_golpeCooldown = true;
                m_golpeTimer = 0.5f;
            } else {
                // Aplicar golpe
                m_bloqueMuro.golpesRestantes--;
                
                if (m_bloqueMuro.golpesRestantes > 0) {
                    std::string mensaje = "Golpe restante: " + std::to_string(m_bloqueMuro.golpesRestantes) + "/5";
                    mostrarMensaje(mensaje, 1.5f, sf::Color::Green);
                    
                    // Mostrar mensaje flotante en el bloque
                    m_bloqueMuro.mostrandoMensaje = true;
                    m_bloqueMuro.mensajeTimer = 1.5f;
                    
                    std::cout << "Golpe al muro! Restantes: " << m_bloqueMuro.golpesRestantes << std::endl;
                } else {
                    // MURO ROTO - Teletransportar al Centinela 2
                    m_bloqueMuro.muroRoto = true;
                    mostrarMensaje("MURO ROTO! Has sido transportado...", 2.f, sf::Color::Green);
                    
                    std::cout << "MURO ROTO! Teletransportando al Centinela 2..." << std::endl;
                    
                    // Teletransportar al Centinela 2 
                    LevelTree& levelTree = game->getLevelTree();
                    if (levelTree.jumpToNode("centinela2")) {
                        std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                        if (newState) {
                            game->changeState(std::move(newState));
                        }
                    }
                }
                
                m_golpeCooldown = true;
                m_golpeTimer = 0.5f;
            }
        }
    } else {
        fPresionado = false;
    }
}


void Nivel4State::dibujarMuro() {
    if (m_bloqueMuro.muroRoto) return;
    
    sf::RectangleShape muro(sf::Vector2f(m_bloqueMuro.area.size.x, m_bloqueMuro.area.size.y));
    muro.setPosition(sf::Vector2f(m_bloqueMuro.area.position.x, m_bloqueMuro.area.position.y));
    muro.setFillColor(sf::Color(139, 69, 19, 180));  // Color marrón (madera/piedra)
    muro.setOutlineThickness(2.f);
    muro.setOutlineColor(sf::Color::White);
    window->draw(muro);
    
}

void Nivel4State::handleEvent(const sf::Event& event) {
    // Manejar tecla M para tutorial (abrir/cerrar)
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::M) {
            if (m_mostrarTutorial || m_mostrarTutorialPorTecla) {
                // Cerrar tutorial
                m_mostrarTutorial = false;
                m_mostrarTutorialPorTecla = false;
            } else {
                // Abrir tutorial
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
            return;
        }
    }
    
     // Manejar inventario 
    Inventory* inv = m_player.getInventory();
    if (inv) {
        inv->handleEvent(event, *window);
    }

    // Prioridad: Minijuego de Dardos activo
    if (m_dartsMinigame.isActive()) {
        m_dartsMinigame.handleEvent(event, *window);
        // Cerrar con F 
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::F) {
                m_dartsMinigame.deactivate();
                m_skipPauseThisFrame = true;
                std::cout << "Minijuego de dardos cerrado con F" << std::endl;
            }
        }
        return;
    }

    // Prioridad: Minijuego de Tetris activo
    if (m_tetris.isActive()) {
        m_tetris.handleEvent(event, *window);
        // Cerrar con F 
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::F) {
                m_tetris.deactivate();
                m_skipPauseThisFrame = true;
                std::cout << "Minijuego de Tetris cerrado con F" << std::endl;
            }
        }
        return;
    }
}

void Nivel4State::update(float dt) {
   // Actualizar tamaño de ventana
    sf::Vector2u currentSize = window->getSize();
    if (currentSize != m_lastWindowSize) {
        m_lastWindowSize = currentSize;
        actualizarUIPosiciones();
        window->setView(window->getDefaultView());
    }

    // Teletransportes
    if (m_player.getHurtbox().findIntersection(m_teleportZone).has_value()) {
        m_player.setPosition(m_teleportDestination.x, m_teleportDestination.y);
    }
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

    // Actualizar signos vitales
    m_vitalSignsAndres.update(dt);
    m_vitalSignsAndrea.update(dt);
    
    // ===== VERIFICAR MUERTE DE PACIENTES (AÑADIR ESTO) =====
    if (!m_gameOverTriggered) {
        // Verificar si algún paciente acaba de morir
        if (m_vitalSignsAndres.justDied()) {
            m_vitalSignsAndres.clearJustDied();
            m_mostrandoMuerte = true;
            m_muerteTimer = 3.0f;
            m_mensajeMuerte = "¡HAS MATADO A ANDRES!\nFallo en el minijuego de dardos";
            m_gameOverTriggered = true;
        }
        else if (m_vitalSignsAndrea.justDied()) {
            m_vitalSignsAndrea.clearJustDied();
            m_mostrandoMuerte = true;
            m_muerteTimer = 3.0f;
            m_mensajeMuerte = "¡HAS MATADO A ANDREA!\nFallo en el minijuego de patrones";
            m_gameOverTriggered = true;
        }
    }
    
    // Manejar el timer de muerte
    if (m_mostrandoMuerte) {
        m_muerteTimer -= dt;
        if (m_muerteTimer <= 0.f) {
            m_mostrandoMuerte = false;
            // Mostrar Game Over después del mensaje
            game->changeState(std::make_unique<GameOverState>(window, game));
            return;
        }
    }

    sf::Vector2f posAnterior = m_player.getPosition();

    // Minijuego de Dardos activo
    if (m_dartsMinigame.isActive()) {
        m_dartsMinigame.update(dt);
        m_player.update(dt);
        return;
    }

    // Minijuego de Tetris activo
    if (m_tetris.isActive()) {
        m_tetris.update(dt);
        m_player.update(dt);
        return;
    }

    sf::FloatRect playerBounds = m_player.getHurtbox();

    //  INTERACCION CON TETRIS 
    m_cercaTetris = playerBounds.findIntersection(m_tetrisArea).has_value();

    static bool fTetrisPresionado = false;  // Cambiar nombre de variable

    if (m_cercaTetris && !m_tetris.isActive() && !m_dartsMinigame.isActive()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {  // Cambiar de R a F
            if (!fTetrisPresionado) {
                fTetrisPresionado = true;

                if (m_vitalSignsAndrea.isStabilized()) {
                    mostrarMensaje("Andrea ya esta estabilizada. Puedes continuar", 2.f, sf::Color::Yellow);
                } else if (m_vitalSignsAndrea.isGameOver()) {
                    m_vitalSignsAndrea.reset();
                    m_tetris.reset();
                    m_tetris.activate();
                    mostrarMensaje("Reiniciando patron de Andrea...", 1.5f, sf::Color::Cyan);
                } else if (!m_vitalSignsAndrea.isStabilized()) {
                    m_tetris.reset();
                    m_tetris.activate();
                }
            }
        } else {
            fTetrisPresionado = false;
        }
    }

   //  INTERACCION CON DARDOS 
    m_cercaDarts = playerBounds.findIntersection(m_dartsArea).has_value();

    static bool fDartsPresionado = false;  // Cambiar nombre de variable

    if (m_cercaDarts && !m_dartsMinigame.isActive()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {  // Cambiar de R a F
            if (!fDartsPresionado) {
                fDartsPresionado = true;

                if (m_vitalSignsAndres.isGameOver()) {
                    m_vitalSignsAndres.reset();
                }

                if (!m_vitalSignsAndres.isStabilized()) {
                    m_dartsMinigame.reset();
                    m_dartsMinigame.activate();
                } else {
                    mostrarMensaje("Paciente estabilizado. Ahora ayuda a Andrea", 2.f, sf::Color::Yellow);
                }
            }
        } else {
            fDartsPresionado = false;
        }
    }

    //  VERIFICAR SI AMBOS ESTAN ESTABILIZADOS 
    bool andresEstabilizado = m_vitalSignsAndres.isStabilized();
    bool andreaEstabilizada = m_vitalSignsAndrea.isStabilized();

    if (andresEstabilizado && andreaEstabilizada && !m_ambosEstabilizados) {
        m_ambosEstabilizados = true;
        m_mostrandoMensajeVictoria = true;
        m_mensajeVictoriaTimer = 3.0f;
    }

    // Actualizar timer del mensaje de victoria
    if (m_mostrandoMensajeVictoria) {
        m_mensajeVictoriaTimer -= dt;
        if (m_mensajeVictoriaTimer <= 0.f) {
            m_mostrandoMensajeVictoria = false;
        }
    }

    //  MOVIMIENTO 
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

    // Verificar recoleccion de herramienta
    if (!m_herramientaRecogida) {
        m_cercaHerramienta = m_player.getHurtbox().findIntersection(m_herramientaArea).has_value();
        
        static bool rHerramientaPresionado = false;
        if (m_cercaHerramienta && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
            if (!rHerramientaPresionado) {
                rHerramientaPresionado = true;
                m_herramientaRecogida = true;
                
                // Agregar al inventario
                Inventory* inv = m_player.getInventory();
                if (inv) {
                    Item herramienta("Herramienta", sf::Color(192, 192, 192), "assets/images/niveles/nivel4/herramienta.png");
                    inv->addItem(herramienta);
                }
                
                mostrarMensaje("Herramienta recogida!", 2.f, sf::Color::Green);
            }
        } else {
            rHerramientaPresionado = false;
        }
    }

    // Colisiones
    for (const auto& obj : m_mapaFisico) {
        if (m_player.getHurtbox().findIntersection(obj).has_value()) {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }

    //  CAMARA
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

    //  VERIFICAR SALIDA  
    verificarSalidaNivel();

    //  TELETRANSPORTE POST JUEGO 
    verificarTeletransportePostJuego();
    verificarInteraccionMuro(dt);

    //  PAUSA 
    if (!m_mostrarTutorial && !m_mostrarTutorialPorTecla && !m_escapeConsumed) {
        static bool escapeProcesado = false;
        if (!m_mostrarTutorial && !m_mostrarTutorialPorTecla) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                if (!escapeProcesado) {
                    escapeProcesado = true;
                    game->pushState(std::make_unique<PauseState>(window, game));
                }
            } else {
                escapeProcesado = false;
            }
        }
    }

    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        m_escapeConsumed = false;
    }

    if (m_skipPauseThisFrame) {
        m_skipPauseThisFrame = false;
    }
}
void Nivel4State::verificarSalidaNivel() {
    // Verificar si el jugador esta en el area del ascensor
    m_cercaAscensor = m_player.getHurtbox().findIntersection(m_ascensorArea).has_value();
    
    static bool ePresionado = false;
    
    // NO procesar salida si el inventario está abierto
    Inventory* inv = m_player.getInventory();
    if (inv && inv->isOpen()) {
        return;  
    }
    
    if (m_cercaAscensor && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
        if (!ePresionado) {
            ePresionado = true;
            // Verificar si ambos pacientes estan estabilizados
            if (m_ambosEstabilizados) {
                std::cout << "Saliendo del nivel 4 - Ambos pacientes estabilizados..." << std::endl;
                
                // Cerrar cualquier minijuego activo ANTES de guardar
                if (m_tetris.isActive()) {
                    m_tetris.deactivate();
                }
                if (m_dartsMinigame.isActive()) {
                    m_dartsMinigame.deactivate();
                }
                
                if (game) 
                {
                    game->getLevelTree().jumpToNode("nivel4");
                    std::cout << "LevelTree actualizado a nivel4" << std::endl;
                    
                    // Guardar progreso
                    if (game->tienePartidaActiva()) {
                        game->getSaveManager().setNivelActual(5, 5);
                        game->guardarPartidaActual();
                    }
                    game->avanzarNivel();
                } else {
                    std::cerr << "ERROR: game es nullptr en verificarSalidaNivel" << std::endl;
                }
            } else {
                // Mostrar mensaje si no estan ambos estabilizados
                if (!m_vitalSignsAndres.isStabilized() && !m_vitalSignsAndrea.isStabilized()) {
                    mostrarMensaje("Debes estabilizar a Andres y Andrea para salir", 2.0f, sf::Color::Yellow);
                } else if (!m_vitalSignsAndres.isStabilized()) {
                    mostrarMensaje("Primero debes estabilizar a Andres (Zona de Dardos)", 2.0f, sf::Color::Yellow);
                } else if (!m_vitalSignsAndrea.isStabilized()) {
                    mostrarMensaje("Debes estabilizar a Andrea (Zona de Patrones)", 2.0f, sf::Color::Yellow);
                }
            }
        }
    } else {
        ePresionado = false;
    }
}
void Nivel4State::verificarTeletransportePostJuego() {
    if (!m_vitalSignsAndres.isStabilized()) return;

    // NO procesar teletransporte si el inventario está abierto
    Inventory* inv = m_player.getInventory();
    if (inv && inv->isOpen()) {
        return;
    }

    bool cercaDartsTeletransporte = m_player.getHurtbox().findIntersection(m_dartsArea).has_value();

    static bool fTeletransportePresionado = false;

    if (cercaDartsTeletransporte && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
        if (!fTeletransportePresionado) {
            fTeletransportePresionado = true;
            sf::Vector2f destinoTeletransporte(100.f, 400.f);
            m_player.setPosition(destinoTeletransporte.x, destinoTeletransporte.y);
            mostrarMensaje("Teletransportado... Ahora ve a ayudar a Andrea", 2.f, sf::Color::Green);
        }
    } else {
        fTeletransportePresionado = false;
    }
}

void Nivel4State::draw() {
    if (!window) return;

    // Vista del mundo
    window->setView(m_camera);

    if (m_background)
        window->draw(*m_background);
    else {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(50, 50, 70));
        window->draw(fallback);
    }

    m_player.draw(*window);

    // Dibujar herramienta en el mapa
    if (!m_herramientaRecogida && m_herramientaMapSprite) {
        window->draw(*m_herramientaMapSprite);
    }
    
    // Minijuegos
    if (m_dartsMinigame.isActive()) {
        window->setView(window->getDefaultView());
        m_dartsMinigame.draw(*window);
    }

    if (m_tetris.isActive()) {
        window->setView(window->getDefaultView());
        m_tetris.draw(*window);
    }

    if (m_mostrandoMuerte) {
        sf::View defaultView = window->getDefaultView();
        window->setView(defaultView);
        
        sf::Vector2u winSize = window->getSize();
        float winW = static_cast<float>(winSize.x);
        float winH = static_cast<float>(winSize.y);
        
        // Fondo semitransparente
        sf::RectangleShape overlay(sf::Vector2f(winW, winH));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);
        
        // Panel del mensaje
        sf::RectangleShape panel(sf::Vector2f(winW * 0.7f, 200.f));
        panel.setFillColor(sf::Color(50, 0, 0, 220));
        panel.setOutlineThickness(4.f);
        panel.setOutlineColor(sf::Color::Red);
        panel.setPosition(sf::Vector2f(winW * 0.15f, winH * 0.35f));
        window->draw(panel);
        
        // Texto del mensaje 
        sf::Text muerteText(m_font, m_mensajeMuerte, 30);
        muerteText.setFillColor(sf::Color::Red);
        muerteText.setOutlineThickness(2.f);
        muerteText.setOutlineColor(sf::Color::Black);
        muerteText.setStyle(sf::Text::Style::Bold);
        
        // Centrar el texto 
        sf::FloatRect bounds = muerteText.getLocalBounds();
        muerteText.setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        muerteText.setPosition(sf::Vector2f(winW / 2.f, winH * 0.45f));
        window->draw(muerteText);
        
        // Texto de espera 
        sf::Text waitText(m_font, "Game Over en " + std::to_string((int)std::ceil(m_muerteTimer)) + "...", 20);
        waitText.setFillColor(sf::Color::White);
        waitText.setOutlineThickness(1.f);
        waitText.setOutlineColor(sf::Color::Black);
        
        bounds = waitText.getLocalBounds();
        waitText.setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        waitText.setPosition(sf::Vector2f(winW / 2.f, winH * 0.55f));
        window->draw(waitText);
    }

    // UI EN VISTA POR DEFECTO
    window->setView(window->getDefaultView());


    // Texto de interaccion de Tetris
    if (m_cercaTetris && !m_tetris.isActive() && !m_dartsMinigame.isActive() && m_textoInteraccion && m_fontLoaded) {
         if (m_vitalSignsAndrea.isStabilized()) {
            m_textoInteraccion->setString("Andrea estable. Continua tu camino");
        } else if (m_vitalSignsAndrea.isGameOver()) {
            m_textoInteraccion->setString("Andrea en peligro. Presiona F para reintentar");
        } else {
            m_textoInteraccion->setString("Presiona F para salvar a Andrea");
        }
        
        // Configurar borde y posicion
        m_textoInteraccion->setOutlineThickness(1.5f);
        m_textoInteraccion->setOutlineColor(sf::Color::Black);
        m_textoInteraccion->setCharacterSize(22);  // Un poco mas grande
        
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 100.f)); // Mas arriba
        window->draw(*m_textoInteraccion);
    }

    // Texto de interaccion para la herramienta
    if (!m_herramientaRecogida && m_cercaHerramienta && m_textoInteraccion && m_fontLoaded) {
        m_textoInteraccion->setString("Presiona F para recoger la herramienta");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 90.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto de interaccion para DARDOS
    if (m_cercaDarts && !m_dartsMinigame.isActive() && m_textoInteraccion && m_fontLoaded) {
        if (m_vitalSignsAndres.isStabilized()) {
            m_textoInteraccion->setString("Paciente estable. Ve a ayudar a Andrea | Presiona F");
        } else if (m_vitalSignsAndres.isGameOver()) {
            m_textoInteraccion->setString("Paciente muerto. Presiona F para reintentar");
        } else {
            m_textoInteraccion->setString("Presiona F para salvar a Andres");
        }
        
        // Configurar borde y posicion
        m_textoInteraccion->setOutlineThickness(1.5f);
        m_textoInteraccion->setOutlineColor(sf::Color::Black);
        m_textoInteraccion->setCharacterSize(22);
        
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 100.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto de interaccion para el ASCENSOR (SALIDA)
    if (m_cercaAscensor && m_textoInteraccion && m_fontLoaded) {
        if (m_ambosEstabilizados) {
            m_textoInteraccion->setString("Presiona F para ir al siguiente nivel");
        } else {
            if (!m_vitalSignsAndres.isStabilized() && !m_vitalSignsAndrea.isStabilized()) {
                m_textoInteraccion->setString("Necesitas estabilizar a Andres y Andrea");
            } else if (!m_vitalSignsAndres.isStabilized()) {
                m_textoInteraccion->setString("Necesitas estabilizar a Andres (Zona de Dardos)");
            } else if (!m_vitalSignsAndrea.isStabilized()) {
                m_textoInteraccion->setString("Necesitas estabilizar a Andrea (Zona de Patrones)");
            }
        }
        
        // Configurar de posicion
        m_textoInteraccion->setOutlineThickness(1.5f);
        m_textoInteraccion->setOutlineColor(sf::Color::Black);
        m_textoInteraccion->setCharacterSize(22);
        
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 100.f));
        window->draw(*m_textoInteraccion);
    }
    // Texto de interacción para el MURO ROMPIBLE (CENTINELA)
    if (m_cercaBloqueMuro && !m_bloqueMuro.muroRoto && m_textoInteraccion && m_fontLoaded) {
        Inventory* inv = m_player.getInventory();
        bool tieneHerramienta = false;
        
        if (inv) {
            for (int i = 0; i < 20; i++) {
                Item* item = inv->getItem(i);
                if (item && item->name == "Herramienta") {
                    tieneHerramienta = true;
                    break;
                }
            }
        }
        
        if (tieneHerramienta) {
            m_textoInteraccion->setString("Presiona F para romper el muro (" + 
                                        std::to_string(m_bloqueMuro.golpesRestantes) + "/5 golpes restantes)");
        } else {
            m_textoInteraccion->setString("Necesitas una herramienta para romper este muro");
        }
        
        // Configurar estilo del texto
        m_textoInteraccion->setOutlineThickness(1.5f);
        m_textoInteraccion->setOutlineColor(sf::Color::Black);
        m_textoInteraccion->setCharacterSize(22);
        
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 100.f));
        window->draw(*m_textoInteraccion);
    }
    // Mensaje temporal
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f && !m_textoMensaje->getString().isEmpty()) {
        sf::Vector2u winSize = window->getSize();
        m_textoMensaje->setPosition(sf::Vector2f(winSize.x / 2.f, winSize.y / 3.f));
        window->draw(*m_textoMensaje);
    }

    // Mensaje de victoria 
    if (m_mostrandoMensajeVictoria) {
        sf::Vector2u winSize = window->getSize();
        float winW = static_cast<float>(winSize.x);
        float winH = static_cast<float>(winSize.y);

        sf::RectangleShape fondoMensaje;
        fondoMensaje.setSize(sf::Vector2f(winW * 0.8f, 120.f));
        fondoMensaje.setFillColor(sf::Color(0, 0, 0, 200));
        fondoMensaje.setOutlineThickness(3.f);
        fondoMensaje.setOutlineColor(sf::Color(0, 255, 0));
        fondoMensaje.setPosition(sf::Vector2f(winW * 0.1f, winH * 0.35f));

        sf::Text textoVictoria(m_font,
            "AMBOS PACIENTES ESTABILIZADOS!\nVe al ascensor y presiona F",
            30);
        textoVictoria.setFillColor(sf::Color::Green);
        textoVictoria.setOutlineThickness(1.f);
        textoVictoria.setOutlineColor(sf::Color::Black);
        textoVictoria.setStyle(sf::Text::Bold);

        sf::FloatRect bounds = textoVictoria.getLocalBounds();
        textoVictoria.setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        textoVictoria.setPosition(sf::Vector2f(winW / 2.f, winH * 0.42f));

        window->draw(fondoMensaje);
        window->draw(textoVictoria);
    }

    // Signos vitales
    m_vitalSignsAndres.draw(*window);
    m_vitalSignsAndrea.draw(*window);

    // Inventario
    Inventory* inv = m_player.getInventory();
    if (inv) inv->draw(*window);

    // Coordenadas debug
    if (m_textoCoordenadas && m_fontLoaded) {
        window->draw(*m_textoCoordenadas);
    }
    // MOSTRAR MENSAJE DE MUERTE 
    if (m_mostrandoMuerte) {
        sf::View defaultView = window->getDefaultView();
        window->setView(defaultView);
        
        sf::Vector2u winSize = window->getSize();
        float winW = static_cast<float>(winSize.x);
        float winH = static_cast<float>(winSize.y);
        
        // Fondo semitransparente
        sf::RectangleShape overlay(sf::Vector2f(winW, winH));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);
        
        // Panel del mensaje
        sf::RectangleShape panel(sf::Vector2f(winW * 0.7f, 200.f));
        panel.setFillColor(sf::Color(50, 0, 0, 220));
        panel.setOutlineThickness(4.f);
        panel.setOutlineColor(sf::Color::Red);
        panel.setPosition(sf::Vector2f(winW * 0.15f, winH * 0.35f));
        window->draw(panel);
        
        // TEXTO DEL MENSAJE DE MUERTE - CORREGIDO
        sf::Text muerteText(m_font, m_mensajeMuerte, 30);
        muerteText.setFillColor(sf::Color::Red);
        muerteText.setOutlineThickness(2.f);
        muerteText.setOutlineColor(sf::Color::Black);
        muerteText.setStyle(sf::Text::Style::Bold);
        
        // Centrar el texto - USAR / 2.f, NO / 2.5
        sf::FloatRect boundsMuerte = muerteText.getLocalBounds();
        muerteText.setOrigin(sf::Vector2f(boundsMuerte.size.x / 2.f, boundsMuerte.size.y / 2.f));
        muerteText.setPosition(sf::Vector2f(winW / 2.f, winH * 0.45f));
        window->draw(muerteText);
        
        // TEXTO DE ESPERA - CORREGIDO
        sf::Text waitText(m_font, "Game Over en " + std::to_string((int)std::ceil(m_muerteTimer)) + "...", 20);
        waitText.setFillColor(sf::Color::White);
        waitText.setOutlineThickness(1.5f);
        waitText.setOutlineColor(sf::Color::Black);
        
        // Centrar el texto de espera - USAR / 2.f
        sf::FloatRect boundsWait = waitText.getLocalBounds();
        waitText.setOrigin(sf::Vector2f(boundsWait.size.x / 2.f, boundsWait.size.y / 2.f));
        waitText.setPosition(sf::Vector2f(winW / 2.f, winH * 0.55f));
        window->draw(waitText);
    }
    // Tutorial
    if (m_mostrarTutorial || m_mostrarTutorialPorTecla) {
        sf::RectangleShape overlay(sf::Vector2f(window->getSize().x, window->getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);

        if (m_fontLoaded) {
            sf::Text tutorialText(m_font);
            tutorialText.setString(
                "ESTABILIZAR SIGNOS VITALES\n"
                "Debes estabilizar a Andres y Andrea para poder salir.\n\n"
                "ZONA DE ANDRES (DARDOS):\n"
                "- Acercate a la zona de dardos y presiona F\n"
                "- Gana el minijuego para estabilizar a Andres\n\n"
                "ZONA DE ANDREA (PATRONES):\n"
                "- Acercate a la zona de patrones y presiona F\n"
                "- Completa los patrones para estabilizar a Andrea\n\n"
                "Una vez ambos estabilizados, ve al ASCENSOR y presiona F\n\n"
                "[M] Abrir/Cerrar"
            );
            tutorialText.setCharacterSize(18);
            tutorialText.setFillColor(sf::Color::White);
            sf::FloatRect textBounds = tutorialText.getLocalBounds();
            tutorialText.setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            tutorialText.setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y / 2.f));
            window->draw(tutorialText);
        }
    }
}

void Nivel4State::mostrarMensaje(const std::string& texto, float duracion, sf::Color color) {
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

void Nivel4State::actualizarUIPosiciones() {
    if (!window) return;

    sf::Vector2u winSize = window->getSize();
    float winW = static_cast<float>(winSize.x);
    float winH = static_cast<float>(winSize.y);

    float scale = winH / 720.f;
    scale = std::clamp(scale, 0.7f, 1.5f);

    // Minijuego de dardos
    float dartsW = winW * 0.8f;
    float dartsH = winH * 0.93f;
    m_dartsMinigame.setSize(sf::Vector2f(dartsW, dartsH));
    m_dartsMinigame.setPosition(sf::Vector2f(
        (winW - dartsW) * 0.5f,
        (winH - dartsH) * 0.5f
    ));

    // Minijuego de Tetris
    float tetrisW = std::min(winW * 0.65f, 650.f);
    float tetrisH = std::min(winH * 0.7f, 550.f);
    m_tetris.setSize(sf::Vector2f(tetrisW, tetrisH));
    m_tetris.setPosition(sf::Vector2f(
        (winW - tetrisW) * 0.5f,
        (winH - tetrisH) * 0.5f
    ));

    float marginBottom = 100.f * scale;
    float marginLeftAndres = 20.f * scale;
    float marginRightAndrea = 20.f * scale;

    // ANDRÉS - Esquina inferior IZQUIERDA
    m_vitalSignsAndres.setAnchorRight(false);
    m_vitalSignsAndres.setLeftMargin(marginLeftAndres);
    m_vitalSignsAndres.setBottomMargin(marginBottom);

    // ANDREA - Esquina inferior DERECHA
    m_vitalSignsAndrea.setAnchorRight(true);    
    m_vitalSignsAndrea.setLeftMargin(marginRightAndrea);  
    m_vitalSignsAndrea.setBottomMargin(marginBottom);
}