#include "MuerteCentinelaState.hpp"
#include "Game.hpp"
#include "MenuState.hpp"
#include <iostream>

MuerteCentinelaState::MuerteCentinelaState(sf::RenderWindow* window, Game* game, bool puedeReintentar)
    : State(window, game), m_puedeReintentar(puedeReintentar) {
    
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "❌ Error cargando fuente en MuerteCentinelaState" << std::endl;
    }
    
    // Fondo oscuro
    m_background.setSize(sf::Vector2f(1280.f, 720.f));
    m_background.setFillColor(sf::Color(0, 0, 0, 200));
    
    // Panel central
    m_panel.setSize(sf::Vector2f(600.f, 350.f));
    m_panel.setPosition(sf::Vector2f(340.f, 185.f));
    m_panel.setFillColor(sf::Color(30, 30, 50, 240));
    m_panel.setOutlineThickness(3.f);
    m_panel.setOutlineColor(sf::Color::Red);
    
    // Título
    m_title = std::make_unique<sf::Text>(m_font, "HAS FRACASADO", 40);
    m_title->setFillColor(sf::Color::Red);
    sf::FloatRect titleBounds = m_title->getLocalBounds();
    m_title->setOrigin(sf::Vector2f(titleBounds.size.x / 2.f, 0.f));
    m_title->setPosition(sf::Vector2f(640.f, 220.f));
    
    // Mensaje según el modo
    if (m_puedeReintentar) {
        m_message = std::make_unique<sf::Text>(m_font,
            "Elegiste el Camino Agradable.\n\n"
            "Puedes volver a intentar el centinela desde\n"
            "el punto de guardado anterior.", 18);
        m_message->setFillColor(sf::Color(200, 255, 200));
        
        m_buttonText = std::make_unique<sf::Text>(m_font, "REINTENTAR CENTINELA", 22);
    } else {
        m_message = std::make_unique<sf::Text>(m_font,
            "Elegiste el Camino con Consecuencias.\n\n"
            "Tu decision fue permanente.\n"
            "No hay vuelta atras.", 18);
        m_message->setFillColor(sf::Color(255, 200, 200));
        
        m_buttonText = std::make_unique<sf::Text>(m_font, "VOLVER AL MENU", 22);
    }
    
    sf::FloatRect msgBounds = m_message->getLocalBounds();
    m_message->setOrigin(sf::Vector2f(msgBounds.size.x / 2.f, 0.f));
    m_message->setPosition(sf::Vector2f(640.f, 290.f));
    
    // Botón
    m_button.setSize(sf::Vector2f(300.f, 60.f));
    m_button.setPosition(sf::Vector2f(490.f, 420.f));
    m_button.setFillColor(sf::Color(100, 0, 0, 200));
    m_button.setOutlineThickness(2.f);
    m_button.setOutlineColor(sf::Color::Red);
    
    m_buttonText->setFillColor(sf::Color::White);
    sf::FloatRect btnBounds = m_buttonText->getLocalBounds();
    m_buttonText->setOrigin(sf::Vector2f(btnBounds.size.x / 2.f, btnBounds.size.y / 2.f));
    m_buttonText->setPosition(sf::Vector2f(640.f, 450.f));
    
    std::cout << "💀 Muerte en centinela. Puede reintentar: " << (m_puedeReintentar ? "SI" : "NO") << std::endl;
}

void MuerteCentinelaState::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
    
    m_buttonHover = m_button.getGlobalBounds().contains(mousePos);
    m_button.setFillColor(m_buttonHover ? sf::Color(150, 0, 0, 200) : sf::Color(100, 0, 0, 200));
    
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Left && m_buttonHover) {
            if (m_puedeReintentar) {
                // Reintentar centinela
                game->reintentarCentinela();
                std::cout << "🔄 Reintentando centinela desde checkpoint" << std::endl;
            } else {
                // Volver al menú
                game->returnToMenu();
                std::cout << "🏠 Volviendo al menu principal" << std::endl;
            }
        }
    }
    
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            game->returnToMenu();
        }
        if (keyEvent->code == sf::Keyboard::Key::Enter) {
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
    
    window->draw(m_background);
    window->draw(m_panel);
    window->draw(*m_title);
    window->draw(*m_message);
    window->draw(m_button);
    window->draw(*m_buttonText);
}