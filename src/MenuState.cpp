#include "MenuState.hpp"
#include "Lobby.hpp" 
#include "Game.hpp"  
#include <iostream>

MenuState::MenuState(sf::RenderWindow* window, Game* game) : State(window, game) 
{
    miMenu = std::make_unique<Menu>(static_cast<float>(window->getSize().x), static_cast<float>(window->getSize().y));
    mostrarConfig = false;
}

void MenuState::update(float deltaTiempo) 
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
    miMenu->actualizar(mousePos);

    // Lógica de un solo clic (evita que el programa se vuelva loco)
    static bool procesandoClick = false;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        if (!procesandoClick) {
            procesandoClick = true;

            // BOTÓN ENTRAR -> IR AL LOBBY
            if (miMenu->verificarClick(mousePos)) {
                std::cout << "Cambiando a LobbyState..." << std::endl;
                // Usamos la ventana y el game que recibimos en el constructor
                // Usa la variable 'window' que heredaste de State, no 'this->window'
				game->changeState(std::make_unique<LobbyState>(window, game));
                return; // Salimos de la función inmediatamente para evitar errores
            }

            // BOTÓN CONFIGURACIÓN
            if (miMenu->verificarClickConfig(mousePos)) {
                mostrarConfig = !mostrarConfig;
            }

            // BOTÓN SALIR
            if (miMenu->verificarClickSalir(mousePos)) {
                window->close();
            }
        }
    } else {
        procesandoClick = false;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        mostrarConfig = false;
    }
}

void MenuState::draw()
{
    miMenu->dibujar(*window);

    if (mostrarConfig) {
        sf::RectangleShape cuadroConfig({600.f, 400.f});
        cuadroConfig.setFillColor(sf::Color(40, 40, 40, 240));
        cuadroConfig.setOutlineThickness(3);
        cuadroConfig.setOutlineColor(sf::Color::White);
        cuadroConfig.setPosition({340.f, 160.f});
        window->draw(cuadroConfig);
    }
}