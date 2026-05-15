#include "Lobby.hpp"
#include "PauseState.hpp"
#include "Nivel2State.hpp"
#include <iostream>
#include <cmath>

LobbyState::LobbyState(sf::RenderWindow *window, Game *game)
    : State(window, game), m_background(nullptr), m_cercaAscensor(false), m_textoInteraccion(nullptr)
{
    // 1. CARGAR JUGADOR
    m_player.loadAssets();
    m_player.setPosition(600, 300);
    m_player.setSpeed(300.0f);

    m_worldSize = sf::Vector2f(1280.f, 720.f);
    m_camera = sf::View(sf::Vector2f(640.f, 360.f), sf::Vector2f(1280.f, 720.f));

    // 2. CARGAR FONDO DEL LOBBY
    if (m_backgroundTexture.loadFromFile("assets/images/lobby/background.png"))
    {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2f scale(
            1280.0f / m_backgroundTexture.getSize().x,
            720.0f / m_backgroundTexture.getSize().y);
        m_background->setScale(scale);
    }
    else
    {
        std::cerr << "Error: No se pudo cargar fondo del lobby" << std::endl;
    }

    // 3. CONFIGURAR COLISIONES
    configurarColisiones();

    // 4. CONFIGURAR ÁREA DEL ASCENSOR
    m_ascensorArea = sf::FloatRect(
    sf::Vector2f(1025.f * 1280.f/1377.f, 39.f * 720.f/768.f),
    sf::Vector2f(204.f * 1280.f/1377.f, 212.f * 720.f/768.f));

    // 5. CONFIGURAR TEXTO DE INTERACCIÓN
    if (m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf"))
    {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setString("Presiona F para usar el ascensor");
        m_textoInteraccion->setCharacterSize(20);
        m_textoInteraccion->setFillColor(sf::Color::White);
         m_textoInteraccion->setOutlineThickness(1.5f);
        m_textoInteraccion->setOutlineColor(sf::Color::Black);
        m_textoInteraccion->setPosition(sf::Vector2f(640.f, 650.f));

        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
    }

    // Verificar si hay partida activa
    if (!game->tienePartidaActiva())
    {
        std::cout << "No hay partida activa. Se recomienda crear una nueva partida." << std::endl;
    }
    game->setIsInLevel(true);
}

void LobbyState::update(float dt)
{
    sf::Vector2f posAnterior = m_player.getPosition();

    // MOVIMIENTO
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

    if (movimiento.x != 0 || movimiento.y != 0)
    {
        float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
        movimiento /= length;
    }

    m_player.move(movimiento, dt);
    m_player.update(dt);

    // Verificación de colisiones
    for (const auto &obj : m_mapaFisico)
    {
        if (m_player.getHurtbox().findIntersection(obj.getBounds()).has_value())
        {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }

    // VERIFICAR SI EL JUGADOR ESTÁ CERCA DEL ASCENSOR
    std::optional<sf::FloatRect> interseccion = m_player.getHurtbox().findIntersection(m_ascensorArea);
    m_cercaAscensor = interseccion.has_value();

    // INTERACCIÓN CON ASCENSOR
    static bool rProcesado = false;
    if (m_cercaAscensor && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
    {
        if (!rProcesado)
        {
            rProcesado = true;

            // GUARDAR PARTIDA antes de cambiar de nivel
            if (game->tienePartidaActiva())
            {
                game->getSaveManager().setNivelActual(1, 1); // Nivel 1, Nodo 1
                game->guardarPartidaActual();
                std::cout << "💾 Partida guardada automáticamente al entrar al Nivel 1" << std::endl;
            }

            std::cout << "Transición al Nivel 1" << std::endl;
            game->changeState(std::make_unique<Nivel2State>(window, game));
            return;
        }
    }
    else
    {
        rProcesado = false;
    }

    // PAUSA
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

void LobbyState::draw()
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
        fallback.setFillColor(sf::Color(30, 30, 50));
        window->draw(fallback);
    }

    // DEBUG: Dibujar colisiones (F3)
    if (m_debugMode)
    {
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
        ascensorDebug.setPosition(sf::Vector2f(m_ascensorArea.position.x, m_ascensorArea.position.y));
        ascensorDebug.setSize(sf::Vector2f(m_ascensorArea.size.x, m_ascensorArea.size.y));
        ascensorDebug.setFillColor(sf::Color(0, 255, 0, 80));
        ascensorDebug.setOutlineThickness(2.f);
        ascensorDebug.setOutlineColor(sf::Color::Green);
        window->draw(ascensorDebug);
    }

    // Jugador (se dibuja en el mundo)
    m_player.draw(*window);

    // ===== FASE 2: UI (texto de interacción) =====
    window->setView(window->getDefaultView());

    if (m_cercaAscensor && m_textoInteraccion)
    {
        sf::Vector2u winSize = window->getSize();
        m_textoInteraccion->setPosition(sf::Vector2f(winSize.x / 2.f, winSize.y - 70.f));
        window->draw(*m_textoInteraccion);
    }
}
void LobbyState::configurarColisiones()
{
    m_mapaFisico.clear();
    
    // Escala de la imagen original (1377x768) a cámara (1280x720)
    float escalaX = 1280.f / 1377.f;
    float escalaY = 720.f / 768.f;
    
    auto crear = [&](float x, float y, float w, float h) {
        m_mapaFisico.emplace_back(
            x * escalaX, y * escalaY,
            w * escalaX, h * escalaY);
    };
    
    // Pared superior (0,0) a (1377,211)
    crear(0.f, 0.f, 1377.f, 211.f);
    
    // PC esquina izquierda (1231,211) a (146,76)
    crear(1231.f, 211.f, 146.f, 76.f);
    
    // PC centro (633,421) a (126,128)
    crear(633.f, 421.f, 126.f, 128.f);
    
    // Lavaplatos (341,683) a (140,74)
    crear(341.f, 683.f, 140.f, 74.f);
    
    // Borde izquierdo (0,0) a (19,768)
    crear(0.f, 0.f, 19.f, 768.f);
    
    // Bebedero (19,475) a (66,164)
    crear(19.f, 475.f, 66.f, 164.f);
    
    // Parlante (19,211) a (184,136)
    crear(19.f, 211.f, 184.f, 136.f);
    
    // PC arriba (223,211) a (214,78)
    crear(223.f, 211.f, 214.f, 78.f);
    
    // Cama (1099,597) a (278,171)
    crear(1099.f, 597.f, 278.f, 171.f);

    // Borde inferior (0,755) a (1377,13)
    crear(0.f, 755.f, 1377.f, 13.f);
    
    // Signos (1278,519) a (94,54)
    crear(1278.f, 519.f, 94.f, 54.f);
    
    // Borde derecho (1360,0) a (17,768)
    crear(1360.f, 0.f, 17.f, 768.f);
    
    // Papelera (905,680) a (45,76)
    crear(905.f, 680.f, 45.f, 76.f);
}

void LobbyState::handleEvent(const sf::Event &event)
{
    if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::F3)
        {
            m_debugMode = !m_debugMode;
        }
    }
}