#include "Estados/Niveles/Centinelas/Centinela3/Centinela3State.hpp"
#include "Estados/PauseState.hpp"
#include "Configuracion/CoordenadasDebug.hpp"
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>

// CONSTRUCTOR DEL NIVEL CENTINELA 3
// Inicializa todas las variables, carga recursos y prepara el nivel
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
      m_jugadorDetectado(false),
      m_tiempoDeteccion(0.0f),
      m_tiempoMensaje(0.0f),
      m_dialogoDecisionActivo(false),
      m_opcionSeleccionada(0),
      m_palabraIngresada(""),
      m_letraActual(0),
      m_tiempoHackeo(0.0f),
      m_letrasCayendoOffset(0.0f),
      m_interruptorSeleccionado(0),
      m_tiempoLimite(180.0f),
      m_tiempoAlarma(0.0f),
      m_intensidadAlarma(0.0f),
      m_panelAyudaActivo(true)
{

       // Cargar musica del centinela 3
    std::ifstream archivoMusica("assets/sounds/centinela3.mp3");
    if (archivoMusica.good())
    {
        archivoMusica.close();
        game->cambiarMusica("assets/sounds/centinela3.mp3");
    }
    else
    {
        std::cerr << "Archivo de musica no encontrado: assets/sounds/centinela3.mp3" << std::endl;
    }
    
    // Cargar los recursos del jugador y establecer su posicion inicial
    m_player.loadAssets();
    m_player.setPosition(255.f, 240.f);
    m_player.setSpeed(250.0f);
    
    // Cargar la imagen de fondo del nivel
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
    
    // Configurar la camara que seguira al jugador
    sf::Vector2u windowSize = window->getSize();
    m_camera = sf::View(sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f), sf::Vector2f(1280.f, 720.f));
    
    // Llamar a las funciones que configuran los elementos del nivel
    configurarColisiones();
    configurarCamaras();
    configurarTerminales();
    
    // Cargar la fuente para todos los textos del nivel
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (m_fontLoaded)
    {
        // Texto que aparece cuando el jugador esta cerca de algo interactuable
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setCharacterSize(20);
        m_textoInteraccion->setFillColor(sf::Color::White);
        m_textoInteraccion->setOutlineThickness(1.5f);
        m_textoInteraccion->setOutlineColor(sf::Color::Black);
        
        // Texto para mensajes temporales en pantalla
        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(22);
        m_textoMensaje->setFillColor(sf::Color::Yellow);
        m_textoMensaje->setOutlineThickness(1.5f);
        m_textoMensaje->setOutlineColor(sf::Color::Black);
        m_textoMensaje->setStyle(sf::Text::Bold);
    }
    
    // Lista de palabras en espanol para el minijuego de hackeo
    m_palabrasDisponibles = {
        "SISTEMA", "ACCESO", "HACKEO", "CLAVE", "RED",
        "SERVIDOR", "USUARIO", "ARCHIVO", "DATOS", "SEGURIDAD",
        "PROTOCOLO", "CONEXION", "REMOTO", "ADMIN", "ROOT",
        "TERMINAL", "ENLACE", "PUERTO", "PROXY", "DOMINIO"
    };
    
    game->setIsInLevel(true);
}

// CONFIGURAR COLISIONES DEL MAPA
// Define todas las paredes y obstaculos con los que el jugador puede chocar
void Centinela3State::configurarColisiones()
{
    m_mapaFisico.clear();
    
    // Pared 1: y de 0 a 250, x de 0 a 1670
    m_mapaFisico.emplace_back(0.f, 0.f, 1670.f, 250.f);
    
    // Pared 2: y de 0 a 700, x de 870 a 975
    m_mapaFisico.emplace_back(870.f, 0.f, 105.f, 700.f);
    
    // Pared 3: y de 525 a 700, x de 815 a 1063
    m_mapaFisico.emplace_back(815.f, 525.f, 248.f, 175.f);
    
    // Pared 4: y de 0 a 330, x de 355 a 535
    m_mapaFisico.emplace_back(355.f, 0.f, 180.f, 330.f);
    
    // Pared 5: y de 820 a 930, x de 415 a 585
    m_mapaFisico.emplace_back(415.f, 820.f, 170.f, 110.f);
    
    // Pared 6: y de 785 a 930, x de 716 a 794
    m_mapaFisico.emplace_back(716.f, 785.f, 78.f, 145.f);
    
    // Pared 7: y de 810 a 930, x de 890 a 1000
    m_mapaFisico.emplace_back(890.f, 810.f, 110.f, 120.f);
    
    // Pared 8: y de 840 a 930, x de 1110 a 1162
    m_mapaFisico.emplace_back(1110.f, 840.f, 52.f, 90.f);
    
    // Pared 9: y de 620 a 687, x de 1568 a 1665
    m_mapaFisico.emplace_back(1568.f, 620.f, 97.f, 67.f);
    
    // Pared 10: y de 0 a 333, x de 1150 a 1418
    m_mapaFisico.emplace_back(1150.f, 0.f, 268.f, 333.f);
}

// CONFIGURAR CAMARAS DE VIGILANCIA
// Establece la posicion, direccion, alcance y angulo de cada camara
void Centinela3State::configurarCamaras()
{
    m_camaras.clear();
    
    // CAMARAS BLANCAS - Se desactivan con el primer minijuego
    // Camara blanca 1: posicion (90,260), apunta en diagonal derecha-abajo
    m_camaras.push_back({sf::Vector2f(90.f, 260.f), sf::Vector2f(0.7f, 0.7f), 250.f, 60.f, false, true});
    // Camara blanca 2: posicion (790,290), apunta hacia la izquierda
    m_camaras.push_back({sf::Vector2f(790.f, 290.f), sf::Vector2f(-1.f, 0.f), 250.f, 55.f, false, true});
    
    // CAMARAS ROJAS - Se desactivan con el segundo minijuego
    // Camara roja 1: posicion (1057,376), apunta en diagonal derecha-abajo
    m_camaras.push_back({sf::Vector2f(1057.f, 376.f), sf::Vector2f(0.7f, 0.7f), 280.f, 45.f, true, true});
    // Camara roja 2: posicion (1595,520), apunta hacia la izquierda
    m_camaras.push_back({sf::Vector2f(1595.f, 520.f), sf::Vector2f(-1.f, 0.f), 280.f, 45.f, true, true});
}

