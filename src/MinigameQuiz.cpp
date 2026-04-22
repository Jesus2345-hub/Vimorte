#include "MinigameQuiz.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

MinigameQuiz::MinigameQuiz()
    : m_isActive(false), m_juegoTerminado(false), m_preguntasRespondidas(0),
      m_respuestasCorrectas(0), m_preguntasPorRonda(5), m_opcionSeleccionada(-1),
      m_mostrandoResultado(false), m_tiempoResultado(0.f), m_gen(m_rd())
{
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

void MinigameQuiz::configurarUI()
{
    m_tieneImagen.clear();
    for (int i = 0; i < 3; i++)
    {
        m_tieneImagen.push_back(false);
    }
    // Cargar textura de la pizarra
    if (m_pizarraTexture.loadFromFile("assets/images/niveles/nivel1/pizarra.png"))
    {
        m_pizarraSprite = std::make_unique<sf::Sprite>(m_pizarraTexture);

        float escala = 1.7f;
        m_pizarraSprite->setScale(sf::Vector2f(escala, escala));

        sf::FloatRect bounds = m_pizarraSprite->getLocalBounds();
        m_pizarraSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_pizarraSprite->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f + 230.f,
            m_position.y + m_size.y / 2.f + 30.f));
    }

    // Cargar fuente
    if (m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf"))
    {
        m_tituloText = std::make_unique<sf::Text>(m_font);
        m_tituloText->setString("LECCION DE MATEMATICAS");
        m_tituloText->setCharacterSize(34);
        m_tituloText->setFillColor(sf::Color::White);
        m_tituloText->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f - 200.f,
            m_position.y + 15.f + 40.f));

        m_preguntaText = std::make_unique<sf::Text>(m_font);
        m_preguntaText->setCharacterSize(38);
        m_preguntaText->setFillColor(sf::Color::White);
        m_preguntaText->setStyle(sf::Text::Bold);

        m_puntuacionText = std::make_unique<sf::Text>(m_font);
        m_puntuacionText->setCharacterSize(28);
        m_puntuacionText->setFillColor(sf::Color::Yellow);
        m_puntuacionText->setPosition(sf::Vector2f(
            m_position.x + m_size.x - 220.f + 50.f,
            m_position.y + 15.f + 45.f));

        m_instruccionesText = std::make_unique<sf::Text>(m_font);
        m_instruccionesText->setString("Haz clic en la respuesta correcta | ESC para salir");
        m_instruccionesText->setCharacterSize(16);
        m_instruccionesText->setFillColor(sf::Color(180, 180, 180));
        m_instruccionesText->setPosition(sf::Vector2f(
            m_position.x + 20.f + 230.f,
            m_position.y + m_size.y - 35.f + 50.f));

        m_resultadoText = std::make_unique<sf::Text>(m_font);
        m_resultadoText->setCharacterSize(40);
        m_resultadoText->setStyle(sf::Text::Bold);
    }

    // Crear textos para las opciones (máximo 4)
    m_textosBotones.clear();
    for (int i = 0; i < 4; i++)
    {
        if (m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf"))
        {
            auto texto = std::make_unique<sf::Text>(m_font);
            texto->setCharacterSize(34);
            texto->setFillColor(sf::Color(200, 200, 200));
            m_textosBotones.push_back(std::move(texto));
        }
    }

    // Crear sprites para imágenes (máximo 3)
    m_imagenesBotones.clear();
    m_texturasBotones.clear();
    m_bordesHover.clear();
    m_tieneImagen.clear();

    for (int i = 0; i < 3; i++)
    {
        m_texturasBotones.push_back(sf::Texture());

        // Crear sprite SIN textura (usando el constructor con textura vacía)
        // Cargamos una textura mínima desde un archivo o usamos un placeholder
        sf::Texture tempTex;
        // No necesitamos crear el sprite aquí, lo crearemos cuando carguemos la imagen real

        m_imagenesBotones.push_back(nullptr); // Placeholder

        auto borde = std::make_unique<sf::RectangleShape>();
        borde->setFillColor(sf::Color::Transparent);
        borde->setOutlineThickness(4.f);
        borde->setOutlineColor(sf::Color::Yellow);
        m_bordesHover.push_back(std::move(borde));

        m_tieneImagen.push_back(false);
    }

    m_botonHover.clear();
    for (int i = 0; i < 4; i++)
    {
        m_botonHover.push_back(false);
    }
}

