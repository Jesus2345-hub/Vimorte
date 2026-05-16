#include "Nivel3State.hpp"
#include "PauseState.hpp"
#include "Game.hpp"
#include "CoordenadasDebug.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <random> 

// ============================================================
// CONSTRUCTOR
// ============================================================
Nivel3State::Nivel3State(sf::RenderWindow* window, Game* game)
    : State(window, game)
    , m_tiempoRestante(180.0f)
    , m_bombaExploto(false)
    , m_bombaDesactivada(false)
    , m_cablesCorrectos(0)
    , m_fontLoaded(false)
{
    m_msjActual.texto = "";
    m_msjActual.tiempoRestante = 0.0f;
    m_msjActual.color = sf::Color::Yellow;
    
    m_player.loadAssets();
    m_player.setPosition(840.f, 320.f);
    m_player.setSpeed(300.0f);
    
    if (game->tienePartidaActiva()) {
        const auto& items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
        auto it = std::find(items.begin(), items.end(), "TutorialNivel3Visto");
        if (it == items.end()) {
            m_mostrarTutorial = true;
            game->getSaveManager().addItemRecolectado("TutorialNivel3Visto");
        }
    }
    
    // Cargar fondo
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel3/background.png")) {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x),
                                    static_cast<float>(textureSize.y));
    } else {
        std::cout << "No se pudo cargar el fondo del nivel 3" << std::endl;
        m_worldSize = sf::Vector2f(2000.f, 1500.f);
        m_background = nullptr;
    }
    
    sf::Vector2u windowSize = window->getSize();
    m_camera = sf::View(
        sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
        sf::Vector2f(1280.f, 720.f)
    );
    m_lastWindowSize = windowSize;
    
    // Generar orden aleatorio
    std::random_device rd;
    std::mt19937 gen(rd());
    m_ordenCorrectoBomba = {0, 1, 2, 3};
    std::shuffle(m_ordenCorrectoBomba.begin(), m_ordenCorrectoBomba.end(), gen);

    std::cout << "[NIVEL 3] Orden correcto: ";
    for (int idx : m_ordenCorrectoBomba) {
        std::cout << nombreColor(idx) << " ";
    }
    std::cout << std::endl;
    
    // Áreas de interacción
    m_bombaArea    = sf::FloatRect(sf::Vector2f(1000.f, 100.f), sf::Vector2f(330.f, 350.f));
    m_pista1Area   = sf::FloatRect(sf::Vector2f(0.f, 0.f), sf::Vector2f(450.f, 450.f));
    m_pista2Area   = sf::FloatRect(sf::Vector2f(0.f, 850.f), sf::Vector2f(369.f, 280.f));
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(775.f, 210.f), sf::Vector2f(145.f, 116.f));
    
    configurarColisiones();
    
    // Tamaño base para minijuegos
    float w = windowSize.x * 0.8f;
    float h = windowSize.y * 0.8f;
    float x = (windowSize.x - w) / 2.f;
    float y = (windowSize.y - h) / 2.f;
    
    // ===== MINIJUEGO DE CABLES (BOMBA - FINAL) =====
    m_minijuegoCables.setSize(sf::Vector2f(w, h));
    m_minijuegoCables.setPosition(sf::Vector2f(x, y));
    
    m_minijuegoCables.setOnComplete([this]() {
        m_bombaDesactivada = true;
        m_textoBomba->setString("✅ BOMBA DESACTIVADA");
        m_textoBomba->setFillColor(sf::Color::Green);
        mostrarMensaje("¡Bomba desactivada! Ve al ascensor.", 4.0f, sf::Color::Green);
    });
    
    m_minijuegoCables.setOnFail([this]() {
        explotarBomba();
    });
    
    // ===== MINIJUEGO DE MEMORIA (PISTA 1) =====
    m_minijuegoMemoria.setSize(sf::Vector2f(w, h));
    m_minijuegoMemoria.setPosition(sf::Vector2f(x, y));
    
    m_minijuegoMemoria.setOnComplete([this]() {
        m_pista1Encontrada = true;
        m_pista1Texto = "Pista 1: " + generarTextoPista(m_ordenCorrectoBomba, 0, 3);
        mostrarMensaje("Pista 1 desbloqueada! Extremos revelados.", 4.0f, sf::Color::Cyan);
    });
    
    // ===== MINIJUEGO DE PATRÓN (PISTA 2) =====
    m_minijuegoPatron.setSize(sf::Vector2f(w, h));
    m_minijuegoPatron.setPosition(sf::Vector2f(x, y));
    
    m_minijuegoPatron.setOnComplete([this]() {
        m_pista2Encontrada = true;
        m_pista2Texto = "Pista 2: " + generarTextoPista(m_ordenCorrectoBomba, 1, 2);
        mostrarMensaje("Pista 2 desbloqueada! Centro revelado.", 4.0f, sf::Color::Green);
    });
    
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (m_fontLoaded) {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setCharacterSize(18);
        m_textoInteraccion->setFillColor(sf::Color::White);
        
        m_textoTiempo = std::make_unique<sf::Text>(m_font);
        m_textoTiempo->setCharacterSize(28);
        m_textoTiempo->setFillColor(sf::Color::White);
        m_textoTiempo->setString("3:00");
        
        m_textoBomba = std::make_unique<sf::Text>(m_font);
        m_textoBomba->setCharacterSize(20);
        m_textoBomba->setFillColor(sf::Color::Red);
        m_textoBomba->setString("BOMBA ACTIVA");
        
        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(20);
        m_textoMensaje->setFillColor(sf::Color::Yellow);
    }
    
    if (game->tienePartidaActiva()) {
        game->getSaveManager().setNivelActual(3, 3);
        game->guardarPartidaActual();
    }
    
        // ===== INICIALIZAR SISTEMA DE EXPLOSION =====
    m_explosionIniciada = false;
    m_grietaAbierta = false;
    m_jugadorMuerto = false;
    m_tiempoMuerte = 0.0f;
    m_debeIrACentinela = false;
    m_tiempoAntesCentinela = 0.0f;
    
    // Posicion de la bomba (centro del area de la bomba)
    m_posicionBomba = sf::Vector2f(1165.f, 275.f);
    
    // Area de muerte: el jugador debe estar en la otra esquina para salvarse
    m_areaExplosion = sf::FloatRect(
        sf::Vector2f(m_posicionBomba.x - 1000.f, m_posicionBomba.y - 400.f),
        sf::Vector2f(1600.f, 1600.f)
    );
    
    // Area donde aparecera la grieta (en la pared superior derecha)
    m_areaGrieta = sf::FloatRect(
        sf::Vector2f(900.f, 50.f),
        sf::Vector2f(250.f, 250.f)
    );

    game->setIsInLevel(true);
    
    mostrarMensaje("DESACTIVA LA BOMBA! Tienes 3 minutos!", 4.0f, sf::Color::Red);
}

