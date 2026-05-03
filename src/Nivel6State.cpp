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
      m_fontLoaded(false)
{
    // Mensaje temporal
    m_msjActual.texto = "";
    m_msjActual.tiempoRestante = 0.0f;
    m_msjActual.color = sf::Color::Yellow;

    // Cargar jugador
    m_player.loadAssets();
    m_player.setPosition(800.f, 600.f);
    m_player.setSpeed(300.0f);

    // Verificar tutorial (primera vez)
    if (game->tienePartidaActiva())
    {
        const auto &items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
        auto it = std::find(items.begin(), items.end(), "TutorialNivel6Visto");

        if (it == items.end())
        {
            m_mostrarTutorial = true;
            game->getSaveManager().addItemRecolectado("TutorialNivel6Visto");
            std::cout << "Primer ingreso al Nivel 6: Mostrando tutorial" << std::endl;
        }
    }

    // Cargar fondo PNG
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel6/background.png"))
    {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x),
                                   static_cast<float>(textureSize.y));
        std::cout << "✅ Nivel 6 cargado. Tamaño: " << m_worldSize.x << "x" << m_worldSize.y << std::endl;
    }
    else
    {
        std::cerr << "❌ Error: No se pudo cargar background del nivel 6" << std::endl;
        m_worldSize = sf::Vector2f(1754.f, 1587.f);
    }

    // Cámara fija
    sf::Vector2u windowSize = window->getSize();
    float fixedWidth = 1280.f;
    float fixedHeight = 720.f;
    m_camera = sf::View(
        sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
        sf::Vector2f(fixedWidth, fixedHeight));
    m_lastWindowSize = windowSize;

    // Área de salida
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(1550.f, 1350.f), sf::Vector2f(120.f, 180.f));

    // Colisiones
    configurarColisiones();

    // Fuente
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (!m_fontLoaded)
    {
        std::cerr << "⚠️ No se pudo cargar la fuente en Nivel6State" << std::endl;
    }

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
        game->getSaveManager().setNivelActual(6, 6);
        game->guardarPartidaActual();
        std::cout << "💾 Partida guardada en Nivel 6" << std::endl;
    }

    // Posicionar el gallo en el gallinero
    m_gallo.setPosition(1471.f, 773.f); // Posición del gallinero (ajustable)
    m_gallo.setLimites(1350.f, 1600.f); // Límites de movimiento horizontal

    m_galloArea = sf::FloatRect(sf::Vector2f(0.f, 0.f), sf::Vector2f(0.f, 0.f));
    m_cercaGallo = false;

    // Rifle (objeto a recoger en posición 522, 500 - ajustable)
    m_rifleRecogido = false;
    m_rifleArea = sf::FloatRect(sf::Vector2f(522.f, 500.f), sf::Vector2f(40.f, 40.f));
    m_cercaRifle = false;

    // Minijuego RoosterHunt
    float rw = windowSize.x * 0.8f;
    float rh = windowSize.y * 0.85f;
    m_roosterHuntMinigame.setSize(sf::Vector2f(rw, rh));
    m_roosterHuntMinigame.setPosition(sf::Vector2f(
        (windowSize.x - rw) / 2.f,
        (windowSize.y - rh) / 2.f));

    game->setIsInLevel(true);
    std::cout << "✅ Nivel6State inicializado correctamente" << std::endl;
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
            if (game->tienePartidaActiva())
            {
                const auto &items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
                auto it = std::find(items.begin(), items.end(), "TutorialNivel6Visto");
                if (it != items.end())
                {
                    m_mostrarTutorialPorTecla = true;
                }
                else
                {
                    m_mostrarTutorial = true;
                }
            }
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
                std::cout << "🐔 Minijuego RoosterHunt cerrado" << std::endl;
                return;
            }
        }
    }

    Inventory *inv = m_player.getInventory();
    if (inv)
    {
        inv->handleEvent(event, *window);
    }
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
        {
            m_textoMensaje->setString("");
        }
    }

    // ========== DETECTAR RIFLE ==========
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
                mostrarMensaje("¡Has recogido el rifle! Ve al gallinero", 2.f, sf::Color::Green);
                std::cout << "🔫 Rifle recogido!" << std::endl;
            }
        }
        else
        {
            rRiflePresionado = false;
        }
    }

    // ========== ACTIVAR ROOSTERHUNT CON GALLO ==========
    if (m_rifleRecogido && m_cercaGallo && !m_roosterHuntMinigame.isActive())
    {
        static bool rGalloPresionado = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
        {
            if (!rGalloPresionado)
            {
                rGalloPresionado = true;
                m_roosterHuntMinigame.activate();
                std::cout << "🐔 Minijuego RoosterHunt activado!" << std::endl;
            }
        }
        else
        {
            rGalloPresionado = false;
        }
    }

    // Si el minijuego está activo
    if (m_roosterHuntMinigame.isActive())
    {
        m_roosterHuntMinigame.update(dt);
        m_player.update(dt);

        // Cámara
        sf::Vector2f playerPos = m_player.getPosition();
        sf::Vector2f cameraPos = playerPos;
        float halfWidth = 1280.f / 2.f;
        float halfHeight = 720.f / 2.f;
        if (halfWidth * 2.f >= m_worldSize.x)
        {
            cameraPos.x = m_worldSize.x / 2.f;
        }
        else
        {
            if (cameraPos.x < halfWidth)
                cameraPos.x = halfWidth;
            if (cameraPos.x > m_worldSize.x - halfWidth)
                cameraPos.x = m_worldSize.x - halfWidth;
        }
        if (halfHeight * 2.f >= m_worldSize.y)
        {
            cameraPos.y = m_worldSize.y / 2.f;
        }
        else
        {
            if (cameraPos.y < halfHeight)
                cameraPos.y = halfHeight;
            if (cameraPos.y > m_worldSize.y - halfHeight)
                cameraPos.y = m_worldSize.y - halfHeight;
        }
        m_camera.setCenter(cameraPos);

        return;
    }

    sf::Vector2f posAnterior = m_player.getPosition();

    Inventory *inv = m_player.getInventory();
    if (!inv || !inv->isOpen())
    {
        sf::Vector2f movimiento(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            movimiento.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            movimiento.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            movimiento.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            movimiento.x += 1.f;

        if (movimiento.x != 0.f || movimiento.y != 0.f)
        {
            float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
            movimiento /= length;
        }
        m_player.move(movimiento, dt);
    }

    m_player.update(dt);

    // Actualizar gallo
    m_gallo.update(dt);

    // Colisiones con paredes
    for (const auto &obj : m_mapaFisico)
    {
        if (m_player.getHurtbox().findIntersection(obj.getBounds()).has_value())
        {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }

    // Colisión con el gallo
    m_galloArea = m_gallo.getBounds();
    if (m_player.getHurtbox().findIntersection(m_galloArea).has_value())
    {
        m_player.setPosition(posAnterior.x, posAnterior.y);
    }

    // Verificar si está cerca del gallo
    m_cercaGallo = m_player.getHurtbox().findIntersection(m_galloArea).has_value();

    sf::Vector2f playerPos = m_player.getPosition();
    sf::Vector2f cameraPos = playerPos;
    float halfWidth = 1280.f / 2.f;
    float halfHeight = 720.f / 2.f;
    if (halfWidth * 2.f >= m_worldSize.x)
    {
        cameraPos.x = m_worldSize.x / 2.f;
    }
    else
    {
        if (cameraPos.x < halfWidth)
            cameraPos.x = halfWidth;
        if (cameraPos.x > m_worldSize.x - halfWidth)
            cameraPos.x = m_worldSize.x - halfWidth;
    }
    if (halfHeight * 2.f >= m_worldSize.y)
    {
        cameraPos.y = m_worldSize.y / 2.f;
    }
    else
    {
        if (cameraPos.y < halfHeight)
            cameraPos.y = halfHeight;
        if (cameraPos.y > m_worldSize.y - halfHeight)
            cameraPos.y = m_worldSize.y - halfHeight;
    }
    m_camera.setCenter(cameraPos);

    verificarSalidaNivel();

    if (!m_mostrarTutorial && !m_mostrarTutorialPorTecla)
    {
        static bool escapeProcesado = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
        {
            if (!escapeProcesado)
            {
                escapeProcesado = true;
                game->pushState(std::make_unique<PauseState>(window, game));
            }
        }
        else
        {
            escapeProcesado = false;
        }
    }
}

void Nivel6State::draw()
{
    if (!window)
        return;

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

    // Dibujar gallo
    m_gallo.draw(*window);
    // Dibujar jugador
    m_player.draw(*window);

     // DEBUG: Dibujar rifle
    if (!m_rifleRecogido)
    {
        sf::RectangleShape rifleDebug(sf::Vector2f(m_rifleArea.size.x, m_rifleArea.size.y));
        rifleDebug.setPosition(sf::Vector2f(m_rifleArea.position.x, m_rifleArea.position.y));
        rifleDebug.setFillColor(sf::Color(255, 165, 0, 150));
        rifleDebug.setOutlineThickness(2.f);
        rifleDebug.setOutlineColor(sf::Color::Magenta);
        window->draw(rifleDebug);
    }

    window->setView(window->getDefaultView());

    if (m_cercaPuertaSalida && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona E para avanzar al siguiente nivel");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

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

    Inventory *inv = m_player.getInventory();
    if (inv)
        inv->draw(*window);

    // Texto para recoger rifle
    if (!m_rifleRecogido && m_cercaRifle && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona R para recoger el rifle");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    // Texto para jugar con el gallo
    if (m_rifleRecogido && m_cercaGallo && !m_roosterHuntMinigame.isActive() && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona R para cazar gallos");
        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
        m_textoInteraccion->setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y - 70.f));
        window->draw(*m_textoInteraccion);
    }

    // Dibujar minijuego
    if (m_roosterHuntMinigame.isActive())
    {
        window->setView(window->getDefaultView());
        m_roosterHuntMinigame.draw(*window);
    }
}

void Nivel6State::configurarColisiones()
{
    m_mapaFisico.clear();

    m_mapaFisico.emplace_back(-10.f, 0.f, 10.f, m_worldSize.y);
    m_mapaFisico.emplace_back(m_worldSize.x, 0.f, 10.f, m_worldSize.y);
    m_mapaFisico.emplace_back(0.f, -10.f, m_worldSize.x, 10.f);
    m_mapaFisico.emplace_back(0.f, m_worldSize.y, m_worldSize.x, 10.f);

    std::cout << "✅ Colisiones del Nivel 6 configuradas" << std::endl;
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
    std::cout << "📢 " << texto << std::endl;
}