#pragma once
#include "State.hpp"
#include "MenuState.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

class PauseState : public State 
{
private:
    sf::RectangleShape m_background;
    sf::RectangleShape m_panel;
    
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_title;
    std::unique_ptr<sf::Text> m_resumeText;
    std::unique_ptr<sf::Text> m_menuText;
    std::unique_ptr<sf::Text> m_exitText;
    bool mostrarConfig;
    int seleccionConfig;
    std::unique_ptr<sf::Text> m_configBtn;
    
public:
    PauseState(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    ~PauseState() override = default;
};