// ============================================================
// CONFIGURAR COLISIONES
// ============================================================
void Nivel3State::configurarColisiones() {
    m_mapaFisico.clear();
    
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 0.f), sf::Vector2f(m_worldSize.x, 15.f));
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 0.f), sf::Vector2f(15.f, m_worldSize.y));
    m_mapaFisico.emplace_back(sf::Vector2f(m_worldSize.x - 15.f, 0.f), sf::Vector2f(15.f, m_worldSize.y));
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, m_worldSize.y - 15.f), sf::Vector2f(m_worldSize.x, 15.f));
    
    //borde de pared
    m_mapaFisico.emplace_back(sf::Vector2f(644.f, 0.f), sf::Vector2f(30.f, 841.f));
    m_mapaFisico.emplace_back(sf::Vector2f(570.f, 0.f), sf::Vector2f(80.f, 280.f));
    m_mapaFisico.emplace_back(sf::Vector2f(649.f, 405.f), sf::Vector2f(241.f, 81.f));
    m_mapaFisico.emplace_back(sf::Vector2f(722.f, 410.f), sf::Vector2f(128.f, 144.f));
    m_mapaFisico.emplace_back(sf::Vector2f(728.f, 645.f), sf::Vector2f(10.f, 210.f));
    m_mapaFisico.emplace_back(sf::Vector2f(885.f, 765.f), sf::Vector2f(225.f, 65.f));
    m_mapaFisico.emplace_back(sf::Vector2f(1095.f, 650.f), sf::Vector2f(17.f, 233.f));
      m_mapaFisico.emplace_back(sf::Vector2f(1050.f, 0.f), sf::Vector2f(335.f, 375.f));
        m_mapaFisico.emplace_back(sf::Vector2f(1120.f, 0.f), sf::Vector2f(244.f, 435.f));
        
    m_mapaFisico.emplace_back(sf::Vector2f(735.f, 0.f), sf::Vector2f(41.f, 355.f));
    m_mapaFisico.emplace_back(sf::Vector2f(920.f, 0.f), sf::Vector2f(56.f, 355.f));
    
    //borde de la parte superior izquierda
    m_mapaFisico.emplace_back(sf::Vector2f(644.f, 0.f), sf::Vector2f(740.f, 292.f));
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 0.f), sf::Vector2f(228.f, 34.f));
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 264.f), sf::Vector2f(450.f, 130.f));
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 0.f), sf::Vector2f(225.f, 392.f));
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 0.f), sf::Vector2f(570.f, 219.f));
    
    //borde de abajo a la izquierda
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 738.f), sf::Vector2f(123.f, 374.f));
    m_mapaFisico.emplace_back(sf::Vector2f(0.f, 1015.f), sf::Vector2f(365.f, 107.f));
    m_mapaFisico.emplace_back(sf::Vector2f(260.f, 940.f), sf::Vector2f(105.f, 185.f));
}

