#include "MinigameQuiz.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

MinigameQuiz::MinigameQuiz()
    : m_isActive(false), m_juegoTerminado(false), m_preguntasRespondidas(0),
      m_respuestasCorrectas(0), m_preguntasPorRonda(5), m_opcionSeleccionada(-1),
      m_mostrandoResultado(false), m_tiempoResultado(0.f), m_gen(m_rd())
{
    // Fondo semi-transparente
    m_background.setFillColor(sf::Color(0, 0, 0, 200));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(100, 100, 100));

    inicializarBancoPreguntas();
}

void MinigameQuiz::setPosition(const sf::Vector2f &pos)
{
    m_position = pos;
    m_background.setPosition(pos);
}

void MinigameQuiz::setSize(const sf::Vector2f &size)
{
    m_size = size;
    m_background.setSize(size);
    configurarUI();
}

void MinigameQuiz::configurarUI() {
    // Cargar textura de la pizarra
    if (m_pizarraTexture.loadFromFile("assets/images/niveles/nivel1/pizarra.png")) {
        m_pizarraSprite = std::make_unique<sf::Sprite>(m_pizarraTexture);
        
        float escala = 1.7f;
        m_pizarraSprite->setScale(sf::Vector2f(escala, escala));
        
        sf::FloatRect bounds = m_pizarraSprite->getLocalBounds();
        m_pizarraSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_pizarraSprite->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f + 230.f,
            m_position.y + m_size.y / 2.f + 30.f
        ));
    }
    
    // Cargar fuente
    if (m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        m_tituloText = std::make_unique<sf::Text>(m_font);
        m_tituloText->setString("LECCION DE MATEMATICAS");
        m_tituloText->setCharacterSize(34);  // MÁS GRANDE
        m_tituloText->setFillColor(sf::Color::White);
        m_tituloText->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f - 200.f,
            m_position.y + 15.f + 40.f
        ));
        
        m_preguntaText = std::make_unique<sf::Text>(m_font);
        m_preguntaText->setCharacterSize(38);  // MÁS GRANDE
        m_preguntaText->setFillColor(sf::Color::White);
        m_preguntaText->setStyle(sf::Text::Bold);
        
        m_puntuacionText = std::make_unique<sf::Text>(m_font);
        m_puntuacionText->setCharacterSize(28);  // MÁS GRANDE
        m_puntuacionText->setFillColor(sf::Color::Yellow);
        m_puntuacionText->setPosition(sf::Vector2f(
            m_position.x + m_size.x - 220.f + 50.f,
            m_position.y + 15.f + 45.f
        ));
        
        m_instruccionesText = std::make_unique<sf::Text>(m_font);
        m_instruccionesText->setString("Haz clic en la respuesta correcta | ESC para salir");
        m_instruccionesText->setCharacterSize(16);
        m_instruccionesText->setFillColor(sf::Color(180, 180, 180));
        m_instruccionesText->setPosition(sf::Vector2f(
            m_position.x + 20.f + 230.f,
            m_position.y + m_size.y - 35.f + 50.f
        ));
        
        m_resultadoText = std::make_unique<sf::Text>(m_font);
        m_resultadoText->setCharacterSize(40);  // MÁS GRANDE
        m_resultadoText->setStyle(sf::Text::Bold);
    }
    
    // Crear textos para las opciones
    m_textosBotones.clear();
    m_botonHover.clear();
    
    for (int i = 0; i < 4; i++) {
        if (m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
            auto texto = std::make_unique<sf::Text>(m_font);
            texto->setCharacterSize(34);
            texto->setFillColor(sf::Color(200, 200, 200));
            m_textosBotones.push_back(std::move(texto));
        }
        m_botonHover.push_back(false);
    }
}

