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
    m_player.setPosition(800.f, 600.f);
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

    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(1550.f, 1350.f), sf::Vector2f(120.f, 180.f));
    configurarColisiones();

    // Rifle
    m_rifleRecogido = false;
    m_rifleArea = sf::FloatRect(sf::Vector2f(522.f, 500.f), sf::Vector2f(40.f, 40.f));
    m_cercaRifle = false;

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

    static bool ePresionado = false;
    if (m_cercaPuertaSalida)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
        {
            if (!ePresionado)
            {
                ePresionado = true;
                std::cout << "🚪 Saliendo del Nivel 6..." << std::endl;
                game->avanzarNivel();
            }
        }
        else
        {
            ePresionado = false;
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
        if (m_cercaRifle && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
        {
            if (!rRiflePresionado)
            {
                rRiflePresionado = true;
                m_rifleRecogido = true;

                // AGREGAR AL INVENTARIO
                Inventory *inv = m_player.getInventory();
                if (inv)
                {
                    Item rifle("Rifle", sf::Color(139, 90, 43)); // Color marrón
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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
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
                Item gallina("Gallina", sf::Color(255, 220, 180)); // Color crema
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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
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
                Item dientes("Dientes", sf::Color::White); // Color blanco
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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
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
                    Item llave("Llave", sf::Color(255, 215, 0)); // Color dorado
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

    // Jugador (adelante)
    m_player.draw(*window);

    // DEBUG: Rifle
    if (!m_rifleRecogido)
    {
        sf::RectangleShape rifleDebug(sf::Vector2f(m_rifleArea.size.x, m_rifleArea.size.y));
        rifleDebug.setPosition(sf::Vector2f(m_rifleArea.position.x, m_rifleArea.position.y));
        rifleDebug.setFillColor(sf::Color(255, 0, 255, 150));
        rifleDebug.setOutlineThickness(2.f);
        rifleDebug.setOutlineColor(sf::Color::Magenta);
        window->draw(rifleDebug);
    }

    // ===== FASE 2: UI =====
    window->setView(window->getDefaultView());

    // Texto rifle
    if (!m_rifleRecogido && m_cercaRifle && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona R para recoger el rifle");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto gallo (cazar)
    if (m_rifleRecogido && m_cercaGallo && !m_roosterHuntMinigame.isActive() && !m_tictactoeMinigame.isActive() && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona R para cazar gallos");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto despertar joven
    if (m_gallinaObtenida && m_joven.estaDormido() && m_cercaJoven &&
        !m_tictactoeMinigame.isActive() && !m_roosterHuntMinigame.isActive() &&
        m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona R para despertar al joven");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto jugar vieja
    if (!m_joven.estaDormido() && m_cercaJoven && !m_tictactoeMinigame.isActive() &&
        !m_roosterHuntMinigame.isActive() && !m_dientesObtenidos &&
        m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona R y jugamos la vieja");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto ya tiene dientes
    if (m_dientesObtenidos && m_cercaJoven && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Ya tienes los dientes, que mas quieres de mi?");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
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
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto entregar dientes
    if (m_dientesObtenidos && m_abuelita.estaNormal() && m_cercaAbuelita && !m_llaveObtenida &&
        m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona R para entregar los dientes");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto llave obtenida
    if (m_llaveObtenida && m_cercaAbuelita && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("¡Gracias! Ya puedes salir del nivel");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto salida
    if (m_cercaPuertaSalida && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona E para avanzar al siguiente nivel");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
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
                "Encuentra el rifle, caza gallinas y despierta\n"
                "al guardian para conseguir los dientes.\n\n"
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

    m_mapaFisico.emplace_back(-10.f, 0.f, 10.f, m_worldSize.y);
    m_mapaFisico.emplace_back(m_worldSize.x, 0.f, 10.f, m_worldSize.y);
    m_mapaFisico.emplace_back(0.f, -10.f, m_worldSize.x, 10.f);
    m_mapaFisico.emplace_back(0.f, m_worldSize.y, m_worldSize.x, 10.f);

    std::cout << "Colisiones del Nivel 6 configuradas" << std::endl;
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