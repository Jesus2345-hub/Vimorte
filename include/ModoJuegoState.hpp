#ifndef MODOJUEGOSTATE_HPP
#define MODOJUEGOSTATE_HPP

#include "State.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

class ModoJuegoState : public State {
private:
    sf::Font m_font;
    sf::RectangleShape m_background;
    sf::RectangleShape m_panel;
    
    std::unique_ptr<sf::Text> m_title;
    std::unique_ptr<sf::Text> m_description;
    
    std::unique_ptr<sf::Text> m_opcionAgradableTitulo;
    std::unique_ptr<sf::Text> m_opcionAgradableDesc;
    std::unique_ptr<sf::Text> m_opcionConsecuenciasTitulo;
    std::unique_ptr<sf::Text> m_opcionConsecuenciasDesc;
    
    sf::RectangleShape m_botonAgradable;
    sf::RectangleShape m_botonConsecuencias;
    
    bool m_hoverAgradable = false;
    bool m_hoverConsecuencias = false;
    
    std::string m_nombreJugador;
    int m_slotId;
    
public:
    ModoJuegoState(sf::RenderWindow* window, Game* game, const std::string& nombre, int slotId);
    
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
};

#endif