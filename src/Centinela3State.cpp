#include "Centinela3State.hpp"
#include "PauseState.hpp"
#include "CoordenadasDebug.hpp"
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>

// ============ CONSTRUCTOR ============
Centinela3State::Centinela3State(sf::RenderWindow *window, Game *game)
    : State(window, game),
      m_background(nullptr),
      m_fontLoaded(false),
      m_debugMode(false),
      m_activo(true),
      m_camarasBlancasDesactivadas(false),
      m_camarasRojasDesactivadas(false),
      m_puertaAbierta(false),
      m_juegoCompletado(false),
      m_minijuegoActivo(false),
      m_minijuegoActual(0),
      m_indiceSecuencia(0),
      m_tiempoMostrandoSecuencia(0.0f),
      m_mostrandoSecuencia(true),
      m_tiempoInput(0.0f),
      m_jugadorDetectado(false),
      m_tiempoDeteccion(0.0f),
      m_tiempoMensaje(0.0f),
      m_dialogoDecisionActivo(false),
      m_opcionSeleccionada(0)
{
    // Configurar jugador
    m_player.loadAssets();
    m_player.setPosition(100.f, 100.f);
    m_player.setSpeed(250.0f);
    
    // Cargar fondo
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/centinela3/background.png"))
    {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x), static_cast<float>(textureSize.y));
    }
    else
    {
        std::cerr << "Error: No se pudo cargar background del centinela 3" << std::endl;
        m_worldSize = sf::Vector2f(1280.f, 720.f);
    }
    
    // Configurar cámara
    sf::Vector2u windowSize = window->getSize();
    m_camera = sf::View(sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f), sf::Vector2f(1280.f, 720.f));
    m_lastWindowSize = windowSize;
    
    // Configurar sistemas
    configurarColisiones();
    configurarCamaras();
    configurarTerminales();
    
    // Cargar fuente
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (m_fontLoaded)
    {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setCharacterSize(20);
        m_textoInteraccion->setFillColor(sf::Color::White);
        m_textoInteraccion->setOutlineThickness(1.5f);
        m_textoInteraccion->setOutlineColor(sf::Color::Black);
        
        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(22);
        m_textoMensaje->setFillColor(sf::Color::Yellow);
        m_textoMensaje->setOutlineThickness(1.5f);
        m_textoMensaje->setOutlineColor(sf::Color::Black);
        m_textoMensaje->setStyle(sf::Text::Bold);
    }
    
    game->setIsInLevel(true);
    
    std::cout << "=== CENTINELA 3 ACTIVADO ===" << std::endl;
    std::cout << "Objetivo: Evita las camaras, desactiva las blancas, luego las rojas, y escapa por la puerta" << std::endl;
}

// ============ CONFIGURACIÓN DE COLISIONES ============
void Centinela3State::configurarColisiones()
{
    m_mapaFisico.clear();
    
    // Límites del mapa (paredes exteriores)
    m_mapaFisico.emplace_back(0.f, 0.f, m_worldSize.x, 20.f);
    m_mapaFisico.emplace_back(0.f, m_worldSize.y - 20.f, m_worldSize.x, 20.f);
    m_mapaFisico.emplace_back(0.f, 0.f, 20.f, m_worldSize.y);
    m_mapaFisico.emplace_back(m_worldSize.x - 20.f, 0.f, 20.f, m_worldSize.y);
    
    // Paredes internas - PASILLO PRINCIPAL
    m_mapaFisico.emplace_back(200.f, 200.f, 15.f, 400.f);
    m_mapaFisico.emplace_back(600.f, 100.f, 15.f, 500.f);
    m_mapaFisico.emplace_back(1000.f, 200.f, 15.f, 400.f);
    
    // Paredes horizontales
    m_mapaFisico.emplace_back(200.f, 200.f, 400.f, 15.f);
    m_mapaFisico.emplace_back(600.f, 100.f, 400.f, 15.f);
    m_mapaFisico.emplace_back(200.f, 600.f, 815.f, 15.f);
    
    // Obstáculos adicionales
    m_mapaFisico.emplace_back(350.f, 350.f, 100.f, 100.f);
    m_mapaFisico.emplace_back(750.f, 350.f, 100.f, 100.f);
    
    // Puerta final (pared que bloquea hasta completar)
    m_mapaFisico.emplace_back(1100.f, 400.f, 30.f, 200.f);
}

// ============ CONFIGURACIÓN DE CÁMARAS ============
void Centinela3State::configurarCamaras()
{
    m_camaras.clear();
    
    // ===== CÁMARAS BLANCAS (Primer minijuego) =====
    m_camaras.push_back({
        sf::Vector2f(300.f, 100.f),
        sf::Vector2f(0.f, 1.f),
        250.f,
        60.f,
        false,
        true
    });
    
    m_camaras.push_back({
        sf::Vector2f(500.f, 100.f),
        sf::Vector2f(0.f, 1.f),
        250.f,
        55.f,
        false,
        true
    });
    
    m_camaras.push_back({
        sf::Vector2f(400.f, 350.f),
        sf::Vector2f(1.f, 0.f),
        200.f,
        50.f,
        false,
        true
    });
    
    m_camaras.push_back({
        sf::Vector2f(700.f, 350.f),
        sf::Vector2f(-1.f, 0.f),
        200.f,
        50.f,
        false,
        true
    });
    
    // ===== CÁMARAS ROJAS (Segundo minijuego) =====
    m_camaras.push_back({
        sf::Vector2f(850.f, 200.f),
        sf::Vector2f(0.7f, 0.7f),
        280.f,
        45.f,
        true,
        true
    });
    
    m_camaras.push_back({
        sf::Vector2f(1050.f, 200.f),
        sf::Vector2f(-0.7f, 0.7f),
        280.f,
        45.f,
        true,
        true
    });
    
    m_camaras.push_back({
        sf::Vector2f(950.f, 500.f),
        sf::Vector2f(0.f, -1.f),
        300.f,
        70.f,
        true,
        true
    });
    
    m_camaras.push_back({
        sf::Vector2f(1150.f, 500.f),
        sf::Vector2f(0.f, -1.f),
        300.f,
        70.f,
        true,
        true
    });
}

