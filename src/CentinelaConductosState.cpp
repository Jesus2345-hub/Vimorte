#include "CentinelaConductosState.hpp"
#include "PauseState.hpp"
#include "MuerteCentinelaState.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

// ============================================================
// CONSTRUCTOR
// ============================================================
CentinelaConductosState::CentinelaConductosState(sf::RenderWindow *window, Game *game)
    : State(window, game), m_tiempoRestante(60.0f), m_tiempoAgotado(false), m_nivelCompletado(false), m_fontLoaded(false), m_mensajeTimer(0.f), m_parpadeoTimer(0.f), m_parpadeoVisible(true)
{
    // ============================================================
    // FONDO OSCURO
    // ============================================================
    m_background.setSize(sf::Vector2f(1280.f, 720.f));
    m_background.setFillColor(sf::Color(25, 25, 30));

    // Cargar fondo del conducto
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel7/conductos_bg.png"))
    {
        m_backgroundSprite = std::make_unique<sf::Sprite>(m_backgroundTexture);

        // Escalar para que ocupe toda la pantalla
        float escalaX = 1280.f / 1024.f;
        float escalaY = 720.f / 1053.f;
        float escala = std::min(escalaX, escalaY);

        m_backgroundSprite->setScale(sf::Vector2f(escala, escala));
        m_backgroundSprite->setPosition(sf::Vector2f(0.f, 0.f));

        std::cout << "Fondo de conductos cargado" << std::endl;
    }
    else
    {
        std::cerr << "Error cargando conductos_bg.png" << std::endl;
    }

    // ============================================================
    // JUGADOR
    // ============================================================
    m_player.loadAssets();
    m_player.setPosition(100.f, 350.f);
    m_player.setSpeed(200.f);

    // ============================================================
    // CONFIGURAR LABERINTO
    // ============================================================
    configurarLaberinto();

    // ============================================================
    // SALIDA
    // ============================================================
    m_exitZone.setSize(sf::Vector2f(60.f, 60.f));
    m_exitZone.setFillColor(sf::Color(0, 255, 0, 100));
    m_exitZone.setOutlineThickness(3.f);
    m_exitZone.setOutlineColor(sf::Color::Green);
    m_exitZone.setPosition(sf::Vector2f(1150.f, 580.f));
    m_exitBounds = m_exitZone.getGlobalBounds();

    // ============================================================
    // FUENTE Y TEXTOS
    // ============================================================
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");

    if (m_fontLoaded)
    {
        m_tituloText = std::make_unique<sf::Text>(m_font, "ESCAPE POR LOS CONDUCTOS", 28);
        m_tituloText->setFillColor(sf::Color(255, 150, 50));
        m_tituloText->setOutlineThickness(1.f);
        m_tituloText->setOutlineColor(sf::Color::Black);

        m_cronometroText = std::make_unique<sf::Text>(m_font, "", 32);
        m_cronometroText->setOutlineThickness(1.f);
        m_cronometroText->setOutlineColor(sf::Color::Black);

        m_instruccionesText = std::make_unique<sf::Text>(m_font,
                                                         "WASD/Flechas: Mover | ESC: Pausa", 14);
        m_instruccionesText->setFillColor(sf::Color(150, 150, 150));

        m_mensajeText = std::make_unique<sf::Text>(m_font, "", 20);
        m_mensajeText->setFillColor(sf::Color::Yellow);
        m_mensajeText->setOutlineThickness(1.f);
        m_mensajeText->setOutlineColor(sf::Color::Black);
    }

    mostrarMensaje("ENCUENTRA LA SALIDA ANTES DE QUEDARTE SIN OXIGENO!", 4.0f);

    if (game->tienePartidaActiva())
    {
        game->guardarPartidaActual();
    }

    std::cout << "Centinela: Escape por los Conductos inicializado" << std::endl;
    game->setIsInLevel(true);
}