// CONFIGURAR TERMINALES Y PUERTA DE SALIDA
// Define las areas donde el jugador puede interactuar con F
void Centinela3State::configurarTerminales()
{
    // Terminal BLANCA - Minijuego 1 de hackeo
    // Area: y de 0 a 365, x de 300 a 600
    m_terminalBlancas.areaInteraccion = sf::FloatRect(sf::Vector2f(300.f, 0.f), sf::Vector2f(300.f, 365.f));
    m_terminalBlancas.completado = false;
    m_terminalBlancas.textoMensaje = "Terminal BLANCA - Presiona F para hackear";
    
    // Terminal ROJA - Minijuego 2 de interruptores
    // Area: y de 810 a 930, x de 1075 a 1190
    m_terminalRojas.areaInteraccion = sf::FloatRect(sf::Vector2f(1075.f, 810.f), sf::Vector2f(115.f, 120.f));
    m_terminalRojas.completado = false;
    m_terminalRojas.textoMensaje = "Terminal ROJA - Presiona F para hackear";

    // Puerta de SALIDA - Se abre automaticamente al completar las camaras rojas
    // Area: y de 0 a 300, x de 1470 a 1630
    m_areaPuerta = sf::FloatRect(sf::Vector2f(1470.f, 0.f), sf::Vector2f(160.f, 300.f));
}

// VERIFICAR SI EL JUGADOR ES DETECTADO POR ALGUNA CAMARA
// Recorre todas las camaras activas y comprueba si el jugador esta dentro de su triangulo de vision
bool Centinela3State::jugadorDetectadoPorCamaras()
{
    sf::Vector2f jugadorPos = m_player.getPosition();
    
    for (const auto& camara : m_camaras) {
        // Si la camara esta apagada, no detecta
        if (!camara.activa) continue;
        // Las camaras rojas solo detectan si las blancas ya fueron desactivadas
        if (camara.esTipoRojo && !m_camarasBlancasDesactivadas) continue;
        
        // Calcular distancia del jugador a la camara
        sf::Vector2f diff = jugadorPos - camara.posicion;
        float distancia = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (distancia > camara.alcance) continue;
        
        // Calcular el angulo entre el jugador y la direccion de la camara
        sf::Vector2f dirNormalizada = diff / distancia;
        float dotProduct = dirNormalizada.x * camara.direccion.x + dirNormalizada.y * camara.direccion.y;
        float angulo = std::acos(dotProduct) * 180.0f / 3.14159f;
        
        // Si el angulo es menor que la mitad de la apertura, el jugador esta dentro del triangulo
        if (angulo <= camara.anguloApertura / 2.0f) return true;
    }
    return false;
}

// DIBUJAR LAS CAMARAS DE VIGILANCIA
// Dibuja el triangulo de vision y el punto de cada camara activa
void Centinela3State::dibujarCamaras()
{
    for (const auto& camara : m_camaras) {
        if (!camara.activa) continue;
        
        // Obtener los 3 vertices del triangulo de vision
        auto triangulo = camara.obtenerTriangulo();
        sf::ConvexShape visionTriangle;
        visionTriangle.setPointCount(3);
        visionTriangle.setPoint(0, triangulo[0]);
        visionTriangle.setPoint(1, triangulo[1]);
        visionTriangle.setPoint(2, triangulo[2]);
        
        // Color segun el tipo de camara
        if (camara.esTipoRojo) {
            visionTriangle.setFillColor(sf::Color(255, 0, 0, 80));
            visionTriangle.setOutlineColor(sf::Color(255, 0, 0, 180));
        } else {
            visionTriangle.setFillColor(sf::Color(255, 255, 255, 80));
            visionTriangle.setOutlineColor(sf::Color(255, 255, 255, 180));
        }
        visionTriangle.setOutlineThickness(2.f);
        window->draw(visionTriangle);
        
        // Dibujar el punto central de la camara
        sf::CircleShape camaraPunto(5.f);
        camaraPunto.setFillColor(camara.esTipoRojo ? sf::Color::Red : sf::Color::White);
        camaraPunto.setOutlineColor(sf::Color::Black);
        camaraPunto.setOutlineThickness(1.f);
        camaraPunto.setOrigin(sf::Vector2f(5.f, 5.f));
        camaraPunto.setPosition(camara.posicion);
        window->draw(camaraPunto);
    }
}

// INICIAR MINIJUEGO 1: HACKEO DE TERMINAL
// El jugador debe adivinar una palabra letra por letra
void Centinela3State::iniciarMinijuegoHackeo()
{
    m_minijuegoActivo = true;
    m_minijuegoActual = 1;
    
    // Elegir una palabra aleatoria de la lista
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, (int)m_palabrasDisponibles.size() - 1);
    m_palabraObjetivo = m_palabrasDisponibles[dis(gen)];
    m_palabraIngresada = std::string(m_palabraObjetivo.size(), ' ');
    m_letraActual = 0;
    m_tiempoHackeo = 25.0f;
    m_letrasCayendoOffset = 0.0f;
    
    // Revelar algunas letras como pistas para ayudar al jugador
    std::uniform_int_distribution<> disPos(0, (int)m_palabraObjetivo.size() - 1);
    int letrasARevelar = std::max(1, (int)m_palabraObjetivo.size() / 3);
    
    for (int i = 0; i < letrasARevelar; i++) {
        int pos = disPos(gen);
        m_palabraIngresada[pos] = m_palabraObjetivo[pos];
    }
    
    // La primera letra siempre se revela como pista
    m_palabraIngresada[0] = m_palabraObjetivo[0];
    
    // Encontrar el primer espacio vacio para empezar a escribir
    m_letraActual = 0;
    while (m_letraActual < (int)m_palabraObjetivo.size() && m_palabraIngresada[m_letraActual] != ' ') {
        m_letraActual++;
    }
}

// ACTUALIZAR MINIJUEGO DE HACKEO
// Reduce el tiempo y controla el efecto de letras cayendo
void Centinela3State::actualizarMinijuegoHackeo(float dt)
{
    if (!m_minijuegoActivo || m_minijuegoActual != 1) return;
    
    m_tiempoHackeo -= dt;
    m_letrasCayendoOffset += dt * 80.f;
    
    // Si se acaba el tiempo, el minijuego falla
    if (m_tiempoHackeo <= 0.0f) {
        m_minijuegoActivo = false;
        m_tiempoMensaje = 2.0f;
        m_textoMensaje->setString("Tiempo agotado! Intenta de nuevo");
    }
}