void MinigameQuiz::inicializarBancoPreguntas() {
    m_bancoPreguntas = {
        {"Cuanto es 7 x 8?", {"48", "56", "64", "72"}, 1},
        {"Cual es la raiz cuadrada de 144?", {"10", "11", "12", "14"}, 2},
        {"Cuanto es 15 + 27?", {"42", "32", "52", "38"}, 0},
        {"Cuanto es 100 / 4?", {"20", "25", "30", "15"}, 1},
        {"Cual es el 20% de 80?", {"12", "14", "16", "18"}, 2},
        {"Cuanto es 3 al cubo?", {"9", "18", "27", "36"}, 2},
        {"Cuanto es 45 - 19?", {"24", "26", "28", "30"}, 1},
        {"Area de un cuadrado de lado 6?", {"24", "30", "36", "42"}, 2},
        {"Cuanto es 9 x 6?", {"48", "54", "56", "64"}, 1},
        {"Cuanto es 120 / 5?", {"20", "22", "24", "26"}, 2},
        {"Cuanto es 13 + 18?", {"29", "31", "33", "35"}, 1},
        {"Cuanto es 8 x 7?", {"54", "56", "58", "60"}, 1},
        {"Cual es el 50% de 150?", {"65", "70", "75", "80"}, 2},
        {"Cuanto es 81 / 9?", {"7", "8", "9", "10"}, 2},
        {"Cuanto es 6 al cuadrado?", {"30", "32", "34", "36"}, 3}
    };
}

void MinigameQuiz::activate()
{
    m_isActive = true;
    reset();
}

void MinigameQuiz::deactivate()
{
    m_isActive = false;
}

void MinigameQuiz::reset()
{
    m_juegoTerminado = false;
    m_preguntasRespondidas = 0;
    m_respuestasCorrectas = 0;
    m_mostrandoResultado = false;
    m_opcionSeleccionada = -1;

    // Reiniciar banco de preguntas disponibles
    m_preguntasDisponibles.clear();
    for (int i = 0; i < (int)m_bancoPreguntas.size(); i++)
    {
        m_preguntasDisponibles.push_back(i);
    }

    seleccionarSiguientePregunta();
    actualizarTextos();
}

void MinigameQuiz::seleccionarSiguientePregunta()
{
    if (m_preguntasDisponibles.empty())
    {
        for (int i = 0; i < (int)m_bancoPreguntas.size(); i++)
        {
            m_preguntasDisponibles.push_back(i);
        }
    }

    std::uniform_int_distribution<> dist(0, m_preguntasDisponibles.size() - 1);
    int index = dist(m_gen);
    int preguntaId = m_preguntasDisponibles[index];
    m_preguntasDisponibles.erase(m_preguntasDisponibles.begin() + index);

    m_preguntaActual = &m_bancoPreguntas[preguntaId];
}

void MinigameQuiz::actualizarTextos() {
    if (!m_preguntaActual) return;
    
    // PREGUNTA
    m_preguntaText->setString(m_preguntaActual->enunciado);
    sf::FloatRect bounds = m_preguntaText->getLocalBounds();
    m_preguntaText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    m_preguntaText->setPosition(sf::Vector2f(
        m_position.x + m_size.x / 2.f,
        m_position.y + m_size.y / 2.f - 120.f  // Más arriba
    ));
    
    // RESPUESTAS - Cuadrícula 2x2
    float inicioX = m_position.x + m_size.x / 2.f - 200.f;
    float inicioY = m_position.y + m_size.y / 2.f - 30.f;

    //Separacion entre opciones
    float espacioX = 350.f;  // Más separadas
    float espacioY = 70.f;   // Más separadas
    
    for (int i = 0; i < 4 && i < (int)m_preguntaActual->opciones.size(); i++) {
        m_textosBotones[i]->setString(m_preguntaActual->opciones[i]);
        
        int fila = i / 2;
        int columna = i % 2;
        
        float x = inicioX + columna * espacioX;
        float y = inicioY + fila * espacioY;
        
        m_textosBotones[i]->setPosition(sf::Vector2f(x, y));
    }
    
    m_puntuacionText->setString(std::to_string(m_respuestasCorrectas) + "/" + std::to_string(m_preguntasPorRonda));
}

