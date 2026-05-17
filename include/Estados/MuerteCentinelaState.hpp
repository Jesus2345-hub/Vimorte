#ifndef MUERTECENTINELASTATE_HPP
#define MUERTECENTINELASTATE_HPP

#include "Estados/State.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

class MuerteCentinelaState : public State {
private:
    sf::Font m_font;
    sf::RectangleShape m_background;
    sf::RectangleShape m_panel;
    
    std::unique_ptr<sf::Text> m_title;
    std::unique_ptr<sf::Text> m_message;
    std::unique_ptr<sf::Text> m_buttonText;
    
    sf::RectangleShape m_button;
    bool m_buttonHover = false;
    
    bool m_puedeReintentar;
    
public:
    MuerteCentinelaState(sf::RenderWindow* window, Game* game, bool puedeReintentar);
    
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
};

#endif