void MinigameQuiz::inicializarBancoPreguntas()
{
    m_bancoPreguntas = {
        // Preguntas de TEXTO (4 opciones)
        {"Cuanto es 7 x 8?", {"48", "56", "64", "72"}, {}, false, 1},
        {"Cual es la raiz cuadrada de 144?", {"10", "11", "12", "14"}, {}, false, 2},
        {"Cuanto es 15 + 27?", {"42", "32", "52", "38"}, {}, false, 0},
        {"Cuanto es 100 / 4?", {"20", "25", "30", "15"}, {}, false, 1},
        {"Cual es el 20% de 80?", {"12", "14", "16", "18"}, {}, false, 2},
        {"Cuanto es 3 al cubo?", {"9", "18", "27", "36"}, {}, false, 2},
        {"Cuanto es 45 - 19?", {"24", "26", "28", "30"}, {}, false, 1},
        {"Area de un cuadrado de lado 6?", {"24", "30", "36", "42"}, {}, false, 2},
        {"Cuanto es 9 x 6?", {"48", "54", "56", "64"}, {}, false, 1},
        {"Cuanto es 120 / 5?", {"20", "22", "24", "26"}, {}, false, 2},
        {"Cuanto es 13 + 18?", {"29", "31", "33", "35"}, {}, false, 1},
        {"Cuanto es 8 x 7?", {"54", "56", "58", "60"}, {}, false, 1},
        {"Cual es el 50% de 150?", {"65", "70", "75", "80"}, {}, false, 2},
        {"Cuanto es 81 / 9?", {"7", "8", "9", "10"}, {}, false, 2},
        {"Cuanto es 6 al cuadrado?", {"30", "32", "34", "36"}, {}, false, 3},

        // PREGUNTA CON IMÁGENES 1: ¿Qué es más pequeño?
        {"Que es mas pequenio?", {}, {"assets/images/quiz/mosquito.png", "assets/images/quiz/canasta.png", "assets/images/quiz/saturno.png"}, true, 0},

        // PREGUNTA CON IMÁGENES 2: ¿Cuál es más lento?
        {"Cual es mas lento?", {}, {"assets/images/quiz/cohete.png", "assets/images/quiz/perezoso.png", "assets/images/quiz/bicicleta.png"}, true, 1}};
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

    m_preguntasDisponibles.clear();
    for (int i = 0; i < (int)m_bancoPreguntas.size(); i++)
    {
        m_preguntasDisponibles.push_back(i);
    }

    seleccionarSiguientePregunta();

    if (m_preguntaActual->esPreguntaConImagenes)
    {
        actualizarImagenes();
    }
    else
    {
        actualizarTextos();
    }
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

void MinigameQuiz::ocultarOpcionesTexto()
{
    for (auto &texto : m_textosBotones)
    {
        texto->setString("");
    }
}

void MinigameQuiz::ocultarOpcionesImagen()
{
}

void MinigameQuiz::actualizarTextos()
{
    if (!m_preguntaActual)
        return;

    ocultarOpcionesImagen();

    m_preguntaText->setString(m_preguntaActual->enunciado);
    sf::FloatRect bounds = m_preguntaText->getLocalBounds();
    m_preguntaText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    m_preguntaText->setPosition(sf::Vector2f(
        m_position.x + m_size.x / 2.f,
        m_position.y + m_size.y / 2.f - 120.f));

    float inicioX = m_position.x + m_size.x / 2.f - 200.f;
    float inicioY = m_position.y + m_size.y / 2.f - 30.f;
    float espacioX = 350.f;
    float espacioY = 70.f;

    for (int i = 0; i < 4 && i < (int)m_preguntaActual->opciones.size(); i++)
    {
        m_textosBotones[i]->setString(m_preguntaActual->opciones[i]);

        int fila = i / 2;
        int columna = i % 2;

        float x = inicioX + columna * espacioX;
        float y = inicioY + fila * espacioY;

        m_textosBotones[i]->setPosition(sf::Vector2f(x, y));
    }

    // Limpiar textos sobrantes
    for (int i = m_preguntaActual->opciones.size(); i < 4; i++)
    {
        m_textosBotones[i]->setString("");
    }

    m_puntuacionText->setString(std::to_string(m_respuestasCorrectas) + "/" + std::to_string(m_preguntasPorRonda));
}

void MinigameQuiz::actualizarImagenes()
{
    if (!m_preguntaActual)
        return;

    ocultarOpcionesTexto();

    m_preguntaText->setString(m_preguntaActual->enunciado);
    sf::FloatRect bounds = m_preguntaText->getLocalBounds();
    m_preguntaText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    m_preguntaText->setPosition(sf::Vector2f(
        m_position.x + m_size.x / 2.f,
        m_position.y + m_size.y / 2.f - 140.f));

    // Limpiar sprites anteriores
    m_imagenesBotones.clear();
    for (int i = 0; i < 3; i++)
    {
        m_tieneImagen[i] = false;
    }

    // Cargar texturas y crear sprites
    for (int i = 0; i < 3 && i < (int)m_preguntaActual->imagenes.size(); i++)
    {
        if (m_texturasBotones[i].loadFromFile(m_preguntaActual->imagenes[i]))
        {
            // Crear sprite con la textura cargada
            auto sprite = std::make_unique<sf::Sprite>(m_texturasBotones[i]);
            sprite->setScale(sf::Vector2f(0.8f, 0.8f));

            // Centrar origen
            sf::FloatRect imgBounds = sprite->getLocalBounds();
            sprite->setOrigin(sf::Vector2f(imgBounds.size.x / 2.f, imgBounds.size.y / 2.f));

            // Posicionar en fila (3 imágenes)
            float espacioX = 200.f;
            float inicioX = m_position.x + m_size.x / 2.f - espacioX;
            float x = inicioX + i * espacioX;
            float y = m_position.y + m_size.y / 2.f + 20.f;

            sprite->setPosition(sf::Vector2f(x, y));

            // Guardar sprite
            if (i < (int)m_imagenesBotones.size())
            {
                m_imagenesBotones[i] = std::move(sprite);
            }
            else
            {
                m_imagenesBotones.push_back(std::move(sprite));
            }

            // Configurar borde hover
            m_bordesHover[i]->setSize(sf::Vector2f(imgBounds.size.x + 10.f, imgBounds.size.y + 10.f));
            m_bordesHover[i]->setOrigin(sf::Vector2f((imgBounds.size.x + 10.f) / 2.f, (imgBounds.size.y + 10.f) / 2.f));
            m_bordesHover[i]->setPosition(sf::Vector2f(x, y));

            m_tieneImagen[i] = true;
        }
        else
        {
            std::cerr << "Error cargando: " << m_preguntaActual->imagenes[i] << std::endl;
            m_tieneImagen[i] = false;
        }
    }

    m_puntuacionText->setString(std::to_string(m_respuestasCorrectas) + "/" + std::to_string(m_preguntasPorRonda));
}

void MinigameQuiz::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
    if (!m_isActive || m_juegoTerminado || m_mostrandoResultado)
        return;

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (m_preguntaActual->esPreguntaConImagenes)
    {
        // Hover para imágenes
        for (int i = 0; i < 3; i++)
        {
            if (m_tieneImagen[i] && m_imagenesBotones[i])
            {
                sf::FloatRect bounds = m_imagenesBotones[i]->getGlobalBounds();
                m_botonHover[i] = bounds.contains(mousePos);
            }
        }

        // Clic en imágenes
        if (event.is<sf::Event::MouseButtonPressed>())
        {
            const auto &mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
            if (mouseEvent->button == sf::Mouse::Button::Left)
            {
                for (int i = 0; i < 3; i++)
                {
                    if (m_tieneImagen[i] && m_imagenesBotones[i])
                    {
                        if (m_imagenesBotones[i]->getGlobalBounds().contains(mousePos))
                        {
                            verificarRespuesta(i);
                            break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        // Hover para textos
        for (int i = 0; i < 4 && i < (int)m_textosBotones.size(); i++)
        {
            if (!m_textosBotones[i]->getString().isEmpty())
            {
                sf::FloatRect textBounds = m_textosBotones[i]->getGlobalBounds();
                m_botonHover[i] = textBounds.contains(mousePos);
                m_textosBotones[i]->setFillColor(m_botonHover[i] ? sf::Color::Yellow : sf::Color(200, 200, 200));
            }
        }

        // Clic en textos
        if (event.is<sf::Event::MouseButtonPressed>())
        {
            const auto &mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
            if (mouseEvent->button == sf::Mouse::Button::Left)
            {
                for (int i = 0; i < 4 && i < (int)m_textosBotones.size(); i++)
                {
                    if (!m_textosBotones[i]->getString().isEmpty())
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
    }
}

void MinigameQuiz::verificarRespuesta(int opcion)
{
    bool correcta = (opcion == m_preguntaActual->respuestaCorrecta);

    if (correcta)
    {
        m_respuestasCorrectas++;
        mostrarResultadoTemporal("CORRECTO", sf::Color::Green);

        m_preguntasRespondidas++;

        if (m_respuestasCorrectas >= m_preguntasPorRonda)
        {
            m_juegoTerminado = true;
            m_tituloText->setString("LECCION COMPLETADA");
            m_puntuacionText->setString(std::to_string(m_respuestasCorrectas) + "/" + std::to_string(m_preguntasPorRonda));
            return;
        }

        seleccionarSiguientePregunta();
        if (m_preguntaActual->esPreguntaConImagenes)
        {
            actualizarImagenes();
        }
        else
        {
            actualizarTextos();
        }
    }
    else
    {
        mostrarResultadoTemporal("INCORRECTO", sf::Color::Red);
        m_juegoTerminado = true;
        m_tituloText->setString("HAS FALLADO");
        m_puntuacionText->setString(std::to_string(m_respuestasCorrectas) + "/" + std::to_string(m_preguntasPorRonda));
    }
}

void MinigameQuiz::mostrarResultadoTemporal(const std::string &mensaje, const sf::Color &color)
{
    m_mostrandoResultado = true;
    m_tiempoResultado = 1.5f;
    m_mensajeResultado = mensaje;
    m_colorResultado = color;

    m_resultadoText->setString(mensaje);
    m_resultadoText->setFillColor(color);

    sf::FloatRect bounds = m_resultadoText->getLocalBounds();
    m_resultadoText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    m_resultadoText->setPosition(sf::Vector2f(
        m_position.x + m_size.x / 2.f,
        m_position.y + m_size.y - 120.f + 50.f));
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

    // Dibujar opciones
    if (!m_mostrandoResultado && !m_juegoTerminado)
    {
        if (m_preguntaActual && m_preguntaActual->esPreguntaConImagenes)
        {
            // Dibujar imágenes
            for (int i = 0; i < 3; i++)
            {
                if (m_tieneImagen[i] && m_imagenesBotones[i])
                {
                    // Dibujar borde hover si corresponde
                    if (m_botonHover[i])
                    {
                        window.draw(*m_bordesHover[i]);
                    }
                    window.draw(*m_imagenesBotones[i]);
                }
            }
        }
        else
        {
            // Dibujar textos
            for (int i = 0; i < 4 && i < (int)m_textosBotones.size(); i++)
            {
                if (m_textosBotones[i] && !m_textosBotones[i]->getString().isEmpty())
                {
                    window.draw(*m_textosBotones[i]);
                }
            }
        }
    }

    if (m_mostrandoResultado && m_resultadoText)
    {
        window.draw(*m_resultadoText);
    }
}