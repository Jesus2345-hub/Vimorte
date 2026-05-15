#include "Nivel6State.hpp"
#include "PauseState.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

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
        std::cerr << "Error: No se pudo cargar background del nivel 6" << std::endl;
        m_worldSize = sf::Vector2f(1754.f, 1587.f);
    }

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
    m_rifleArea = sf::FloatRect(sf::Vector2f(522.f, 500.f), sf::Vector2f(40.f, 40.f));
    m_cercaRifle = false;

    // Cargar sprite del rifle en el mapa
    if (m_rifleMapTexture.loadFromFile("assets/images/items/rifle.png"))
    {
        m_rifleMapSprite = std::make_unique<sf::Sprite>(m_rifleMapTexture);
        m_rifleMapSprite->setScale(sf::Vector2f(0.05f, 0.05f)); // Ajustar tamaño
        sf::FloatRect bounds = m_rifleMapSprite->getLocalBounds();
        m_rifleMapSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_rifleMapSprite->setPosition(sf::Vector2f(
            m_rifleArea.position.x + m_rifleArea.size.x / 2.f,
            m_rifleArea.position.y + m_rifleArea.size.y / 2.f));
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
    game->cambiarMusica("assets/sounds/nivel6.ogg");
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
            if (keyEvent->code == sf::Keyboard::Key::Escape)
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
            if (keyEvent->code == sf::Keyboard::Key::Escape)
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
                    std::cout << "Saliendo del Nivel 6 con la llave..." << std::endl;
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
                m_llaveObtenida = true;

                // AGREGAR AL INVENTARIO
                Inventory *inv = m_player.getInventory();
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

    // ===== BORDES DEL MUNDO =====
    m_mapaFisico.emplace_back(0.f, 0.f, 31.f, m_worldSize.y);           // Izquierda
    m_mapaFisico.emplace_back(m_worldSize.x, 0.f, 10.f, m_worldSize.y); // Derecha
    m_mapaFisico.emplace_back(0.f, -10.f, m_worldSize.x, 10.f);         // Arriba
    m_mapaFisico.emplace_back(0.f, m_worldSize.y, m_worldSize.x, 10.f); // Abajo

    // ===== PARED SUPERIOR (0,0) a (1812,211) =====
    m_mapaFisico.emplace_back(0.f, 0.f, 1812.f, 211.f);

    // ===== PARED IZQUIERDA SALA ASCENSOR (653,0) a (691,347) =====
    m_mapaFisico.emplace_back(653.f, 0.f, 38.f, 347.f);

    // ===== PARED DERECHA SALA ASCENSOR (1237,0) a (1281,347) =====
    m_mapaFisico.emplace_back(1237.f, 0.f, 44.f, 347.f);

    // ===== PARED SALA ASCENSOR CON PASILLO (867,351) a (1511,443) =====
    m_mapaFisico.emplace_back(867.f, 351.f, 644.f, 92.f);

    // ===== PARED SALA ABUELA (1603,351) a (1812,441) =====
    m_mapaFisico.emplace_back(1603.f, 351.f, 209.f, 90.f);

    // ===== PARED ASCENSOR-PASILLO (573,349) a (737,439) =====
    m_mapaFisico.emplace_back(573.f, 349.f, 164.f, 90.f);

    // ===== PARED IZQUIERDA PASILLO (573,349) a (605,725) =====
    m_mapaFisico.emplace_back(573.f, 349.f, 32.f, 376.f);

    // ===== PARED PASILLO (577,575) a (1167,717) =====
    m_mapaFisico.emplace_back(577.f, 575.f, 590.f, 142.f);

    // ===== PASILLO ABAJO VERTICAL IZQUIERDA (1151,571) a (1171,773) =====
    m_mapaFisico.emplace_back(1151.f, 571.f, 20.f, 202.f);

    // ===== PASILLO ABAJO VERTICAL IZQ PARTE ABAJO (1151,835) a (1171,1016) =====
    m_mapaFisico.emplace_back(1151.f, 835.f, 20.f, 181.f);

    // ===== PARED GRANJA (1345,571) a (1811,673) =====
    m_mapaFisico.emplace_back(1345.f, 571.f, 466.f, 102.f);

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