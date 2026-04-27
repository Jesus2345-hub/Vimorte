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
    m_vitalSigns()
{
    
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
    m_player.setPosition(900.f, 800.f);
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
    m_camera = sf::View(sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
                        sf::Vector2f(static_cast<float>(windowSize.x),
                                     static_cast<float>(windowSize.y)));

    
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(1550.f, 1350.f), sf::Vector2f(120.f, 180.f));
    configurarColisiones();

    // Configurar área de dardos 
    m_dartsArea = sf::FloatRect(sf::Vector2f(30.f, 40.f ), sf::Vector2f( 150.f,  150.f));   
    m_dartsTriggerRadius = 150.f;

    // Configurar minijuego de dardos
    m_dartsMinigame.setSize(sf::Vector2f(900.f, 600.f));
    m_dartsMinigame.setPosition(sf::Vector2f(
        (windowSize.x - 900.f) / 2.f,
        (windowSize.y - 600.f) / 2.f
    ));
    m_dartsMinigame.setVitalSigns(&m_vitalSigns);
    
    // Configurar signos estáticos de Andrea (oscilan pero no se modifican con dardos)
    sf::Vector2u winSize = window->getSize();
    float panelWidth = 200.f;             
    float marginRight = 20.f;             
    float leftPos = winSize.x - panelWidth - marginRight;

    m_vitalSignsStatic.setLeftMargin(leftPos);
    m_vitalSignsStatic.setBottomMargin(100.f);   
    m_vitalSignsStatic.setTitle("SIGNOS VITALES - ANDREA");

    // Fuente
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");

    if (m_fontLoaded) {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setCharacterSize(20);
        m_textoInteraccion->setFillColor(sf::Color::White);

        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(24);
        m_textoMensaje->setFillColor(sf::Color::Yellow);
    }

    // Guardado automático
    if (game->tienePartidaActiva()) {
        game->getSaveManager().setNivelActual(4, 4);
        game->guardarPartidaActual();
        std::cout << "Partida guardada en NivelSara1m" << std::endl;
    }
     // Zona de teletransporte (coordenadas absolutas del mundo)
    m_teleportZone = sf::FloatRect(sf::Vector2f(70.f, 320.f), sf::Vector2f(60.f, 60.f));
    m_teleportDestination = sf::Vector2f(1500.f, 350.f);
     // Segundo teletransporte (coordenadas absolutas del mundo)
    m_teleportZone2 = sf::FloatRect(sf::Vector2f(1200.f, 120.f), sf::Vector2f(60.f, 60.f));
    m_teleportDestination2 = sf::Vector2f(450.f, 120.f);
    std::cout << "NivelSara1State inicializado correctamente" << std::endl;
}