// ============================================================
// ACTUALIZAR CRONÓMETRO
// ============================================================
void Nivel3State::actualizarCronometro(float dt) {
    if (m_bombaDesactivada || m_bombaExploto) return;
    
    m_tiempoRestante -= dt;
    
    if (m_tiempoRestante <= 0.0f) {
        m_tiempoRestante = 0.0f;
        explotarBomba();
    }
    
    int minutos = static_cast<int>(m_tiempoRestante) / 60;
    int segundos = static_cast<int>(m_tiempoRestante) % 60;
    
    std::stringstream ss;
    ss << minutos << ":" << std::setfill('0') << std::setw(2) << segundos;
    
    if (m_textoTiempo) {
        m_textoTiempo->setString(ss.str());
        m_textoTiempo->setFillColor(getColorTiempo());
    }
}

// ============================================================
// COLOR DEL CRONÓMETRO
// ============================================================
sf::Color Nivel3State::getColorTiempo() const {
    if (m_tiempoRestante > 120.0f) return sf::Color::White;
    if (m_tiempoRestante > 60.0f)  return sf::Color::Yellow;
    if (m_tiempoRestante > 30.0f)  return sf::Color(255, 165, 0);
    return sf::Color::Red;
}

// ============================================================
// EXPLOTAR BOMBA
// ============================================================
void Nivel3State::explotarBomba() {
    if (m_explosionIniciada) return;
    
    m_bombaExploto = true;
    m_explosionIniciada = true;
    
    std::cout << "BOOM! La bomba ha explotado." << std::endl;
    
    // Iniciar efecto visual de explosion
    m_explosion.iniciar(m_posicionBomba, 1.0f);
    
    // Iniciar grieta en la pared
    m_grieta.iniciar(
        sf::Vector2f(m_areaGrieta.position.x, m_areaGrieta.position.y),
        sf::Vector2f(m_areaGrieta.size.x, m_areaGrieta.size.y)
    );
    
    // Verificar si el jugador esta en el area de muerte
    sf::Vector2f posicionJugador = m_player.getPosition();
    bool jugadorEnAreaExplosion = m_areaExplosion.contains(posicionJugador);
    
    if (jugadorEnAreaExplosion) {
        // JUGADOR MUERE - Estaba demasiado cerca de la bomba
        m_jugadorMuerto = true;
        m_tiempoMuerte = 3.0f;
        
        if (m_textoBomba) {
            m_textoBomba->setString("HAS MUERTO");
            m_textoBomba->setFillColor(sf::Color::Red);
        }
        mostrarMensaje("Estabas demasiado cerca! La explosion te ha matado.", 3.0f, sf::Color::Red);
        std::cout << "Jugador en area de explosion. MUERTE." << std::endl;
    } else {
        // JUGADOR SOBREVIVE - Estaba lejos de la bomba
        m_jugadorMuerto = false;
        
        if (m_textoBomba) {
            m_textoBomba->setString("BOMBA EXPLOTO (Sobreviviste)");
            m_textoBomba->setFillColor(sf::Color(255, 150, 0));
        }
        mostrarMensaje("La bomba exploto! Pero estabas a salvo. Se ha abierto una grieta.", 4.0f, sf::Color::Yellow);
        std::cout << "Jugador fuera de area de explosion. SOBREVIVE. Grieta abierta." << std::endl;
    }
}

