#ifndef COORDENADASDEBUG_HPP
#define COORDENADASDEBUG_HPP

#include <SFML/Graphics.hpp>
#include <memory>

class CoordenadasDebug {
public:
    static CoordenadasDebug& getInstance();
    
    void actualizar(sf::RenderWindow* window, const sf::View& camara, const sf::Vector2f& posicionJugador);
    void dibujar(sf::RenderWindow& window);
    void setVisible(bool visible) { m_visible = visible; }
    void toggleVisible() { m_visible = !m_visible; }
	bool isVisible() const { return m_visible; }
private:
    CoordenadasDebug();
    sf::Font m_font;
    bool m_fontLoaded = false;
    std::unique_ptr<sf::Text> m_texto;
    bool m_visible = false;  // Empieza oculto, se activa con tecla
};

#endif