// ============================================================
// CONFIGURAR LABERINTO
// ============================================================
void CentinelaConductosState::configurarLaberinto()
{
    m_walls.clear();
    m_wallBounds.clear();

    auto crearPared = [&](float x, float y, float w, float h)
    {
        // Escalar las coordenadas al tamaño de la ventana (1280x720)
        float escalaX = 1280.f / 1024.f;
        float escalaY = 720.f / 1053.f;

        sf::RectangleShape wall(sf::Vector2f(w * escalaX, h * escalaY));
        wall.setPosition(sf::Vector2f(x * escalaX, y * escalaY));
        wall.setFillColor(sf::Color::Transparent); // Invisibles (solo colisión)
        wall.setOutlineThickness(0.f);
        m_walls.push_back(wall);
        m_wallBounds.push_back(wall.getGlobalBounds());
    };

    // ============================================================
    // PAREDES EXTERIORES
    // ============================================================
    crearPared(0.f, 0.f, 310.f, 40.f);     // Techo izquierdo
    crearPared(470.f, 0.f, 554.f, 40.f);   // Techo derecho
    crearPared(0.f, 0.f, 40.f, 410.f);     // Lateral izq superior
    crearPared(0.f, 600.f, 40.f, 453.f);   // Lateral izq inferior
    crearPared(984.f, 0.f, 40.f, 1053.f);  // Lateral derecho
    crearPared(0.f, 1013.f, 1024.f, 40.f); // Piso

    // ============================================================
    // BLOQUES INTERNOS
    // ============================================================
    crearPared(310.f, 290.f, 540.f, 400.f); // Bloque central grande
    crearPared(310.f, 40.f, 160.f, 250.f);  // Divisor superior
    crearPared(120.f, 120.f, 190.f, 170.f); // Codo superior izq
    crearPared(120.f, 410.f, 190.f, 470.f); // Pasillo izquierdo
    crearPared(720.f, 810.f, 264.f, 203.f); // Bloque inferior der
    crearPared(580.f, 810.f, 40.f, 203.f);  // Divisor inferior

    std::cout << "Laberinto creado con " << m_wallBounds.size() << " paredes" << std::endl;
}

// ============================================================
// MANEJAR EVENTOS
// ============================================================
void CentinelaConductosState::handleEvent(const sf::Event &event)
{
    if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Escape)
        {
            game->pushState(std::make_unique<PauseState>(window, game));
        }
    }
}

// ============================================================
// VERIFICAR SALIDA
// ============================================================
void CentinelaConductosState::verificarSalida()
{
    if (m_player.getHurtbox().findIntersection(m_exitBounds).has_value())
    {
        m_nivelCompletado = true;
        std::cout << "Salida encontrada! Final Alternativo" << std::endl;
        mostrarMensaje("¡ESCAPASTE! Has encontrado la salida.", 3.0f);
    }
}

// ============================================================
// ACTUALIZAR CRONÓMETRO
// ============================================================
void CentinelaConductosState::actualizarCronometro(float dt)
{
    if (m_nivelCompletado || m_tiempoAgotado)
        return;

    m_tiempoRestante -= dt;

    if (m_tiempoRestante <= 0.f)
    {
        m_tiempoRestante = 0.f;
        m_tiempoAgotado = true;
        mostrarMensaje("SE ACABO EL OXIGENO...", 3.0f);
    }

    if (m_cronometroText)
    {
        int segundos = static_cast<int>(m_tiempoRestante);
        std::stringstream ss;
        ss << "OXIGENO: " << segundos << "s";
        m_cronometroText->setString(ss.str());

        if (m_tiempoRestante > 30.f)
        {
            m_cronometroText->setFillColor(sf::Color::White);
        }
        else if (m_tiempoRestante > 15.f)
        {
            m_cronometroText->setFillColor(sf::Color::Yellow);
        }
        else if (m_tiempoRestante > 5.f)
        {
            m_cronometroText->setFillColor(sf::Color(255, 165, 0));
        }
        else
        {
            m_cronometroText->setFillColor(sf::Color::Red);
        }
    }

    if (m_tiempoRestante < 10.f)
    {
        m_parpadeoTimer += dt;
        if (m_parpadeoTimer > 0.5f)
        {
            m_parpadeoTimer = 0.f;
            m_parpadeoVisible = !m_parpadeoVisible;
        }
    }
}

// ============================================================
// MOSTRAR MENSAJE TEMPORAL
// ============================================================
void CentinelaConductosState::mostrarMensaje(const std::string &texto, float duracion)
{
    if (m_mensajeText)
    {
        m_mensajeText->setString(texto);
    }
    m_mensajeTimer = duracion;
}

