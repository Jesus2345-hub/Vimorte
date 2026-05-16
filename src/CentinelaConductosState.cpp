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
    : State(window, game), m_tiempoRestante(45.0f), m_tiempoAgotado(false), m_nivelCompletado(false), m_fontLoaded(false), m_mensajeTimer(0.f), m_parpadeoTimer(0.f), m_parpadeoVisible(true), m_finalCargado(false)
{
    // ============================================================
    // FONDO
    // ============================================================
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel7/conductos_bg.png"))
    {
        m_backgroundSprite = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u texSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(texSize.x), static_cast<float>(texSize.y));
    }
    else
    {
        std::cerr << "Error cargando conductos_bg.png" << std::endl;
        m_worldSize = sf::Vector2f(1537.f, 1023.f);
    }

    // Configurar cámara fija 1280x720
    m_camera = sf::View(sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
                        sf::Vector2f(1280.f, 720.f));

    // ============================================================
    // JUGADOR
    // ============================================================
    m_player.loadAssets();
    m_player.setPosition(264.f, 900.f);
    m_player.setSpeed(200.f);

    // ============================================================
    // CONFIGURAR LABERINTO
    // ============================================================
    configurarLaberinto();
    inicializarHumos();

    // ============================================================
    // SALIDA - Rejilla en esquina superior derecha
    // ============================================================
    if (m_rejillaSalidaTexture.loadFromFile("assets/images/niveles/nivel7/rejilla.png"))
    {
        m_rejillaSalidaSprite = std::make_unique<sf::Sprite>(m_rejillaSalidaTexture);
        m_rejillaSalidaSprite->setScale(sf::Vector2f(0.15f, 0.15f));
        sf::FloatRect bounds = m_rejillaSalidaSprite->getLocalBounds();
        m_rejillaSalidaSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_rejillaSalidaSprite->setPosition(sf::Vector2f(1061.f, 120.f));
    }

    // Área de interacción para salir (fuera de la zona de colisión)
    m_exitBounds = sf::FloatRect(sf::Vector2f(1000.f, 60.f), sf::Vector2f(120.f, 120.f));

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
                                                         "WASD/Flechas: Mover | ESC: Pausa | F3: Debug", 14);
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
        sf::RectangleShape wall(sf::Vector2f(w, h));
        wall.setPosition(sf::Vector2f(x, y));
        wall.setFillColor(sf::Color::Transparent);
        wall.setOutlineThickness(0.f);
        m_walls.push_back(wall);
        m_wallBounds.push_back(wall.getGlobalBounds());
    };

    // Borde superior: (0,0) a (1537,55)
    crearPared(0.f, 0.f, 1537.f, 55.f);

    // Pared superior: (369,55) a (1168,116)
    crearPared(369.f, 55.f, 1168.f, 116.f);

    // Borde inferior: (0,997) a (1537,26)
    crearPared(0.f, 997.f, 1537.f, 26.f);

    // Borde izquierdo
    crearPared(0.f, 0.f, 40.f, 1023.f);

    // Borde derecho
    crearPared(1497.f, 0.f, 40.f, 1023.f);

    // Muro 1: (0,731) a (593,182)
    crearPared(0.f, 731.f, 593.f, 182.f);

    // Muro 2: (0,417) a (141,314)
    crearPared(0.f, 417.f, 141.f, 314.f);

    // Muro central 1: (293,285) a (562,360)
    crearPared(293.f, 285.f, 562.f, 360.f);

    // Muro central 2: (951,260) a (460,385)
    crearPared(951.f, 260.f, 460.f, 385.f);

    // Muro 3: (109,131) a (391,190)
    crearPared(109.f, 131.f, 391.f, 190.f);

    // Muro 4: (673,731) a (827,186)
    crearPared(673.f, 731.f, 827.f, 186.f);

    // Pared izquierda: (191,893) a (48,107)
    crearPared(191.f, 893.f, 48.f, 107.f);

    // Pared derecha: (887,915) a (30,85)
    crearPared(887.f, 915.f, 30.f, 85.f);

    // Pared izquierda de la sala de arriba: (951,171) a (74,89)
    crearPared(951.f, 171.f, 74.f, 89.f);

    std::cout << "✅ Laberinto creado con " << m_wallBounds.size() << " paredes" << std::endl;
}

