#include "Nivel7State.hpp"
#include "PauseState.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

Nivel7State::Nivel7State(sf::RenderWindow *window, Game *game)
    : State(window, game),
      m_background(nullptr),
      m_cercaPuertaSalida(false),
      m_mostrarTutorial(false),
      m_mostrarTutorialPorTecla(false),
      m_fontLoaded(false)
{
    m_msjActual.texto = "";
    m_msjActual.tiempoRestante = 0.0f;
    m_msjActual.color = sf::Color::Yellow;

    // Cargar jugador
    m_player.loadAssets();
    m_player.setPosition(800.f, 600.f);
    m_player.setSpeed(300.0f);

    // Tutorial (primera vez)
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
        m_worldSize = sf::Vector2f(1812.f, 1016.f);
    }

    // Cámara fija
    m_camera = sf::View(sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
                        sf::Vector2f(1280.f, 720.f));
    m_lastWindowSize = window->getSize();

    // Área de salida (ajustar después)
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(866.f, 61.f), sf::Vector2f(192.f, 151.f));

    // Colisiones básicas (luego añadimos más)
    configurarColisiones();

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

    game->setIsInLevel(true);
    std::cout << "✅ Nivel7State inicializado correctamente" << std::endl;

    // Balón en la mitad del mapa
    m_balon.setPosition(m_worldSize.x / 2.f, m_worldSize.y / 2.f);
    m_balonEnArco = false;

    // Área del arco (AJUSTAR SEGÚN TU MAPA)
    m_arcoArea = sf::FloatRect(sf::Vector2f(200.f, 200.f), sf::Vector2f(100.f, 150.f));
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
                std::cout << "🚪 Saliendo del Nivel 7..." << std::endl;
                game->avanzarNivel();
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
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f)
    {
        m_msjActual.tiempoRestante -= dt;
        if (m_msjActual.tiempoRestante <= 0.0f)
            m_textoMensaje->setString("");
    }

    sf::Vector2f posAnterior = m_player.getPosition();

    // ========== INTERACCIÓN CON EL BALÓN (COLISIÓN CIRCULAR) ==========
    m_balon.update(dt);

    // Detectar si el jugador toca el balón (usando distancia circular)
    sf::Vector2f jugadorPos = m_player.getPosition();
    sf::Vector2f balonPos = m_balon.getPosition();
    float distJugadorBalon = std::sqrt(
        (jugadorPos.x - balonPos.x) * (jugadorPos.x - balonPos.x) +
        (jugadorPos.y - balonPos.y) * (jugadorPos.y - balonPos.y));

    float radioBalon = m_balon.getRadius();
    float radioJugador = 25.f; // Radio aproximado del jugador

    if (distJugadorBalon < radioBalon + radioJugador)
    {
        // Dirección del empuje
        sf::Vector2f dirBalon = balonPos - jugadorPos;
        float dist = std::sqrt(dirBalon.x * dirBalon.x + dirBalon.y * dirBalon.y);

        if (dist > 0.f)
        {
            dirBalon /= dist;
            m_balon.empujar(dirBalon, 250.f);

            // Separar jugador del balón
            jugadorPos -= dirBalon * 3.f;
            m_player.setPosition(jugadorPos.x, jugadorPos.y);
        }
    }

    // Actualizar área del balón para el arco
    m_balonArea = m_balon.getBounds();

    // Verificar si el balón llegó al arco
    if (!m_balonEnArco && m_balonArea.findIntersection(m_arcoArea).has_value())
    {
        m_balonEnArco = true;
        mostrarMensaje("¡GOL! Has metido el balón al arco", 3.f, sf::Color::Green);
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

    // Colisiones del balón con paredes
    sf::FloatRect balonBounds = m_balon.getBounds();

    for (const auto &obj : m_mapaFisico)
    {
        if (balonBounds.findIntersection(obj.getBounds()).has_value())
        {
            sf::FloatRect pared = obj.getBounds();

            // Calcular centros
            float centroBalonX = balonPos.x;
            float centroBalonY = balonPos.y;
            float centroParedX = pared.position.x + pared.size.x / 2.f;
            float centroParedY = pared.position.y + pared.size.y / 2.f;

            // Diferencia horizontal vs vertical
            float diffX = std::abs(centroBalonX - centroParedX) / (pared.size.x / 2.f + balonBounds.size.x / 2.f);
            float diffY = std::abs(centroBalonY - centroParedY) / (pared.size.y / 2.f + balonBounds.size.y / 2.f);

            if (diffX > diffY)
            {
                // Rebote horizontal
                float dirX = (centroBalonX < centroParedX) ? -1.f : 1.f;
                m_balon.empujar(sf::Vector2f(dirX, 0.f), 200.f);
            }
            else
            {
                // Rebote vertical
                float dirY = (centroBalonY < centroParedY) ? -1.f : 1.f;
                m_balon.empujar(sf::Vector2f(0.f, dirY), 200.f);
            }
            break;
        }
    }

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

    // Pausa
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

    window->setView(m_camera);

    if (m_background)
        window->draw(*m_background);
    else
    {
        sf::RectangleShape fb(m_worldSize);
        fb.setFillColor(sf::Color(50, 30, 30));
        window->draw(fb);
    }

    // DEBUG: Dibujar colisiones
    for (const auto &obj : m_mapaFisico)
    {
        sf::RectangleShape colision;
        colision.setPosition(sf::Vector2f(obj.getBounds().position.x, obj.getBounds().position.y));
        colision.setSize(sf::Vector2f(obj.getBounds().size.x, obj.getBounds().size.y));
        colision.setFillColor(sf::Color(255, 0, 0, 80));
        colision.setOutlineThickness(1.f);
        colision.setOutlineColor(sf::Color::Red);
        window->draw(colision);
    }

    // DEBUG: Dibujar colisión circular del balón
    sf::CircleShape balonDebug(m_balon.getRadius());
    balonDebug.setPosition(m_balon.getPosition());
    balonDebug.setOrigin(sf::Vector2f(m_balon.getRadius(), m_balon.getRadius()));
    balonDebug.setFillColor(sf::Color(0, 255, 0, 80));
    balonDebug.setOutlineThickness(1.f);
    balonDebug.setOutlineColor(sf::Color::Green);
    window->draw(balonDebug);

    // DEBUG: Dibujar colisión circular del jugador (en los pies)
    sf::CircleShape jugadorDebug(25.f);
    sf::Vector2f posJugador = m_player.getPosition();
    jugadorDebug.setPosition(sf::Vector2f(posJugador.x, posJugador.y + 30.f));
    jugadorDebug.setOrigin(sf::Vector2f(25.f, 25.f));
    jugadorDebug.setFillColor(sf::Color(0, 0, 255, 80));
    jugadorDebug.setOutlineThickness(1.f);
    jugadorDebug.setOutlineColor(sf::Color::Blue);
    window->draw(jugadorDebug);

    // DEBUG: Dibujar área del arco
    sf::RectangleShape arcoDebug(sf::Vector2f(m_arcoArea.size.x, m_arcoArea.size.y));
    arcoDebug.setPosition(sf::Vector2f(m_arcoArea.position.x, m_arcoArea.position.y));
    arcoDebug.setFillColor(sf::Color(255, 255, 0, 80));
    arcoDebug.setOutlineThickness(2.f);
    arcoDebug.setOutlineColor(sf::Color::Yellow);
    window->draw(arcoDebug);

    // Dibujar balón
    m_balon.draw(*window);

    m_player.draw(*window);

    window->setView(window->getDefaultView());

    if (m_cercaPuertaSalida && m_textoInteraccion && m_fontLoaded)
    {
        m_textoInteraccion->setString("Presiona R para salir del nivel");
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
            tt.setString("NIVEL 7\n\nMete el balon al arco y encesta los tiros libres.\n\n[ESC] Cerrar | [M] Ayuda");
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

    // Bordes del mundo
    m_mapaFisico.emplace_back(-10.f, 0.f, 10.f, m_worldSize.y);
    m_mapaFisico.emplace_back(m_worldSize.x, 0.f, 10.f, m_worldSize.y);
    m_mapaFisico.emplace_back(0.f, -10.f, m_worldSize.x, 10.f);
    m_mapaFisico.emplace_back(0.f, m_worldSize.y, m_worldSize.x, 10.f);

    std::cout << "✅ Colisiones del Nivel 7 configuradas (básicas)" << std::endl;
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