// ============ CONFIGURACIÓN DE TERMINALES ============
void Centinela3State::configurarTerminales()
{
    m_terminalBlancas.areaInteraccion = sf::FloatRect(sf::Vector2f(550.f, 500.f), sf::Vector2f(60.f, 60.f));
    m_terminalBlancas.completado = false;
    m_terminalBlancas.textoMensaje = "Terminal de desactivacion - Camaras BLANCAS\nPresiona F para hackear";
    
    m_terminalRojas.areaInteraccion = sf::FloatRect(sf::Vector2f(900.f, 500.f), sf::Vector2f(60.f, 60.f));
    m_terminalRojas.completado = false;
    m_terminalRojas.textoMensaje = "Terminal de desactivacion - Camaras ROJAS\nPresiona F para hackear";
    
    m_terminalPuerta.areaInteraccion = sf::FloatRect(sf::Vector2f(1100.f, 300.f), sf::Vector2f(80.f, 80.f));
    m_terminalPuerta.completado = false;
    m_terminalPuerta.textoMensaje = "Puerta de salida\nPresiona F para abrir";
}

// ============ DETECCIÓN DEL JUGADOR ============
bool Centinela3State::jugadorDetectadoPorCamaras()
{
    sf::Vector2f jugadorPos = m_player.getPosition();
    
    for (const auto& camara : m_camaras) {
        if (!camara.activa) continue;
        if (camara.esTipoRojo && !m_camarasBlancasDesactivadas) continue;
        
        sf::Vector2f diff = jugadorPos - camara.posicion;
        float distancia = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        
        if (distancia > camara.alcance) continue;
        
        sf::Vector2f dirNormalizada = diff / distancia;
        float dotProduct = dirNormalizada.x * camara.direccion.x + dirNormalizada.y * camara.direccion.y;
        float angulo = std::acos(dotProduct) * 180.0f / 3.14159f;
        
        if (angulo <= camara.anguloApertura / 2.0f) {
            return true;
        }
    }
    
    return false;
}

// ============ DIBUJAR CÁMARAS ============
void Centinela3State::dibujarCamaras()
{
    for (const auto& camara : m_camaras) {
        if (!camara.activa) continue;
        if (camara.esTipoRojo && !m_camarasBlancasDesactivadas) continue;
        
        auto triangulo = camara.obtenerTriangulo();
        
        sf::ConvexShape visionTriangle;
        visionTriangle.setPointCount(3);
        visionTriangle.setPoint(0, triangulo[0]);
        visionTriangle.setPoint(1, triangulo[1]);
        visionTriangle.setPoint(2, triangulo[2]);
        
        if (camara.esTipoRojo) {
            visionTriangle.setFillColor(sf::Color(255, 0, 0, 80));
            visionTriangle.setOutlineColor(sf::Color(255, 0, 0, 180));
        } else {
            visionTriangle.setFillColor(sf::Color(255, 255, 255, 80));
            visionTriangle.setOutlineColor(sf::Color(255, 255, 255, 180));
        }
        visionTriangle.setOutlineThickness(2.f);
        
        window->draw(visionTriangle);
        
        sf::CircleShape camaraPunto(5.f);
        camaraPunto.setFillColor(camara.esTipoRojo ? sf::Color::Red : sf::Color::White);
        camaraPunto.setOutlineColor(sf::Color::Black);
        camaraPunto.setOutlineThickness(1.f);
        camaraPunto.setOrigin(sf::Vector2f(5.f, 5.f));
        camaraPunto.setPosition(camara.posicion);
        window->draw(camaraPunto);
    }
}

// ============ MINIJUEGOS DE DESACTIVACIÓN (1 y 2) ============
void Centinela3State::iniciarMinijuegoDesactivacion(int tipo)
{
    m_minijuegoActivo = true;
    m_minijuegoActual = tipo;
    m_secuenciaBotones.clear();
    m_indiceSecuencia = 0;
    m_mostrandoSecuencia = true;
    m_tiempoMostrandoSecuencia = 0.0f;
    m_tiempoInput = 0.0f;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disTamano(5, 8);
    std::uniform_int_distribution<> disBoton(0, 3);
    
    int tamano = disTamano(gen);
    for (int i = 0; i < tamano; i++) {
        m_secuenciaBotones.push_back(disBoton(gen));
    }
}

void Centinela3State::actualizarMinijuegoDesactivacion(float dt)
{
    if (!m_minijuegoActivo || m_minijuegoActual == 3) return;
    
    if (m_mostrandoSecuencia) {
        m_tiempoMostrandoSecuencia += dt;
        if (m_tiempoMostrandoSecuencia >= 3.0f) {
            m_mostrandoSecuencia = false;
            m_indiceSecuencia = 0;
        }
        return;
    }
    
    m_tiempoInput += dt;
    if (m_tiempoInput >= 10.0f) {
        m_minijuegoActivo = false;
        m_tiempoMensaje = 2.0f;
        m_textoMensaje->setString("Tiempo agotado! Intenta de nuevo");
    }
}

