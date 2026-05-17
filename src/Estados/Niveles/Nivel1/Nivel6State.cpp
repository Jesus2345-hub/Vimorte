#include "Estados/Niveles/Nivel1/Nivel6State.hpp"
#include "Estados/PauseState.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <fstream>

Nivel6State::Nivel6State(sf::RenderWindow *window, Game *game)
    : State(window, game),
      m_background(nullptr),
      m_cercaPuertaSalida(false),
      m_mostrarPuertaSalida(true),
      m_escapeConsumed(false),
      m_mostrarTutorial(false),
      m_mostrarTutorialPorTecla(false),
      m_fontLoaded(false),
      m_bloquearR(false)
{
    m_msjActual.texto = "";
    m_msjActual.tiempoRestante = 0.0f;
    m_msjActual.color = sf::Color::Yellow;

    m_player.loadAssets();
    m_player.setPosition(962.f, 261.f);
    m_player.setSpeed(300.0f);

    if (game->tienePartidaActiva())
    {
        const auto &items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
        auto it = std::find(items.begin(), items.end(), "TutorialNivel6Visto");
        if (it == items.end())
        {
            m_mostrarTutorial = true;
            game->getSaveManager().addItemRecolectado("TutorialNivel6Visto");
        }
    }

    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel6/background.png"))
    {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x), static_cast<float>(textureSize.y));
        std::cout << "Nivel 6 cargado. Tamaño: " << m_worldSize.x << "x" << m_worldSize.y << std::endl;
    }
    else
    {
        //
        std::cerr << "Error: No se pudo cargar background del nivel 6" << std::endl;
        m_worldSize = sf::Vector2f(1754.f, 1587.f);
    }

    // Cargar textura de mesa
    if (m_mesaTexture.loadFromFile("assets/images/niveles/nivel6/mesa.png"))
    {
        m_mesaTexture.setSmooth(true);

        sf::Vector2u texSize = m_mesaTexture.getSize();
        float tamanoMesa = 110.f;
        float escalaX = tamanoMesa / texSize.x;
        float escalaY = tamanoMesa / texSize.y;
        float mitad = tamanoMesa / 2.f;

        // Columna izquierda (X=122)
        auto mesa1 = std::make_unique<sf::Sprite>(m_mesaTexture);
        mesa1->setScale(sf::Vector2f(escalaX, escalaY));
        mesa1->setOrigin(sf::Vector2f(texSize.x / 2.f, texSize.y / 2.f));
        mesa1->setPosition(sf::Vector2f(122.f, 459.f));
        m_mesas.push_back(std::move(mesa1));
        m_mesasBounds.push_back(sf::FloatRect(sf::Vector2f(122.f - mitad, 459.f - mitad), sf::Vector2f(tamanoMesa, tamanoMesa)));

        auto mesa2 = std::make_unique<sf::Sprite>(m_mesaTexture);
        mesa2->setScale(sf::Vector2f(escalaX, escalaY));
        mesa2->setOrigin(sf::Vector2f(texSize.x / 2.f, texSize.y / 2.f));
        mesa2->setPosition(sf::Vector2f(122.f, 654.f));
        m_mesas.push_back(std::move(mesa2));
        m_mesasBounds.push_back(sf::FloatRect(sf::Vector2f(122.f - mitad, 654.f - mitad), sf::Vector2f(tamanoMesa, tamanoMesa)));

        auto mesa3 = std::make_unique<sf::Sprite>(m_mesaTexture);
        mesa3->setScale(sf::Vector2f(escalaX, escalaY));
        mesa3->setOrigin(sf::Vector2f(texSize.x / 2.f, texSize.y / 2.f));
        mesa3->setPosition(sf::Vector2f(122.f, 849.f));
        m_mesas.push_back(std::move(mesa3));
        m_mesasBounds.push_back(sf::FloatRect(sf::Vector2f(122.f - mitad, 849.f - mitad), sf::Vector2f(tamanoMesa, tamanoMesa)));

        // Columna derecha (X=397)
        auto mesa4 = std::make_unique<sf::Sprite>(m_mesaTexture);
        mesa4->setScale(sf::Vector2f(escalaX, escalaY));
        mesa4->setOrigin(sf::Vector2f(texSize.x / 2.f, texSize.y / 2.f));
        mesa4->setPosition(sf::Vector2f(397.f, 459.f));
        m_mesas.push_back(std::move(mesa4));
        m_mesasBounds.push_back(sf::FloatRect(sf::Vector2f(397.f - mitad, 459.f - mitad), sf::Vector2f(tamanoMesa, tamanoMesa)));

        auto mesa5 = std::make_unique<sf::Sprite>(m_mesaTexture);
        mesa5->setScale(sf::Vector2f(escalaX, escalaY));
        mesa5->setOrigin(sf::Vector2f(texSize.x / 2.f, texSize.y / 2.f));
        mesa5->setPosition(sf::Vector2f(397.f, 654.f));
        m_mesas.push_back(std::move(mesa5));
        m_mesasBounds.push_back(sf::FloatRect(sf::Vector2f(397.f - mitad, 654.f - mitad), sf::Vector2f(tamanoMesa, tamanoMesa)));

        auto mesa6 = std::make_unique<sf::Sprite>(m_mesaTexture);
        mesa6->setScale(sf::Vector2f(escalaX, escalaY));
        mesa6->setOrigin(sf::Vector2f(texSize.x / 2.f, texSize.y / 2.f));
        mesa6->setPosition(sf::Vector2f(397.f, 849.f));
        m_mesas.push_back(std::move(mesa6));
        m_mesasBounds.push_back(sf::FloatRect(sf::Vector2f(397.f - mitad, 849.f - mitad), sf::Vector2f(tamanoMesa, tamanoMesa)));
    }

    // ===== CAMBUR (523, 291) =====
    if (m_camburTexture.loadFromFile("assets/images/niveles/nivel6/cambur.png"))
    {
        m_camburTexture.setSmooth(true);
        m_camburSprite = std::make_unique<sf::Sprite>(m_camburTexture);

        float escalaCambur = 0.16f;
        m_camburSprite->setScale(sf::Vector2f(escalaCambur, escalaCambur));

        sf::FloatRect bounds = m_camburSprite->getLocalBounds();
        m_camburSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_camburSprite->setPosition(sf::Vector2f(523.f, 291.f));

        m_camburArea = sf::FloatRect(sf::Vector2f(523.f - 40.f, 291.f - 40.f), sf::Vector2f(80.f, 80.f));
    }

    // ===== DESTORNILLADOR encima de la mesa 5 (397, 654) =====
    if (m_destornilladorTexture.loadFromFile("assets/images/items/destornillador.png"))
    {
        m_destornilladorTexture.setSmooth(true);
        m_destornilladorSprite = std::make_unique<sf::Sprite>(m_destornilladorTexture);

        float escalaDestornillador = 0.015f; // ← AJUSTA ESTE VALOR
        m_destornilladorSprite->setScale(sf::Vector2f(escalaDestornillador, escalaDestornillador));

        sf::FloatRect bounds = m_destornilladorSprite->getLocalBounds();
        m_destornilladorSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_destornilladorSprite->setPosition(sf::Vector2f(397.f, 610.f));

        m_destornilladorArea = sf::FloatRect(sf::Vector2f(397.f - 40.f, 600.f - 40.f), sf::Vector2f(80.f, 80.f));
    }

    // ===== REJILLA DEL CENTINELA (1603, 631) =====
    if (m_rejillaTexture.loadFromFile("assets/images/niveles/nivel6/rejilla.png"))
    {
        m_rejillaSprite = std::make_unique<sf::Sprite>(m_rejillaTexture);
        m_rejillaSprite->setScale(sf::Vector2f(0.12f, 0.12f));
        sf::FloatRect bounds = m_rejillaSprite->getLocalBounds();
        m_rejillaSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_rejillaSprite->setPosition(sf::Vector2f(1603.f, 651.f));
    }

    m_entradaCentinelaArea = sf::FloatRect(sf::Vector2f(1570.f, 620.f), sf::Vector2f(70.f, 70.f));

    sf::Vector2u windowSize = window->getSize();
    float fixedWidth = 1280.f;
    float fixedHeight = 720.f;
    m_camera = sf::View(sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
                        sf::Vector2f(fixedWidth, fixedHeight));
    m_lastWindowSize = windowSize;

    // Área de salida SOLO en la parte inferior del ascensor
    m_puertaSalidaArea = sf::FloatRect(
        sf::Vector2f(866.f, 61.f), // X, Y (parte baja del ascensor)
        sf::Vector2f(192.f, 216.f) // Ancho, Alto (solo 216px de altura)
    );

    configurarColisiones();

    // Rifle
    m_rifleRecogido = false;
    // Rifle encima de la mesa 2 (122, 654) - tamaño 100
    m_rifleArea = sf::FloatRect(sf::Vector2f(122.f - 50.f, 654.f - 90.f), sf::Vector2f(100.f, 100.f));
    m_cercaRifle = false;

    // Cargar sprite del rifle en el mapa
    if (m_rifleMapTexture.loadFromFile("assets/images/items/rifle.png"))
    {
        m_rifleMapSprite = std::make_unique<sf::Sprite>(m_rifleMapTexture);
        m_rifleMapSprite->setScale(sf::Vector2f(0.05f, 0.05f)); // Ajustar tamaño
        sf::FloatRect bounds = m_rifleMapSprite->getLocalBounds();
        m_rifleMapSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_rifleMapSprite->setPosition(sf::Vector2f(122.f, 610.f));
    }

    // Gallo
    m_gallo.setPosition(1491.f, 773.f);
    m_gallo.setLimites(1370.f, 1620.f);
    m_cercaGallo = false;

    // Joven
    m_joven.setPosition(303.f, 260.f);
    m_joven.setScale(0.2f, 0.2f);
    m_cercaJoven = false;

    // Abuelita
    m_abuelita.setPosition(1540.f, 327.f);
    m_abuelita.setScale(0.12f, 0.12f);
    m_cercaAbuelita = false;
    m_llaveObtenida = false;
    m_dientesObtenidos = false;
    m_gallinaObtenida = false;

    // RoosterHunt
    float rw = windowSize.x * 0.8f;
    float rh = windowSize.y * 0.85f;
    m_roosterHuntMinigame.setSize(sf::Vector2f(rw, rh));
    m_roosterHuntMinigame.setPosition(sf::Vector2f((windowSize.x - rw) / 2.f, (windowSize.y - rh) / 2.f));

    // Tic Tac Toe
    float ttw = windowSize.x * 0.7f;
    float tth = windowSize.y * 0.8f;
    m_tictactoeMinigame.setSize(sf::Vector2f(ttw, tth));
    m_tictactoeMinigame.setPosition(sf::Vector2f((windowSize.x - ttw) / 2.f, (windowSize.y - tth) / 2.f));

    // Fuente
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (m_fontLoaded)
    {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setCharacterSize(20);
        m_textoInteraccion->setFillColor(sf::Color::White);
        m_textoInteraccion->setOutlineThickness(1.5f);
        m_textoInteraccion->setOutlineColor(sf::Color::Black);
        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(24);
        m_textoMensaje->setFillColor(sf::Color::Yellow);
    }

    if (game->tienePartidaActiva())
    {
        game->getSaveManager().setNivelActual(6, 6);
        game->guardarPartidaActual();
    }

    game->setIsInLevel(true);

    // Verificar si el archivo existe antes de cambiarlo
    std::ifstream file("assets/sounds/nivel1.ogg");
    if (file.good())
    {
        file.close();
        game->cambiarMusica("assets/sounds/nivel1.ogg");
    }
    else
    {
        std::cout << "⚠️ Archivo de música no encontrado: assets/sounds/nivel1.ogg" << std::endl;
    }

    std::cout << "Nivel6State inicializado correctamente" << std::endl;
}