// DIBUJAR MINIJUEGO DE HACKEO
// Muestra la interfaz de hackeo con la palabra, pistas y tiempo
void Centinela3State::dibujarMinijuegoHackeo()
{
    if (!m_minijuegoActivo || m_minijuegoActual != 1) return;
    
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    
    // Fondo oscuro semitransparente
    sf::RectangleShape overlay(sf::Vector2f(winW, winH));
    overlay.setFillColor(sf::Color(0, 0, 0, 230));
    window->draw(overlay);
    
    // Panel principal del minijuego
    float panelW = 650.f;
    float panelH = 450.f;
    sf::RectangleShape panel(sf::Vector2f(panelW, panelH));
    panel.setFillColor(sf::Color(5, 10, 5, 250));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(0, 255, 0, 150));
    panel.setPosition(sf::Vector2f(winW / 2.f - panelW / 2.f, winH / 2.f - panelH / 2.f));
    window->draw(panel);
    
    // Titulo del minijuego
    sf::Text titulo(m_font);
    titulo.setString("HACKEO DE TERMINAL");
    titulo.setCharacterSize(26);
    titulo.setFillColor(sf::Color::Green);
    titulo.setStyle(sf::Text::Bold);
    sf::FloatRect titBounds = titulo.getLocalBounds();
    titulo.setOrigin(sf::Vector2f(titBounds.size.x / 2.f, 0.f));
    titulo.setPosition(sf::Vector2f(winW / 2.f, winH / 2.f - panelH / 2.f + 15.f));
    window->draw(titulo);
    
    // Linea decorativa
    sf::RectangleShape linea(sf::Vector2f(panelW - 40.f, 1.f));
    linea.setFillColor(sf::Color(0, 255, 0, 100));
    linea.setPosition(sf::Vector2f(winW / 2.f - panelW / 2.f + 20.f, winH / 2.f - panelH / 2.f + 50.f));
    window->draw(linea);
    
    // Efecto de letras cayendo al estilo Matrix en el fondo
    std::string letrasMatrix = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int col = 0; col < 20; col++) {
        float x = winW / 2.f - panelW / 2.f + 30.f + col * 30.f;
        for (int fila = 0; fila < 12; fila++) {
            float y = winH / 2.f - panelH / 2.f + 60.f + fila * 25.f - fmod(m_letrasCayendoOffset + col * 15.f, 300.f);
            if (y > winH / 2.f - panelH / 2.f + 50.f && y < winH / 2.f + panelH / 2.f - 10.f) {
                sf::Text letraMatrix(m_font);
                std::random_device rd;
                letraMatrix.setString(std::string(1, letrasMatrix[rd() % letrasMatrix.size()]));
                letraMatrix.setCharacterSize(12);
                letraMatrix.setFillColor(sf::Color(0, 100, 0, 60));
                letraMatrix.setPosition(sf::Vector2f(x, y));
                window->draw(letraMatrix);
            }
        }
    }
    
    // Recuadro de acceso al sistema
    sf::RectangleShape loginBox(sf::Vector2f(500.f, 220.f));
    loginBox.setFillColor(sf::Color(0, 15, 0, 220));
    loginBox.setOutlineThickness(1.f);
    loginBox.setOutlineColor(sf::Color(0, 200, 0, 120));
    loginBox.setPosition(sf::Vector2f(winW / 2.f - 250.f, winH / 2.f - 90.f));
    window->draw(loginBox);
    
    // Etiqueta de acceso
    sf::Text loginLabel(m_font);
    loginLabel.setString("SYSTEM LOGIN - Completa la palabra:");
    loginLabel.setCharacterSize(14);
    loginLabel.setFillColor(sf::Color(0, 200, 0));
    loginLabel.setPosition(sf::Vector2f(winW / 2.f - 235.f, winH / 2.f - 80.f));
    window->draw(loginLabel);
    
    // Mostrar la palabra con letras reveladas y asteriscos para las ocultas
    std::string palabraMostrada = "> ";
    for (size_t i = 0; i < m_palabraObjetivo.size(); i++) {
        if (m_palabraIngresada[i] != ' ') {
            palabraMostrada += m_palabraIngresada[i];
        } else {
            palabraMostrada += "*";
        }
        palabraMostrada += " ";
    }
    
    sf::Text palabraOcultaText(m_font);
    palabraOcultaText.setString(palabraMostrada);
    palabraOcultaText.setCharacterSize(30);
    palabraOcultaText.setFillColor(sf::Color(0, 255, 0));
    palabraOcultaText.setStyle(sf::Text::Bold);
    palabraOcultaText.setPosition(sf::Vector2f(winW / 2.f - 230.f, winH / 2.f - 35.f));
    window->draw(palabraOcultaText);
    
    // Contador de progreso
    int pistas = 0;
    int total = (int)m_palabraObjetivo.size();
    for (int i = 0; i < total; i++) {
        if (m_palabraIngresada[i] != ' ') pistas++;
    }
    
    sf::Text pistasText(m_font);
    pistasText.setString("Progreso: " + std::to_string(pistas) + "/" + std::to_string(total) + " letras");
    pistasText.setCharacterSize(16);
    pistasText.setFillColor(sf::Color::Cyan);
    pistasText.setPosition(sf::Vector2f(winW / 2.f - 230.f, winH / 2.f + 15.f));
    window->draw(pistasText);
    
    // Instrucciones de teclas
    sf::Text tecladoLabel(m_font);
    tecladoLabel.setString("Presiona las teclas [A-Z] para completar la palabra");
    tecladoLabel.setCharacterSize(12);
    tecladoLabel.setFillColor(sf::Color(0, 150, 0));
    tecladoLabel.setPosition(sf::Vector2f(winW / 2.f - 230.f, winH / 2.f + 45.f));
    window->draw(tecladoLabel);
    
    // Tiempo restante
    sf::Text tiempoText(m_font);
    char buffer[30];
    snprintf(buffer, sizeof(buffer), "TIEMPO: %.1fs", m_tiempoHackeo);
    tiempoText.setString(buffer);
    tiempoText.setCharacterSize(18);
    tiempoText.setFillColor(m_tiempoHackeo < 5.0f ? sf::Color::Red : sf::Color::Green);
    tiempoText.setStyle(sf::Text::Bold);
    tiempoText.setPosition(sf::Vector2f(winW / 2.f - 230.f, winH / 2.f + 75.f));
    window->draw(tiempoText);
    
    // Instrucciones adicionales
    sf::Text instrucciones(m_font);
    instrucciones.setString("Letras verdes = pistas o aciertos\nAsteriscos (*) = letras por adivinar\n[ESC] Cancelar");
    instrucciones.setCharacterSize(12);
    instrucciones.setFillColor(sf::Color(0, 130, 0));
    instrucciones.setPosition(sf::Vector2f(winW / 2.f - 230.f, winH / 2.f + 105.f));
    window->draw(instrucciones);
    
    // Barra de progreso del tiempo
    float barraAncho = (m_tiempoHackeo / 25.0f) * 460.f;
    sf::RectangleShape barraFondo(sf::Vector2f(460.f, 8.f));
    barraFondo.setFillColor(sf::Color(0, 50, 0));
    barraFondo.setPosition(sf::Vector2f(winW / 2.f - 230.f, winH / 2.f - panelH / 2.f + panelH - 30.f));
    window->draw(barraFondo);
    
    sf::RectangleShape barraProgreso(sf::Vector2f(barraAncho, 8.f));
    barraProgreso.setFillColor(m_tiempoHackeo < 5.0f ? sf::Color::Red : sf::Color::Green);
    barraProgreso.setPosition(sf::Vector2f(winW / 2.f - 230.f, winH / 2.f - panelH / 2.f + panelH - 30.f));
    window->draw(barraProgreso);
}