void Centinela3State::dibujarMinijuegoDesactivacion()
{
    if (!m_minijuegoActivo || (m_minijuegoActual != 1 && m_minijuegoActual != 2)) return;
    
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    
    sf::RectangleShape overlay(sf::Vector2f(winW, winH));
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    window->draw(overlay);
    
    float panelW = 500.f;
    float panelH = 300.f;
    sf::RectangleShape panel(sf::Vector2f(panelW, panelH));
    panel.setFillColor(sf::Color(20, 20, 40, 245));
    panel.setOutlineThickness(3.f);
    panel.setOutlineColor(m_minijuegoActual == 1 ? sf::Color::White : sf::Color::Red);
    panel.setPosition(sf::Vector2f(winW / 2.f - panelW / 2.f, winH / 2.f - panelH / 2.f));
    window->draw(panel);
    
    sf::Text titulo(m_font);
    titulo.setString(m_minijuegoActual == 1 ? "DESACTIVAR CAMARAS BLANCAS" : "DESACTIVAR CAMARAS ROJAS");
    titulo.setCharacterSize(24);
    titulo.setFillColor(m_minijuegoActual == 1 ? sf::Color::White : sf::Color::Red);
    titulo.setStyle(sf::Text::Bold);
    sf::FloatRect titBounds = titulo.getLocalBounds();
    titulo.setOrigin(sf::Vector2f(titBounds.size.x / 2.f, 0.f));
    titulo.setPosition(sf::Vector2f(winW / 2.f, winH / 2.f - panelH / 2.f + 20.f));
    window->draw(titulo);
    
    if (m_mostrandoSecuencia) {
        std::string secuenciaStr = "Secuencia: ";
        for (int boton : m_secuenciaBotones) {
            switch (boton) {
                case 0: secuenciaStr += "W "; break;
                case 1: secuenciaStr += "A "; break;
                case 2: secuenciaStr += "S "; break;
                case 3: secuenciaStr += "D "; break;
            }
        }
        
        sf::Text secuenciaText(m_font);
        secuenciaText.setString(secuenciaStr);
        secuenciaText.setCharacterSize(28);
        secuenciaText.setFillColor(sf::Color::Yellow);
        sf::FloatRect secBounds = secuenciaText.getLocalBounds();
        secuenciaText.setOrigin(sf::Vector2f(secBounds.size.x / 2.f, secBounds.size.y / 2.f));
        secuenciaText.setPosition(sf::Vector2f(winW / 2.f, winH / 2.f));
        window->draw(secuenciaText);
        
        sf::Text memorizaText(m_font);
        memorizaText.setString("MEMORIZA LA SECUENCIA!");
        memorizaText.setCharacterSize(18);
        memorizaText.setFillColor(sf::Color(255, 150, 0));
        sf::FloatRect memBounds = memorizaText.getLocalBounds();
        memorizaText.setOrigin(sf::Vector2f(memBounds.size.x / 2.f, 0.f));
        memorizaText.setPosition(sf::Vector2f(winW / 2.f, winH / 2.f + panelH / 2.f - 50.f));
        window->draw(memorizaText);
    } else {
        std::string progresoStr = "Ingresa la secuencia: [";
        for (int i = 0; i < (int)m_secuenciaBotones.size(); i++) {
            if (i < m_indiceSecuencia) {
                switch (m_secuenciaBotones[i]) {
                    case 0: progresoStr += "W"; break;
                    case 1: progresoStr += "A"; break;
                    case 2: progresoStr += "S"; break;
                    case 3: progresoStr += "D"; break;
                }
            } else {
                progresoStr += "_";
            }
            if (i < (int)m_secuenciaBotones.size() - 1) progresoStr += " ";
        }
        progresoStr += "]";
        
        sf::Text progresoText(m_font);
        progresoText.setString(progresoStr);
        progresoText.setCharacterSize(24);
        progresoText.setFillColor(sf::Color::Green);
        sf::FloatRect progBounds = progresoText.getLocalBounds();
        progresoText.setOrigin(sf::Vector2f(progBounds.size.x / 2.f, progBounds.size.y / 2.f));
        progresoText.setPosition(sf::Vector2f(winW / 2.f, winH / 2.f));
        window->draw(progresoText);
        
        float tiempoRestante = 10.0f - m_tiempoInput;
        sf::Text tiempoText(m_font);
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "Tiempo: %.1f", tiempoRestante);
        tiempoText.setString(buffer);
        tiempoText.setCharacterSize(18);
        tiempoText.setFillColor(tiempoRestante < 3.0f ? sf::Color::Red : sf::Color::White);
        sf::FloatRect timeBounds = tiempoText.getLocalBounds();
        tiempoText.setOrigin(sf::Vector2f(timeBounds.size.x / 2.f, 0.f));
        tiempoText.setPosition(sf::Vector2f(winW / 2.f, winH / 2.f + panelH / 2.f - 50.f));
        window->draw(tiempoText);
    }
}

