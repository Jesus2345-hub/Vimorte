#include "Estados/Niveles/Nivel6/Nivel7State.hpp"
#include "Estados/PauseState.hpp"
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
    m_player.setPosition(800.f, 260.f);
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
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(798.f, 62.f), sf::Vector2f(182.f, 172.f));

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

    // ===== CARGAR MÚSICA DEL NIVEL 7 =====
    std::ifstream file("assets/sounds/nivel6.ogg");
    if (file.good())
    {
        file.close();
        game->cambiarMusica("assets/sounds/nivel6.ogg");
        std::cout << "🎵 Música del Nivel 6 cargada" << std::endl;
    }
    else
    {
        std::cout << "⚠️ Archivo de música no encontrado: assets/sounds/nivel6.ogg" << std::endl;
    }

    game->setIsInLevel(true);
    std::cout << "Nivel7State inicializado correctamente" << std::endl;
}

void Nivel7State::handleEvent(const sf::Event &event)
{
    if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::M)
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
        // ===== NUEVO: F3 para debug =====
        if (keyPressed->code == sf::Keyboard::Key::F3)
        {
            m_debugMode = !m_debugMode;
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
            if (event.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::F)
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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
        {
            if (!rPresionado)
            {
                rPresionado = true;

                // ===== VERIFICAR QUE HAYA 3 GOLES =====
                if (m_goles >= m_golesParaGanar && m_llaveObtenida)
                {
                    std::cout << "🚪 Saliendo del Nivel 7..." << std::endl;
                    game->avanzarNivel();
                }
                else if (m_goles < m_golesParaGanar)
                {
                    mostrarMensaje("Necesitas meter " + std::to_string(m_golesParaGanar - m_goles) + " goles mas", 2.f, sf::Color::Yellow);
                }
                else if (!m_llaveObtenida)
                {
                    mostrarMensaje("Necesitas la llave. Ganale a LeBron en los tiros libres.", 2.f, sf::Color::Red);
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
    // ===== DETECTAR CAMBIO DE TAMAÑO DE VENTANA (F11) - DEBE IR PRIMERO ABSOLUTO =====
    sf::Vector2u currentSize = window->getSize();
    static sf::Vector2u lastSize = currentSize;
       if (currentSize != lastSize)
    {
        lastSize = currentSize;
        
        // Reconfigurar la camara del nivel con el nuevo tamaño de ventana
        // Esto arregla que las entidades aparezcan en la parte negra
        m_camera = sf::View(
            sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
            sf::Vector2f(1280.f, 720.f)
        );
        
        float minijuegoW = currentSize.x * 0.75f;
        float minijuegoH = currentSize.y * 0.85f;
        float minijuegoX = (currentSize.x - minijuegoW) / 2.f;
        float minijuegoY = (currentSize.y - minijuegoH) / 2.f;

        // Forzar recarga de texturas porque la ventana se recreo
        // Las texturas de la GPU se invalidan al llamar a window->create()
        m_baloncestoMinigame.recargarTexturas();
        
        m_baloncestoMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
        m_baloncestoMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    }

    // ===== BALONCESTO ACTIVO =====
    if (m_baloncestoMinigame.isActive())
    {
        m_baloncestoMinigame.update(dt);
        m_player.update(dt);

        if (m_baloncestoMinigame.isGameWon() && !m_llaveObtenida && !m_baloncestoMinigame.isGameLost())
        {
            m_llaveObtenida = true;
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
            if (cp.x < hw) cp.x = hw;
            if (cp.x > m_worldSize.x - hw) cp.x = m_worldSize.x - hw;
        }
        if (hh * 2 >= m_worldSize.y)
            cp.y = m_worldSize.y / 2;
        else
        {
            if (cp.y < hh) cp.y = hh;
            if (cp.y > m_worldSize.y - hh) cp.y = m_worldSize.y - hh;
        }
        m_camera.setCenter(cp);
        return;
    }

    m_lebron.update(dt);

    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f)
    {
        m_msjActual.tiempoRestante -= dt;
        if (m_msjActual.tiempoRestante <= 0.0f)
            m_textoMensaje->setString("");
    }

    sf::Vector2f posAnterior = m_player.getPosition();

    // ========== ACTUALIZAR BALONES ==========
    // Radio de colisión circular del jugador (25px, subido 5px = 20px hacia arriba)
    float radioJugador = 25.f;
    sf::Vector2f jugadorPos = m_player.getPosition();
    // Centro del círculo de colisión: 5px más arriba que el centro del sprite
    sf::Vector2f centroColisionJugador(jugadorPos.x, jugadorPos.y + 15.f);

    for (size_t i = 0; i < m_balones.size(); i++)
    {
        m_balones[i]->update(dt);

        sf::Vector2f balonPos = m_balones[i]->getPosition();
        float radioBalon = m_balones[i]->getRadius();
        sf::FloatRect balonBounds = m_balones[i]->getBounds();

        // ===== COLISIÓN CIRCULAR JUGADOR-BALÓN =====
        float distJugadorBalon = std::sqrt(
            (centroColisionJugador.x - balonPos.x) * (centroColisionJugador.x - balonPos.x) +
            (centroColisionJugador.y - balonPos.y) * (centroColisionJugador.y - balonPos.y));

        if (distJugadorBalon < radioJugador + radioBalon)
        {
            sf::Vector2f dirBalon = balonPos - centroColisionJugador;
            float dist = std::sqrt(dirBalon.x * dirBalon.x + dirBalon.y * dirBalon.y);
            if (dist > 0.f)
            {
                dirBalon /= dist;
                m_balones[i]->empujar(dirBalon, 300.f);

                // Empujar al jugador en dirección opuesta
                jugadorPos -= dirBalon * 5.f;
                m_player.setPosition(jugadorPos.x, jugadorPos.y);
                centroColisionJugador = sf::Vector2f(jugadorPos.x, jugadorPos.y - 5.f);
            }
        }

        // ===== COLISIÓN BALÓN-PAREDES =====
        for (const auto &obj : m_mapaFisico)
        {
            if (balonBounds.findIntersection(obj.getBounds()).has_value())
            {
                sf::FloatRect pared = obj.getBounds();
                float centroBalonX = balonPos.x;
                float centroBalonY = balonPos.y;
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

        // ===== COLISIÓN BALÓN-LEBRON =====
        sf::FloatRect lebronBounds = m_lebron.getBounds();
        if (balonBounds.findIntersection(lebronBounds).has_value())
        {
            sf::Vector2f centroLebron(lebronBounds.position.x + lebronBounds.size.x / 2.f,
                                      lebronBounds.position.y + lebronBounds.size.y / 2.f);
            sf::Vector2f dir = balonPos - centroLebron;
            float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (dist > 0.f)
            {
                dir /= dist;
                m_balones[i]->empujar(dir, 250.f);
            }
        }

        // ===== COLISIÓN ENTRE BALONES =====
        for (size_t j = i + 1; j < m_balones.size(); j++)
        {
            sf::Vector2f balonPosJ = m_balones[j]->getPosition();
            float radioBalonJ = m_balones[j]->getRadius();

            sf::Vector2f delta = balonPosJ - balonPos;
            float distancia = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            float minDistancia = radioBalon + radioBalonJ;

            if (distancia < minDistancia)
            {
                // Separar balones
                sf::Vector2f normal = delta / distancia;
                float overlap = minDistancia - distancia;
                sf::Vector2f separacion = normal * overlap * 0.5f;

                m_balones[i]->setPosition(balonPos.x - separacion.x, balonPos.y - separacion.y);
                m_balones[j]->setPosition(balonPosJ.x + separacion.x, balonPosJ.y + separacion.y);

                // Intercambiar velocidades (rebote elástico)
                sf::Vector2f tempVel = m_balones[i]->getVelocity();
                m_balones[i]->setVelocity(m_balones[j]->getVelocity() * 0.9f);
                m_balones[j]->setVelocity(tempVel * 0.9f);
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
        if (m_cercaBalonBasket && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
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
        if (m_cercaDestornillador && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
        {
            if (!rCentinelaPresionado)
            {
                rCentinelaPresionado = true;
                mostrarMensaje("Entrando al centinela...", 2.f, sf::Color::Yellow);
                game->detenerMusica();
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

    // ===== DIBUJAR DEBUG (F3) =====
    if (m_debugMode)
    {
        // 1. Colisiones del mapa (rojo)
        for (const auto &obj : m_mapaFisico)
        {
            sf::RectangleShape colision;
            colision.setPosition(obj.getBounds().position);
            colision.setSize(obj.getBounds().size);
            colision.setFillColor(sf::Color(255, 0, 0, 100));
            colision.setOutlineThickness(2.f);
            colision.setOutlineColor(sf::Color::Red);
            window->draw(colision);
        }

        // 2. Círculo de colisión del JUGADOR (azul)
        sf::Vector2f jugadorPos = m_player.getPosition();
        float radioJugador = 25.f;
        sf::CircleShape jugadorCollisionCircle(radioJugador);
        jugadorCollisionCircle.setPosition(sf::Vector2f(jugadorPos.x, jugadorPos.y + 15.f));
        jugadorCollisionCircle.setOrigin(sf::Vector2f(radioJugador, radioJugador));
        jugadorCollisionCircle.setFillColor(sf::Color(0, 0, 255, 80));
        jugadorCollisionCircle.setOutlineThickness(2.f);
        jugadorCollisionCircle.setOutlineColor(sf::Color::Blue);
        window->draw(jugadorCollisionCircle);

        // 3. Radio de colisión circular de los BALONES (cyan)
        for (size_t i = 0; i < m_balones.size(); i++)
        {
            sf::CircleShape balonDebug(m_balones[i]->getRadius());
            balonDebug.setPosition(m_balones[i]->getPosition());
            balonDebug.setOrigin(sf::Vector2f(m_balones[i]->getRadius(), m_balones[i]->getRadius()));
            balonDebug.setFillColor(sf::Color(0, 255, 255, 80));
            balonDebug.setOutlineThickness(1.5f);
            balonDebug.setOutlineColor(sf::Color::Cyan);
            window->draw(balonDebug);
        }

        // 4. Hitbox de LeBron (magenta)
        sf::FloatRect lebronBounds = m_lebron.getBounds();
        sf::RectangleShape lebronDebug;
        lebronDebug.setPosition(sf::Vector2f(lebronBounds.position.x, lebronBounds.position.y));
        lebronDebug.setSize(sf::Vector2f(lebronBounds.size.x, lebronBounds.size.y));
        lebronDebug.setFillColor(sf::Color(255, 0, 255, 80));
        lebronDebug.setOutlineThickness(2.f);
        lebronDebug.setOutlineColor(sf::Color::Magenta);
        window->draw(lebronDebug);

        // 5. Área del ASCENSOR (verde)
        sf::RectangleShape ascensorDebug;
        ascensorDebug.setPosition(sf::Vector2f(m_puertaSalidaArea.position.x, m_puertaSalidaArea.position.y));
        ascensorDebug.setSize(sf::Vector2f(m_puertaSalidaArea.size.x, m_puertaSalidaArea.size.y));
        ascensorDebug.setFillColor(sf::Color(0, 255, 0, 80));
        ascensorDebug.setOutlineThickness(2.f);
        ascensorDebug.setOutlineColor(sf::Color::Green);
        window->draw(ascensorDebug);
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

    // Minijuego
    if (m_baloncestoMinigame.isActive())
    {
        window->setView(window->getDefaultView());
        m_baloncestoMinigame.draw(*window);
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

    // ===== FASE 2: UI =====
    window->setView(window->getDefaultView());

    // Texto de goles (esquina superior)
    if (m_fontLoaded && m_textoInteraccion)
    {
        std::string textoGoles = "Goles: " + std::to_string(m_goles) + "/" + std::to_string(m_golesParaGanar);
        m_textoInteraccion->setString(textoGoles);
        m_textoInteraccion->setFillColor(sf::Color::Yellow);
        m_textoInteraccion->setOutlineThickness(1.5f);
        m_textoInteraccion->setOutlineColor(sf::Color::Black);
        sf::FloatRect b = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(b.size.x / 2.f, b.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, 30.f));
        window->draw(*m_textoInteraccion);
    }

    // ===== TEXTOS DE INTERACCIÓN CENTRALES =====
    if (m_fontLoaded && m_textoInteraccion)
    {
        float winW = static_cast<float>(window->getSize().x);
        float winH = static_cast<float>(window->getSize().y);
        std::string textoActual = "";
        bool mostrarTexto = true;
        
        // 1. PRIORIDAD: Ascensor (salida)
        if (m_cercaPuertaSalida)
        {
            if (m_llaveObtenida)
            {
                textoActual = "Presiona F para salir del nivel";
            }
            else
            {
                textoActual = "Necesitas la llave. Ganale a LeBron en los tiros libres";
            }
        }
        // 2. LeBron (si está cerca y NO está en el ascensor)
        else if (m_cercaLebron && !m_llaveObtenida)
        {
            if (m_tieneBalonBasket)
            {
                textoActual = "Presiona F para jugar tiros libres con LeBron";
            }
            else
            {
                textoActual = "Necesitas un balon de baloncesto.Buscalo en la cancha";
            }
        }
        // 3. Si ya tiene la llave y está cerca de LeBron
        else if (m_cercaLebron && m_llaveObtenida)
        {
            textoActual = "Ya tienes la llave. Dirigete al ascensor";
        }
        else
        {
            mostrarTexto = false;
        }
        
        // Dibujar el mensaje centrado si hay texto
        if (mostrarTexto && !textoActual.empty())
        {
            m_textoInteraccion->setString(textoActual);
            m_textoInteraccion->setFillColor(sf::Color::Yellow);
            m_textoInteraccion->setOutlineThickness(1.5f);
            m_textoInteraccion->setOutlineColor(sf::Color::Black);
            
            sf::FloatRect bounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
            
            // SUBIR el mensaje cuando está cerca de LeBron
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 75.f));
            
            window->draw(*m_textoInteraccion);
        }
    }

    // ===== TEXTO DESTORNILLADOR =====
    if (!m_tieneDestornillador && m_cercaDestornillador && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona F para recoger el destornillador");
        m_textoInteraccion->setFillColor(sf::Color::Yellow);
        m_textoInteraccion->setOutlineThickness(1.5f);
        m_textoInteraccion->setOutlineColor(sf::Color::Black);
        sf::FloatRect b = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(b.size.x / 2.f, b.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 75.f));
        window->draw(*m_textoInteraccion);
    }

    // ===== TEXTO ENTRADA CENTINELA =====
    if (m_tieneDestornillador && m_cercaEntradaCentinela && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona F para abrir el ducto");
        m_textoInteraccion->setFillColor(sf::Color::Yellow);
        m_textoInteraccion->setOutlineThickness(1.5f);
        m_textoInteraccion->setOutlineColor(sf::Color::Black);
        sf::FloatRect b = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(b.size.x / 2.f, b.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 75.f));
        window->draw(*m_textoInteraccion);
    }

    // ===== TUTORIAL =====
    if (m_mostrarTutorial || m_mostrarTutorialPorTecla)
    {
        sf::RectangleShape overlay(sf::Vector2f(window->getSize().x, window->getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);
        if (m_fontLoaded)
        {
            sf::Text tt(m_font);
            tt.setString("NIVEL 7 - FUTBOL\n\nLleva los 3 balones al arco para poder salir.\n\n[M] Cerrar/Ayuda");
            tt.setCharacterSize(20);
            tt.setFillColor(sf::Color::White);
            sf::FloatRect b = tt.getLocalBounds();
            tt.setOrigin(sf::Vector2f(b.size.x / 2.f, b.size.y / 2.f));
            tt.setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y / 2.f));
            window->draw(tt);
        }
    }

    Inventory *inv = m_player.getInventory();
    if (inv)
        inv->draw(*window);
}

void Nivel7State::configurarColisiones()
{
    m_mapaFisico.clear();

    // ===== BORDES EXTERIORES =====
    // Borde izquierdo (0,0) a (33,941)
    m_mapaFisico.emplace_back(0.f, 0.f, 33.f, 941.f);

    // Borde inferior (0,935) a (1672,941)
    m_mapaFisico.emplace_back(0.f, 935.f, 1672.f, 6.f);

    // Borde derecho (1658,184) a (1672,941)
    m_mapaFisico.emplace_back(1658.f, 184.f, 14.f, 757.f);

    // ===== PARED SUPERIOR (CORREGIDA) =====
    // Pared Superior (0,0) a (1150,206)
    m_mapaFisico.emplace_back(0.f, 0.f, 1150.f, 206.f);

    // Pared2 (1149,0) a (1186,288)
    m_mapaFisico.emplace_back(1149.f, 0.f, 37.f, 288.f);

    // Pared5 (1186,0) a (1672,184)
    m_mapaFisico.emplace_back(1186.f, 0.f, 486.f, 184.f);

    // ===== LETREROS Y TABLEROS =====
    // Letrero (1082,288) a (1290,357)
    m_mapaFisico.emplace_back(1082.f, 288.f, 208.f, 69.f);

    // Tablero (1398,182) a (1491,203)
    m_mapaFisico.emplace_back(1398.f, 182.f, 93.f, 21.f);

    // ===== TUBOS =====
    // Tubo izquierdo (190,173) a (203,275)
    m_mapaFisico.emplace_back(190.f, 173.f, 13.f, 102.f);

    // Tubo derecho (418,181) a (432,275)
    m_mapaFisico.emplace_back(418.f, 181.f, 14.f, 94.f);

    // ===== BANCAS (ASIENTOS) =====
    // Banca 1 (559,265) a (591,378)
    m_mapaFisico.emplace_back(559.f, 265.f, 32.f, 113.f);

    // Banca 2 (541,544) a (599,658)
    m_mapaFisico.emplace_back(541.f, 544.f, 58.f, 114.f);

    // Banca 3 (607,676) a (734,710)
    m_mapaFisico.emplace_back(607.f, 676.f, 127.f, 34.f);

    // Banca 4 (770,676) a (899,710)
    m_mapaFisico.emplace_back(770.f, 676.f, 129.f, 34.f);

    // Banca 5 (927,676) a (1054,710)
    m_mapaFisico.emplace_back(927.f, 676.f, 127.f, 34.f);

    // ===== PAREDES INTERNAS =====
    // Pared1 (591,206) a (638,328)
    m_mapaFisico.emplace_back(591.f, 206.f, 47.f, 122.f);

    // Pared3 (591,328) a (690,420)
    m_mapaFisico.emplace_back(591.f, 328.f, 99.f, 92.f);

    // Pared4 (799,328) a (1267,420)
    m_mapaFisico.emplace_back(799.f, 328.f, 468.f, 92.f);

    // Murito (599,515) a (623,536)
    m_mapaFisico.emplace_back(599.f, 515.f, 24.f, 21.f);

    // Pared 6 (599,536) a (1088,673)
    m_mapaFisico.emplace_back(599.f, 536.f, 489.f, 137.f);

    // Pared 7 (1067,673) a (1088,941)
    m_mapaFisico.emplace_back(1067.f, 673.f, 21.f, 268.f);

    // Pared 8 (1247,515) a (1269,609)
    m_mapaFisico.emplace_back(1247.f, 515.f, 22.f, 94.f);

    // Pared 9 (1247,609) a (1672,742)
    m_mapaFisico.emplace_back(1247.f, 609.f, 425.f, 133.f);

    std::cout << "✅ Colisiones del Nivel 7 actualizadas: " << m_mapaFisico.size() << " paredes" << std::endl;
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