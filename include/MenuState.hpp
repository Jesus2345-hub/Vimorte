#pragma once
#include "State.hpp"
#include "Menu.hpp"
#include <memory>

class MenuState : public State 
{
private:
    std::unique_ptr<Menu> miMenu;
    bool mostrarConfig; // Para controlar la visibilidad de la configuración

public:
    MenuState(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    ~MenuState() override = default;
};