void Centinela3State::completarMinijuego()
{
    m_minijuegoActivo = false;
    
    if (m_minijuegoActual == 1) {
        m_camarasBlancasDesactivadas = true;
        m_terminalBlancas.completado = true;
        
        for (auto& camara : m_camaras) {
            if (!camara.esTipoRojo) {
                camara.activa = false;
            }
        }
        
        m_tiempoMensaje = 3.0f;
        m_textoMensaje->setString("CAMARAS BLANCAS DESACTIVADAS!\nAhora puedes acceder a la zona de camaras rojas");
    } else if (m_minijuegoActual == 2) {
        m_camarasRojasDesactivadas = true;
        m_terminalRojas.completado = true;
        
        for (auto& camara : m_camaras) {
            if (camara.esTipoRojo) {
                camara.activa = false;
            }
        }
        
        m_tiempoMensaje = 3.0f;
        m_textoMensaje->setString("CAMARAS ROJAS DESACTIVADAS!\nVe a la puerta de salida para escapar");
    }
}

// ============ MINIJUEGO DE LA PUERTA (3) ============
void Centinela3State::iniciarMinijuegoPuerta()
{
    m_minijuegoActivo = true;
    m_minijuegoActual = 3;
    m_codigoIngresado = "";
    m_codigoCorrecto = "7391";
}

void Centinela3State::actualizarMinijuegoPuerta(float dt)
{
    // El minijuego de puerta se maneja con eventos de teclado
}

void Centinela3State::dibujarMinijuegoPuerta()
{
    if (!m_minijuegoActivo || m_minijuegoActual != 3) return;
    
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    
    sf::RectangleShape overlay(sf::Vector2f(winW, winH));
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    window->draw(overlay);
    
    float panelW = 450.f;
    float panelH = 350.f;
    sf::RectangleShape panel(sf::Vector2f(panelW, panelH));
    panel.setFillColor(sf::Color(20, 20, 40, 245));
    panel.setOutlineThickness(3.f);
    panel.setOutlineColor(sf::Color::Green);
    panel.setPosition(sf::Vector2f(winW / 2.f - panelW / 2.f, winH / 2.f - panelH / 2.f));
    window->draw(panel);
    
    sf::Text titulo(m_font);
    titulo.setString("ABRIR PUERTA DE SALIDA");
    titulo.setCharacterSize(24);
    titulo.setFillColor(sf::Color::Green);
    titulo.setStyle(sf::Text::Bold);
    sf::FloatRect titBounds = titulo.getLocalBounds();
    titulo.setOrigin(sf::Vector2f(titBounds.size.x / 2.f, 0.f));
    titulo.setPosition(sf::Vector2f(winW / 2.f, winH / 2.f - panelH / 2.f + 20.f));
    window->draw(titulo);
    
    std::string codigoMostrado = "Codigo: ";
    for (int i = 0; i < 4; i++) {
        if (i < (int)m_codigoIngresado.length()) {
            codigoMostrado += m_codigoIngresado[i];
        } else {
            codigoMostrado += "_";
        }
        codigoMostrado += " ";
    }
    
    sf::Text codigoText(m_font);
    codigoText.setString(codigoMostrado);
    codigoText.setCharacterSize(36);
    codigoText.setFillColor(sf::Color::Yellow);
    sf::FloatRect codBounds = codigoText.getLocalBounds();
    codigoText.setOrigin(sf::Vector2f(codBounds.size.x / 2.f, codBounds.size.y / 2.f));
    codigoText.setPosition(sf::Vector2f(winW / 2.f, winH / 2.f - 30.f));
    window->draw(codigoText);
    
    sf::Text tecladoText(m_font);
    tecladoText.setString("Usa las teclas numericas [1-9] y [0]\nPresiona ENTER para confirmar\nPresiona ESC para cancelar");
    tecladoText.setCharacterSize(18);
    tecladoText.setFillColor(sf::Color::White);
    sf::FloatRect tecBounds = tecladoText.getLocalBounds();
    tecladoText.setOrigin(sf::Vector2f(tecBounds.size.x / 2.f, 0.f));
    tecladoText.setPosition(sf::Vector2f(winW / 2.f, winH / 2.f + 50.f));
    window->draw(tecladoText);
}

// ============ DIÁLOGO DE DECISIÓN FINAL ============
void Centinela3State::mostrarDialogoDecision()
{
    m_dialogoDecisionActivo = true;
    m_opcionSeleccionada = 0;
    
    m_textoDialogoDecision = std::make_unique<sf::Text>(m_font);
    m_textoDialogoDecision->setString(
        "         === DECISION FINAL ===\n\n"
        "Has logrado desactivar todas las camaras\n"
        "y llegaste a la puerta de salida.\n\n"
        "Tienes una decision crucial que tomar:"
    );
    m_textoDialogoDecision->setCharacterSize(20);
    m_textoDialogoDecision->setFillColor(sf::Color::White);
    m_textoDialogoDecision->setOutlineThickness(1.5f);
    m_textoDialogoDecision->setOutlineColor(sf::Color::Black);
    m_textoDialogoDecision->setStyle(sf::Text::Bold);
    
    m_textoOpcion1 = std::make_unique<sf::Text>(m_font);
    m_textoOpcion1->setString("    > ESCAPAR DEL LABORATORIO <");
    m_textoOpcion1->setCharacterSize(24);
    m_textoOpcion1->setFillColor(sf::Color::Yellow);
    m_textoOpcion1->setOutlineThickness(1.5f);
    m_textoOpcion1->setOutlineColor(sf::Color::Black);
    
    m_textoOpcion2 = std::make_unique<sf::Text>(m_font);
    m_textoOpcion2->setString("    > QUEDARSE EN EL LABORATORIO <");
    m_textoOpcion2->setCharacterSize(24);
    m_textoOpcion2->setFillColor(sf::Color(150, 150, 150));
    m_textoOpcion2->setOutlineThickness(1.5f);
    m_textoOpcion2->setOutlineColor(sf::Color::Black);
}

