#pragma once
#include "Estados/State.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

class GameOverState : public State
{
private:
    sf::RectangleShape m_fondo;
    sf::Font m_fuente;
    
    std::unique_ptr<sf::Text> m_titulo;
    std::unique_ptr<sf::Text> m_tituloSombra;  // NUEVO: sombra 3D
    std::unique_ptr<sf::Text> m_subtitulo;     // NUEVO: texto "Has perdido el nivel"
    std::unique_ptr<sf::Text> m_reintentarText;
    std::unique_ptr<sf::Text> m_menuText;
    
    sf::RectangleShape m_botonReintentar;
    sf::RectangleShape m_botonMenu;
    
    bool m_hoverReintentar = false;
    bool m_hoverMenu = false;
    
    // Para animación
    float m_brilloReintentar;
    float m_brilloMenu;
    float m_tiempoAnimacion;
    
public:
    GameOverState(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
};