// INICIAR MINIJUEGO 2: PANEL DE INTERRUPTORES
// El jugador debe igualar un patron de interruptores ON/OFF
void Centinela3State::iniciarMinijuegoInterruptores()
{
    m_minijuegoActivo = true;
    m_minijuegoActual = 2;
    
    // Inicializar los 6 interruptores en OFF
    m_interruptores.resize(6, false);
    m_patronObjetivo.resize(6);
    m_interruptorSeleccionado = 0;
    
    // Generar un patron aleatorio de ON/OFF
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    
    for (int i = 0; i < 6; i++) {
        m_patronObjetivo[i] = dis(gen);
    }
}

// ACTUALIZAR MINIJUEGO DE INTERRUPTORES
void Centinela3State::actualizarMinijuegoInterruptores(float dt)
{
    // Este minijuego no necesita logica de tiempo
}

// DIBUJAR MINIJUEGO DE INTERRUPTORES
// Muestra el panel con el patron objetivo y los interruptores del jugador
void Centinela3State::dibujarMinijuegoInterruptores()
{
    if (!m_minijuegoActivo || m_minijuegoActual != 2) return;
    
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    
    // Fondo oscuro
    sf::RectangleShape overlay(sf::Vector2f(winW, winH));
    overlay.setFillColor(sf::Color(0, 0, 0, 230));
    window->draw(overlay);
    
    // Panel principal
    float panelW = 650.f;
    float panelH = 380.f;
    sf::RectangleShape panel(sf::Vector2f(panelW, panelH));
    panel.setFillColor(sf::Color(20, 20, 35, 250));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color::Red);
    panel.setPosition(sf::Vector2f(winW / 2.f - panelW / 2.f, winH / 2.f - panelH / 2.f));
    window->draw(panel);
    
    // Titulo
    sf::Text titulo(m_font);
    titulo.setString("PANEL DE INTERRUPTORES");
    titulo.setCharacterSize(26);
    titulo.setFillColor(sf::Color(255, 100, 100));
    titulo.setStyle(sf::Text::Bold);
    sf::FloatRect titBounds = titulo.getLocalBounds();
    titulo.setOrigin(sf::Vector2f(titBounds.size.x / 2.f, 0.f));
    titulo.setPosition(sf::Vector2f(winW / 2.f, winH / 2.f - panelH / 2.f + 15.f));
    window->draw(titulo);
    
    // Etiqueta del patron objetivo
    sf::Text patronLabel(m_font);
    patronLabel.setString("PATRON OBJETIVO:");
    patronLabel.setCharacterSize(18);
    patronLabel.setFillColor(sf::Color::Yellow);
    patronLabel.setPosition(sf::Vector2f(winW / 2.f - 290.f, winH / 2.f - panelH / 2.f + 60.f));
    window->draw(patronLabel);
    
    float startX = winW / 2.f - 250.f;
    float startY = winH / 2.f - panelH / 2.f + 90.f;
    
    // Dibujar los cuadros del patron objetivo (arriba)
    for (int i = 0; i < 6; i++) {
        sf::RectangleShape cajita(sf::Vector2f(50.f, 40.f));
        cajita.setPosition(sf::Vector2f(startX + i * 85.f, startY));
        cajita.setFillColor(m_patronObjetivo[i] ? sf::Color(0, 200, 0, 200) : sf::Color(80, 80, 80, 200));
        cajita.setOutlineThickness(1.f);
        cajita.setOutlineColor(sf::Color(150, 150, 150));
        window->draw(cajita);
        
        sf::Text cajitaText(m_font);
        cajitaText.setString(m_patronObjetivo[i] ? "ON" : "OFF");
        cajitaText.setCharacterSize(16);
        cajitaText.setFillColor(sf::Color::White);
        sf::FloatRect cajBounds = cajitaText.getLocalBounds();
        cajitaText.setOrigin(sf::Vector2f(cajBounds.size.x / 2.f, cajBounds.size.y / 2.f));
        cajitaText.setPosition(sf::Vector2f(startX + i * 85.f + 25.f, startY + 20.f));
        window->draw(cajitaText);
    }
    
    // Linea separadora
    sf::RectangleShape separador(sf::Vector2f(panelW - 40.f, 2.f));
    separador.setFillColor(sf::Color(100, 100, 100, 150));
    separador.setPosition(sf::Vector2f(winW / 2.f - panelW / 2.f + 20.f, startY + 60.f));
    window->draw(separador);
    
    // Etiqueta de los interruptores del jugador
    sf::Text tusLabel(m_font);
    tusLabel.setString("TUS INTERRUPTORES (A/D para mover, SPACE para cambiar):");
    tusLabel.setCharacterSize(14);
    tusLabel.setFillColor(sf::Color(200, 200, 200));
    tusLabel.setPosition(sf::Vector2f(winW / 2.f - 290.f, startY + 75.f));
    window->draw(tusLabel);
    
    float startY2 = startY + 100.f;
    
    // Dibujar los interruptores del jugador (abajo)
    for (int i = 0; i < 6; i++) {
        sf::RectangleShape cajita(sf::Vector2f(50.f, 40.f));
        cajita.setPosition(sf::Vector2f(startX + i * 85.f, startY2));
        
        // Color segun el estado del interruptor
        if (m_interruptores[i]) {
            cajita.setFillColor(sf::Color(0, 200, 0, 200));
        } else {
            cajita.setFillColor(sf::Color(120, 40, 40, 200));
        }
        
        // Borde amarillo si coincide con el patron
        if (m_interruptores[i] == m_patronObjetivo[i]) {
            cajita.setOutlineThickness(3.f);
            cajita.setOutlineColor(sf::Color::Yellow);
        } else {
            cajita.setOutlineThickness(1.f);
            cajita.setOutlineColor(sf::Color(100, 100, 100));
        }
        
        // Borde cyan para el interruptor seleccionado
        if (i == m_interruptorSeleccionado) {
            cajita.setOutlineThickness(3.f);
            cajita.setOutlineColor(sf::Color::Cyan);
        }
        
        window->draw(cajita);
        
        // Texto ON/OFF dentro de cada cajita
        sf::Text cajitaText(m_font);
        cajitaText.setString(m_interruptores[i] ? "ON" : "OFF");
        cajitaText.setCharacterSize(16);
        cajitaText.setFillColor(sf::Color::White);
        sf::FloatRect cajBounds = cajitaText.getLocalBounds();
        cajitaText.setOrigin(sf::Vector2f(cajBounds.size.x / 2.f, cajBounds.size.y / 2.f));
        cajitaText.setPosition(sf::Vector2f(startX + i * 85.f + 25.f, startY2 + 20.f));
        window->draw(cajitaText);
        
        // Numero debajo de cada interruptor
        sf::Text numText(m_font);
        numText.setString(std::to_string(i + 1));
        numText.setCharacterSize(12);
        numText.setFillColor(sf::Color(150, 150, 150));
        numText.setPosition(sf::Vector2f(startX + i * 85.f + 18.f, startY2 + 45.f));
        window->draw(numText);
    }
    
    // Contador de interruptores correctos
    int correctos = 0;
    for (int i = 0; i < 6; i++) {
        if (m_interruptores[i] == m_patronObjetivo[i]) correctos++;
    }
    
    sf::Text contadorText(m_font);
    contadorText.setString("Correctos: " + std::to_string(correctos) + "/6");
    contadorText.setCharacterSize(18);
    contadorText.setFillColor(correctos == 6 ? sf::Color::Green : sf::Color::Yellow);
    contadorText.setPosition(sf::Vector2f(winW / 2.f - 290.f, startY2 + 75.f));
    window->draw(contadorText);
    
    // Instrucciones de control
    sf::Text instrucciones(m_font);
    instrucciones.setString("[A/D] Mover  [SPACE] Cambiar ON/OFF  [ESC] Cancelar");
    instrucciones.setCharacterSize(14);
    instrucciones.setFillColor(sf::Color(150, 150, 150));
    instrucciones.setPosition(sf::Vector2f(winW / 2.f - 290.f, winH / 2.f + panelH / 2.f - 40.f));
    window->draw(instrucciones);
}

