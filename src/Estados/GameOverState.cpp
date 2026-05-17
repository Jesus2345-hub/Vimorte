#include "Estados/GameOverState.hpp"
#include "Configuracion/Game.hpp"
#include "Estructuras/LevelTree.hpp"
#include <iostream>
#include <cmath>

GameOverState::GameOverState(sf::RenderWindow* window, Game* game)
    : State(window, game)
{
    // Fuente
    if (!m_fuente.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente en GameOverState" << std::endl;
    }
    
    // Fondo negro completo
    m_fondo.setSize(sf::Vector2f(1280.f, 720.f));
    m_fondo.setFillColor(sf::Color::Black);
    
    // ============================================
    // TÍTULO "GAME OVER" CON EFECTO
    // ============================================
    m_titulo = std::make_unique<sf::Text>(m_fuente, "GAME OVER", 90);
    m_titulo->setFillColor(sf::Color::Red);
    m_titulo->setStyle(sf::Text::Style::Bold);
    sf::FloatRect tb = m_titulo->getLocalBounds();
    m_titulo->setOrigin({tb.size.x / 2.f, tb.size.y / 2.f});
    m_titulo->setPosition({640.f, 180.f});
    
    // Sombra del título (efecto 3D)
    m_tituloSombra = std::make_unique<sf::Text>(m_fuente, "GAME OVER", 90);
    m_tituloSombra->setFillColor(sf::Color(100, 0, 0));
    m_tituloSombra->setStyle(sf::Text::Style::Bold);
    m_tituloSombra->setOrigin({tb.size.x / 2.f, tb.size.y / 2.f});
    m_tituloSombra->setPosition({642.f, 182.f});
    
    // ============================================
    // SUBTÍTULO
    // ============================================
    m_subtitulo = std::make_unique<sf::Text>(m_fuente, "Has perdido el nivel", 28);
    m_subtitulo->setFillColor(sf::Color(200, 200, 200));
    sf::FloatRect stb = m_subtitulo->getLocalBounds();
    m_subtitulo->setOrigin({stb.size.x / 2.f, stb.size.y / 2.f});
    m_subtitulo->setPosition({640.f, 260.f});
    
    // ============================================
    // BOTÓN REINTENTAR (con diseño moderno)
    // ============================================
    m_reintentarText = std::make_unique<sf::Text>(m_fuente, "REINTENTAR", 38);
    m_reintentarText->setFillColor(sf::Color::White);
    m_reintentarText->setStyle(sf::Text::Style::Bold);
    
    m_botonReintentar.setSize(sf::Vector2f(350.f, 80.f));
    m_botonReintentar.setFillColor(sf::Color(220, 50, 50));  // Rojo más vibrante
    m_botonReintentar.setOutlineThickness(3.f);
    m_botonReintentar.setOutlineColor(sf::Color::White);
    m_botonReintentar.setOrigin({175.f, 40.f});
    m_botonReintentar.setPosition({640.f, 360.f});
    
    sf::FloatRect br = m_reintentarText->getLocalBounds();
    m_reintentarText->setOrigin({br.size.x / 2.f, br.size.y / 2.f});
    m_reintentarText->setPosition({640.f, 360.f});
    
    // ============================================
    // BOTÓN MENÚ PRINCIPAL (con diseño moderno)
    // ============================================
    m_menuText = std::make_unique<sf::Text>(m_fuente, "MENU PRINCIPAL", 38);
    m_menuText->setFillColor(sf::Color::White);
    m_menuText->setStyle(sf::Text::Style::Bold);
    
    m_botonMenu.setSize(sf::Vector2f(350.f, 80.f));
    m_botonMenu.setFillColor(sf::Color(60, 60, 100));  // Azul grisáceo
    m_botonMenu.setOutlineThickness(3.f);
    m_botonMenu.setOutlineColor(sf::Color::White);
    m_botonMenu.setOrigin({175.f, 40.f});
    m_botonMenu.setPosition({640.f, 470.f});
    
    sf::FloatRect bm = m_menuText->getLocalBounds();
    m_menuText->setOrigin({bm.size.x / 2.f, bm.size.y / 2.f});
    m_menuText->setPosition({640.f, 470.f});
    
    // ============================================
    // EFECTO DE BRILLO EN BOTONES (animación)
    // ============================================
    m_brilloReintentar = 0.f;
    m_brilloMenu = 0.f;
    m_tiempoAnimacion = 0.f;
}