// ============================================================
// INICIALIZAR HUMOS
// ============================================================
void CentinelaConductosState::inicializarHumos()
{
    m_humos.clear();

    // Posiciones y rotaciones para cada humo
    struct InfoHumo
    {
        sf::Vector2f posicion;
        float rotacion;
    };

    std::vector<InfoHumo> infoHumos = {
        {{503.f, 687.f}, 180.f},        // Hacia abajo
        {{547.f, 687.f}, 180.f},        // Hacia abajo
        {{1069.f, 687.f}, 180.f},       // Hacia abajo
        {{1113.f, 687.f}, 180.f},       // Hacia abajo
        {{1319.f, 687.f}, 180.f},       // Hacia abajo
        {{1363.f, 687.f}, 180.f},       // Hacia abajo
        {{900.f, 591.f}, 90.f},         // Hacia la derecha
        {{900.f, 547.f}, 90.f},         // Hacia la derecha
        {{900.f, 389.f}, 90.f},         // Hacia la derecha
        {{900.f, 345.f}, 90.f},         // Hacia la derecha
        {{1461.f, 515.f + 10.f}, 90.f}, // Hacia la derecha
        {{1461.f, 471.f + 10.f}, 90.f}, // Hacia la derecha
        {{1461.f, 427.f + 10.f}, 90.f}, // Hacia la derecha
        {{1461.f, 383.f + 10.f}, 90.f}, // Hacia la derecha
        {{1186, 257 - 40}, 0.f},
        {{1230, 257 - 40}, 0.f},
    };

    // Tamaño real de las imágenes
    float anchoImagen = 213.f;
    float altoImagen = 443.f;

    // Escala para que el alto sea 86 y el ancho proporcional
    float escala = 86.f / altoImagen;           // 86/443 = 0.194
    float anchoEscalado = anchoImagen * escala; // ~41.3
    float altoEscalado = 86.f;

    for (size_t i = 0; i < infoHumos.size(); i++)
    {
        HumoToxico humo;
        humo.posicion = infoHumos[i].posicion;
        humo.anchoColision = anchoEscalado;
        humo.altoColision = altoEscalado;
        humo.rotacion = infoHumos[i].rotacion;

        for (int f = 0; f < 5; f++)
        {
            sf::Texture tex;
            std::string ruta = "assets/images/niveles/nivel7/humo_" + std::to_string(f) + ".png";
            if (tex.loadFromFile(ruta))
            {
                humo.texturas.push_back(std::move(tex));
            }
        }

        if (!humo.texturas.empty())
        {
            humo.sprite = std::make_unique<sf::Sprite>(humo.texturas[0]);

            for (int j = 0; j < 5; j++)
                humo.escalas[j] = escala;

            humo.sprite->setOrigin(sf::Vector2f(anchoImagen / 2.f, altoImagen / 2.f));
            humo.sprite->setPosition(humo.posicion);
            humo.sprite->setScale(sf::Vector2f(escala, escala));
            humo.sprite->setRotation(sf::degrees(humo.rotacion));
        }

        // Intercambiar ancho/alto si está en horizontal (90° o 270°)
        float anchoHitbox = humo.anchoColision;
        float altoHitbox = humo.altoColision;

        if (humo.rotacion == 90.f || humo.rotacion == 270.f)
        {
            anchoHitbox = humo.altoColision; // Intercambiados
            altoHitbox = humo.anchoColision; // Intercambiados
        }

        humo.areaColision = sf::FloatRect(
            sf::Vector2f(humo.posicion.x - anchoHitbox / 2.f,
                         humo.posicion.y - altoHitbox / 2.f),
            sf::Vector2f(anchoHitbox, altoHitbox));

        humo.cicloTimer = static_cast<float>(i) * 1.2f;

        m_humos.push_back(std::move(humo));
    }

    std::cout << "✅ " << m_humos.size() << " humos inicializados" << std::endl;
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
        if (keyPressed->code == sf::Keyboard::Key::F3)
        {
            m_debugMode = !m_debugMode;
        }
    }
}

