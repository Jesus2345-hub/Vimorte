#include "Nivel7State.hpp"
#include "PauseState.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
//
Nivel7State::Nivel7State(sf::RenderWindow *window, Game *game)
    : State(window, game),
      m_background(nullptr),
      m_cercaPuertaSalida(false),
      m_mostrarTutorial(false),
      m_mostrarTutorialPorTecla(false),
      m_fontLoaded(false),
      m_balonEnArco(false),
      m_goles(0)
{
    m_msjActual.texto = "";
    m_msjActual.tiempoRestante = 0.0f;
    m_msjActual.color = sf::Color::Yellow;

    // Cargar jugador
    m_player.loadAssets();
    m_player.setPosition(800.f, 270.f);
    m_player.setSpeed(300.0f);

    // Tutorial

    if (game->tienePartidaActiva())
    {
        const auto &items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
        auto it = std::find(items.begin(), items.end(), "TutorialNivel7Visto");
        if (it == items.end())
        {
            m_mostrarTutorial = true;
            game->getSaveManager().addItemRecolectado("TutorialNivel7Visto");
        }
    }

    // Cargar fondo
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel7/background.png"))
    {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x),
                                   static_cast<float>(textureSize.y));
        std::cout << "✅ Nivel 7 cargado. Tamaño: " << m_worldSize.x << "x" << m_worldSize.y << std::endl;
    }
    else
    {
        std::cerr << "❌ Error: No se pudo cargar background del nivel 7" << std::endl;
        m_worldSize = sf::Vector2f(1672.f, 941.f);
    }

    // Cámara fija (el mapa es más pequeño, ajustamos)
    m_camera = sf::View(sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
                        sf::Vector2f(1280.f, 720.f));
    m_lastWindowSize = window->getSize();

    // Área de salida (ascensor)
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(604.f, 0.f), sf::Vector2f(36.f, 329.f));

    // Colisiones del mapa
    configurarColisiones();

    // ===== BALONES =====
    m_balones.clear();
    m_balonEnArco.clear();

    // Balón 1 (907, 848)
    auto b1 = std::make_unique<Balon>();
    b1->setPosition(907.f, 848.f);
    m_balones.push_back(std::move(b1));
    m_balonEnArco.push_back(false);

    // Balón 2 (1443, 551)
    auto b2 = std::make_unique<Balon>();
    b2->setPosition(1443.f, 551.f);
    m_balones.push_back(std::move(b2));
    m_balonEnArco.push_back(false);

    // Balón 3 (1547, 849)
    auto b3 = std::make_unique<Balon>();
    b3->setPosition(1547.f, 849.f);
    m_balones.push_back(std::move(b3));
    m_balonEnArco.push_back(false);

    // Área del arco (200,180) a (421,272)
    m_arcoArea = sf::FloatRect(sf::Vector2f(200.f, 180.f), sf::Vector2f(221.f, 92.f));
    m_goles = 0;
    m_golesParaGanar = 3;

    // Balón de basket en el mapa
    if (m_balonBasketMapTexture.loadFromFile("assets/images/niveles/nivel7/balon_basket.png"))
    {
        m_balonBasketMapSprite = std::make_unique<sf::Sprite>(m_balonBasketMapTexture);
        m_balonBasketMapSprite->setScale(sf::Vector2f(0.20f, 0.20f));
        sf::FloatRect bounds = m_balonBasketMapSprite->getLocalBounds();
        m_balonBasketMapSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_balonBasketMapSprite->setPosition(sf::Vector2f(1003.f, 691.f)); // Centro del balón
    }

    // Destornillador (a la izquierda del balón de basket)
    m_tieneDestornillador = false;
    m_destornilladorArea = sf::FloatRect(sf::Vector2f(900.f, 680.f), sf::Vector2f(40.f, 40.f));
    m_cercaDestornillador = false;

    // Destornillador en el mapa
    if (m_destornilladorMapTexture.loadFromFile("assets/images/items/destornillador.png"))
    {
        m_destornilladorMapSprite = std::make_unique<sf::Sprite>(m_destornilladorMapTexture);
        m_destornilladorMapSprite->setScale(sf::Vector2f(0.015f, 0.015f));
        sf::FloatRect bounds = m_destornilladorMapSprite->getLocalBounds();
        m_destornilladorMapSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_destornilladorMapSprite->setPosition(sf::Vector2f(
            m_destornilladorArea.position.x + m_destornilladorArea.size.x / 2.f,
            m_destornilladorArea.position.y + m_destornilladorArea.size.y / 2.f));
    }

    // Entrada al centinela
    m_entradaCentinelaArea = sf::FloatRect(sf::Vector2f(1437.f, 717.f), sf::Vector2f(28.f, 34.f));
    m_cercaEntradaCentinela = false;
    // Cargar sprite de la rejilla
    if (m_rejillaTexture.loadFromFile("assets/images/niveles/nivel7/rejilla.png"))
    {
        m_rejillaSprite = std::make_unique<sf::Sprite>(m_rejillaTexture);
        m_rejillaSprite->setScale(sf::Vector2f(0.15f, 0.15f)); // Ajusta según necesites
        sf::FloatRect bounds = m_rejillaSprite->getLocalBounds();
        m_rejillaSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_rejillaSprite->setPosition(sf::Vector2f(
            m_entradaCentinelaArea.position.x + m_entradaCentinelaArea.size.x / 2.f,
            m_entradaCentinelaArea.position.y + m_entradaCentinelaArea.size.y / 2.f));
    }

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

    // Guardado automático
    if (game->tienePartidaActiva())
    {
        game->getSaveManager().setNivelActual(7, 7);
        game->guardarPartidaActual();
    }

    // Baloncesto
    m_tieneBalonBasket = false;
    m_balonBasketArea = sf::FloatRect(sf::Vector2f(1003.f, 691.f), sf::Vector2f(40.f, 40.f));
    m_cercaBalonBasket = false;
    m_lebron.setPosition(1523.f, 231.f); // Centro de LeBron
    m_lebron.setScale(0.2f, 0.2f);
    m_cercaLebron = false;
    m_llaveObtenida = false;

    sf::Vector2u winSize = window->getSize();
    float bw = winSize.x * 0.75f;
    float bh = winSize.y * 0.85f;
    m_baloncestoMinigame.setSize(sf::Vector2f(bw, bh));
    m_baloncestoMinigame.setPosition(sf::Vector2f((winSize.x - bw) / 2.f, (winSize.y - bh) / 2.f));

    game->setIsInLevel(true);
    std::cout << "Nivel7State inicializado correctamente" << std::endl;
}

