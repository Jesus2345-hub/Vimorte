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
    std::unique_ptr<sf::Text> m_saveText;
    std::unique_ptr<sf::Text> m_loadText;
    std::unique_ptr<sf::Text> m_menuText;
    std::unique_ptr<sf::Text> m_exitText;
    
    bool mostrarConfig;
    int seleccionConfig;
    std::unique_ptr<sf::Text> m_configBtn;
    
    // Hover states
    bool m_resumeHover = false;
    bool m_saveHover = false;
    bool m_loadHover = false;
    bool m_menuHover = false;
    bool m_exitHover = false;
    bool m_configHover = false;
    
public:
    PauseState(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    ~PauseState() override = default;
};