// ============================================================
// ACTUALIZAR HUMOS
// ============================================================
void CentinelaConductosState::actualizarHumos(float dt)
{
    float anchoImagen = 213.f;
    float altoImagen = 443.f;

    for (auto &humo : m_humos)
    {
        humo.cicloTimer += dt;

        switch (humo.fase)
        {
        case HumoToxico::Fase::APAGADO:
            humo.frameActual = -1;
            if (humo.cicloTimer >= humo.duracionApagado)
            {
                humo.fase = HumoToxico::Fase::APARECIENDO;
                humo.faseTimer = 0.f;
                humo.frameActual = 0;
                humo.activo = true;
                humo.cicloTimer = 0.f;
            }
            break;

        case HumoToxico::Fase::APARECIENDO:
            humo.faseTimer += dt;
            humo.frameTimer += dt;
            if (humo.frameTimer >= humo.frameDuration)
            {
                humo.frameTimer = 0.f;
                humo.frameActual++;
                if (humo.frameActual > 1)
                {
                    humo.fase = HumoToxico::Fase::ACTIVO;
                    humo.faseTimer = 0.f;
                    humo.frameActual = 2;
                }
            }
            break;

        case HumoToxico::Fase::ACTIVO:
            humo.faseTimer += dt;
            humo.frameTimer += dt;
            if (humo.frameTimer >= humo.frameDuration)
            {
                humo.frameTimer = 0.f;
                if (humo.frameActual == 2)
                    humo.frameActual = 3;
                else
                    humo.frameActual = 2;
            }
            if (humo.faseTimer >= humo.duracionActivo)
            {
                humo.fase = HumoToxico::Fase::DESAPARECIENDO;
                humo.faseTimer = 0.f;
                humo.frameActual = 4;
            }
            break;

        case HumoToxico::Fase::DESAPARECIENDO:
            humo.faseTimer += dt;
            humo.frameTimer += dt;
            if (humo.frameTimer >= humo.frameDuration)
            {
                humo.frameTimer = 0.f;
                if (humo.frameActual == 4)
                {
                    humo.frameActual = 0;
                }
                else if (humo.frameActual == 0)
                {
                    humo.fase = HumoToxico::Fase::APAGADO;
                    humo.activo = false;
                    humo.frameActual = -1;
                    humo.cicloTimer = 0.f;
                }
            }
            break;
        }

        if (humo.frameActual >= 0 && humo.frameActual < static_cast<int>(humo.texturas.size()) && humo.sprite)
        {
            int f = humo.frameActual;
            humo.sprite->setTexture(humo.texturas[f]);
            humo.sprite->setOrigin(sf::Vector2f(anchoImagen / 2.f, altoImagen / 2.f));
            humo.sprite->setScale(sf::Vector2f(humo.escalas[f], humo.escalas[f]));
            humo.sprite->setPosition(humo.posicion);
            humo.sprite->setRotation(sf::degrees(humo.rotacion));
        }
    }
}

// ============================================================
// VERIFICAR COLISIÓN CON HUMOS
// ============================================================
bool CentinelaConductosState::verificarColisionHumos()
{
    for (auto &humo : m_humos)
    {
        if (humo.activo)
        {
            float anchoHitbox = humo.anchoColision;
            float altoHitbox = humo.altoColision;

            if (humo.rotacion == 90.f || humo.rotacion == 270.f)
            {
                anchoHitbox = humo.altoColision;
                altoHitbox = humo.anchoColision;
            }

            humo.areaColision = sf::FloatRect(
                sf::Vector2f(humo.posicion.x - anchoHitbox / 2.f,
                             humo.posicion.y - altoHitbox / 2.f),
                sf::Vector2f(anchoHitbox, altoHitbox));

            if (m_player.getHurtbox().findIntersection(humo.areaColision).has_value())
            {
                return true;
            }
        }
    }
    return false;
}