void Centinela3State::handleDecisionInput()
{
    static bool upPressed = false;
    static bool downPressed = false;
    static bool enterPressed = false;
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up)) {
        if (!upPressed) {
            upPressed = true;
            m_opcionSeleccionada = 0;
            if (m_textoOpcion1) m_textoOpcion1->setFillColor(sf::Color::Yellow);
            if (m_textoOpcion2) m_textoOpcion2->setFillColor(sf::Color(150, 150, 150));
        }
    } else {
        upPressed = false;
    }
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)) {
        if (!downPressed) {
            downPressed = true;
            m_opcionSeleccionada = 1;
            if (m_textoOpcion1) m_textoOpcion1->setFillColor(sf::Color(150, 150, 150));
            if (m_textoOpcion2) m_textoOpcion2->setFillColor(sf::Color::Magenta);
        }
    } else {
        downPressed = false;
    }
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Enter)) {
        if (!enterPressed) {
            enterPressed = true;
            m_dialogoDecisionActivo = false;
            
            LevelTree& levelTree = game->getLevelTree();
            
            if (m_opcionSeleccionada == 0) {
                // FINAL BUENO
                if (levelTree.jumpToNode("final_bueno_centinela3")) {
                    std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                    if (newState) {
                        game->changeState(std::move(newState));
                    }
                }
            } else {
                // FINAL MALO
                if (levelTree.jumpToNode("final_malo_centinela3")) {
                    std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                    if (newState) {
                        game->changeState(std::move(newState));
                    }
                }
            }
        }
    } else {
        enterPressed = false;
    }
}

// ============ MANEJO DE EVENTOS ============
void Centinela3State::handleEvent(const sf::Event& event)
{
    // F3 para debug de coordenadas
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::F3) {
            m_debugMode = !m_debugMode;
            CoordenadasDebug::getInstance().setVisible(m_debugMode);
        }
    }
    
    if (!m_activo) return;
    
    // Diálogo de decisión final
    if (m_dialogoDecisionActivo) {
        handleDecisionInput();
        return;
    }
    
    // Minijuego activo
    if (m_minijuegoActivo) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            
            // Minijuegos 1 y 2 (secuencia WASD)
            if (m_minijuegoActual == 1 || m_minijuegoActual == 2) {
                if (m_mostrandoSecuencia) return;
                
                int botonPresionado = -1;
                switch (keyPressed->code) {
                    case sf::Keyboard::Key::W: botonPresionado = 0; break;
                    case sf::Keyboard::Key::A: botonPresionado = 1; break;
                    case sf::Keyboard::Key::S: botonPresionado = 2; break;
                    case sf::Keyboard::Key::D: botonPresionado = 3; break;
                    default: break;
                }
                
                if (botonPresionado != -1) {
                    if (botonPresionado == m_secuenciaBotones[m_indiceSecuencia]) {
                        m_indiceSecuencia++;
                        if (m_indiceSecuencia >= (int)m_secuenciaBotones.size()) {
                            completarMinijuego();
                        }
                    } else {
                        m_minijuegoActivo = false;
                        m_tiempoMensaje = 2.0f;
                        m_textoMensaje->setString("Secuencia incorrecta! Intenta de nuevo");
                    }
                }
                
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    m_minijuegoActivo = false;
                }
            }
            
            // Minijuego 3 (código numérico)
            if (m_minijuegoActual == 3) {
                if (m_codigoIngresado.length() < 4) {
                    switch (keyPressed->code) {
                        case sf::Keyboard::Key::Num0:
                        case sf::Keyboard::Key::Numpad0:
                            m_codigoIngresado += "0"; break;
                        case sf::Keyboard::Key::Num1:
                        case sf::Keyboard::Key::Numpad1:
                            m_codigoIngresado += "1"; break;
                        case sf::Keyboard::Key::Num2:
                        case sf::Keyboard::Key::Numpad2:
                            m_codigoIngresado += "2"; break;
                        case sf::Keyboard::Key::Num3:
                        case sf::Keyboard::Key::Numpad3:
                            m_codigoIngresado += "3"; break;
                        case sf::Keyboard::Key::Num4:
                        case sf::Keyboard::Key::Numpad4:
                            m_codigoIngresado += "4"; break;
                        case sf::Keyboard::Key::Num5:
                        case sf::Keyboard::Key::Numpad5:
                            m_codigoIngresado += "5"; break;
                        case sf::Keyboard::Key::Num6:
                        case sf::Keyboard::Key::Numpad6:
                            m_codigoIngresado += "6"; break;
                        case sf::Keyboard::Key::Num7:
                        case sf::Keyboard::Key::Numpad7:
                            m_codigoIngresado += "7"; break;
                        case sf::Keyboard::Key::Num8:
                        case sf::Keyboard::Key::Numpad8:
                            m_codigoIngresado += "8"; break;
                        case sf::Keyboard::Key::Num9:
                        case sf::Keyboard::Key::Numpad9:
                            m_codigoIngresado += "9"; break;
                        default: break;
                    }
                }
                
                if (keyPressed->code == sf::Keyboard::Key::Enter) {
                    if (m_codigoIngresado == m_codigoCorrecto) {
                        m_minijuegoActivo = false;
                        m_puertaAbierta = true;
                        m_terminalPuerta.completado = true;
                        m_juegoCompletado = true;
                        
                        m_tiempoMensaje = 3.0f;
                        m_textoMensaje->setString("PUERTA ABIERTA!\nPresiona F en la puerta para tomar tu decision final");
                    } else {
                        m_codigoIngresado = "";
                        m_tiempoMensaje = 2.0f;
                        m_textoMensaje->setString("Codigo incorrecto! Intenta de nuevo");
                    }
                }
                
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    m_minijuegoActivo = false;
                    m_codigoIngresado = "";
                }
                
                if (keyPressed->code == sf::Keyboard::Key::Backspace) {
                    if (!m_codigoIngresado.empty()) {
                        m_codigoIngresado.pop_back();
                    }
                }
            }
        }
        return;
    }
    
    // Interacción con terminales y puerta
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::F) {
            sf::Vector2f playerPos = m_player.getPosition();
            sf::FloatRect playerHurtbox = m_player.getHurtbox();
            
            if (!m_terminalBlancas.completado && 
                playerHurtbox.findIntersection(m_terminalBlancas.areaInteraccion).has_value()) {
                iniciarMinijuegoDesactivacion(1);
                return;
            }
            
            if (m_camarasBlancasDesactivadas && !m_terminalRojas.completado && 
                playerHurtbox.findIntersection(m_terminalRojas.areaInteraccion).has_value()) {
                iniciarMinijuegoDesactivacion(2);
                return;
            }
            
            if (m_camarasRojasDesactivadas && !m_puertaAbierta && 
                playerHurtbox.findIntersection(m_terminalPuerta.areaInteraccion).has_value()) {
                iniciarMinijuegoPuerta();
                return;
            }
            
            if (m_puertaAbierta && 
                playerHurtbox.findIntersection(m_terminalPuerta.areaInteraccion).has_value()) {
                mostrarDialogoDecision();
                return;
            }
        }
    }
}