void GameOverState::update(float dt)
{
    m_tiempoAnimacion += dt;
    
    // Efecto de pulsación suave en los botones
    float pulso = (std::sin(m_tiempoAnimacion * 3.f) + 1.f) / 2.f;  // 0 a 1
    float brilloExtra = pulso * 30.f;
    
    // Actualizar colores según hover + animación
    if (m_hoverReintentar) {
        m_botonReintentar.setFillColor(sf::Color(255, 80, 80));
        m_reintentarText->setFillColor(sf::Color::Yellow);
    } else {
        m_botonReintentar.setFillColor(sf::Color(200, 40, 40));
        m_reintentarText->setFillColor(sf::Color::White);
    }
    
    if (m_hoverMenu) {
        m_botonMenu.setFillColor(sf::Color(100, 100, 150));
        m_menuText->setFillColor(sf::Color::Yellow);
    } else {
        m_botonMenu.setFillColor(sf::Color(60, 60, 100));
        m_menuText->setFillColor(sf::Color::White);
    }
}

void GameOverState::handleEvent(const sf::Event& event)
{
    sf::Vector2f mouse = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
    
    // Hover
    m_hoverReintentar = m_botonReintentar.getGlobalBounds().contains(mouse);
    m_hoverMenu = m_botonMenu.getGlobalBounds().contains(mouse);
    
    // Click
    if (const auto* click = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (click->button == sf::Mouse::Button::Left)
        {
            if (m_hoverReintentar)
            {
                std::cout << "Reiniciando nivel..." << std::endl;
                LevelNode* nodoActual = game->getLevelTree().getCurrentNode();
                if (nodoActual)
                {
                    auto nuevoEstado = game->getLevelTree().createCurrentState(window, game);
                    if (nuevoEstado)
                        game->changeState(std::move(nuevoEstado));
                }
                else
                {
                    game->returnToMenu();
                }
            }
            else if (m_hoverMenu)
            {
                game->returnToMenu();
            }
        }
    }
}

void GameOverState::draw()
{
    sf::View vista(sf::Vector2f(640.f, 360.f), sf::Vector2f(1280.f, 720.f));
    window->setView(vista);
    
    // Fondo negro
    window->draw(m_fondo);
    
    // Línea decorativa superior
    sf::RectangleShape lineaSup(sf::Vector2f(500.f, 3.f));
    lineaSup.setFillColor(sf::Color::Red);
    lineaSup.setOrigin({250.f, 0.f});
    lineaSup.setPosition({640.f, 100.f});
    window->draw(lineaSup);
    
    // Línea decorativa inferior
    sf::RectangleShape lineaInf(sf::Vector2f(500.f, 3.f));
    lineaInf.setFillColor(sf::Color::Red);
    lineaInf.setOrigin({250.f, 0.f});
    lineaInf.setPosition({640.f, 620.f});
    window->draw(lineaInf);
    
    // Título con sombra
    if (m_tituloSombra) window->draw(*m_tituloSombra);
    if (m_titulo) window->draw(*m_titulo);
    
    // Subtítulo
    if (m_subtitulo) window->draw(*m_subtitulo);
    
    // Botones
    window->draw(m_botonReintentar);
    window->draw(*m_reintentarText);
    
    window->draw(m_botonMenu);
    window->draw(*m_menuText);
}