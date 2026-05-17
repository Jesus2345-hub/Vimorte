#include "Estados/AdminMenuState.hpp"
#include "Configuracion/Game.hpp"
#include <iostream>
#include <cmath>

AdminMenuState::AdminMenuState(sf::RenderWindow *window, Game *game)
    : State(window, game), m_scrollOffset(0.f), m_maxScroll(0.f)
{
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf"))
    {
        std::cerr << "Error cargando fuente en AdminMenu" << std::endl;
    }

    // Cámara fija 1280x720
    m_camera = sf::View(sf::Vector2f(640.f, 360.f), sf::Vector2f(1280.f, 720.f));

    m_background.setSize(sf::Vector2f(1280.f, 720.f));
    m_background.setFillColor(sf::Color(10, 10, 20, 255));

    m_panel.setSize(sf::Vector2f(1200.f, 580.f));
    m_panel.setPosition(sf::Vector2f(40.f, 70.f));
    m_panel.setFillColor(sf::Color(20, 20, 40, 240));
    m_panel.setOutlineThickness(3.f);
    m_panel.setOutlineColor(sf::Color(255, 0, 0));

    m_title = std::make_unique<sf::Text>(m_font, "MODO ADMINISTRADOR - ARBOL DE NIVELES", 30);
    m_title->setFillColor(sf::Color::Red);
    m_title->setStyle(sf::Text::Bold);

    m_instructionText = std::make_unique<sf::Text>(m_font,
                                                   "Click en nodo: Saltar a nivel | Scroll: Navegar | ESC: Volver", 13);
    m_instructionText->setFillColor(sf::Color(150, 150, 150));
    m_instructionText->setOutlineThickness(1.5f);
    m_instructionText->setOutlineColor(sf::Color::Black);

    // Barra de desplazamiento
    m_scrollBar.setSize(sf::Vector2f(12.f, 580.f));
    m_scrollBar.setPosition(sf::Vector2f(1248.f, 70.f));
    m_scrollBar.setFillColor(sf::Color(40, 40, 50, 200));
    m_scrollBar.setOutlineThickness(1.f);
    m_scrollBar.setOutlineColor(sf::Color(100, 100, 100));

    m_scrollThumb.setSize(sf::Vector2f(10.f, 60.f));
    m_scrollThumb.setPosition(sf::Vector2f(1249.f, 70.f));
    m_scrollThumb.setFillColor(sf::Color(150, 150, 150, 200));
    m_scrollThumb.setOutlineThickness(1.f);
    m_scrollThumb.setOutlineColor(sf::Color(200, 200, 200));

    construirArbolVisual(game->getLevelTree().getRoot(), 600.f, 130.f, 500.f, 150.f, 0);

    m_maxScroll = std::max(0.f, m_arbolTotalY - 350.f);
}

void AdminMenuState::construirArbolVisual(LevelNode *nodo, float x, float y, float espacioX, float espacioY, int profundidad)
{
    if (!nodo)
        return;

    NodoVisual visual;
    visual.nodo = nodo;
    visual.x = x;
    visual.y = y;
    visual.espacioXOriginal = espacioX;
    visual.profundidad = profundidad;

    std::string nombreMostrar = nodo->displayName;
    if (nombreMostrar.length() > 14)
        nombreMostrar = nombreMostrar.substr(0, 13) + ".";

    visual.texto = std::make_unique<sf::Text>(m_font, nombreMostrar, 12);
    visual.texto->setFillColor(sf::Color::White);
    visual.texto->setOutlineThickness(1.5f);
    visual.texto->setOutlineColor(sf::Color::Black);

    float anchoCaja = 110.f;
    float altoCaja = 35.f;

    visual.caja.setSize(sf::Vector2f(anchoCaja, altoCaja));
    visual.caja.setOrigin(sf::Vector2f(anchoCaja / 2.f, altoCaja / 2.f));
    visual.caja.setPosition(sf::Vector2f(x, y));
    visual.caja.setOutlineThickness(2.f);

    if (nodo->type == LevelType::CENTINELA)
    {
        visual.caja.setFillColor(sf::Color(80, 40, 0, 230));
        visual.caja.setOutlineColor(sf::Color(255, 150, 50));
    }
    else if (nodo->id.find("final_") != std::string::npos || nodo->id.find("Final") != std::string::npos)
    {
        visual.caja.setFillColor(sf::Color(0, 50, 0, 230));
        visual.caja.setOutlineColor(sf::Color(100, 255, 100));
    }
    else
    {
        visual.caja.setFillColor(sf::Color(60, 60, 70, 230));
        visual.caja.setOutlineColor(sf::Color(180, 180, 180));
    }

    sf::FloatRect tb = visual.texto->getLocalBounds();
    visual.texto->setOrigin(sf::Vector2f(tb.size.x / 2.f, tb.size.y / 2.f));
    visual.texto->setPosition(sf::Vector2f(x, y));

    m_nodosVisuales.push_back(std::move(visual));

    if (y > m_arbolTotalY)
        m_arbolTotalY = y;

    float hijoY = y + espacioY;
    float hijoEspacioX = espacioX / 2.2f;

    if (nodo->left)
    {
        float hijoX = x - hijoEspacioX;

        m_lineas.push_back(sf::VertexArray(sf::PrimitiveType::Lines, 2));
        m_lineas.back()[0].position = sf::Vector2f(x, y + altoCaja / 2.f);
        m_lineas.back()[1].position = sf::Vector2f(hijoX, hijoY - altoCaja / 2.f);
        m_lineas.back()[0].color = sf::Color(120, 120, 120);
        m_lineas.back()[1].color = sf::Color(120, 120, 120);

        construirArbolVisual(nodo->left.get(), hijoX, hijoY, hijoEspacioX, espacioY, profundidad + 1);
    }

    if (nodo->right)
    {
        float hijoX = x + hijoEspacioX;

        m_lineas.push_back(sf::VertexArray(sf::PrimitiveType::Lines, 2));
        m_lineas.back()[0].position = sf::Vector2f(x, y + altoCaja / 2.f);
        m_lineas.back()[1].position = sf::Vector2f(hijoX, hijoY - altoCaja / 2.f);
        m_lineas.back()[0].color = sf::Color(200, 150, 50, 150);
        m_lineas.back()[1].color = sf::Color(200, 150, 50, 150);

        construirArbolVisual(nodo->right.get(), hijoX, hijoY, hijoEspacioX, espacioY, profundidad + 1);
    }
}