// COMPLETAR UN MINIJUEGO
// Desactiva las camaras correspondientes y avanza el progreso del nivel
void Centinela3State::completarMinijuego()
{
    m_minijuegoActivo = false;
    
    if (m_minijuegoActual == 1) {
        // Desactivar todas las camaras blancas
        m_camarasBlancasDesactivadas = true;
        m_terminalBlancas.completado = true;
        for (auto& camara : m_camaras) {
            if (!camara.esTipoRojo) camara.activa = false;
        }
        m_tiempoMensaje = 3.0f;
        m_textoMensaje->setString("CAMARAS BLANCAS DESACTIVADAS!\nAhora puedes acceder a la zona de camaras rojas");
    } else if (m_minijuegoActual == 2) {
        // Desactivar todas las camaras rojas y abrir la puerta de salida
        m_camarasRojasDesactivadas = true;
        m_terminalRojas.completado = true;
        for (auto& camara : m_camaras) {
            if (camara.esTipoRojo) camara.activa = false;
        }
        m_puertaAbierta = true;
        m_juegoCompletado = true;
        m_tiempoMensaje = 3.0f;
        m_textoMensaje->setString("CAMARAS ROJAS DESACTIVADAS!\nPUERTA ABIERTA! Ve a la salida para escapar");
    }
}

// MOSTRAR DIALOGO DE DECISION FINAL
void Centinela3State::mostrarDialogoDecision()
{
    m_dialogoDecisionActivo = true;
    m_opcionSeleccionada = 0;
    
    m_textoDialogoDecision = std::make_unique<sf::Text>(m_font);
    m_textoDialogoDecision->setString(
        "         === DECISION FINAL ===\n\n"
        "Has logrado desactivar todas las camaras\n"
        "y la puerta de salida esta abierta.\n\n"
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

// MANEJAR LA ENTRADA DEL DIALOGO DE DECISION
void Centinela3State::manejarDecisionInput()
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
    } else { upPressed = false; }
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)) {
        if (!downPressed) {
            downPressed = true;
            m_opcionSeleccionada = 1;
            if (m_textoOpcion1) m_textoOpcion1->setFillColor(sf::Color(150, 150, 150));
            if (m_textoOpcion2) m_textoOpcion2->setFillColor(sf::Color::Magenta);
        }
    } else { downPressed = false; }
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Enter)) {
        if (!enterPressed) {
            enterPressed = true;
            m_dialogoDecisionActivo = false;
            
            LevelTree& levelTree = game->getLevelTree();
            
            if (m_opcionSeleccionada == 0) {
                if (levelTree.jumpToNode("final_bueno_centinela3")) {
                    std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                    if (newState) game->changeState(std::move(newState));
                }
            } else {
                if (levelTree.jumpToNode("final_malo_centinela3")) {
                    std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                    if (newState) game->changeState(std::move(newState));
                }
            }
        }
    } else { enterPressed = false; }
}