void Nivel6State::handleEvent(const sf::Event &event)
{
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
            // Forzar mostrar el tutorial SIEMPRE al presionar M
            m_mostrarTutorialPorTecla = true;
            std::cout << "M presionada - Mostrando tutorial del Nivel 6" << std::endl;
        }
        if (keyPressed->code == sf::Keyboard::Key::F3)
        {
            m_debugMode = !m_debugMode;
        }
    }

    if (m_roosterHuntMinigame.isActive())
    {
        m_roosterHuntMinigame.handleEvent(event, *window);
        if (event.is<sf::Event::KeyPressed>())
        {
            const auto &keyEvent = event.getIf<sf::Event::KeyPressed>();
            if (keyEvent->code == sf::Keyboard::Key::F)
            {
                m_roosterHuntMinigame.deactivate();
                window->setMouseCursorVisible(true);
                m_bloquearR = true;
                m_tiempoUltimaR.restart();
                return;
            }
        }
    }
    else if (m_tictactoeMinigame.isActive())
    {
        m_tictactoeMinigame.handleEvent(event, *window);
        if (event.is<sf::Event::KeyPressed>())
        {
            const auto &keyEvent = event.getIf<sf::Event::KeyPressed>();
            if (keyEvent->code == sf::Keyboard::Key::F)
            {
                m_tictactoeMinigame.deactivate();
                return;
            }
        }
    }

    Inventory *inv = m_player.getInventory();
    if (inv)
        inv->handleEvent(event, *window);
}

