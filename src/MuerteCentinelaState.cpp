#include "MuerteCentinelaState.hpp"
#include "Game.hpp"
#include <iostream>

MuerteCentinelaState::MuerteCentinelaState(sf::RenderWindow* window, Game* game, bool puedeReintentar)
    : State(window, game)
    , m_puedeReintentar(puedeReintentar)
    , m_buttonHover(false)
{
    // Cargar fuente
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente en MuerteCentinelaState" << std::endl;
    }
    
    // Fondo oscuro
    m_background.setSize(sf::Vector2f(1280.f, 720.f));
    m_background.setFillColor(sf::Color(0, 0, 0, 200));
    
    // Panel central
    m_panel.setSize(sf::Vector2f(600.f, 400.f));
    m_panel.setPosition(sf::Vector2f(340.f, 160.f));
    m_panel.setFillColor(sf::Color(30, 10, 10, 240));
    m_panel.setOutlineThickness(4.f);
    m_panel.setOutlineColor(sf::Color::Red);
    
    // Título
    m_title = std::make_unique<sf::Text>(m_font, "HAS MUERTO", 48);
    m_title->setFillColor(sf::Color::Red);
    m_title->setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = m_title->getLocalBounds();
    m_title->setOrigin(sf::Vector2f(titleBounds.size.x / 2.f, titleBounds.size.y / 2.f));
    m_title->setPosition(sf::Vector2f(640.f, 220.f));
    
    // Mensaje según el modo
    std::string mensaje;
    if (m_puedeReintentar) {
        mensaje = "Estas en MODO HISTORIA.\nPuedes volver a intentarlo.\n\nEl checkpoint se ha guardado.";
    } else {
        mensaje = "Estas en MODO SUPERVIVENCIA.\nLas consecuencias son permanentes.\n\nLa partida ha terminado.";
    }
    
    m_message = std::make_unique<sf::Text>(m_font, mensaje, 22);
    m_message->setFillColor(sf::Color(255, 200, 200));
    sf::FloatRect msgBounds = m_message->getLocalBounds();
    m_message->setOrigin(sf::Vector2f(msgBounds.size.x / 2.f, msgBounds.size.y / 2.f));
    m_message->setPosition(sf::Vector2f(640.f, 320.f));
    
    // Botón
    m_button.setSize(sf::Vector2f(300.f, 60.f));
    m_button.setPosition(sf::Vector2f(490.f, 420.f));
    m_button.setFillColor(sf::Color(150, 0, 0, 200));
    m_button.setOutlineThickness(3.f);
    m_button.setOutlineColor(sf::Color::Red);
    
    std::string btnTexto = m_puedeReintentar ? "REINTENTAR" : "VOLVER AL MENU";
    m_buttonText = std::make_unique<sf::Text>(m_font, btnTexto, 28);
    m_buttonText->setFillColor(sf::Color::White);
    sf::FloatRect btnBounds = m_buttonText->getLocalBounds();
    m_buttonText->setOrigin(sf::Vector2f(btnBounds.size.x / 2.f, btnBounds.size.y / 2.f));
    m_buttonText->setPosition(sf::Vector2f(640.f, 450.f));
    
    std::cout << "MuerteCentinelaState creado. Reintentar: " << (m_puedeReintentar ? "SI" : "NO") << std::endl;
}

void MuerteCentinelaState::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
    
    // Hover del botón
    m_buttonHover = m_button.getGlobalBounds().contains(mousePos);
    m_button.setFillColor(m_buttonHover ? sf::Color(200, 0, 0, 200) : sf::Color(150, 0, 0, 200));
    m_button.setOutlineColor(m_buttonHover ? sf::Color::White : sf::Color::Red);
    
    // Click
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseEvent->button == sf::Mouse::Button::Left && m_buttonHover) {
            if (m_puedeReintentar) {
                // Modo agradable: reintentar centinela
                std::cout << "Reintentando centinela..." << std::endl;
                game->reintentarCentinela();
            } else {
                // Modo consecuencias: volver al menú
                std::cout << "Volviendo al menú principal..." << std::endl;
                game->returnToMenu();
            }
        }
    }
    
    // Tecla Enter o Espacio también funcionan
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Enter || 
            keyEvent->code == sf::Keyboard::Key::Space) {
            if (m_puedeReintentar) {
                game->reintentarCentinela();
            } else {
                game->returnToMenu();
            }
        }
    }
}

void MuerteCentinelaState::update(float dt) {
    // No necesita actualización
}

void MuerteCentinelaState::draw() {
    if (!window) return;
    
    window->setView(window->getDefaultView());
    
    // Fondo oscuro
    m_background.setSize(sf::Vector2f(
        static_cast<float>(window->getSize().x),
        static_cast<float>(window->getSize().y)
    ));
    window->draw(m_background);
    
    // Panel central centrado
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    float centerX = winW / 2.f;
    float centerY = winH / 2.f;
    
    float panelW = winW * 0.47f;
    float panelH = winH * 0.56f;
    m_panel.setSize(sf::Vector2f(panelW, panelH));
    m_panel.setPosition(sf::Vector2f(centerX - panelW / 2.f, centerY - panelH / 2.f));
    window->draw(m_panel);
    
    // Título
    if (m_title) {
        m_title->setPosition(sf::Vector2f(centerX, centerY - panelH * 0.25f));
        window->draw(*m_title);
    }
    
    // Mensaje
    if (m_message) {
        m_message->setPosition(sf::Vector2f(centerX, centerY));
        window->draw(*m_message);
    }
    
    // Botón
    float btnW = panelW * 0.55f;
    float btnH = panelH * 0.15f;
    m_button.setSize(sf::Vector2f(btnW, btnH));
    m_button.setPosition(sf::Vector2f(centerX - btnW / 2.f, centerY + panelH * 0.25f));
    window->draw(m_button);
    
    if (m_buttonText) {
        m_buttonText->setPosition(sf::Vector2f(centerX, centerY + panelH * 0.25f + btnH / 2.f));
        window->draw(*m_buttonText);
    }
}