// ============================================================
// ACTUALIZAR TAMAÑO DE MINIJUEGOS
// ============================================================
void Nivel3State::actualizarTextosMinijuegos() {
    sf::Vector2u currentSize = window->getSize();
    float w = currentSize.x * 0.8f;
    float h = currentSize.y * 0.8f;
    float x = (currentSize.x - w) / 2.f;
    float y = (currentSize.y - h) / 2.f;
    
    m_minijuegoCables.setSize(sf::Vector2f(w, h));
    m_minijuegoCables.setPosition(sf::Vector2f(x, y));
    m_minijuegoCables.recalcularPosiciones();  // Añadir esta línea
    
    m_minijuegoMemoria.setSize(sf::Vector2f(w, h));
    m_minijuegoMemoria.setPosition(sf::Vector2f(x, y));
    // Si MinigameMemoria tiene recalcularPosiciones, llamarlo también
    
    m_minijuegoPatron.setSize(sf::Vector2f(w, h));
    m_minijuegoPatron.setPosition(sf::Vector2f(x, y));
    // Si MinigamePatron tiene recalcularPosiciones, llamarlo también
}

// ============================================================
// MANEJAR EVENTOS
// ============================================================
void Nivel3State::handleEvent(const sf::Event& event) {
    // ===== MINIJUEGO DE CABLES ACTIVO =====
    if (m_minijuegoCables.isActive()) {
        m_minijuegoCables.handleEvent(event, *window);
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                m_minijuegoCables.deactivate();
                m_escapeConsumed = true;
                return;
            }
        }
        return;
    }
    
    // ===== MINIJUEGO DE MEMORIA ACTIVO =====
    if (m_minijuegoMemoria.isActive()) {
        m_minijuegoMemoria.handleEvent(event, *window);
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                m_minijuegoMemoria.deactivate();
                m_escapeConsumed = true;
                return;
            }
        }
        return;
    }
    
    // ===== MINIJUEGO DE PATRÓN ACTIVO =====
    if (m_minijuegoPatron.isActive()) {
        m_minijuegoPatron.handleEvent(event, *window);
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                m_minijuegoPatron.deactivate();
                m_escapeConsumed = true;
                return;
            }
        }
        return;
    }
    
    // ===== TECLAS GLOBALES =====
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Escape) {
            if (m_mostrarTutorial || m_mostrarTutorialPorTecla) {
                m_mostrarTutorial = false;
                m_mostrarTutorialPorTecla = false;
                m_escapeConsumed = true;
                return;
            }
        }
        
        if (keyPressed->code == sf::Keyboard::Key::M) {
            if (game->tienePartidaActiva()) {
                const auto& items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
                auto it = std::find(items.begin(), items.end(), "TutorialNivel3Visto");
                m_mostrarTutorialPorTecla = (it != items.end());
                if (!m_mostrarTutorialPorTecla) m_mostrarTutorial = true;
            } else {
                m_mostrarTutorialPorTecla = true;
            }
        }
        
        if (keyPressed->code == sf::Keyboard::Key::F3) {
            CoordenadasDebug::getInstance().toggleVisible();
        }
    }
    
    // Eventos del inventario (con tecla E)
    Inventory* inv = m_player.getInventory();
    if (inv) inv->handleEvent(event, *window);
}

