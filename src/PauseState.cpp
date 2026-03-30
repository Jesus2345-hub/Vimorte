#include "PauseState.hpp"
#include "Game.hpp"

PauseState::PauseState(sf::RenderWindow* window, Game* game) 
    : State(window, game)
{
    // Cargar fuente (usa la misma que el menú)
    if (!m_font.openFromFile("assets/fonts/VCR_OSD_MONO.ttf")) {
        // Fallback a arial si no existe
        m_font.openFromFile("assets/fonts/arial.ttf");
    }
    
    // Fondo semi-transparente oscuro
    m_background.setSize(sf::Vector2f(1280, 720));
    m_background.setFillColor(sf::Color(0, 0, 0, 180));
    
    // Panel central
    m_panel.setSize(sf::Vector2f(500, 380));
    m_panel.setFillColor(sf::Color(30, 30, 50, 240));
    m_panel.setOutlineThickness(4);
    m_panel.setOutlineColor(sf::Color::White);
    m_panel.setPosition(sf::Vector2f(390, 170));
    
    // Título
    m_title = std::make_unique<sf::Text>(m_font, "PAUSA", 56);
    m_title->setFillColor(sf::Color::Yellow);
    sf::FloatRect titleBounds = m_title->getLocalBounds();
    m_title->setPosition(sf::Vector2f(640 - titleBounds.size.x / 2, 210));
    
    // Botón "Continuar"
    m_resumeText = std::make_unique<sf::Text>(m_font, "CONTINUAR", 32);
    m_resumeText->setFillColor(sf::Color::White);
    sf::FloatRect resumeBounds = m_resumeText->getLocalBounds();
    m_resumeText->setPosition(sf::Vector2f(640 - resumeBounds.size.x / 2, 310));
    
    // Botón "Menú Principal"
    m_menuText = std::make_unique<sf::Text>(m_font, "MENU PRINCIPAL", 32);
    m_menuText->setFillColor(sf::Color::White);
    sf::FloatRect menuBounds = m_menuText->getLocalBounds();
    m_menuText->setPosition(sf::Vector2f(640 - menuBounds.size.x / 2, 390));
    
    // Botón "Salir" (opcional)
    m_exitText = std::make_unique<sf::Text>(m_font, "SALIR", 32);
    m_exitText->setFillColor(sf::Color::White);
    sf::FloatRect exitBounds = m_exitText->getLocalBounds();
    m_exitText->setPosition(sf::Vector2f(640 - exitBounds.size.x / 2, 470));
}

void PauseState::update(float dt) 
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
    sf::Vector2f mouseF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    
    // Detectar hover en botones
    sf::FloatRect resumeBounds = m_resumeText->getGlobalBounds();
    sf::FloatRect menuBounds = m_menuText->getGlobalBounds();
    sf::FloatRect exitBounds = m_exitText->getGlobalBounds();
    
    bool resumeHover = resumeBounds.contains(mouseF);
    bool menuHover = menuBounds.contains(mouseF);
    bool exitHover = exitBounds.contains(mouseF);
    
    // Cambiar color al hacer hover
    m_resumeText->setFillColor(resumeHover ? sf::Color(200, 200, 100) : sf::Color::White);
    m_menuText->setFillColor(menuHover ? sf::Color(200, 200, 100) : sf::Color::White);
    m_exitText->setFillColor(exitHover ? sf::Color(200, 200, 100) : sf::Color::White);
    
    // Detectar clic
    static bool clickProcesado = false;
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        if (!clickProcesado) {
            clickProcesado = true;
            
            if (resumeHover) {
                game->popState();  // Volver al juego
            }
            else if (menuHover) {
                game->returnToMenu();
            }
            else if (exitHover) {
                window->close();
            }
        }
    } else {
        clickProcesado = false;
    }
    
    // ESC para cerrar pausa (volver al juego)
    static bool escapeProcesado = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        if (!escapeProcesado) {
            escapeProcesado = true;
            game->popState();
        }
    } else {
        escapeProcesado = false;
    }
}

void PauseState::draw()
{
    if (!window) return;
    
    // Fondo oscuro
    window->draw(m_background);
    
    // Panel
    window->draw(m_panel);
    
    // Textos
    if (m_title) window->draw(*m_title);
    if (m_resumeText) window->draw(*m_resumeText);
    if (m_menuText) window->draw(*m_menuText);
    if (m_exitText) window->draw(*m_exitText);
}