void NivelSara1State::configurarColisiones() {
    m_mapaFisico.clear();
    // Paredes exteriores
    m_mapaFisico.emplace_back(sf::Vector2f(30.f, 12.f), sf::Vector2f(m_worldSize.x - 60.f, 130.f));
    m_mapaFisico.emplace_back(sf::Vector2f(18.f, 12.f), sf::Vector2f(20.f, m_worldSize.y - 24.f));
    m_mapaFisico.emplace_back(sf::Vector2f(m_worldSize.x - 50.f, 12.f), sf::Vector2f(20.f, m_worldSize.y - 24.f));
    m_mapaFisico.emplace_back(sf::Vector2f(30.f, m_worldSize.y - 60.f), sf::Vector2f(m_worldSize.x - 60.f, 30.f));
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
    m_vitalSigns.update(dt);
    m_vitalSignsStatic.update(dt);
    // Comprobar si el paciente murió o se estabilizó mientras el minijuego estaba activo
    if (m_dartsMinigame.isActive()) {
        if (m_vitalSigns.isGameOver()) {
            m_dartsMinigame.deactivate();
            mostrarMensaje("El paciente ha muerto. Presiona R para reintentar", 3.f, sf::Color::White);
        } else if (m_vitalSigns.isStabilized()) {
            m_dartsMinigame.deactivate();
            mostrarMensaje("Paciente estabilizado. Ya puedes salir por la puerta", 3.f, sf::Color::Green);
        }
    }

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
            if (m_vitalSigns.isGameOver()) {
                m_vitalSigns.reset();
            }
            // Solo activamos si NO está estabilizado
            if (!m_vitalSigns.isStabilized()) {
                m_dartsMinigame.reset();
                m_dartsMinigame.activate();
            } else {
                mostrarMensaje("El paciente ya esta estable. Sal por la puerta.", 2.f, sf::Color::Yellow);
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

    // Cámara centrada en el jugador
    sf::Vector2f playerPos = m_player.getPosition();
    sf::Vector2f cameraPos = playerPos;
    float halfWidth = static_cast<float>(windowSize.x) / 2.f;
    float halfHeight = static_cast<float>(windowSize.y) / 2.f;

    if (halfWidth * 2.f >= m_worldSize.x)
        cameraPos.x = m_worldSize.x / 2.f;
    else
        cameraPos.x = std::clamp(cameraPos.x, halfWidth, m_worldSize.x - halfWidth);

    if (halfHeight * 2.f >= m_worldSize.y)
        cameraPos.y = m_worldSize.y / 2.f;
    else
        cameraPos.y = std::clamp(cameraPos.y, halfHeight, m_worldSize.y - halfHeight);

    m_camera.setCenter(cameraPos);

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

    // Elementos de depuración (colisiones)
    window->setView(m_camera);
    for (const auto& rect : m_mapaFisico) {
        sf::RectangleShape bloque;
        bloque.setPosition(sf::Vector2f(rect.position.x, rect.position.y));
        bloque.setSize(sf::Vector2f(rect.size.x, rect.size.y));
        bloque.setFillColor(sf::Color(255, 0, 0, 100));
        bloque.setOutlineThickness(2.f);
        bloque.setOutlineColor(sf::Color::Red);
        window->draw(bloque);
    }
    // Área de dardos (debug)
    sf::RectangleShape dartsRect(m_dartsArea.size);
    dartsRect.setPosition(m_dartsArea.position);
    dartsRect.setFillColor(sf::Color(0, 255, 0, 120));
    dartsRect.setOutlineThickness(2.f);
    dartsRect.setOutlineColor(sf::Color::Green);
    window->draw(dartsRect);

    //teletrasnportacion
    sf::RectangleShape teleRect(m_teleportZone.size);
    teleRect.setPosition(m_teleportZone.position);
    teleRect.setFillColor(sf::Color(255, 0, 255, 100)); 
    window->draw(teleRect);

    sf::RectangleShape teleRect2(m_teleportZone2.size);
    teleRect2.setPosition(m_teleportZone2.position);
    teleRect2.setFillColor(sf::Color(0, 255, 255, 150));
    teleRect2.setOutlineThickness(2.f);
    teleRect2.setOutlineColor(sf::Color::Cyan);
    window->draw(teleRect2);

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
    if (m_vitalSigns.isStabilized()) {
        m_textoInteraccion->setString("Paciente estable. Ve a la puerta (E)");
    } else if (m_vitalSigns.isGameOver()) {
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

    // 3. Minijuego (si está activo) - SE DIBUJA PRIMERO QUE LOS SIGNOS VITALES
    if (m_dartsMinigame.isActive()) {
        m_dartsMinigame.draw(*window);
    }

    // 4. Signos vitales (se dibujan DESPUÉS del minijuego para que sus mensajes no queden tapados)
    m_vitalSigns.draw(*window);
    m_vitalSignsStatic.draw(*window);

    // 5. Inventario (siempre al final)
    Inventory* inv = m_player.getInventory();
    if (inv) inv->draw(*window);
    
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