// ============================================================
// ACTUALIZAR
// ============================================================
void Nivel3State::update(float dt) {
    // ===== MINIJUEGO DE CABLES ACTIVO =====
    if (m_minijuegoCables.isActive()) {
        actualizarCronometro(dt);
        m_minijuegoCables.update(dt);
        m_player.update(dt);
        sf::Vector2f pos = m_player.getPosition();
        pos.x = std::clamp(pos.x, 640.f, m_worldSize.x - 640.f);
        pos.y = std::clamp(pos.y, 360.f, m_worldSize.y - 360.f);
        m_camera.setCenter(pos);
        CoordenadasDebug::getInstance().actualizar(window, m_camera, m_player.getPosition());
        return;
    }
    
    // ===== MINIJUEGO DE MEMORIA ACTIVO =====
    if (m_minijuegoMemoria.isActive()) {
        actualizarCronometro(dt);
        m_minijuegoMemoria.update(dt);
        m_player.update(dt);
        sf::Vector2f pos = m_player.getPosition();
        pos.x = std::clamp(pos.x, 640.f, m_worldSize.x - 640.f);
        pos.y = std::clamp(pos.y, 360.f, m_worldSize.y - 360.f);
        m_camera.setCenter(pos);
        CoordenadasDebug::getInstance().actualizar(window, m_camera, m_player.getPosition());
        return;
    }
    
    // ===== MINIJUEGO DE PATRON ACTIVO =====
    if (m_minijuegoPatron.isActive()) {
        actualizarCronometro(dt);
        m_minijuegoPatron.update(dt);
        m_player.update(dt);
        sf::Vector2f pos = m_player.getPosition();
        pos.x = std::clamp(pos.x, 640.f, m_worldSize.x - 640.f);
        pos.y = std::clamp(pos.y, 360.f, m_worldSize.y - 360.f);
        m_camera.setCenter(pos);
        CoordenadasDebug::getInstance().actualizar(window, m_camera, m_player.getPosition());
        return;
    }
    
    // Actualizar cronometro
    actualizarCronometro(dt);
    
    // ===== ACTUALIZAR EXPLOSION =====
    if (m_explosionIniciada) {
        m_explosion.actualizar(dt);
        m_grieta.actualizar(dt);
        
        // Aplicar temblor a la camara durante la explosion
        if (m_explosion.estaActivo()) {
            sf::Vector2f temblor = m_explosion.obtenerDesplazamientoTemblor();
            sf::Vector2f posJugador = m_player.getPosition();
            m_camera.setCenter(posJugador + temblor);
        }
        
        // Si el jugador murio, esperar y cargar final malo
        if (m_jugadorMuerto) {
            m_tiempoMuerte -= dt;
            if (m_tiempoMuerte <= 0.0f) {
                window->setView(window->getDefaultView());
                LevelTree& arbolNiveles = game->getLevelTree();
                if (arbolNiveles.jumpToNode("final_malo_centinela3")) {
                    std::unique_ptr<State> nuevoEstado = arbolNiveles.createCurrentState(window, game);
                    if (nuevoEstado) game->changeState(std::move(nuevoEstado));
                }
                return;
            }
        }
        
        // Si la explosion termino y el jugador sobrevivio
        if (m_explosion.haTerminado() && !m_jugadorMuerto) {
            m_grieta.establecerProgresoAnimacion(1.0f);
            m_grietaAbierta = true;
            
            // Activar bandera para ir al centinela
            if (!m_debeIrACentinela) {
                m_debeIrACentinela = true;
                m_tiempoAntesCentinela = 2.0f;
            }
        }
    }
    
    // ===== IR AL CENTINELA DESPUES DE LA PAUSA =====
    if (m_debeIrACentinela) {
        m_tiempoAntesCentinela -= dt;
        if (m_tiempoAntesCentinela <= 0.0f && game) {
            m_debeIrACentinela = false;
            window->setView(window->getDefaultView());
            game->irACentinela();
            return;
        }
    }
    
    sf::Vector2u currentSize = window->getSize();
    if (currentSize != m_lastWindowSize) {
        m_lastWindowSize = currentSize;
        actualizarTextosMinijuegos();
    }

    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f) {
        m_msjActual.tiempoRestante -= dt;
        if (m_msjActual.tiempoRestante <= 0.0f) m_textoMensaje->setString("");
    }
    
    sf::Vector2f posAnterior = m_player.getPosition();
    
    // Verificar cercania a zonas
    m_cercaBomba  = m_player.getHurtbox().findIntersection(m_bombaArea).has_value();
    m_cercaPista1 = m_player.getHurtbox().findIntersection(m_pista1Area).has_value();
    m_cercaPista2 = m_player.getHurtbox().findIntersection(m_pista2Area).has_value();
    m_cercaGrieta = m_grietaAbierta && m_player.getHurtbox().findIntersection(m_areaGrieta).has_value();
    
    // ===== BOMBA (TECLA F) - Minijuego de cables =====
    static bool fBombaPresionado = false;
    if (m_cercaBomba && !m_bombaDesactivada && !m_bombaExploto && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
        if (!fBombaPresionado) {
            fBombaPresionado = true;
            
            if (m_pista1Encontrada) m_minijuegoCables.setPista1(m_pista1Texto);
            if (m_pista2Encontrada) m_minijuegoCables.setPista2(m_pista2Texto);
            m_minijuegoCables.setOrdenCorrecto(m_ordenCorrectoBomba);
            
            m_minijuegoCables.activate();
        }
    } else {
        fBombaPresionado = false;
    }
    
    // ===== PISTA 1 (TECLA F) - Minijuego de memoria =====
    static bool fPista1Presionado = false;
    if (m_cercaPista1 && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
        if (!fPista1Presionado) {
            fPista1Presionado = true;
            if (!m_minijuegoMemoria.isCompleted()) {
                m_minijuegoMemoria.activate();
            }
        }
    } else {
        fPista1Presionado = false;
    }
    
    // ===== PISTA 2 (TECLA F) - Minijuego de patron =====
    static bool fPista2Presionado = false;
    if (m_cercaPista2 && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
        if (!fPista2Presionado) {
            fPista2Presionado = true;
            if (!m_minijuegoPatron.isCompleted()) {
                m_minijuegoPatron.activate();
            }
        }
    } else {
        fPista2Presionado = false;
    }
    
    // Movimiento del jugador
    sf::Vector2f movimiento(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    movimiento.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  movimiento.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  movimiento.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) movimiento.x += 1.f;
    
    if (movimiento.x != 0.f || movimiento.y != 0.f) {
        float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
        movimiento /= length;
    }
    
    m_player.move(movimiento, dt);
    m_player.update(dt);
    
    // Colisiones
    for (const auto& obj : m_mapaFisico) {
        if (m_player.getHurtbox().findIntersection(obj).has_value()) {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }
    
    // Camara
    sf::Vector2f playerPos = m_player.getPosition();
    playerPos.x = std::clamp(playerPos.x, 640.f, m_worldSize.x - 640.f);
    playerPos.y = std::clamp(playerPos.y, 360.f, m_worldSize.y - 360.f);
    m_camera.setCenter(playerPos);
    
    // Salida del nivel (tecla F)
    m_cercaPuertaSalida = m_player.getHurtbox().findIntersection(m_puertaSalidaArea).has_value();
    
    static bool fSalidaPresionado = false;
    if (m_cercaPuertaSalida && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
        if (!fSalidaPresionado) {
            fSalidaPresionado = true;
            
            if (m_bombaDesactivada) {
                std::cout << "Saliendo del nivel 3..." << std::endl;
                game->avanzarNivel();
            } else if (m_bombaExploto && m_jugadorMuerto) {
                mostrarMensaje("Es demasiado tarde...", 3.0f, sf::Color::Red);
            } else if (m_bombaExploto && !m_jugadorMuerto) {
                mostrarMensaje("La grieta te ha abierto un nuevo camino...", 3.0f, sf::Color::Yellow);
            } else {
                mostrarMensaje("Desactiva la bomba primero!", 2.0f, sf::Color::Red);
            }
        }
    } else {
        fSalidaPresionado = false;
    }
    
    // Pausa
    if (!m_mostrarTutorial && !m_mostrarTutorialPorTecla && !m_escapeConsumed) {
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
    
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) m_escapeConsumed = false;
    
    CoordenadasDebug::getInstance().actualizar(window, m_camera, m_player.getPosition());
}

