#ifndef MENUSTATE_HPP
#define MENUSTATE_HPP

#include "State.hpp"
#include "Menu.hpp"
#include <memory>
#include <string>

class MenuState : public State {
private:
    std::unique_ptr<Menu> miMenu;
    bool mostrarConfig;
    int seleccionConfig;
    
    // Modo admin
    std::string m_adminInput;
    bool m_adminMode;
    sf::RectangleShape m_adminButton;
    std::unique_ptr<sf::Text> m_adminText;
    bool m_adminHover;
    
public:
    MenuState(sf::RenderWindow* window, Game* game);
    ~MenuState() override = default;

    void update(float deltaTiempo) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
};

#endif