void Nivel6State::verificarEntradaCentinela() {}

void Nivel6State::verificarSalidaNivel()
{
    m_cercaPuertaSalida = m_player.getHurtbox().findIntersection(m_puertaSalidaArea).has_value();

    static bool rPresionadoSalida = false;
    if (m_cercaPuertaSalida)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
        {
            if (!rPresionadoSalida)
            {
                rPresionadoSalida = true;

                // Verificar si tiene la llave en el inventario
                Inventory *inv = m_player.getInventory();
                bool tieneLlave = false;
                if (inv)
                {
                    for (int i = 0; i < 15; i++)
                    {
                        Item *item = inv->getItem(i);
                        if (item && item->name == "Llave")
                        {
                            tieneLlave = true;
                            break;
                        }
                    }
                }

                if (tieneLlave || m_llaveObtenida)
                {
                    game->detenerMusica();
                    game->avanzarNivel();
                }
                else
                {
                    mostrarMensaje("Necesitas la llave para salir. Busca a la abuelita.", 2.f, sf::Color::Red);
                }
            }
        }
        else
        {
            rPresionadoSalida = false;
        }
    }
}

void Nivel6State::update(float dt)
{
    // ===== DETECTAR CAMBIO DE TAMAÑO DE VENTANA (F11) - DEBE IR PRIMERO =====
    sf::Vector2u currentSize = window->getSize();
    static sf::Vector2u lastSize = currentSize;
    if (currentSize != lastSize)
    {
        lastSize = currentSize;
        float minijuegoW = currentSize.x * 0.8f;
        float minijuegoH = currentSize.y * 0.85f;
        float minijuegoX = (currentSize.x - minijuegoW) / 2.f;
        float minijuegoY = (currentSize.y - minijuegoH) / 2.f;
        
        m_roosterHuntMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
        m_roosterHuntMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
        
        float ttw = currentSize.x * 0.7f;
        float tth = currentSize.y * 0.8f;
        float ttx = (currentSize.x - ttw) / 2.f;
        float tty = (currentSize.y - tth) / 2.f;
        
        m_tictactoeMinigame.setSize(sf::Vector2f(ttw, tth));
        m_tictactoeMinigame.setPosition(sf::Vector2f(ttx, tty));
    }

    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f)
    {
        m_msjActual.tiempoRestante -= dt;
        if (m_msjActual.tiempoRestante <= 0.0f)
            m_textoMensaje->setString("");
    }

    

    if (m_bloquearR && m_tiempoUltimaR.getElapsedTime().asSeconds() > 0.5f)
    {
        m_bloquearR = false;
    }

    // Actualizar entidades SIEMPRE
    m_gallo.update(dt);
    m_gallo.verificarColisionJugador(m_player.getHurtbox());
    m_joven.update(dt);
    m_abuelita.update(dt);

    sf::Vector2f posAnterior = m_player.getPosition();

    // Áreas de entidades
    m_galloArea = m_gallo.getBounds();
    sf::FloatRect galloInteraccion(sf::Vector2f(m_galloArea.position.x - 60.f, m_galloArea.position.y - 60.f), sf::Vector2f(m_galloArea.size.x + 120.f, m_galloArea.size.y + 120.f));
    m_cercaGallo = m_player.getHurtbox().findIntersection(galloInteraccion).has_value();

    m_jovenArea = m_joven.getBounds();
    sf::FloatRect jovenInteraccion(sf::Vector2f(m_jovenArea.position.x - 80.f, m_jovenArea.position.y - 80.f), sf::Vector2f(m_jovenArea.size.x + 160.f, m_jovenArea.size.y + 160.f));
    m_cercaJoven = m_player.getHurtbox().findIntersection(jovenInteraccion).has_value();

    m_abuelitaArea = m_abuelita.getBounds();
    sf::FloatRect abuelitaInteraccion(sf::Vector2f(m_abuelitaArea.position.x - 60.f, m_abuelitaArea.position.y - 60.f), sf::Vector2f(m_abuelitaArea.size.x + 120.f, m_abuelitaArea.size.y + 120.f));
    m_cercaAbuelita = m_player.getHurtbox().findIntersection(abuelitaInteraccion).has_value();

    // Rifle
    if (!m_rifleRecogido)
    {
        m_cercaRifle = m_player.getHurtbox().findIntersection(m_rifleArea).has_value();
        static bool rRiflePresionado = false;
        if (m_cercaRifle && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
        {
            if (!rRiflePresionado)
            {
                rRiflePresionado = true;
                m_rifleRecogido = true;

                // AGREGAR AL INVENTARIO
                Inventory *inv = m_player.getInventory();
                if (inv)
                {
                    Item rifle("Rifle", sf::Color(139, 90, 43), "assets/images/items/rifle.png"); // Color marrón
                    inv->addItem(rifle);
                }

                mostrarMensaje("Rifle recogido!", 2.f, sf::Color::Green);
            }
        }
        else
            rRiflePresionado = false;
    }

    // Activar RoosterHunt
    if (m_rifleRecogido && m_cercaGallo && !m_roosterHuntMinigame.isActive() && !m_tictactoeMinigame.isActive())
    {
        static bool rGalloPresionado = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
        {
            if (!rGalloPresionado)
            {
                rGalloPresionado = true;
                m_roosterHuntMinigame.activate();
            }
        }
        else
            rGalloPresionado = false;
    }

    if (m_roosterHuntMinigame.isActive())
    {
        m_roosterHuntMinigame.update(dt);
        m_player.update(dt);
        window->setMouseCursorVisible(false);

        if (m_roosterHuntMinigame.isGameWon() && !m_gallinaObtenida)
        {
            m_gallinaObtenida = true;

            // AGREGAR AL INVENTARIO
            Inventory *inv = m_player.getInventory();
            if (inv)
            {
                Item gallina("Gallina", sf::Color(255, 220, 180), "assets/images/items/gallo.png"); // Color crema
                inv->addItem(gallina);
            }

            m_bloquearR = true;
            m_tiempoUltimaR.restart();
            mostrarMensaje("Gallina obtenida!", 2.f, sf::Color::Green);
        }
        sf::Vector2f pp = m_player.getPosition();
        sf::Vector2f cp = pp;
        float hw = 640.f, hh = 360.f;
        if (hw * 2 >= m_worldSize.x)
            cp.x = m_worldSize.x / 2;
        else
        {
            if (cp.x < hw)
                cp.x = hw;
            if (cp.x > m_worldSize.x - hw)
                cp.x = m_worldSize.x - hw;
        }
        if (hh * 2 >= m_worldSize.y)
            cp.y = m_worldSize.y / 2;
        else
        {
            if (cp.y < hh)
                cp.y = hh;
            if (cp.y > m_worldSize.y - hh)
                cp.y = m_worldSize.y - hh;
        }
        m_camera.setCenter(cp);
        return;
    }

    // Despertar joven
    if (m_gallinaObtenida && m_joven.estaDormido() && m_cercaJoven && !m_tictactoeMinigame.isActive() && !m_roosterHuntMinigame.isActive() && !m_bloquearR)
    {
        static bool rJovenPresionado = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
        {
            if (!rJovenPresionado)
            {
                rJovenPresionado = true;
                m_joven.despertar();
                // Quitar gallina del inventario
                Inventory *inv = m_player.getInventory();
                if (inv)
                {
                    for (int i = 0; i < 15; i++)
                    {
                        Item *item = inv->getItem(i);
                        if (item && item->name == "Gallina")
                        {
                            inv->removeItem(i);
                            break;
                        }
                    }
                }
                m_bloquearR = true;
                m_tiempoUltimaR.restart();
                mostrarMensaje("Quieres jugar la vieja?", 2.f, sf::Color::Yellow);
            }
        }
        else
            rJovenPresionado = false;
    }

    // Jugar Tic Tac Toe
    if (!m_joven.estaDormido() && m_cercaJoven && !m_tictactoeMinigame.isActive() && !m_roosterHuntMinigame.isActive() && !m_dientesObtenidos && !m_bloquearR)
    {
        static bool rTictactoePresionado = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
        {
            if (!rTictactoePresionado)
            {
                rTictactoePresionado = true;
                m_tictactoeMinigame.activate();
            }
        }
        else
            rTictactoePresionado = false;
    }

    if (m_tictactoeMinigame.isActive())
    {
        m_tictactoeMinigame.update(dt);
        m_player.update(dt);

        if (m_tictactoeMinigame.isGameWon() && !m_dientesObtenidos)
        {
            m_dientesObtenidos = true;

            // AGREGAR AL INVENTARIO
            Inventory *inv = m_player.getInventory();
            if (inv)
            {
                Item dientes("Dientes", sf::Color::White, "assets/images/items/dientes.png"); // Color blanco
                inv->addItem(dientes);
            }

            mostrarMensaje("Dientes conseguidos!", 2.f, sf::Color::Green);
        }
        sf::Vector2f pp = m_player.getPosition();
        sf::Vector2f cp = pp;
        float hw = 640.f, hh = 360.f;
        if (hw * 2 >= m_worldSize.x)
            cp.x = m_worldSize.x / 2;
        else
        {
            if (cp.x < hw)
                cp.x = hw;
            if (cp.x > m_worldSize.x - hw)
                cp.x = m_worldSize.x - hw;
        }
        if (hh * 2 >= m_worldSize.y)
            cp.y = m_worldSize.y / 2;
        else
        {
            if (cp.y < hh)
                cp.y = hh;
            if (cp.y > m_worldSize.y - hh)
                cp.y = m_worldSize.y - hh;
        }
        m_camera.setCenter(cp);
        return;
    }

    // Entregar dientes a abuelita
    if (m_dientesObtenidos && m_abuelita.estaNormal() && m_cercaAbuelita && !m_llaveObtenida)
    {
        static bool rAbuelitaPresionado = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
        {
            if (!rAbuelitaPresionado)
            {
                rAbuelitaPresionado = true;
                m_abuelita.sonreir();
                // Quitar dientes del inventario
                Inventory *inv = m_player.getInventory();
                if (inv)
                {
                    for (int i = 0; i < 15; i++)
                    {
                        Item *item = inv->getItem(i);
                        if (item && item->name == "Dientes")
                        {
                            inv->removeItem(i);
                            break;
                        }
                    }
                }

                m_llaveObtenida = true;

                // AGREGAR AL INVENTARIO
                inv = m_player.getInventory();
                if (inv)
                {
                    Item llave("Llave", sf::Color(255, 215, 0), "assets/images/items/llave.png"); // Color dorado
                    inv->addItem(llave);
                }

                mostrarMensaje("Llave obtenida!", 3.f, sf::Color::Green);
            }
        }
        else
            rAbuelitaPresionado = false;
    }

    // Controlar sonrisa de la abuelita
    if (m_llaveObtenida && m_cercaAbuelita)
    {
        m_abuelita.sonreir();
    }
    else if (m_llaveObtenida && !m_cercaAbuelita)
    {
        m_abuelita.ponerNormal();
    }

    // ===== RECOGER CAMBUR =====
    if (!m_camburRecogido)
    {
        m_cercaCambur = m_player.getHurtbox().findIntersection(m_camburArea).has_value();
        static bool rCamburPresionado = false;
        if (m_cercaCambur && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
        {
            if (!rCamburPresionado)
            {
                rCamburPresionado = true;
                m_camburRecogido = true;

                Inventory *inv = m_player.getInventory();
                if (inv)
                {
                    Item cambur("Cambur", sf::Color(255, 255, 0), "assets/images/niveles/nivel6/cambur.png");
                    inv->addItem(cambur);
                }
                mostrarMensaje("Cambur recogido!", 2.f, sf::Color::Yellow);
            }
        }
        else
        {
            rCamburPresionado = false;
        }
    }

    // ===== RECOGER DESTORNILLADOR =====
    if (!m_destornilladorRecogido)
    {
        m_cercaDestornillador = m_player.getHurtbox().findIntersection(m_destornilladorArea).has_value();
        static bool rDestornilladorPresionado = false;
        if (m_cercaDestornillador && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
        {
            if (!rDestornilladorPresionado)
            {
                rDestornilladorPresionado = true;
                m_destornilladorRecogido = true;

                Inventory *inv = m_player.getInventory();
                if (inv)
                {
                    Item destornillador("Destornillador", sf::Color(192, 192, 192), "assets/images/items/destornillador.png");
                    inv->addItem(destornillador);
                }
                mostrarMensaje("Destornillador recogido!", 2.f, sf::Color::Green);
            }
        }
        else
        {
            rDestornilladorPresionado = false;
        }
    }

    // ===== ENTRADA AL CENTINELA =====
    m_cercaEntradaCentinela = m_player.getHurtbox().findIntersection(m_entradaCentinelaArea).has_value();

    if (m_destornilladorRecogido && m_cercaEntradaCentinela)
    {
        static bool fCentinelaPresionado = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
        {
            if (!fCentinelaPresionado)
            {
                fCentinelaPresionado = true;
                mostrarMensaje("Entrando al centinela...", 2.f, sf::Color::Yellow);
                game->detenerMusica();
                game->entrarCentinela();
                return;
            }
        }
        else
        {
            fCentinelaPresionado = false;
        }
    }

    // Movimiento
    Inventory *inv = m_player.getInventory();
    if (!inv || !inv->isOpen())
    {
        sf::Vector2f m(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            m.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            m.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            m.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            m.x += 1.f;
        if (m.x != 0.f || m.y != 0.f)
        {
            float l = std::sqrt(m.x * m.x + m.y * m.y);
            m /= l;
        }
        m_player.move(m, dt);
    }
    m_player.update(dt);

    // Colisiones
    for (const auto &obj : m_mapaFisico)
    {
        if (m_player.getHurtbox().findIntersection(obj.getBounds()).has_value())
        {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }
    // Colisión con mesas
    for (const auto &mesaBound : m_mesasBounds)
    {
        if (m_player.getHurtbox().findIntersection(mesaBound).has_value())
        {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }
    sf::FloatRect gc(sf::Vector2f(m_galloArea.position.x + 5.f, m_galloArea.position.y + 5.f), sf::Vector2f(m_galloArea.size.x - 10.f, m_galloArea.size.y - 10.f));
    if (m_player.getHurtbox().findIntersection(gc).has_value())
        m_player.setPosition(posAnterior.x, posAnterior.y);
    sf::FloatRect jc(sf::Vector2f(m_jovenArea.position.x + 5.f, m_jovenArea.position.y + 5.f), sf::Vector2f(m_jovenArea.size.x - 10.f, m_jovenArea.size.y - 10.f));
    if (m_player.getHurtbox().findIntersection(jc).has_value())
        m_player.setPosition(posAnterior.x, posAnterior.y);
    sf::FloatRect ac(sf::Vector2f(m_abuelitaArea.position.x + 5.f, m_abuelitaArea.position.y + 5.f), sf::Vector2f(m_abuelitaArea.size.x - 10.f, m_abuelitaArea.size.y - 10.f));
    if (m_player.getHurtbox().findIntersection(ac).has_value())
        m_player.setPosition(posAnterior.x, posAnterior.y);

    // Cámara
    sf::Vector2f pp = m_player.getPosition();
    sf::Vector2f cp = pp;
    float hw = 640.f, hh = 360.f;
    if (hw * 2 >= m_worldSize.x)
        cp.x = m_worldSize.x / 2;
    else
    {
        if (cp.x < hw)
            cp.x = hw;
        if (cp.x > m_worldSize.x - hw)
            cp.x = m_worldSize.x - hw;
    }
    if (hh * 2 >= m_worldSize.y)
        cp.y = m_worldSize.y / 2;
    else
    {
        if (cp.y < hh)
            cp.y = hh;
        if (cp.y > m_worldSize.y - hh)
            cp.y = m_worldSize.y - hh;
    }
    m_camera.setCenter(cp);

    verificarSalidaNivel();

    if (!m_mostrarTutorial && !m_mostrarTutorialPorTecla)
    {
        static bool esc = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
        {
            if (!esc)
            {
                esc = true;
                game->pushState(std::make_unique<PauseState>(window, game));
            }
        }
        else
            esc = false;
    }
}

void Nivel6State::draw()
{
    if (!window)
        return;

    // ===== FASE 1: MUNDO CON CÁMARA =====
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

    // Entidades (detrás del jugador)
    m_gallo.draw(*window);
    m_joven.draw(*window);
    m_abuelita.draw(*window);

    // DEBUG: Dibujar colisiones (F3)
    // DEBUG: Dibujar colisiones y hitboxes (F3)
    if (m_debugMode)
    {
        // Colisiones del mapa (rojo)
        for (const auto &obj : m_mapaFisico)
        {
            sf::RectangleShape colision;
            colision.setPosition(sf::Vector2f(obj.getBounds().position.x, obj.getBounds().position.y));
            colision.setSize(sf::Vector2f(obj.getBounds().size.x, obj.getBounds().size.y));
            colision.setFillColor(sf::Color(255, 0, 0, 80));
            colision.setOutlineThickness(2.f);
            colision.setOutlineColor(sf::Color::Red);
            window->draw(colision);
        }

        // Área del ascensor
        sf::RectangleShape ascensorDebug;
        ascensorDebug.setPosition(sf::Vector2f(m_puertaSalidaArea.position.x, m_puertaSalidaArea.position.y));
        ascensorDebug.setSize(sf::Vector2f(m_puertaSalidaArea.size.x, m_puertaSalidaArea.size.y));
        ascensorDebug.setFillColor(sf::Color(0, 255, 0, 80));
        ascensorDebug.setOutlineThickness(2.f);
        ascensorDebug.setOutlineColor(sf::Color::Green);
        window->draw(ascensorDebug);

        // Hitbox gallo (amarillo)
        sf::FloatRect galloBounds = m_gallo.getBounds();
        sf::RectangleShape galloDebug;
        galloDebug.setPosition(sf::Vector2f(galloBounds.position.x, galloBounds.position.y));
        galloDebug.setSize(sf::Vector2f(galloBounds.size.x, galloBounds.size.y));
        galloDebug.setFillColor(sf::Color(255, 255, 0, 80));
        galloDebug.setOutlineThickness(2.f);
        galloDebug.setOutlineColor(sf::Color::Yellow);
        window->draw(galloDebug);

        // Hitbox joven (azul)
        sf::FloatRect jovenBounds = m_joven.getBounds();
        sf::RectangleShape jovenDebug;
        jovenDebug.setPosition(sf::Vector2f(jovenBounds.position.x, jovenBounds.position.y));
        jovenDebug.setSize(sf::Vector2f(jovenBounds.size.x, jovenBounds.size.y));
        jovenDebug.setFillColor(sf::Color(0, 0, 255, 80));
        jovenDebug.setOutlineThickness(2.f);
        jovenDebug.setOutlineColor(sf::Color::Blue);
        window->draw(jovenDebug);

        // Hitbox abuelita (magenta)
        sf::FloatRect abuelitaBounds = m_abuelita.getBounds();
        sf::RectangleShape abuelitaDebug;
        abuelitaDebug.setPosition(sf::Vector2f(abuelitaBounds.position.x, abuelitaBounds.position.y));
        abuelitaDebug.setSize(sf::Vector2f(abuelitaBounds.size.x, abuelitaBounds.size.y));
        abuelitaDebug.setFillColor(sf::Color(255, 0, 255, 80));
        abuelitaDebug.setOutlineThickness(2.f);
        abuelitaDebug.setOutlineColor(sf::Color::Magenta);
        window->draw(abuelitaDebug);

        // Hitbox mesas (amarillo)
        for (const auto &mesaBound : m_mesasBounds)
        {
            sf::RectangleShape mesaDebug;
            mesaDebug.setPosition(sf::Vector2f(mesaBound.position.x, mesaBound.position.y));
            mesaDebug.setSize(sf::Vector2f(mesaBound.size.x, mesaBound.size.y));
            mesaDebug.setFillColor(sf::Color(255, 255, 0, 80));
            mesaDebug.setOutlineThickness(2.f);
            mesaDebug.setOutlineColor(sf::Color::Yellow);
            window->draw(mesaDebug);
        }

        // Área cambur (amarillo)
        if (!m_camburRecogido)
        {
            sf::RectangleShape camburDebug;
            camburDebug.setPosition(sf::Vector2f(m_camburArea.position.x, m_camburArea.position.y));
            camburDebug.setSize(sf::Vector2f(m_camburArea.size.x, m_camburArea.size.y));
            camburDebug.setFillColor(sf::Color(255, 255, 0, 80));
            camburDebug.setOutlineThickness(2.f);
            camburDebug.setOutlineColor(sf::Color::Yellow);
            window->draw(camburDebug);
        }

        // Área destornillador (cyan)
        if (!m_destornilladorRecogido)
        {
            sf::RectangleShape destornilladorDebug;
            destornilladorDebug.setPosition(sf::Vector2f(m_destornilladorArea.position.x, m_destornilladorArea.position.y));
            destornilladorDebug.setSize(sf::Vector2f(m_destornilladorArea.size.x, m_destornilladorArea.size.y));
            destornilladorDebug.setFillColor(sf::Color(0, 255, 255, 80));
            destornilladorDebug.setOutlineThickness(2.f);
            destornilladorDebug.setOutlineColor(sf::Color::Cyan);
            window->draw(destornilladorDebug);
        }

        // Área entrada centinela (magenta)
        sf::RectangleShape centinelaDebug;
        centinelaDebug.setPosition(sf::Vector2f(m_entradaCentinelaArea.position.x, m_entradaCentinelaArea.position.y));
        centinelaDebug.setSize(sf::Vector2f(m_entradaCentinelaArea.size.x, m_entradaCentinelaArea.size.y));
        centinelaDebug.setFillColor(sf::Color(255, 0, 255, 80));
        centinelaDebug.setOutlineThickness(2.f);
        centinelaDebug.setOutlineColor(sf::Color::Magenta);
        window->draw(centinelaDebug);
    }

    // Dibujar mesas
    for (auto &mesa : m_mesas)
    {
        window->draw(*mesa);
    }

    // Cambur
    if (!m_camburRecogido && m_camburSprite)
    {
        window->draw(*m_camburSprite);
    }

    // Destornillador
    if (!m_destornilladorRecogido && m_destornilladorSprite)
    {
        window->draw(*m_destornilladorSprite);
    }

    // Rejilla del centinela
    if (m_rejillaSprite)
    {
        window->draw(*m_rejillaSprite);
    }

    // Jugador (adelante)
    m_player.draw(*window);

    // Rifle en el mapa (sprite)
    if (!m_rifleRecogido && m_rifleMapSprite)
    {
        window->draw(*m_rifleMapSprite);
    }

    // ===== FASE 2: UI =====
    window->setView(window->getDefaultView());

    // Texto rifle
    if (!m_rifleRecogido && m_cercaRifle && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona F para recoger el rifle");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 85.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto gallo (cazar)
    if (m_rifleRecogido && m_cercaGallo && !m_roosterHuntMinigame.isActive() && !m_tictactoeMinigame.isActive() && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona F para cazar gallos");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 85.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto despertar joven
    if (m_gallinaObtenida && m_joven.estaDormido() && m_cercaJoven &&
        !m_tictactoeMinigame.isActive() && !m_roosterHuntMinigame.isActive() &&
        m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona F para despertar al joven");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 85.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto jugar vieja
    if (!m_joven.estaDormido() && m_cercaJoven && !m_tictactoeMinigame.isActive() &&
        !m_roosterHuntMinigame.isActive() && !m_dientesObtenidos &&
        m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Joven: Presiona F y jugamos la vieja");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 85.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto ya tiene dientes
    if (m_dientesObtenidos && m_cercaJoven && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Joven: Ya tienes los dientes, que mas quieres de mi?");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 85.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto abuelita sin dientes
    if (!m_dientesObtenidos && m_abuelita.estaNormal() && m_cercaAbuelita &&
        !m_tictactoeMinigame.isActive() && !m_roosterHuntMinigame.isActive() &&
        m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("La abuelita necesita sus dientes...");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 85.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto entregar dientes
    if (m_dientesObtenidos && m_abuelita.estaNormal() && m_cercaAbuelita && !m_llaveObtenida &&
        m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona F para entregar los dientes");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 85.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto llave obtenida
    if (m_llaveObtenida && m_cercaAbuelita && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Abuelita: Gracias, con esa llave puedes salir de aqui");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 85.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto entrada centinela
    if (m_destornilladorRecogido && m_cercaEntradaCentinela && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona F para abrir el ducto");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 85.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto salida
    if (m_cercaPuertaSalida && m_textoInteraccion && m_fontLoaded)
    {
        if (m_llaveObtenida)
        {
            m_textoInteraccion->setString("Presiona F para salir del nivel");
        }
        else
        {
            m_textoInteraccion->setString("Necesitas la llave para salir");
        }
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 85.f));
        window->draw(*m_textoInteraccion);
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
                "NIVEL 6 - LA BUSQUEDA DE LOS DIENTES\n\n"
                "La abuelita necesita sus dientes.\n\n"
                "Encuentra el rifle, caza galloss y despierta\n"
                "al joven para conseguir los dientes.\n\n"
                "[ESC] Cerrar | [M] Ayuda");
            tutorialText.setCharacterSize(20);
            tutorialText.setFillColor(sf::Color::White);
            sf::FloatRect textBounds = tutorialText.getLocalBounds();
            tutorialText.setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            tutorialText.setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y / 2.f));
            window->draw(tutorialText);
        }
    }

    // Minijuegos
    if (m_roosterHuntMinigame.isActive())
    {
        m_roosterHuntMinigame.draw(*window);
    }

    if (m_tictactoeMinigame.isActive())
    {
        m_tictactoeMinigame.draw(*window);
    }

    // Inventario
    Inventory *inv = m_player.getInventory();
    if (inv)
        inv->draw(*window);
}

void Nivel6State::configurarColisiones()
{
    m_mapaFisico.clear();

    // Pared Superior (0,0) a (1812,215)
    m_mapaFisico.emplace_back(0.f, 0.f, 1812.f, 215.f);

    // Borde izquierdo (0,0) a (39,1016)
    m_mapaFisico.emplace_back(0.f, 0.f, 39.f, 1016.f);

    // Borde inferior (0,1003) a (1812,13)
    m_mapaFisico.emplace_back(0.f, 1003.f, 1812.f, 13.f);

    // Borde derecho (se mantiene del anterior)
    m_mapaFisico.emplace_back(m_worldSize.x, 0.f, 10.f, m_worldSize.y);

    // PC (492,215) a (156,43)
    m_mapaFisico.emplace_back(492.f, 215.f, 156.f, 43.f);

    // Pared 1 (648,215) a (236,132)
    m_mapaFisico.emplace_back(648.f, 215.f, 37.f, 138.f);

    // Pared 2 (1236,215) a (45,88)
    m_mapaFisico.emplace_back(1236.f, 215.f, 45.f, 88.f);

    // Letrero (1169,303) a (215,43)
    m_mapaFisico.emplace_back(1169.f, 303.f, 215.f, 43.f);

    // Pared 3 (573,346) a (157,99)
    m_mapaFisico.emplace_back(573.f, 346.f, 157.f, 99.f);

    // Puerta (732,352) a (34,144)
    m_mapaFisico.emplace_back(732.f, 352.f, 34.f, 144.f);

    // Pared 4 (862,346) a (650,99)
    m_mapaFisico.emplace_back(862.f, 346.f, 650.f, 99.f);

    // Pared 5 (1601,346) a (211,99)
    m_mapaFisico.emplace_back(1601.f, 346.f, 211.f, 99.f);

    // Pared 6 (573,445) a (28,124)
    m_mapaFisico.emplace_back(573.f, 445.f, 28.f, 124.f);

    // Pared 7 (573,569) a (596,154)
    m_mapaFisico.emplace_back(573.f, 569.f, 596.f, 154.f);

    // Pared 8 (1150,692) a (19,87)
    m_mapaFisico.emplace_back(1150.f, 692.f, 19.f, 87.f);

    // Pared 9 (1344,569) a (468,101)
    m_mapaFisico.emplace_back(1344.f, 569.f, 468.f, 101.f);

    // Puerta Gallinero (1344,670) a (67,48)
    m_mapaFisico.emplace_back(1344.f, 670.f, 67.f, 48.f);

    // Tubos (1656,670) a (156,93)
    m_mapaFisico.emplace_back(1656.f, 670.f, 156.f, 93.f);

    // Bebederos (1440,959) a (222,34)
    m_mapaFisico.emplace_back(1440.f, 959.f, 222.f, 34.f);

    // Pared Gallinero (1345,992) a (467,24)
    m_mapaFisico.emplace_back(1345.f, 992.f, 467.f, 24.f);

    // Gris (1325,929) a (21,58)
    m_mapaFisico.emplace_back(1325.f, 929.f, 21.f, 58.f);

    // Pared final (1150,839) a (19,177)
    m_mapaFisico.emplace_back(1150.f, 839.f, 19.f, 177.f);

    std::cout << "✅ Colisiones del Nivel 6 configuradas: " << m_mapaFisico.size() << " paredes" << std::endl;
}

void Nivel6State::jugadorHaMuerto()
{
    game->pushState(std::make_unique<PauseState>(window, game));
}

void Nivel6State::mostrarMensaje(const std::string &texto, float duracion, sf::Color color)
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
    std::cout << "" << texto << std::endl;
}