// ============ UPDATE ============
void Centinela3State::update(float dt)
{
    if (m_dialogoDecisionActivo) return;
    
    if (m_tiempoMensaje > 0.0f) {
        m_tiempoMensaje -= dt;
        if (m_tiempoMensaje <= 0.0f) {
            m_textoMensaje->setString("");
        }
    }
    
    if (m_minijuegoActivo) {
        if (m_minijuegoActual == 1 || m_minijuegoActual == 2) {
            actualizarMinijuegoDesactivacion(dt);
        } else if (m_minijuegoActual == 3) {
            actualizarMinijuegoPuerta(dt);
        }
        return;
    }
    
    m_jugadorDetectado = jugadorDetectadoPorCamaras();
    
    if (m_jugadorDetectado) {
        m_tiempoDeteccion += dt;
        
        if (m_tiempoDeteccion >= 0.5f) {
            LevelTree& levelTree = game->getLevelTree();
            if (levelTree.jumpToNode("final_malo_centinela3")) {
                std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                if (newState) {
                    game->changeState(std::move(newState));
                }
            }
            return;
        }
    } else {
        m_tiempoDeteccion = 0.0f;
    }
    
    sf::Vector2f posAnterior = m_player.getPosition();
    
    sf::Vector2f movimiento(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) 
        movimiento.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) 
        movimiento.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) 
        movimiento.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) 
        movimiento.x += 1.f;
    
    if (movimiento.x != 0.f || movimiento.y != 0.f) {
        float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
        movimiento /= length;
    }
    
    m_player.move(movimiento, dt);
    m_player.update(dt);
    
    for (const auto& obj : m_mapaFisico) {
        if (m_player.getHurtbox().findIntersection(obj.getBounds()).has_value()) {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }
    
    sf::Vector2f playerPos = m_player.getPosition();
    float halfWidth = 640.f, halfHeight = 360.f;
    sf::Vector2f cameraPos = playerPos;
    if (cameraPos.x < halfWidth) cameraPos.x = halfWidth;
    if (cameraPos.x > m_worldSize.x - halfWidth) cameraPos.x = m_worldSize.x - halfWidth;
    if (cameraPos.y < halfHeight) cameraPos.y = halfHeight;
    if (cameraPos.y > m_worldSize.y - halfHeight) cameraPos.y = m_worldSize.y - halfHeight;
    m_camera.setCenter(cameraPos);
    
    if (m_debugMode) {
        CoordenadasDebug::getInstance().actualizar(window, m_camera, playerPos);
    }
    
    static bool escapeProcesado = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        if (!escapeProcesado && !m_minijuegoActivo && !m_dialogoDecisionActivo) {
            escapeProcesado = true;
            game->pushState(std::make_unique<PauseState>(window, game));
        }
    } else {
        escapeProcesado = false;
    }
}