// ============================================================
// DIBUJAR
// ============================================================
void Nivel3State::draw() {
    if (!window) return;
    
    // ===== FASE 1: MUNDO DEL JUEGO =====
    window->setView(m_camera);
    
    if (m_background) window->draw(*m_background);
    else {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(50, 30, 30));
        window->draw(fallback);
    }
    
    // ===== DIBUJAR GRIETA EN EL MUNDO =====
    if (m_grietaAbierta || m_explosionIniciada) {
        m_grieta.dibujar(*window);
    }
    
    // ===== DIBUJAR PARTICULAS DE EXPLOSION =====
    if (m_explosionIniciada && m_explosion.estaActivo()) {
        m_explosion.dibujar(*window);
    }
    
    // Colisiones DEBUG
    if (CoordenadasDebug::getInstance().isVisible()) {
        for (const auto& rect : m_mapaFisico) {
            sf::RectangleShape colision;
            colision.setPosition(rect.position);
            colision.setSize(rect.size);
            colision.setFillColor(sf::Color(255, 0, 0, 100));
            colision.setOutlineColor(sf::Color::Red);
            colision.setOutlineThickness(2.f);
            window->draw(colision);
        }
        
        auto dibujarArea = [&](const sf::FloatRect& area, sf::Color color) {
            sf::RectangleShape rect;
            rect.setPosition(area.position);
            rect.setSize(area.size);
            rect.setFillColor(sf::Color(color.r, color.g, color.b, 50));
            rect.setOutlineColor(color);
            rect.setOutlineThickness(2.f);
            window->draw(rect);
        };
        
        dibujarArea(m_bombaArea, sf::Color(255, 100, 0));
        dibujarArea(m_pista1Area, sf::Color::Cyan);
        dibujarArea(m_pista2Area, sf::Color::Cyan);
        dibujarArea(m_puertaSalidaArea, sf::Color::Green);
        
        // Area de explosion (rojo)
        sf::RectangleShape areaExpRect;
        areaExpRect.setPosition(m_areaExplosion.position);
        areaExpRect.setSize(m_areaExplosion.size);
        areaExpRect.setFillColor(sf::Color(255, 0, 0, 30));
        areaExpRect.setOutlineColor(sf::Color::Red);
        areaExpRect.setOutlineThickness(3.f);
        window->draw(areaExpRect);
        
        // Area de grieta (naranja)
        sf::RectangleShape areaGrietaRect;
        areaGrietaRect.setPosition(m_areaGrieta.position);
        areaGrietaRect.setSize(m_areaGrieta.size);
        areaGrietaRect.setFillColor(sf::Color(255, 100, 0, 30));
        areaGrietaRect.setOutlineColor(sf::Color(255, 100, 0));
        areaGrietaRect.setOutlineThickness(3.f);
        window->draw(areaGrietaRect);
    }
    
    m_player.draw(*window);
    
    // ===== FASE 2: UI =====
    window->setView(window->getDefaultView());
    
    // ===== FLASH Y TEXTO BOOM EN PANTALLA =====
    if (m_explosionIniciada && m_explosion.estaActivo()) {
        float alphaFlash = m_explosion.obtenerAlphaFlash();
        if (alphaFlash > 0.0f) {
            sf::RectangleShape flash(sf::Vector2f(
                static_cast<float>(window->getSize().x),
                static_cast<float>(window->getSize().y)
            ));
            flash.setFillColor(sf::Color(255, 150, 0, static_cast<uint8_t>(alphaFlash)));
            window->draw(flash);
        }
        
        m_explosion.dibujarUI(*window);
    }
    
    CoordenadasDebug::getInstance().dibujar(*window);
    
    if (m_textoTiempo) {
        m_textoTiempo->setPosition(sf::Vector2f(20.f, 20.f));
        window->draw(*m_textoTiempo);
    }
    
    if (m_textoBomba) {
        m_textoBomba->setPosition(sf::Vector2f(20.f, 55.f));
        window->draw(*m_textoBomba);
    }
    
    if (m_fontLoaded && m_textoInteraccion) {
        float winW = static_cast<float>(window->getSize().x);
        float winH = static_cast<float>(window->getSize().y);
        
        auto drawText = [&](const std::string& texto, sf::Color color = sf::Color::White) {
            m_textoInteraccion->setString(texto);
            m_textoInteraccion->setFillColor(color);
            m_textoInteraccion->setOutlineThickness(1.5f);
            m_textoInteraccion->setOutlineColor(sf::Color::Black);
            sf::FloatRect bounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 90.f));
            window->draw(*m_textoInteraccion);
        };
        
        if (m_cercaBomba && !m_bombaDesactivada && !m_bombaExploto)
            drawText("Presiona F para desactivar la bomba", sf::Color::Yellow);
        if (m_cercaBomba && m_bombaDesactivada)
            drawText("Bomba desactivada. Dirigete a la salida.", sf::Color::Green);
        if (m_cercaPista1 && !m_pista1Encontrada)
            drawText("Presiona F para buscar pista", sf::Color::Cyan);
        if (m_cercaPista1 && m_pista1Encontrada)
            drawText("Pista 1 encontrada: " + m_pista1Texto, sf::Color::Cyan);
        if (m_cercaPista2 && !m_pista2Encontrada)
            drawText("Presiona F para buscar pista", sf::Color::Green);
        if (m_cercaPista2 && m_pista2Encontrada)
            drawText("Pista 2 encontrada: " + m_pista2Texto, sf::Color::Green);
        if (m_cercaPuertaSalida && m_bombaDesactivada)
            drawText("Presiona F para salir del edificio", sf::Color::Green);
        if (m_cercaPuertaSalida && !m_bombaDesactivada && !m_bombaExploto)
            drawText("Debes desactivar la bomba antes de salir", sf::Color::Red);
        if (m_grietaAbierta && m_cercaGrieta)
            drawText("Una grieta se ha abierto en la pared por la explosion", sf::Color::Yellow);
    }
    
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f && !m_textoMensaje->getString().isEmpty()) {
        sf::Vector2u winSize = window->getSize();
        sf::FloatRect bounds = m_textoMensaje->getLocalBounds();
        m_textoMensaje->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_textoMensaje->setPosition(sf::Vector2f(winSize.x / 2.f, winSize.y / 3.f));
        window->draw(*m_textoMensaje);
    }
    
    // Minijuegos
    if (m_minijuegoCables.isActive()) {
        m_minijuegoCables.draw(*window);
    }
    if (m_minijuegoMemoria.isActive()) {
        m_minijuegoMemoria.draw(*window);
    }
    if (m_minijuegoPatron.isActive()) {
        m_minijuegoPatron.draw(*window);
    }
    
    Inventory* inv = m_player.getInventory();
    if (inv) inv->draw(*window);
    
    // Tutorial
    if (m_mostrarTutorial || m_mostrarTutorialPorTecla) {
        sf::RectangleShape overlay(sf::Vector2f(window->getSize().x, window->getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);
        
        if (m_fontLoaded) {
            sf::Text tutorialText(m_font);
            tutorialText.setString(
                "ALERTA: BOMBA EN EL EDIFICIO\n\n"
                "Tienes 3 MINUTOS para desactivarla.\n\n"
                "INSTRUCCIONES:\n"
                "- Busca pistas por la habitacion\n"
                "- Acercate a la bomba y presiona F\n"
                "- Corta los cables en el orden correcto\n"
                "- Escapa antes de que explote\n\n"
                "[ESC] Cerrar | [M] Ayuda | [F3] Coordenadas\n"
                "[F] Interactuar | [E] Inventario"
            );
            tutorialText.setCharacterSize(20);
            tutorialText.setFillColor(sf::Color::White);
            sf::FloatRect textBounds = tutorialText.getLocalBounds();
            tutorialText.setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            tutorialText.setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y / 2.f));
            window->draw(tutorialText);
        }
    }
}

// ============================================================
// MOSTRAR MENSAJE TEMPORAL
// ============================================================
void Nivel3State::mostrarMensaje(const std::string& texto, float duracion, sf::Color color) {
    if (!m_textoMensaje) return;
    m_msjActual.texto = texto;
    m_msjActual.tiempoRestante = duracion;
    m_msjActual.color = color;
    m_textoMensaje->setString(texto);
    m_textoMensaje->setFillColor(color);
}

std::string Nivel3State::nombreColor(int indice) const {
    switch (indice) {
        case 0: return "ROJO";
        case 1: return "AZUL";
        case 2: return "VERDE";
        case 3: return "AMARILLO";
        default: return "?";
    }
}

std::string Nivel3State::generarTextoPista(const std::vector<int>& orden,
                                              int revelada1, int revelada2,
                                              int revelada3, int revelada4) const {
    std::string resultado = "";
    
    for (int i = 0; i < 4; i++) {
        if (i > 0) resultado += " -> ";
        
        if (i == revelada1 || i == revelada2 || i == revelada3 || i == revelada4) {
            resultado += nombreColor(orden[i]);
        } else {
            resultado += "??";
        }
    }
    
    return resultado;
}