// ============================================================
// ACTUALIZAR
// ============================================================
void CentinelaConductosState::update(float dt)
{
    if (m_nivelCompletado || m_tiempoAgotado)
    {
        m_mensajeTimer -= dt;
        if (m_mensajeTimer <= 0.f)
        {
            if (m_nivelCompletado)
            {
                std::cout << "Final Alternativo conseguido" << std::endl;
                game->volverDeCentinela();
            }
            else
            {
                GameProgressData &progress = game->getSaveManager().getCurrentProgress();
                game->getSaveManager().addMuerte();

                if (progress.modoElegido == GameProgressData::ModoJuego::CAMINO_AGRADABLE)
                {
                    game->pushState(std::make_unique<MuerteCentinelaState>(window, game, true));
                }
                else
                {
                    game->pushState(std::make_unique<MuerteCentinelaState>(window, game, false));
                }
            }
        }
        return;
    }

    actualizarCronometro(dt);

    if (m_mensajeTimer > 0.f)
    {
        m_mensajeTimer -= dt;
        if (m_mensajeTimer <= 0.f && m_mensajeText)
        {
            m_mensajeText->setString("");
        }
    }

    // Guardar posición anterior
    sf::Vector2f posAnterior = m_player.getPosition();

    // Movimiento con WASD
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
    m_player.update(dt);

    // Límites de la pantalla
    sf::Vector2f playerPos = m_player.getPosition();
    playerPos.x = std::clamp(playerPos.x, 35.f, 1245.f);
    playerPos.y = std::clamp(playerPos.y, 35.f, 685.f);
    m_player.setPosition(playerPos.x, playerPos.y);

    // Colisiones con paredes
    for (const auto &wall : m_wallBounds)
    {
        if (m_player.getHurtbox().findIntersection(wall).has_value())
        {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }

    verificarSalida();
}

// ============================================================
// DIBUJAR
// ============================================================
void CentinelaConductosState::draw() {
    if (!window) return;
    
    window->setView(window->getDefaultView());
    
    // Fondo
    if (m_backgroundSprite) {
        window->draw(*m_backgroundSprite);
    } else {
        window->draw(m_background);
    }
    
    // ============================================================
    // DEBUG: Dibujar colisiones (BORRAR cuando estén bien)
    // ============================================================
    for (auto& wall : m_walls) {
        sf::RectangleShape debugWall = wall;
        debugWall.setFillColor(sf::Color(255, 0, 0, 80));
        debugWall.setOutlineThickness(2.f);
        debugWall.setOutlineColor(sf::Color::Red);
        window->draw(debugWall);
    }
    
    // Efecto de parpadeo
    if (m_tiempoRestante < 10.f && !m_parpadeoVisible && !m_nivelCompletado) {
        sf::RectangleShape overlay(sf::Vector2f(1280.f, 720.f));
        overlay.setFillColor(sf::Color(255, 0, 0, 30));
        window->draw(overlay);
    }
    
    // Salida
    window->draw(m_exitZone);
    
    if (m_fontLoaded) {
        sf::Text salidaText(m_font, "SALIDA", 14);
        salidaText.setFillColor(sf::Color::Green);
        salidaText.setPosition(sf::Vector2f(1155.f, 600.f));
        window->draw(salidaText);
    }
    
    // Jugador
    m_player.draw(*window);
    
    // Título
    if (m_tituloText) {
        sf::FloatRect bounds = m_tituloText->getLocalBounds();
        m_tituloText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, 0.f));
        m_tituloText->setPosition(sf::Vector2f(640.f, 15.f));
        window->draw(*m_tituloText);
    }
    
    // Cronómetro
    if (m_cronometroText) {
        m_cronometroText->setPosition(sf::Vector2f(20.f, 15.f));
        window->draw(*m_cronometroText);
    }
    
    // Instrucciones
    if (m_instruccionesText) {
        m_instruccionesText->setPosition(sf::Vector2f(20.f, 690.f));
        window->draw(*m_instruccionesText);
    }
    
    // Mensaje temporal
    if (m_mensajeText && m_mensajeTimer > 0.f && !m_mensajeText->getString().isEmpty()) {
        sf::FloatRect bounds = m_mensajeText->getLocalBounds();
        m_mensajeText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_mensajeText->setPosition(sf::Vector2f(640.f, 360.f));
        window->draw(*m_mensajeText);
    }
    
    // Pantalla final
    if (m_nivelCompletado || m_tiempoAgotado) {
        sf::RectangleShape overlay(sf::Vector2f(1280.f, 720.f));
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window->draw(overlay);
        
        if (m_fontLoaded) {
            sf::Text resultadoText(m_font, "", 40);
            resultadoText.setStyle(sf::Text::Bold);
            
            if (m_nivelCompletado) {
                resultadoText.setString("¡ESCAPASTE!\nHas encontrado la salida.");
                resultadoText.setFillColor(sf::Color::Green);
            } else {
                resultadoText.setString("SIN OXIGENO...\nNo lograste escapar.");
                resultadoText.setFillColor(sf::Color::Red);
            }
            
            sf::FloatRect bounds = resultadoText.getLocalBounds();
            resultadoText.setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
            resultadoText.setPosition(sf::Vector2f(640.f, 360.f));
            window->draw(resultadoText);
        }
    }
}