// ============ DRAW ============
void Centinela3State::draw()
{
    if (!window) return;
    
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    
    window->setView(m_camera);
    
    if (m_background) {
        window->draw(*m_background);
    } else {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(30, 30, 50));
        window->draw(fallback);
    }
    
    dibujarCamaras();
    
    // Terminal blancas
    {
        sf::RectangleShape terminalBlanca(sf::Vector2f(
            m_terminalBlancas.areaInteraccion.size.x, 
            m_terminalBlancas.areaInteraccion.size.y));
        terminalBlanca.setPosition(m_terminalBlancas.areaInteraccion.position);
        terminalBlanca.setFillColor(sf::Color(255, 255, 255, m_terminalBlancas.completado ? 50 : 150));
        terminalBlanca.setOutlineThickness(2.f);
        terminalBlanca.setOutlineColor(m_terminalBlancas.completado ? sf::Color(100, 100, 100) : sf::Color::White);
        window->draw(terminalBlanca);
    }
    
    // Terminal rojas (visible solo si blancas desactivadas)
    if (m_camarasBlancasDesactivadas) {
        sf::RectangleShape terminalRoja(sf::Vector2f(
            m_terminalRojas.areaInteraccion.size.x, 
            m_terminalRojas.areaInteraccion.size.y));
        terminalRoja.setPosition(m_terminalRojas.areaInteraccion.position);
        terminalRoja.setFillColor(sf::Color(255, 0, 0, m_terminalRojas.completado ? 50 : 150));
        terminalRoja.setOutlineThickness(2.f);
        terminalRoja.setOutlineColor(m_terminalRojas.completado ? sf::Color(100, 100, 100) : sf::Color::Red);
        window->draw(terminalRoja);
    }
    
    // Puerta de salida (visible solo si rojas desactivadas)
    if (m_camarasRojasDesactivadas || m_puertaAbierta) {
        sf::RectangleShape puertaRect(sf::Vector2f(
            m_terminalPuerta.areaInteraccion.size.x, 
            m_terminalPuerta.areaInteraccion.size.y));
        puertaRect.setPosition(m_terminalPuerta.areaInteraccion.position);
        puertaRect.setFillColor(sf::Color(0, 255, 0, m_puertaAbierta ? 180 : 100));
        puertaRect.setOutlineThickness(3.f);
        puertaRect.setOutlineColor(m_puertaAbierta ? sf::Color::Green : sf::Color(0, 150, 0));
        window->draw(puertaRect);
        
        sf::Text exitText(m_font);
        exitText.setString(m_puertaAbierta ? "SALIDA" : "CERRADO");
        exitText.setCharacterSize(16);
        exitText.setFillColor(m_puertaAbierta ? sf::Color::Green : sf::Color::Red);
        exitText.setStyle(sf::Text::Bold);
        sf::FloatRect exitBounds = exitText.getLocalBounds();
        exitText.setOrigin(sf::Vector2f(exitBounds.size.x / 2.f, exitBounds.size.y / 2.f));
        exitText.setPosition(sf::Vector2f(
            m_terminalPuerta.areaInteraccion.position.x + m_terminalPuerta.areaInteraccion.size.x / 2.f,
            m_terminalPuerta.areaInteraccion.position.y - 20.f));
        window->draw(exitText);
    }
    
    m_player.draw(*window);
    
    // Debug: colisiones
    if (m_debugMode) {
        for (const auto& obj : m_mapaFisico) {
            sf::RectangleShape colision;
            colision.setPosition(obj.getBounds().position);
            colision.setSize(obj.getBounds().size);
            colision.setFillColor(sf::Color(255, 0, 0, 100));
            colision.setOutlineThickness(1.f);
            colision.setOutlineColor(sf::Color::Red);
            window->draw(colision);
        }
    }
    
    // Vista de UI
    window->setView(window->getDefaultView());
    
    // Barra de estado superior
    sf::RectangleShape statusBar(sf::Vector2f(winW, 50.f));
    statusBar.setFillColor(sf::Color(0, 0, 0, 180));
    window->draw(statusBar);
    
    if (m_fontLoaded) {
        // Estado cámaras blancas
        sf::Text textoBlancas(m_font);
        textoBlancas.setString(m_camarasBlancasDesactivadas ? 
            "[X] Camaras Blancas: DESACTIVADAS" : 
            "[ ] Camaras Blancas: ACTIVAS");
        textoBlancas.setCharacterSize(16);
        textoBlancas.setFillColor(m_camarasBlancasDesactivadas ? sf::Color::Green : sf::Color::White);
        textoBlancas.setPosition(sf::Vector2f(20.f, 10.f));
        window->draw(textoBlancas);
        
        // Estado cámaras rojas
        sf::Text textoRojas(m_font);
        textoRojas.setString(m_camarasRojasDesactivadas ? 
            "[X] Camaras Rojas: DESACTIVADAS" : 
            "[ ] Camaras Rojas: BLOQUEADAS");
        textoRojas.setCharacterSize(16);
        textoRojas.setFillColor(m_camarasRojasDesactivadas ? sf::Color::Green : sf::Color::Red);
        textoRojas.setPosition(sf::Vector2f(350.f, 10.f));
        window->draw(textoRojas);
        
        // Estado puerta
        sf::Text textoPuerta(m_font);
        textoPuerta.setString(m_puertaAbierta ? 
            "[X] Puerta: ABIERTA" : 
            "[ ] Puerta: CERRADA");
        textoPuerta.setCharacterSize(16);
        textoPuerta.setFillColor(m_puertaAbierta ? sf::Color::Green : sf::Color::Yellow);
        textoPuerta.setPosition(sf::Vector2f(700.f, 10.f));
        window->draw(textoPuerta);
        
        // Advertencia de detección
        if (m_jugadorDetectado) {
            sf::Text textoDeteccion(m_font);
            textoDeteccion.setString("DETECTADO! Alejate de las camaras");
            textoDeteccion.setCharacterSize(18);
            textoDeteccion.setFillColor(sf::Color::Red);
            textoDeteccion.setStyle(sf::Text::Bold);
            sf::FloatRect detBounds = textoDeteccion.getLocalBounds();
            textoDeteccion.setOrigin(sf::Vector2f(detBounds.size.x / 2.f, 0.f));
            textoDeteccion.setPosition(sf::Vector2f(winW / 2.f, winH - 50.f));
            window->draw(textoDeteccion);
        }
    }
    
    // Texto de interacción contextual
    if (m_fontLoaded && m_textoInteraccion && !m_minijuegoActivo && !m_dialogoDecisionActivo) {
        sf::Vector2f playerPos = m_player.getPosition();
        sf::FloatRect playerHurtbox = m_player.getHurtbox();
        
        std::string textoInteraccion;
        
        if (!m_terminalBlancas.completado && 
            playerHurtbox.findIntersection(m_terminalBlancas.areaInteraccion).has_value()) {
            textoInteraccion = "TERMINAL BLANCAS - Presiona F para hackear";
        } else if (m_camarasBlancasDesactivadas && !m_terminalRojas.completado && 
                   playerHurtbox.findIntersection(m_terminalRojas.areaInteraccion).has_value()) {
            textoInteraccion = "TERMINAL ROJAS - Presiona F para hackear";
        } else if (m_camarasRojasDesactivadas && !m_puertaAbierta && 
                   playerHurtbox.findIntersection(m_terminalPuerta.areaInteraccion).has_value()) {
            textoInteraccion = "PUERTA DE SALIDA - Presiona F para abrir";
        } else if (m_puertaAbierta && 
                   playerHurtbox.findIntersection(m_terminalPuerta.areaInteraccion).has_value()) {
            textoInteraccion = "SALIDA - Presiona F para escapar";
        }
        
        if (!textoInteraccion.empty()) {
            m_textoInteraccion->setString(textoInteraccion);
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 80.f));
            window->draw(*m_textoInteraccion);
        }
    }
    
    // Mensaje temporal
    if (m_fontLoaded && m_textoMensaje && m_tiempoMensaje > 0.0f && !m_textoMensaje->getString().isEmpty()) {
        sf::FloatRect msgBounds = m_textoMensaje->getLocalBounds();
        m_textoMensaje->setOrigin(sf::Vector2f(msgBounds.size.x / 2.f, msgBounds.size.y / 2.f));
        m_textoMensaje->setPosition(sf::Vector2f(winW / 2.f, winH / 2.f - 100.f));
        window->draw(*m_textoMensaje);
    }
    
    // Dibujar minijuegos
    if (m_minijuegoActivo) {
        if (m_minijuegoActual == 1 || m_minijuegoActual == 2) {
            dibujarMinijuegoDesactivacion();
        } else if (m_minijuegoActual == 3) {
            dibujarMinijuegoPuerta();
        }
    }
    
    // Diálogo de decisión final
    if (m_dialogoDecisionActivo && m_fontLoaded) {
        sf::RectangleShape overlay(sf::Vector2f(winW, winH));
        overlay.setFillColor(sf::Color(0, 0, 0, 220));
        window->draw(overlay);
        
        float dialogWidth = 650.f;
        float dialogHeight = 400.f;
        float dialogX = winW / 2.f - dialogWidth / 2.f;
        float dialogY = winH / 2.f - dialogHeight / 2.f;
        
        sf::RectangleShape dialogBox(sf::Vector2f(dialogWidth, dialogHeight));
        dialogBox.setFillColor(sf::Color(25, 25, 35, 245));
        dialogBox.setOutlineThickness(3.f);
        dialogBox.setOutlineColor(sf::Color(150, 100, 200, 255));
        dialogBox.setPosition(sf::Vector2f(dialogX, dialogY));
        window->draw(dialogBox);
        
        m_textoDialogoDecision->setOrigin(sf::Vector2f(0.f, 0.f));
        m_textoDialogoDecision->setPosition(sf::Vector2f(dialogX + 50.f, dialogY + 40.f));
        window->draw(*m_textoDialogoDecision);
        
        if (m_opcionSeleccionada == 0) {
            m_textoOpcion1->setFillColor(sf::Color::Yellow);
            m_textoOpcion1->setString("> ESCAPAR DEL LABORATORIO <");
        } else {
            m_textoOpcion1->setFillColor(sf::Color(150, 150, 150));
            m_textoOpcion1->setString("  ESCAPAR DEL LABORATORIO");
        }
        m_textoOpcion1->setOrigin(sf::Vector2f(0.f, 0.f));
        m_textoOpcion1->setPosition(sf::Vector2f(dialogX + 80.f, dialogY + 200.f));
        window->draw(*m_textoOpcion1);
        
        if (m_opcionSeleccionada == 1) {
            m_textoOpcion2->setFillColor(sf::Color::Magenta);
            m_textoOpcion2->setString("> QUEDARSE EN EL LABORATORIO <");
        } else {
            m_textoOpcion2->setFillColor(sf::Color(150, 150, 150));
            m_textoOpcion2->setString("  QUEDARSE EN EL LABORATORIO");
        }
        m_textoOpcion2->setOrigin(sf::Vector2f(0.f, 0.f));
        m_textoOpcion2->setPosition(sf::Vector2f(dialogX + 80.f, dialogY + 270.f));
        window->draw(*m_textoOpcion2);
        
        sf::Text instrucciones(m_font);
        instrucciones.setString("FLECHAS [Up][Down]  |  ENTER para seleccionar");
        instrucciones.setCharacterSize(16);
        instrucciones.setFillColor(sf::Color(150, 150, 150));
        sf::FloatRect instrBounds = instrucciones.getLocalBounds();
        instrucciones.setOrigin(sf::Vector2f(instrBounds.size.x / 2.f, 0.f));
        instrucciones.setPosition(sf::Vector2f(winW / 2.f, dialogY + dialogHeight - 50.f));
        window->draw(instrucciones);
    }
    
    // Debug de coordenadas
    if (m_debugMode) {
        CoordenadasDebug::getInstance().dibujar(*window);
    }
}