void Nivel7State::handleEvent(const sf::Event &event)
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
            m_mostrarTutorialPorTecla = true;
        }
    }

    Inventory *inv = m_player.getInventory();
    if (inv)
        inv->handleEvent(event, *window);

    if (m_baloncestoMinigame.isActive())
    {
        m_baloncestoMinigame.handleEvent(event, *window);
        if (event.is<sf::Event::KeyPressed>())
        {
            if (event.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)
            {
                m_baloncestoMinigame.deactivate();
                return;
            }
        }
    }
}

void Nivel7State::verificarSalidaNivel()
{
    m_cercaPuertaSalida = m_player.getHurtbox().findIntersection(m_puertaSalidaArea).has_value();

    static bool rPresionado = false;
    if (m_cercaPuertaSalida)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
        {
            if (!rPresionado)
            {
                rPresionado = true;

                if (m_llaveObtenida)
                {
                    std::cout << "🚪 Saliendo del Nivel 7..." << std::endl;
                    game->avanzarNivel();
                }
                else
                {
                    mostrarMensaje("Necesitas la llave. Gánale a LeBron en los tiros libres.", 2.f, sf::Color::Red);
                }
            }
        }
        else
        {
            rPresionado = false;
        }
    }
}

void Nivel7State::update(float dt)
{
    m_lebron.update(dt);

    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f)
    {
        m_msjActual.tiempoRestante -= dt;
        if (m_msjActual.tiempoRestante <= 0.0f)
            m_textoMensaje->setString("");
    }

    sf::Vector2f posAnterior = m_player.getPosition();

    // ========== ACTUALIZAR BALONES ==========
    for (size_t i = 0; i < m_balones.size(); i++)
    {
        m_balones[i]->update(dt);

        // Colisión jugador-balón (circular)
        sf::Vector2f jugadorPos = m_player.getPosition();
        sf::Vector2f balonPos = m_balones[i]->getPosition();
        float distJugadorBalon = std::sqrt(
            (jugadorPos.x - balonPos.x) * (jugadorPos.x - balonPos.x) +
            (jugadorPos.y - balonPos.y) * (jugadorPos.y - balonPos.y));

        float radioBalon = m_balones[i]->getRadius();
        float radioJugador = 25.f;

        if (distJugadorBalon < radioBalon + radioJugador)
        {
            sf::Vector2f dirBalon = balonPos - jugadorPos;
            float dist = std::sqrt(dirBalon.x * dirBalon.x + dirBalon.y * dirBalon.y);

            if (dist > 0.f)
            {
                dirBalon /= dist;
                m_balones[i]->empujar(dirBalon, 250.f);
                jugadorPos -= dirBalon * 3.f;
                m_player.setPosition(jugadorPos.x, jugadorPos.y);
            }
        }

        // Colisión balón-paredes
        sf::FloatRect balonBounds = m_balones[i]->getBounds();
        sf::Vector2f bPos = m_balones[i]->getPosition();

        for (const auto &obj : m_mapaFisico)
        {
            if (balonBounds.findIntersection(obj.getBounds()).has_value())
            {
                sf::FloatRect pared = obj.getBounds();
                float centroBalonX = bPos.x;
                float centroBalonY = bPos.y;
                float centroParedX = pared.position.x + pared.size.x / 2.f;
                float centroParedY = pared.position.y + pared.size.y / 2.f;

                float diffX = std::abs(centroBalonX - centroParedX) / (pared.size.x / 2.f + balonBounds.size.x / 2.f);
                float diffY = std::abs(centroBalonY - centroParedY) / (pared.size.y / 2.f + balonBounds.size.y / 2.f);

                if (diffX > diffY)
                {
                    float dirX = (centroBalonX < centroParedX) ? -1.f : 1.f;
                    m_balones[i]->empujar(sf::Vector2f(dirX, 0.f), 200.f);
                }
                else
                {
                    float dirY = (centroBalonY < centroParedY) ? -1.f : 1.f;
                    m_balones[i]->empujar(sf::Vector2f(0.f, dirY), 200.f);
                }
                break;
            }
        }

        // Verificar GOL
        if (!m_balonEnArco[i] && balonBounds.findIntersection(m_arcoArea).has_value())
        {
            m_balonEnArco[i] = true;
            m_goles++;
            mostrarMensaje("¡GOL! " + std::to_string(m_goles) + "/" + std::to_string(m_golesParaGanar), 2.f, sf::Color::Green);

            if (m_goles >= m_golesParaGanar)
            {
                mostrarMensaje("¡HAS MARCADO 3 GOLES! Dirigete al ascensor", 4.f, sf::Color::Green);
            }
        }
    }

    // ========== BALONCETO: Recoger balón ==========
    if (!m_tieneBalonBasket)
    {
        m_cercaBalonBasket = m_player.getHurtbox().findIntersection(m_balonBasketArea).has_value();
        static bool rBalonPresionado = false;
        if (m_cercaBalonBasket && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
        {
            if (!rBalonPresionado)
            {
                rBalonPresionado = true;
                m_tieneBalonBasket = true;

                // Agregar al inventario
                Inventory *inv = m_player.getInventory();
                if (inv)
                {
                    Item balonBasket("Balon Basket", sf::Color(255, 140, 0), "assets/images/niveles/nivel7/balon_basket.png");
                    inv->addItem(balonBasket);
                }

                mostrarMensaje("Balón de baloncesto recogido!", 2.f, sf::Color::Green);
            }
        }
        else
            rBalonPresionado = false;
    }

    // ========== BALONCETO: Interactuar con LeBron ==========
    sf::FloatRect lebronBounds = m_lebron.getBounds();
    sf::FloatRect lebronInteraccion(
        sf::Vector2f(lebronBounds.position.x - 60.f, lebronBounds.position.y - 60.f),
        sf::Vector2f(lebronBounds.size.x + 120.f, lebronBounds.size.y + 120.f));
    m_cercaLebron = m_player.getHurtbox().findIntersection(lebronInteraccion).has_value();

    if (m_tieneBalonBasket && m_cercaLebron && !m_llaveObtenida && !m_baloncestoMinigame.isActive())
    {
        static bool rLebronPresionado = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
        {
            if (!rLebronPresionado)
            {
                rLebronPresionado = true;
                m_baloncestoMinigame.activate();
            }
        }
        else
            rLebronPresionado = false;
    }

    if (!m_tieneBalonBasket && m_cercaLebron && !m_llaveObtenida)
    {
        static bool mensajeLebronMostrado = false;
        if (!mensajeLebronMostrado)
        {
            mostrarMensaje("Quiero jugar baloncesto pero no tengo balon. Tienes uno?", 3.f, sf::Color::Cyan);
            mensajeLebronMostrado = true;
        }
    }

    // Baloncesto activo
    if (m_baloncestoMinigame.isActive())
    {
        m_baloncestoMinigame.update(dt);
        m_player.update(dt);

        if (m_baloncestoMinigame.isGameWon() && !m_llaveObtenida && !m_baloncestoMinigame.isGameLost())
        {
            m_llaveObtenida = true;

            // Agregar llave al inventario
            Inventory *inv = m_player.getInventory();
            if (inv)
            {
                Item llave("Llave", sf::Color(255, 215, 0), "assets/images/items/llave.png");
                inv->addItem(llave);
            }

            mostrarMensaje("Has conseguido la llave! Dirigete al ascensor", 3.f, sf::Color::Green);
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

    // ========== RECOGER DESTORNILLADOR ==========
    if (!m_tieneDestornillador)
    {
        m_cercaDestornillador = m_player.getHurtbox().findIntersection(m_destornilladorArea).has_value();
        static bool rDestornilladorPresionado = false;
        if (m_cercaDestornillador && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
        {
            if (!rDestornilladorPresionado)
            {
                rDestornilladorPresionado = true;
                m_tieneDestornillador = true;

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

    // ========== ENTRADA AL CENTINELA ==========
    m_cercaEntradaCentinela = m_player.getHurtbox().findIntersection(m_entradaCentinelaArea).has_value();

    if (m_tieneDestornillador && m_cercaEntradaCentinela)
    {
        static bool rCentinelaPresionado = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
        {
            if (!rCentinelaPresionado)
            {
                rCentinelaPresionado = true;
                mostrarMensaje("Entrando al centinela...", 2.f, sf::Color::Yellow);
                game->entrarCentinela();
                return;
            }
        }
        else
        {
            rCentinelaPresionado = false;
        }
    }

    // ========== MOVIMIENTO ==========
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

    // ========== COLISIONES JUGADOR ==========
    for (const auto &obj : m_mapaFisico)
    {
        if (m_player.getHurtbox().findIntersection(obj.getBounds()).has_value())
        {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }

    // Colisión con LeBron
    if (m_player.getHurtbox().findIntersection(m_lebron.getBounds()).has_value())
    {
        m_player.setPosition(posAnterior.x, posAnterior.y);
    }

    // ========== CÁMARA ==========
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

    // ========== PAUSA ==========
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

void Nivel7State::draw()
{
    if (!window)
        return;

    // ===== FASE 1: MUNDO CON CÁMARA =====
    window->setView(m_camera);

    if (m_background)
        window->draw(*m_background);
    else
    {
        sf::RectangleShape fb(m_worldSize);
        fb.setFillColor(sf::Color(50, 30, 30));
        window->draw(fb);
    }

    // Dibujar balones (detrás del jugador)
    for (auto &balon : m_balones)
    {
        balon->draw(*window);
    }

    // Balón basket en el mapa
    if (!m_tieneBalonBasket && m_balonBasketMapSprite)
    {
        window->draw(*m_balonBasketMapSprite);
    }

    m_lebron.draw(*window);

    // Texto para jugar con LeBron (tiene balón, cerca, sin llave)
    if (m_tieneBalonBasket && m_cercaLebron && !m_llaveObtenida && !m_baloncestoMinigame.isActive() && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona R para jugar tiros libres con LeBron");
        sf::FloatRect b = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(b.size.x / 2.f, b.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    // Textos UI
    if (m_tieneBalonBasket && m_cercaLebron && !m_llaveObtenida && !m_baloncestoMinigame.isActive())
    {
        // "Presiona R para jugar"
    }
    if (!m_tieneBalonBasket && m_cercaLebron)
    {
        // "Quiero jugar baloncesto..."
    }

    // Minijuego
    if (m_baloncestoMinigame.isActive())
    {
        window->setView(window->getDefaultView());
        m_baloncestoMinigame.draw(*window);
    }

    // Texto de salida
    if (m_cercaPuertaSalida && m_textoInteraccion && m_fontLoaded)
    {
        if (m_llaveObtenida)
        {
            m_textoInteraccion->setString("Presiona R para salir del nivel");
        }
        else
        {
            m_textoInteraccion->setString("Necesitas conseguir la llave. Busca a LeBron.");
        }
        sf::FloatRect b = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(b.size.x / 2.f, b.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    // Destornillador en el mapa
    if (!m_tieneDestornillador && m_destornilladorMapSprite)
    {
        window->draw(*m_destornilladorMapSprite);
    }

    // Rejilla del centinela
    if (m_rejillaSprite)
    {
        window->draw(*m_rejillaSprite);
    }

    // Jugador (adelante)
    m_player.draw(*window);

    // // DEBUG: Dibujar colisiones
    // for (const auto &obj : m_mapaFisico)
    // {
    //     sf::RectangleShape colision;
    //     colision.setPosition(sf::Vector2f(obj.getBounds().position.x, obj.getBounds().position.y));
    //     colision.setSize(sf::Vector2f(obj.getBounds().size.x, obj.getBounds().size.y));
    //     colision.setFillColor(sf::Color(255, 0, 0, 80));
    //     colision.setOutlineThickness(1.f);
    //     colision.setOutlineColor(sf::Color::Red);
    //     window->draw(colision);
    // }

    // DEBUG: Dibujar área del arco
    // sf::RectangleShape arcoDebug(sf::Vector2f(m_arcoArea.size.x, m_arcoArea.size.y));
    // arcoDebug.setPosition(sf::Vector2f(m_arcoArea.position.x, m_arcoArea.position.y));
    // arcoDebug.setFillColor(sf::Color(255, 255, 0, 80));
    // arcoDebug.setOutlineThickness(2.f);
    // arcoDebug.setOutlineColor(sf::Color::Yellow);
    // window->draw(arcoDebug);

    // DEBUG: Dibujar colisiones circulares de los balones
    // for (size_t i = 0; i < m_balones.size(); i++)
    // {
    //     sf::CircleShape balonDebug(m_balones[i]->getRadius());
    //     balonDebug.setPosition(m_balones[i]->getPosition());
    //     balonDebug.setOrigin(sf::Vector2f(m_balones[i]->getRadius(), m_balones[i]->getRadius()));
    //     balonDebug.setFillColor(sf::Color(0, 255, 0, 80));
    //     balonDebug.setOutlineThickness(1.f);
    //     balonDebug.setOutlineColor(sf::Color::Green);
    //     window->draw(balonDebug);
    // }

    // DEBUG: Dibujar colisión circular del jugador
    // sf::CircleShape jugadorDebug(25.f);
    // sf::Vector2f posJugador = m_player.getPosition();
    // jugadorDebug.setPosition(sf::Vector2f(posJugador.x, posJugador.y + 30.f));
    // jugadorDebug.setOrigin(sf::Vector2f(25.f, 25.f));
    // jugadorDebug.setFillColor(sf::Color(0, 0, 255, 80));
    // jugadorDebug.setOutlineThickness(1.f);
    // jugadorDebug.setOutlineColor(sf::Color::Blue);
    // window->draw(jugadorDebug);

    // ===== FASE 2: UI =====
    window->setView(window->getDefaultView());

    // Texto de goles
    if (m_fontLoaded && m_textoInteraccion)
    {
        std::string textoGoles = "Goles: " + std::to_string(m_goles) + "/" + std::to_string(m_golesParaGanar);
        m_textoInteraccion->setString(textoGoles);
        sf::FloatRect b = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(b.size.x / 2.f, b.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, 30.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto de salida
    if (m_cercaPuertaSalida && m_textoInteraccion && m_fontLoaded)
    {
        if (m_goles >= m_golesParaGanar)
        {
            m_textoInteraccion->setString("Presiona R para salir del nivel");
        }
        else
        {
            m_textoInteraccion->setString("Mete " + std::to_string(m_golesParaGanar - m_goles) + " goles mas para salir");
        }
        sf::FloatRect b = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(b.size.x / 2.f, b.size.y / 2.f));
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
            sf::Text tt(m_font);
            tt.setString("NIVEL 7 - FUTBOL\n\nLleva los 3 balones al arco para poder salir.\n\n[ESC] Cerrar | [M] Ayuda");
            tt.setCharacterSize(20);
            tt.setFillColor(sf::Color::White);
            sf::FloatRect b = tt.getLocalBounds();
            tt.setOrigin(sf::Vector2f(b.size.x / 2.f, b.size.y / 2.f));
            tt.setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y / 2.f));
            window->draw(tt);
        }
    }

    // Texto destornillador
    if (!m_tieneDestornillador && m_cercaDestornillador && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona R para recoger el destornillador");
        sf::FloatRect b = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(b.size.x / 2.f, b.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto entrada centinela
    if (m_tieneDestornillador && m_cercaEntradaCentinela && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona R para abrir el ducto (Centinela)");
        sf::FloatRect b = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(b.size.x / 2.f, b.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    Inventory *inv = m_player.getInventory();
    if (inv)
        inv->draw(*window);
}

void Nivel7State::configurarColisiones()
{
    m_mapaFisico.clear();

    // Bordes del mundo
    m_mapaFisico.emplace_back(-10.f, 0.f, 10.f, m_worldSize.y);
    m_mapaFisico.emplace_back(m_worldSize.x, 0.f, 10.f, m_worldSize.y);
    m_mapaFisico.emplace_back(0.f, -10.f, m_worldSize.x, 10.f);
    m_mapaFisico.emplace_back(0.f, m_worldSize.y, m_worldSize.x, 10.f);

    // Borde izquierdo: (0,0) a (35,941)
    m_mapaFisico.emplace_back(0.f, 0.f, 35.f, 941.f);

    // Pared superior: (0,0) a (1186,207)
    m_mapaFisico.emplace_back(0.f, 0.f, 1186.f, 207.f);

    // Pared ascensor izquierda: (604,0) a (640,329)
    m_mapaFisico.emplace_back(604.f, 0.f, 36.f, 329.f);

    // Pared ascensor-pasillo (parte izquierda): (604,329) a (689,419)
    m_mapaFisico.emplace_back(604.f, 329.f, 85.f, 90.f);

    // Pared ascensor-pasillo (parte derecha): (801,329) a (1267,419)
    m_mapaFisico.emplace_back(801.f, 329.f, 466.f, 90.f);

    // Pared pasillo: (600,536) a (1088,673)
    m_mapaFisico.emplace_back(600.f, 536.f, 488.f, 137.f);

    // Pared pasillo vertical: (1065,552) a (1088,941)
    m_mapaFisico.emplace_back(1065.f, 552.f, 23.f, 389.f);

    // Pared Habitación salida-centinela: (1247,611) a (1672,742)
    m_mapaFisico.emplace_back(1247.f, 611.f, 425.f, 131.f);

    // Pared Basket-Pasillo: (1247,515) a (1267,609)
    m_mapaFisico.emplace_back(1247.f, 515.f, 20.f, 94.f);

    std::cout << "✅ Colisiones del Nivel 7 configuradas: " << m_mapaFisico.size() << " paredes" << std::endl;
}

void Nivel7State::mostrarMensaje(const std::string &texto, float duracion, sf::Color color)
{
    if (!m_textoMensaje)
        return;
    m_msjActual.texto = texto;
    m_msjActual.tiempoRestante = duracion;
    m_msjActual.color = color;
    m_textoMensaje->setString(texto);
    m_textoMensaje->setFillColor(color);
    sf::FloatRect b = m_textoMensaje->getLocalBounds();
    m_textoMensaje->setOrigin(sf::Vector2f(b.size.x / 2.f, b.size.y / 2.f));
}