// MANEJAR EVENTOS DE TECLADO
// Controla la interaccion del jugador con el nivel
void Centinela3State::handleEvent(const sf::Event& event)
{
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        // F3 activa el modo debug con coordenadas
        if (keyPressed->code == sf::Keyboard::Key::F3) {
            m_debugMode = !m_debugMode;
            CoordenadasDebug::getInstance().setVisible(m_debugMode);
        }
        
        // Tecla M abre o cierra el panel de ayuda
        if (keyPressed->code == sf::Keyboard::Key::M) {
            m_panelAyudaActivo = !m_panelAyudaActivo;
        }
    }
    
    if (!m_activo) return;
    
    if (m_dialogoDecisionActivo) {
        manejarDecisionInput();
        return;
    }
    
    // Si hay un minijuego activo, procesar sus controles
    if (m_minijuegoActivo) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            
            // Controles del minijuego 1: Hackeo de terminal
            if (m_minijuegoActual == 1) {
                char letra = '\0';
                
                // Mapear cada tecla a su letra correspondiente
                switch (keyPressed->code) {
                    case sf::Keyboard::Key::A: letra = 'A'; break;
                    case sf::Keyboard::Key::B: letra = 'B'; break;
                    case sf::Keyboard::Key::C: letra = 'C'; break;
                    case sf::Keyboard::Key::D: letra = 'D'; break;
                    case sf::Keyboard::Key::E: letra = 'E'; break;
                    case sf::Keyboard::Key::F: letra = 'F'; break;
                    case sf::Keyboard::Key::G: letra = 'G'; break;
                    case sf::Keyboard::Key::H: letra = 'H'; break;
                    case sf::Keyboard::Key::I: letra = 'I'; break;
                    case sf::Keyboard::Key::J: letra = 'J'; break;
                    case sf::Keyboard::Key::K: letra = 'K'; break;
                    case sf::Keyboard::Key::L: letra = 'L'; break;
                    case sf::Keyboard::Key::M: letra = 'M'; break;
                    case sf::Keyboard::Key::N: letra = 'N'; break;
                    case sf::Keyboard::Key::O: letra = 'O'; break;
                    case sf::Keyboard::Key::P: letra = 'P'; break;
                    case sf::Keyboard::Key::Q: letra = 'Q'; break;
                    case sf::Keyboard::Key::R: letra = 'R'; break;
                    case sf::Keyboard::Key::S: letra = 'S'; break;
                    case sf::Keyboard::Key::T: letra = 'T'; break;
                    case sf::Keyboard::Key::U: letra = 'U'; break;
                    case sf::Keyboard::Key::V: letra = 'V'; break;
                    case sf::Keyboard::Key::W: letra = 'W'; break;
                    case sf::Keyboard::Key::X: letra = 'X'; break;
                    case sf::Keyboard::Key::Y: letra = 'Y'; break;
                    case sf::Keyboard::Key::Z: letra = 'Z'; break;
                    default: break;
                }
                
                // Si se presiono una letra valida, buscar si esta en la palabra
                if (letra != '\0') {
                    for (int i = 0; i < (int)m_palabraObjetivo.size(); i++) {
                        if (m_palabraObjetivo[i] == letra && m_palabraIngresada[i] == ' ') {
                            m_palabraIngresada[i] = letra;
                        }
                    }
                    
                    // Verificar si ya se completaron todas las letras
                    bool completo = true;
                    for (int i = 0; i < (int)m_palabraObjetivo.size(); i++) {
                        if (m_palabraIngresada[i] == ' ') {
                            completo = false;
                            break;
                        }
                    }
                    
                    if (completo) {
                        completarMinijuego();
                    }
                }
                
                // ESC cancela el minijuego
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    m_minijuegoActivo = false;
                }
            }
            
            // Controles del minijuego 2: Panel de interruptores
            if (m_minijuegoActual == 2) {
                // A o flecha izquierda: mover seleccion a la izquierda
                if (keyPressed->code == sf::Keyboard::Key::A || keyPressed->code == sf::Keyboard::Key::Left) {
                    m_interruptorSeleccionado = (m_interruptorSeleccionado - 1 + 6) % 6;
                }
                // D o flecha derecha: mover seleccion a la derecha
                if (keyPressed->code == sf::Keyboard::Key::D || keyPressed->code == sf::Keyboard::Key::Right) {
                    m_interruptorSeleccionado = (m_interruptorSeleccionado + 1) % 6;
                }
                // Espacio o Enter: cambiar el interruptor seleccionado
                if (keyPressed->code == sf::Keyboard::Key::Space || keyPressed->code == sf::Keyboard::Key::Enter) {
                    m_interruptores[m_interruptorSeleccionado] = !m_interruptores[m_interruptorSeleccionado];
                    
                    // Verificar si todos los interruptores coinciden con el patron
                    bool completo = true;
                    for (int i = 0; i < 6; i++) {
                        if (m_interruptores[i] != m_patronObjetivo[i]) {
                            completo = false;
                            break;
                        }
                    }
                    if (completo) completarMinijuego();
                }
                // ESC cancela el minijuego
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    m_minijuegoActivo = false;
                }
            }
        }
        return;
    }
    
    // Interaccion con terminales y puerta usando la tecla F
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::F) {
            sf::FloatRect playerHurtbox = m_player.getHurtbox();
            
            // Verificar si el jugador esta cerca de la terminal blanca
            if (!m_terminalBlancas.completado && 
                playerHurtbox.findIntersection(m_terminalBlancas.areaInteraccion).has_value()) {
                iniciarMinijuegoHackeo();
                return;
            }
            
            // Verificar si el jugador esta cerca de la terminal roja
            if (m_camarasBlancasDesactivadas && !m_terminalRojas.completado && 
                playerHurtbox.findIntersection(m_terminalRojas.areaInteraccion).has_value()) {
                iniciarMinijuegoInterruptores();
                return;
            }
            
            // Verificar si el jugador esta cerca de la puerta de salida
            if (m_puertaAbierta && 
                playerHurtbox.findIntersection(m_areaPuerta).has_value()) {
                // Ir directo al final bueno
                LevelTree& levelTree = game->getLevelTree();
                if (levelTree.jumpToNode("final_bueno_centinela3")) {
                    std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                    if (newState) game->changeState(std::move(newState));
                }
                return;
            }
        }
    }
}

// ACTUALIZAR LA LOGICA DEL NIVEL
// Se llama cada frame, controla el tiempo, deteccion, movimiento y alarma
void Centinela3State::update(float dt)
{
    if (m_dialogoDecisionActivo) return;
    
    // Actualizar el temporizador de mensajes
    if (m_tiempoMensaje > 0.0f) {
        m_tiempoMensaje -= dt;
        if (m_tiempoMensaje <= 0.0f) m_textoMensaje->setString("");
    }
    
    // Si hay un minijuego activo, actualizarlo
    if (m_minijuegoActivo) {
        if (m_minijuegoActual == 1) actualizarMinijuegoHackeo(dt);
        if (m_minijuegoActual == 2) actualizarMinijuegoInterruptores(dt);
        return;
    }
    
    // Actualizar el cronometro general de 3 minutos
    if (!m_juegoCompletado) {
        m_tiempoLimite -= dt;
        if (m_tiempoLimite <= 0.0f) {
            m_tiempoLimite = 0.0f;
            // Si se acaba el tiempo, el jugador es atrapado
            LevelTree& levelTree = game->getLevelTree();
            if (levelTree.jumpToNode("final_malo_centinela3")) {
                std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                if (newState) game->changeState(std::move(newState));
            }
            return;
        }
    }
    
    // Controlar el efecto de alarma roja en la pantalla
    // La alarma parpadea cada cierto tiempo
    m_tiempoAlarma += dt;
    if (m_tiempoAlarma > 2.0f) {
        m_tiempoAlarma = 0.0f;
        m_intensidadAlarma = 30.0f;  // Activar el parpadeo rojo
    }
    // Reducir la intensidad gradualmente para que se desvanezca
    if (m_intensidadAlarma > 0.0f) {
        m_intensidadAlarma -= dt * 20.0f;
        if (m_intensidadAlarma < 0.0f) m_intensidadAlarma = 0.0f;
    }
    
    // Verificar si el jugador esta siendo detectado por las camaras
    m_jugadorDetectado = jugadorDetectadoPorCamaras();
    
    if (m_jugadorDetectado) {
        m_tiempoDeteccion += dt;
        // Si el jugador permanece detectado por medio segundo, es atrapado
        if (m_tiempoDeteccion >= 0.5f) {
            LevelTree& levelTree = game->getLevelTree();
            if (levelTree.jumpToNode("final_malo_centinela3")) {
                std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                if (newState) game->changeState(std::move(newState));
            }
            return;
        }
    } else {
        m_tiempoDeteccion = 0.0f;
    }
    
    // Movimiento del jugador con WASD o flechas
    sf::Vector2f posAnterior = m_player.getPosition();
    sf::Vector2f movimiento(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) movimiento.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) movimiento.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) movimiento.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) movimiento.x += 1.f;
    
    // Normalizar el movimiento para que sea uniforme en diagonal
    if (movimiento.x != 0.f || movimiento.y != 0.f) {
        float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
        movimiento /= length;
    }
    
    m_player.move(movimiento, dt);
    m_player.update(dt);
    
    // Verificar colisiones con las paredes del mapa
    for (const auto& obj : m_mapaFisico) {
        if (m_player.getHurtbox().findIntersection(obj.getBounds()).has_value()) {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }
    
    // Actualizar la posicion de la camara para que siga al jugador
    sf::Vector2f playerPos = m_player.getPosition();
    float halfWidth = 640.f, halfHeight = 360.f;
    sf::Vector2f cameraPos = playerPos;
    if (cameraPos.x < halfWidth) cameraPos.x = halfWidth;
    if (cameraPos.x > m_worldSize.x - halfWidth) cameraPos.x = m_worldSize.x - halfWidth;
    if (cameraPos.y < halfHeight) cameraPos.y = halfHeight;
    if (cameraPos.y > m_worldSize.y - halfHeight) cameraPos.y = m_worldSize.y - halfHeight;
    m_camera.setCenter(cameraPos);
    
    // Actualizar coordenadas de debug si esta activado
    if (m_debugMode) CoordenadasDebug::getInstance().actualizar(window, m_camera, playerPos);
    
    // Pausa con la tecla Escape
    static bool escapeProcesado = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        if (!escapeProcesado && !m_minijuegoActivo && !m_dialogoDecisionActivo) {
            escapeProcesado = true;
            game->pushState(std::make_unique<PauseState>(window, game));
        }
    } else { escapeProcesado = false; }
}