void MinigameQuiz::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
    if (!m_isActive || m_juegoTerminado || m_mostrandoResultado)
        return;

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    // Verificar hover en los textos
    for (int i = 0; i < 4 && i < (int)m_textosBotones.size(); i++)
    {
        sf::FloatRect textBounds = m_textosBotones[i]->getGlobalBounds();
        m_botonHover[i] = textBounds.contains(mousePos);

        // Cambiar color en hover
        m_textosBotones[i]->setFillColor(m_botonHover[i] ? sf::Color::Yellow : sf::Color(200, 200, 200));
    }

    // Verificar clic
    if (event.is<sf::Event::MouseButtonPressed>())
    {
        const auto &mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
        if (mouseEvent->button == sf::Mouse::Button::Left)
        {
            for (int i = 0; i < 4 && i < (int)m_textosBotones.size(); i++)
            {
                if (m_textosBotones[i]->getGlobalBounds().contains(mousePos))
                {
                    verificarRespuesta(i);
                    break;
                }
            }
        }
    }
}

void MinigameQuiz::verificarRespuesta(int opcion) {
    bool correcta = (opcion == m_preguntaActual->respuestaCorrecta);
    
    if (correcta) {
        m_respuestasCorrectas++;
        mostrarResultadoTemporal("CORRECTO", sf::Color::Green);
        
        m_preguntasRespondidas++;
        
        // ¿Ganó? (5 correctas)
        if (m_respuestasCorrectas >= m_preguntasPorRonda) {
            m_juegoTerminado = true;
            m_tituloText->setString("LECCION COMPLETADA");
            m_puntuacionText->setString(std::to_string(m_respuestasCorrectas) + "/" + std::to_string(m_preguntasPorRonda));
            return;
        }
        
        // Siguiente pregunta
        seleccionarSiguientePregunta();
        actualizarTextos();
    } else {
        // Falló - TERMINA EL JUEGO
        mostrarResultadoTemporal("INCORRECTO", sf::Color::Red);
        m_juegoTerminado = true;
        m_tituloText->setString("HAS FALLADO");
        m_puntuacionText->setString(std::to_string(m_respuestasCorrectas) + "/" + std::to_string(m_preguntasPorRonda));
    }
}

void MinigameQuiz::mostrarResultadoTemporal(const std::string& mensaje, const sf::Color& color) {
    m_mostrandoResultado = true;
    m_tiempoResultado = 1.5f;
    m_mensajeResultado = mensaje;
    m_colorResultado = color;
    
    // Usar string directo sin caracteres especiales
    m_resultadoText->setString(mensaje);
    m_resultadoText->setFillColor(color);
    
    sf::FloatRect bounds = m_resultadoText->getLocalBounds();
    m_resultadoText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    m_resultadoText->setPosition(sf::Vector2f(
        m_position.x + m_size.x / 2.f,
        m_position.y + m_size.y - 120.f + 50.f
    ));
}

void MinigameQuiz::update(float dt)
{
    if (!m_isActive)
        return;

    if (m_mostrandoResultado)
    {
        m_tiempoResultado -= dt;
        if (m_tiempoResultado <= 0.f)
        {
            m_mostrandoResultado = false;
        }
    }

    if (m_juegoTerminado)
    {
        m_puntuacionText->setString(std::to_string(m_respuestasCorrectas) + "/" + std::to_string(m_preguntasPorRonda));
    }
}

void MinigameQuiz::draw(sf::RenderWindow &window)
{
    if (!m_isActive)
        return;

    window.draw(m_background);

    if (m_pizarraSprite)
    {
        window.draw(*m_pizarraSprite);
    }

    if (m_tituloText)
        window.draw(*m_tituloText);
    if (m_preguntaText)
        window.draw(*m_preguntaText);
    if (m_puntuacionText)
        window.draw(*m_puntuacionText);
    if (m_instruccionesText)
        window.draw(*m_instruccionesText);

    // Dibujar opciones (solo texto, sin cuadros)
    if (!m_mostrandoResultado && !m_juegoTerminado)
    {
        for (int i = 0; i < 4 && i < (int)m_textosBotones.size(); i++)
        {
            if (m_textosBotones[i])
            {
                window.draw(*m_textosBotones[i]);
            }
        }
    }

    // Dibujar mensaje de resultado
    if (m_mostrandoResultado && m_resultadoText)
    {
        window.draw(*m_resultadoText);
    }
}