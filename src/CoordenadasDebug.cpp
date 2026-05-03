#include "CoordenadasDebug.hpp"

CoordenadasDebug& CoordenadasDebug::getInstance() {
    static CoordenadasDebug instance;
    return instance;
}

CoordenadasDebug::CoordenadasDebug() {
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (m_fontLoaded) {
        m_texto = std::make_unique<sf::Text>(m_font);
        m_texto->setCharacterSize(16);
        m_texto->setFillColor(sf::Color::Cyan);
        m_texto->setPosition(sf::Vector2f(10.f, 10.f));
    }
}

void CoordenadasDebug::actualizar(sf::RenderWindow* window, const sf::View& camara, const sf::Vector2f& posicionJugador) {
    if (!m_fontLoaded || !window || !m_visible) return;
    
    sf::Vector2i mousePosPantalla = sf::Mouse::getPosition(*window);
    sf::Vector2f mousePosMundo = window->mapPixelToCoords(mousePosPantalla, camara);
    
    std::string coordsText = "Mouse: X=" + std::to_string((int)mousePosMundo.x) + 
                             " Y=" + std::to_string((int)mousePosMundo.y);
    coordsText += "\nJugador: X=" + std::to_string((int)posicionJugador.x) + 
                  " Y=" + std::to_string((int)posicionJugador.y);
    
    m_texto->setString(coordsText);
}

void CoordenadasDebug::dibujar(sf::RenderWindow& window) {
    if (m_fontLoaded && m_texto && m_visible) {
        window.draw(*m_texto);
    }
}