// ============================================================
// VERIFICAR SALIDA
void CentinelaConductosState::verificarSalida()
{
    if (!m_cercaSalida) return;

    static bool fPresionado = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
        if (!fPresionado) {
            fPresionado = true;
            m_finalCargado = true;
            cargarFinalBueno();
        }
    } else {
        fPresionado = false;
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
    // Si ya se cargó un final, no actualizar nada más
    if (m_finalCargado) {
        return;
    }

    // ===== VERIFICAR MUERTE POR TIEMPO AGOTADO =====
    if (m_tiempoAgotado) {
        m_finalCargado = true;
        cargarFinalMalo();
        return;
    }

    // ===== VERIFICAR COLISIÓN CON HUMO TÓXICO =====
    if (verificarColisionHumos()) {
        m_finalCargado = true;
        cargarFinalMalo();
        return;
    }

    // ===== ACTUALIZAR CRONÓMETRO =====
    actualizarCronometro(dt);

    // ===== ACTUALIZAR HUMOS =====
    actualizarHumos(dt);

    // ===== ACTUALIZAR MENSAJE TEMPORAL =====
    if (m_mensajeTimer > 0.f) {
        m_mensajeTimer -= dt;
        if (m_mensajeTimer <= 0.f && m_mensajeText) {
            m_mensajeText->setString("");
        }
    }

    // ===== MOVIMIENTO DEL JUGADOR =====
    sf::Vector2f posAnterior = m_player.getPosition();

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

    if (movimiento.x != 0.f || movimiento.y != 0.f) {
        float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
        movimiento /= length;
    }

    m_player.move(movimiento, dt);
    m_player.update(dt);

    // ===== COLISIONES CON PAREDES =====
    for (const auto& wall : m_wallBounds) {
        if (m_player.getHurtbox().findIntersection(wall).has_value()) {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }

    // ===== LÍMITES DEL JUGADOR =====
    sf::Vector2f playerPos = m_player.getPosition();
    playerPos.x = std::clamp(playerPos.x, 35.f, m_worldSize.x - 35.f);
    playerPos.y = std::clamp(playerPos.y, 35.f, m_worldSize.y - 35.f);
    m_player.setPosition(playerPos.x, playerPos.y);

    // ===== CÁMARA SIGUE AL JUGADOR =====
    sf::Vector2f cameraPos = m_player.getPosition();
    float halfWidth = 640.f;
    float halfHeight = 360.f;

    if (cameraPos.x < halfWidth)
        cameraPos.x = halfWidth;
    if (cameraPos.x > m_worldSize.x - halfWidth)
        cameraPos.x = m_worldSize.x - halfWidth;
    if (cameraPos.y < halfHeight)
        cameraPos.y = halfHeight;
    if (cameraPos.y > m_worldSize.y - halfHeight)
        cameraPos.y = m_worldSize.y - halfHeight;

    m_camera.setCenter(cameraPos);

    // ===== VERIFICAR SI ESTÁ CERCA DE LA SALIDA =====
    m_cercaSalida = m_player.getHurtbox().findIntersection(m_exitBounds).has_value();

    // ===== VERIFICAR SALIDA (FINAL BUENO) =====
    verificarSalida();

    // ===== PAUSA =====
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

// ============================================================
// DIBUJAR
// ============================================================
void CentinelaConductosState::draw()
{
    if (!window)
        return;

    // ===== FASE 1: MUNDO CON CÁMARA =====
    window->setView(m_camera);

    if (m_backgroundSprite)
    {
        window->draw(*m_backgroundSprite);
    }
    else
    {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(25, 25, 30));
        window->draw(fallback);
    }

    // Colisiones debug (F3)
    if (m_debugMode)
    {
        for (auto &wall : m_walls)
        {
            sf::RectangleShape debugWall = wall;
            debugWall.setFillColor(sf::Color(255, 0, 0, 80));
            debugWall.setOutlineThickness(2.f);
            debugWall.setOutlineColor(sf::Color::Red);
            window->draw(debugWall);
        }

        // Colisiones de humo (naranja)
        for (auto &humo : m_humos)
        {
            if (humo.activo)
            {
                float anchoDebug = humo.anchoColision;
                float altoDebug = humo.altoColision;

                if (humo.rotacion == 90.f || humo.rotacion == 270.f)
                {
                    anchoDebug = humo.altoColision;
                    altoDebug = humo.anchoColision;
                }

                sf::RectangleShape debugHumo(sf::Vector2f(anchoDebug, altoDebug));
                debugHumo.setPosition(sf::Vector2f(humo.posicion.x - anchoDebug / 2.f, humo.posicion.y - altoDebug / 2.f));
                debugHumo.setFillColor(sf::Color(255, 165, 0, 80));
                debugHumo.setOutlineThickness(2.f);
                debugHumo.setOutlineColor(sf::Color(255, 165, 0));
                window->draw(debugHumo);
            }
        }
    }

    // Salida (rejilla)
    if (m_rejillaSalidaSprite)
    {
        window->draw(*m_rejillaSalidaSprite);
    }
    else
    {
        sf::RectangleShape fallbackExit(sf::Vector2f(40.f, 40.f));
        fallbackExit.setFillColor(sf::Color(0, 255, 0, 100));
        fallbackExit.setOutlineThickness(2.f);
        fallbackExit.setOutlineColor(sf::Color::Green);
        fallbackExit.setPosition(sf::Vector2f(1041.f, 104.f));
        window->draw(fallbackExit);
    }

    // Humos tóxicos (debajo del jugador)
    for (auto &humo : m_humos)
    {
        if (humo.frameActual >= 0 && humo.activo && humo.sprite)
        {
            window->draw(*humo.sprite);
        }
    }

    // Jugador (encima de los humos)
    m_player.draw(*window);

    // ===== FASE 2: UI (vista por defecto) =====
    window->setView(window->getDefaultView());

    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);

    // F para salir por el conducto
    if (m_fontLoaded && m_cercaSalida)
    {
        sf::Text salidaText(m_font, "Presiona F para salir por el conducto", 18);
        salidaText.setFillColor(sf::Color::White);
        salidaText.setOutlineThickness(2.f);
        salidaText.setOutlineColor(sf::Color::Black);
        sf::FloatRect bounds = salidaText.getLocalBounds();
        salidaText.setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        salidaText.setPosition(sf::Vector2f(winW / 2.f, winH - 70.f));
        window->draw(salidaText);
    }

    // Título
    if (m_tituloText)
    {
        sf::FloatRect bounds = m_tituloText->getLocalBounds();
        m_tituloText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, 0.f));
        m_tituloText->setPosition(sf::Vector2f(winW / 2.f, 15.f));
        window->draw(*m_tituloText);
    }

    // Cronómetro
    if (m_cronometroText)
    {
        m_cronometroText->setPosition(sf::Vector2f(20.f, 15.f));
        window->draw(*m_cronometroText);
    }

    // Efecto de parpadeo
    if (m_tiempoRestante < 10.f && !m_parpadeoVisible && !m_nivelCompletado)
    {
        sf::RectangleShape overlay(sf::Vector2f(winW, winH));
        overlay.setFillColor(sf::Color(255, 0, 0, 30));
        window->draw(overlay);
    }

    // Mensaje temporal
    if (m_mensajeText && m_mensajeTimer > 0.f && !m_mensajeText->getString().isEmpty())
    {
        sf::FloatRect bounds = m_mensajeText->getLocalBounds();
        m_mensajeText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_mensajeText->setPosition(sf::Vector2f(winW / 2.f, winH / 2.f));
        window->draw(*m_mensajeText);
    }

    // Pantalla final 
    if (m_nivelCompletado || m_tiempoAgotado)
    {
        sf::RectangleShape overlay(sf::Vector2f(winW, winH));
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window->draw(overlay);

        if (m_fontLoaded)
        {
            sf::Text resultadoText(m_font, "", 40);
            resultadoText.setStyle(sf::Text::Bold);

            if (m_nivelCompletado)
            {
                resultadoText.setString("ESCAPASTE\nHas encontrado la salida.");
                resultadoText.setFillColor(sf::Color::Green);
            }
            else
            {
                resultadoText.setString("SIN OXIGENO...\nNo lograste escapar.");
                resultadoText.setFillColor(sf::Color::Red);
            }

            sf::FloatRect bounds = resultadoText.getLocalBounds();
            resultadoText.setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
            resultadoText.setPosition(sf::Vector2f(winW / 2.f, winH / 2.f));
            window->draw(resultadoText);
        }
    }
}

void CentinelaConductosState::cargarFinalBueno()
{
    // Restaurar vista antes de cambiar de estado
    window->setView(window->getDefaultView());
    
    LevelTree& levelTree = game->getLevelTree();
    if (levelTree.jumpToNode("final_bueno_centinela1")) {
        std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
        if (newState) {
            game->changeState(std::move(newState));
        }
    }
}

void CentinelaConductosState::cargarFinalMalo()
{
    // Restaurar vista antes de cambiar de estado
    window->setView(window->getDefaultView());
    
    LevelTree& levelTree = game->getLevelTree();
    if (levelTree.jumpToNode("final_malo_centinela1")) {
        std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
        if (newState) {
            game->changeState(std::move(newState));
        }
    }
}