void AdminMenuState::handleEvent(const sf::Event &event)
{
    sf::Vector2f mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window), m_camera);
    float mouseLocalX = mousePos.x - 40.f;
    float mouseLocalY = mousePos.y - 70.f;

    // Scroll con rueda
    if (const auto *scroll = event.getIf<sf::Event::MouseWheelScrolled>())
    {
        m_scrollOffset = std::clamp(m_scrollOffset - scroll->delta * 40.f, 0.f, m_maxScroll);
    }

    // Mouse presionado
    if (const auto *mouse = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse->button == sf::Mouse::Button::Left)
        {
            // Verificar si click en la barra de scroll
            sf::FloatRect barBounds = m_scrollBar.getGlobalBounds();
            if (barBounds.contains(mousePos))
            {
                m_arrastrandoScroll = true;
                // Mover el thumb a la posición del click
                float porcentaje = (mousePos.y - 70.f) / 580.f;
                m_scrollOffset = std::clamp(porcentaje * m_maxScroll, 0.f, m_maxScroll);
            }
            else
            {
                // Click en nodos
                for (auto &nodo : m_nodosVisuales)
                {
                    float factorExpansion = 1.0f + (m_scrollOffset / 55.f);
                    float centroX = 600.f;
                    float desplazamientoX = m_scrollOffset * 7.8f;
                    float nodoY = nodo.y - m_scrollOffset;

                    float xOriginal = nodo.x;
                    float xCentrado = centroX + (xOriginal - centroX) * factorExpansion + desplazamientoX;

                    sf::RectangleShape cajaTemp = nodo.caja;
                    cajaTemp.setPosition(sf::Vector2f(xCentrado, nodoY));
                    sf::FloatRect bounds = cajaTemp.getGlobalBounds();

                    if (bounds.contains(sf::Vector2f(mouseLocalX, mouseLocalY)))
                    {
                        game->getLevelTree().jumpToNode(nodo.nodo->id);
                        game->detenerMusica();

                        auto newState = game->getLevelTree().createCurrentState(window, game);
                        if (newState)
                            game->changeState(std::move(newState));
                        return;
                    }
                }
            }
        }
    }

    // Mouse soltado
    if (const auto *mouseReleased = event.getIf<sf::Event::MouseButtonReleased>())
    {
        if (mouseReleased->button == sf::Mouse::Button::Left)
        {
            m_arrastrandoScroll = false;
        }
    }

    // Escape
    if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Escape)
        {
            game->popState();
        }
    }
}

