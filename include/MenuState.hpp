#ifndef MENUSTATE_HPP
#define MENUSTATE_HPP

#include "State.hpp"
#include "Menu.hpp"
#include <memory>

class MenuState : public State {
private:
    std::unique_ptr<Menu> miMenu;
    bool mostrarConfig;
    int seleccionConfig;

public:
    MenuState(sf::RenderWindow* window, Game* game);
    ~MenuState() override = default;

    void update(float deltaTiempo) override;
    void draw() override;
};

#endif