// DIBUJAR TODO EL NIVEL
// Renderiza el fondo, camaras, jugador, UI, efectos y minijuegos
void Centinela3State::draw()
{
    if (!window) return;
    
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    
    // Dibujar el mundo del juego con la camara del jugador
    window->setView(m_camera);
    
    // Dibujar el fondo
    if (m_background) window->draw(*m_background);
    else {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(30, 30, 50));
        window->draw(fallback);
    }
    
    // Dibujar las camaras de vigilancia
    dibujarCamaras();
    
    // Dibujar al jugador
    m_player.draw(*window);
    
    // Dibujar colisiones en modo debug
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
    
    // Cambiar a la vista de UI (pantalla fija)
    window->setView(window->getDefaultView());
    
    // Efecto de alarma roja que parpadea en la pantalla
    if (m_intensidadAlarma > 0.0f) {
        sf::RectangleShape alarmaOverlay(sf::Vector2f(winW, winH));
        alarmaOverlay.setFillColor(sf::Color(255, 0, 0, (int)m_intensidadAlarma));
        window->draw(alarmaOverlay);
    }
    
    // Barra de estado superior con informacion del nivel
    sf::RectangleShape statusBar(sf::Vector2f(winW, 50.f));
    statusBar.setFillColor(sf::Color(0, 0, 0, 180));
    window->draw(statusBar);
    
    if (m_fontLoaded) {
        // Mostrar estado de las camaras blancas
        sf::Text textoBlancas(m_font);
        textoBlancas.setString(m_camarasBlancasDesactivadas ? "[X] Camaras Blancas: OFF" : "[ ] Camaras Blancas: ON");
        textoBlancas.setCharacterSize(16);
        textoBlancas.setFillColor(m_camarasBlancasDesactivadas ? sf::Color::Green : sf::Color::White);
        textoBlancas.setPosition(sf::Vector2f(1000.f, 10.f));
        window->draw(textoBlancas);
        
        // Mostrar estado de las camaras rojas
        sf::Text textoRojas(m_font);
        textoRojas.setString(m_camarasRojasDesactivadas ? "[X] Camaras Rojas: OFF" : "[ ] Camaras Rojas: ON");
        textoRojas.setCharacterSize(16);
        textoRojas.setFillColor(m_camarasRojasDesactivadas ? sf::Color::Green : sf::Color::Red);
        textoRojas.setPosition(sf::Vector2f(350.f, 10.f));
        window->draw(textoRojas);
        
        // Mostrar estado de la puerta
        sf::Text textoPuerta(m_font);
        textoPuerta.setString(m_puertaAbierta ? "[X] Puerta: ABIERTA" : "[ ] Puerta: CERRADA");
        textoPuerta.setCharacterSize(16);
        textoPuerta.setFillColor(m_puertaAbierta ? sf::Color::Green : sf::Color::Yellow);
        textoPuerta.setPosition(sf::Vector2f(700.f, 10.f));
        window->draw(textoPuerta);
        
        // Mostrar el cronometro de 3 minutos
        int minutos = static_cast<int>(m_tiempoLimite) / 60;
        int segundos = static_cast<int>(m_tiempoLimite) % 60;
        char bufferCrono[20];
        snprintf(bufferCrono, sizeof(bufferCrono), "TIEMPO: %02d:%02d", minutos, segundos);
        sf::Text textoCronometro(m_font);
        textoCronometro.setString(bufferCrono);
        textoCronometro.setCharacterSize(18);
        textoCronometro.setFillColor(m_tiempoLimite < 30.0f ? sf::Color::Red : sf::Color::White);
        textoCronometro.setStyle(sf::Text::Bold);
        textoCronometro.setPosition(sf::Vector2f(20.f, 10.f));
        window->draw(textoCronometro);
        
        // Mostrar ayuda de tecla M
        sf::Text textoAyudaTecla(m_font);
        textoAyudaTecla.setString("[M] Ayuda");
        textoAyudaTecla.setCharacterSize(14);
        textoAyudaTecla.setFillColor(sf::Color(150, 150, 150));
        textoAyudaTecla.setPosition(sf::Vector2f(winW - 120.f, 10.f));
        window->draw(textoAyudaTecla);
        
        // Advertencia cuando el jugador esta siendo detectado
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
    
    // Texto de interaccion contextual (aparece cuando el jugador esta cerca de algo)
    if (m_fontLoaded && m_textoInteraccion && !m_minijuegoActivo && !m_dialogoDecisionActivo && !m_panelAyudaActivo) {
        sf::FloatRect playerHurtbox = m_player.getHurtbox();
        std::string texto;
        
        if (!m_terminalBlancas.completado && playerHurtbox.findIntersection(m_terminalBlancas.areaInteraccion).has_value())
            texto = "TERMINAL BLANCAS - Presiona F para hackear";
        else if (m_camarasBlancasDesactivadas && !m_terminalRojas.completado && playerHurtbox.findIntersection(m_terminalRojas.areaInteraccion).has_value())
            texto = "TERMINAL ROJAS - Presiona F para hackear";
        else if (m_puertaAbierta && playerHurtbox.findIntersection(m_areaPuerta).has_value())
            texto = "SALIDA - Presiona F para escapar";
        
        if (!texto.empty()) {
            m_textoInteraccion->setString(texto);
            sf::FloatRect tb = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(tb.size.x / 2.f, tb.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 80.f));
            window->draw(*m_textoInteraccion);
        }
    }
    
    // Mensaje temporal en pantalla
    if (m_fontLoaded && m_textoMensaje && m_tiempoMensaje > 0.0f && !m_textoMensaje->getString().isEmpty()) {
        sf::FloatRect mb = m_textoMensaje->getLocalBounds();
        m_textoMensaje->setOrigin(sf::Vector2f(mb.size.x / 2.f, mb.size.y / 2.f));
        m_textoMensaje->setPosition(sf::Vector2f(winW / 2.f, winH / 2.f - 100.f));
        window->draw(*m_textoMensaje);
    }
    
    // Dibujar minijuegos si estan activos
    if (m_minijuegoActivo) {
        if (m_minijuegoActual == 1) dibujarMinijuegoHackeo();
        if (m_minijuegoActual == 2) dibujarMinijuegoInterruptores();
    }
    
    // PANEL DE AYUDA INICIAL
    // Se abre con M y se cierra con M
    if (m_panelAyudaActivo && m_fontLoaded && !m_minijuegoActivo && !m_dialogoDecisionActivo) {
        // Fondo oscuro semitransparente
        sf::RectangleShape overlayAyuda(sf::Vector2f(winW, winH));
        overlayAyuda.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlayAyuda);
        
        // Panel de ayuda
        float panelW = 700.f;
        float panelH = 400.f;
        float panelX = winW / 2.f - panelW / 2.f;
        float panelY = winH / 2.f - panelH / 2.f;
        
        sf::RectangleShape panelAyuda(sf::Vector2f(panelW, panelH));
        panelAyuda.setFillColor(sf::Color(20, 20, 40, 250));
        panelAyuda.setOutlineThickness(3.f);
        panelAyuda.setOutlineColor(sf::Color(255, 100, 0));
        panelAyuda.setPosition(sf::Vector2f(panelX, panelY));
        window->draw(panelAyuda);
        
        // Titulo del panel
        sf::Text tituloAyuda(m_font);
        tituloAyuda.setString("ALERTA DE EMERGENCIA");
        tituloAyuda.setCharacterSize(28);
        tituloAyuda.setFillColor(sf::Color::Red);
        tituloAyuda.setStyle(sf::Text::Bold);
        sf::FloatRect titBounds = tituloAyuda.getLocalBounds();
        tituloAyuda.setOrigin(sf::Vector2f(titBounds.size.x / 2.f, 0.f));
        tituloAyuda.setPosition(sf::Vector2f(winW / 2.f, panelY + 20.f));
        window->draw(tituloAyuda);
        
        // Linea separadora
        sf::RectangleShape lineaAyuda(sf::Vector2f(panelW - 40.f, 2.f));
        lineaAyuda.setFillColor(sf::Color(255, 100, 0, 150));
        lineaAyuda.setPosition(sf::Vector2f(panelX + 20.f, panelY + 60.f));
        window->draw(lineaAyuda);
        
        // Texto de ayuda
        sf::Text textoAyuda(m_font);
        textoAyuda.setString(
            "CORRE! Ya estas a nada de escapar.\n\n"
            "TEN CUIDADO CON LAS CAMARAS.\n"
            "Desactivalas para que no te vean.\n\n"
            "Ya estan buscando de donde viene la explosion.\n"
            "Tienes 3 minutos para escapar.\n\n"
            "Desactiva primero las camaras BLANCAS\n"
            "y luego las camaras ROJAS.\n\n"
            "La puerta de salida se abrira sola\n"
            "cuando todas las camaras esten apagadas."
        );
        textoAyuda.setCharacterSize(18);
        textoAyuda.setFillColor(sf::Color(200, 200, 200));
        textoAyuda.setPosition(sf::Vector2f(panelX + 40.f, panelY + 80.f));
        window->draw(textoAyuda);
        
        // Instruccion de cierre
        sf::Text textoCerrarAyuda(m_font);
        textoCerrarAyuda.setString("[M] Cerrar ayuda");
        textoCerrarAyuda.setCharacterSize(16);
        textoCerrarAyuda.setFillColor(sf::Color::Yellow);
        sf::FloatRect cerrarBounds = textoCerrarAyuda.getLocalBounds();
        textoCerrarAyuda.setOrigin(sf::Vector2f(cerrarBounds.size.x / 2.f, 0.f));
        textoCerrarAyuda.setPosition(sf::Vector2f(winW / 2.f, panelY + panelH - 40.f));
        window->draw(textoCerrarAyuda);
    }
    
    // Dialogo de decision final
    if (m_dialogoDecisionActivo && m_fontLoaded) {
        sf::RectangleShape overlay(sf::Vector2f(winW, winH));
        overlay.setFillColor(sf::Color(0, 0, 0, 220));
        window->draw(overlay);
        
        float dw = 650.f, dh = 400.f;
        float dx = winW / 2.f - dw / 2.f, dy = winH / 2.f - dh / 2.f;
        
        sf::RectangleShape dialogBox(sf::Vector2f(dw, dh));
        dialogBox.setFillColor(sf::Color(25, 25, 35, 245));
        dialogBox.setOutlineThickness(3.f);
        dialogBox.setOutlineColor(sf::Color(150, 100, 200, 255));
        dialogBox.setPosition(sf::Vector2f(dx, dy));
        window->draw(dialogBox);
        
        m_textoDialogoDecision->setOrigin(sf::Vector2f(0.f, 0.f));
        m_textoDialogoDecision->setPosition(sf::Vector2f(dx + 50.f, dy + 40.f));
        window->draw(*m_textoDialogoDecision);
        
        if (m_opcionSeleccionada == 0) {
            m_textoOpcion1->setFillColor(sf::Color::Yellow);
            m_textoOpcion1->setString("> ESCAPAR DEL LABORATORIO <");
        } else {
            m_textoOpcion1->setFillColor(sf::Color(150, 150, 150));
            m_textoOpcion1->setString("  ESCAPAR DEL LABORATORIO");
        }
        m_textoOpcion1->setOrigin(sf::Vector2f(0.f, 0.f));
        m_textoOpcion1->setPosition(sf::Vector2f(dx + 80.f, dy + 200.f));
        window->draw(*m_textoOpcion1);
        
        if (m_opcionSeleccionada == 1) {
            m_textoOpcion2->setFillColor(sf::Color::Magenta);
            m_textoOpcion2->setString("> QUEDARSE EN EL LABORATORIO <");
        } else {
            m_textoOpcion2->setFillColor(sf::Color(150, 150, 150));
            m_textoOpcion2->setString("  QUEDARSE EN EL LABORATORIO");
        }
        m_textoOpcion2->setOrigin(sf::Vector2f(0.f, 0.f));
        m_textoOpcion2->setPosition(sf::Vector2f(dx + 80.f, dy + 270.f));
        window->draw(*m_textoOpcion2);
        
        sf::Text instrucciones(m_font);
        instrucciones.setString("FLECHAS [Up][Down]  |  ENTER para seleccionar");
        instrucciones.setCharacterSize(16);
        instrucciones.setFillColor(sf::Color(150, 150, 150));
        sf::FloatRect ib = instrucciones.getLocalBounds();
        instrucciones.setOrigin(sf::Vector2f(ib.size.x / 2.f, 0.f));
        instrucciones.setPosition(sf::Vector2f(winW / 2.f, dy + dh - 50.f));
        window->draw(instrucciones);
    }
    
    // Dibujar coordenadas de debug
    if (m_debugMode) CoordenadasDebug::getInstance().dibujar(*window);
}