void AdminMenuState::update(float dt)
{
    // Arrastrar barra de scroll
    if (m_arrastrandoScroll)
    {
        sf::Vector2f mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window), m_camera);
        float porcentaje = (mousePos.y - 70.f) / 580.f;
        m_scrollOffset = std::clamp(porcentaje * m_maxScroll, 0.f, m_maxScroll);
    }
}
void AdminMenuState::draw()
{
    if (!window)
        return;

    // ============================================================
    // CREAR RENDERTEXTURE UNA SOLA VEZ
    // ============================================================
    if (!m_textureCreada)
    {
        sf::Vector2u panelSize(1200, 580);
        if (m_arbolTexture.resize(panelSize))
        {
            m_arbolTexture.setSmooth(true);
            m_arbolSprite = std::make_unique<sf::Sprite>(m_arbolTexture.getTexture());
            m_arbolSprite->setPosition(sf::Vector2f(40.f, 70.f));
            m_textureCreada = true;
        }
    }

    // ============================================================
    // DIBUJAR EL ÁRBOL EN LA TEXTURA
    // ============================================================
    if (m_textureCreada)
    {
        m_arbolTexture.clear(sf::Color(20, 20, 40, 240));

        sf::View texturaView(sf::Vector2f(600.f, 290.f), sf::Vector2f(1200.f, 580.f));
        m_arbolTexture.setView(texturaView);

        float factorExpansion = 1.0f + (m_scrollOffset / 55.f);
        float desplazamientoX = m_scrollOffset * 7.8f;
        float centroX = 600.f;

        // Líneas (dibujadas 2 veces para simular grosor)
        for (auto &linea : m_lineas)
        {
            sf::VertexArray lineaDibujo = linea;
            for (size_t i = 0; i < lineaDibujo.getVertexCount(); i++)
            {
                float xOriginal = lineaDibujo[i].position.x;
                lineaDibujo[i].position.x = centroX + (xOriginal - centroX) * factorExpansion + desplazamientoX;
                lineaDibujo[i].position.y -= m_scrollOffset;
            }
            m_arbolTexture.draw(lineaDibujo);

            // Segunda pasada desplazada 1px para más grosor
            sf::VertexArray lineaGruesa = lineaDibujo;
            for (size_t i = 0; i < lineaGruesa.getVertexCount(); i++)
            {
                lineaGruesa[i].position.y += 1.f;
            }
            m_arbolTexture.draw(lineaGruesa);
        }

        // Nodos
        sf::Vector2f mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window), m_camera);
        float mouseLocalX = mousePos.x - 40.f;
        float mouseLocalY = mousePos.y - 70.f;

        for (auto &nodo : m_nodosVisuales)
        {
            float nodoY = nodo.y - m_scrollOffset;

            float xCentrado = centroX + (nodo.x - centroX) * factorExpansion + desplazamientoX;

            sf::RectangleShape caja = nodo.caja;
            caja.setPosition(sf::Vector2f(xCentrado, nodoY));

            sf::FloatRect bounds = caja.getGlobalBounds();
            if (bounds.contains(sf::Vector2f(mouseLocalX, mouseLocalY)))
            {
                caja.setOutlineColor(sf::Color::Yellow);
                caja.setOutlineThickness(3.f);
            }

            m_arbolTexture.draw(caja);

            if (nodo.texto)
            {
                sf::Text texto = *nodo.texto;
                texto.setPosition(sf::Vector2f(xCentrado, nodoY));
                m_arbolTexture.draw(texto);
            }
        }

        m_arbolTexture.display();
    }

    // ============================================================
    // DIBUJAR EN PANTALLA
    // ============================================================
    window->setView(m_camera);

    m_background.setSize(sf::Vector2f(1280.f, 720.f));
    window->draw(m_background);

    // Panel
    window->draw(m_panel);

    // Árbol
    if (m_textureCreada)
    {
        window->draw(*m_arbolSprite);
    }

    // Barra de desplazamiento
    if (m_maxScroll > 0.f)
    {
        float thumbHeight = std::max(30.f, 580.f * (580.f / (m_arbolTotalY + 100.f)));
        float thumbY = 70.f + (m_scrollOffset / m_maxScroll) * (580.f - thumbHeight);

        m_scrollThumb.setSize(sf::Vector2f(10.f, thumbHeight));
        m_scrollThumb.setPosition(sf::Vector2f(1249.f, thumbY));

        window->draw(m_scrollBar);
        window->draw(m_scrollThumb);
    }

    // Título
    if (m_title)
    {
        sf::FloatRect tb = m_title->getLocalBounds();
        m_title->setOrigin(sf::Vector2f(tb.size.x / 2.f, 0.f));
        m_title->setPosition(sf::Vector2f(640.f, 15.f));
        window->draw(*m_title);
    }

    // Instrucciones
    if (m_instructionText)
    {
        sf::FloatRect ib = m_instructionText->getLocalBounds();
        m_instructionText->setOrigin(sf::Vector2f(ib.size.x / 2.f, 0.f));
        m_instructionText->setPosition(sf::Vector2f(640.f, 690.f));
        window->draw(*m_instructionText);
    }
}