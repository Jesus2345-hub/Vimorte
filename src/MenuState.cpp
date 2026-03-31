#include "MenuState.hpp"
#include "Lobby.hpp" 
#include "Game.hpp"  
#include <iostream>
#include <algorithm>

MenuState::MenuState(sf::RenderWindow* window, Game* game) 
    : State(window, game), mostrarConfig(false), seleccionConfig(0) 
{
    window->setView(window->getDefaultView());
    miMenu = std::make_unique<Menu>(static_cast<float>(window->getSize().x), static_cast<float>(window->getSize().y));

    if (m_menuMusic.openFromFile("assets/sounds/lobby.ogg")) {
        m_menuMusic.setLooping(true);
        m_menuMusic.setVolume(game->getRealMusica());
        m_menuMusic.play();
    }
}

void MenuState::update(float dt) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
    miMenu->actualizar(mousePos);

    if (mostrarConfig) {
        // Control de selección de barra
        static bool teclaPresionada = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) && !teclaPresionada) {
            seleccionConfig = (seleccionConfig + 1) % 3;
            teclaPresionada = true;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) && !teclaPresionada) {
            seleccionConfig = (seleccionConfig - 1 + 3) % 3;
            teclaPresionada = true;
        } else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            teclaPresionada = false;
        }

        // Control de volumen
        float mod = 0.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) mod = 0.5f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) mod = -0.5f;

        if (seleccionConfig == 0) game->setVolGeneral(std::clamp(game->getVolGeneral() + mod, 0.f, 100.f));
        if (seleccionConfig == 1) game->setVolMusica(std::clamp(game->getVolMusica() + mod, 0.f, 100.f));
        if (seleccionConfig == 2) game->setVolEfectos(std::clamp(game->getVolEfectos() + mod, 0.f, 100.f));

        m_menuMusic.setVolume(game->getRealMusica());
    }

    static bool clickProcesado = false;
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        if (!clickProcesado) {
            clickProcesado = true;
            
            // PRIORIDAD 1: Configuración (La tuerca)
            if (miMenu->verificarClickConfig(mousePos)) {
                mostrarConfig = !mostrarConfig;
            }
            // PRIORIDAD 2: Jugar (Solo si el menú de config está cerrado para evitar clics fantasma)
            else if (!mostrarConfig && miMenu->verificarClick(mousePos)) {
                std::cout << "BOTON JUGAR PRESIONADO" << std::endl;
                m_menuMusic.stop();
                game->changeState(std::make_unique<LobbyState>(window, game));
                return; 
            }
            // PRIORIDAD 3: Salir
            else if (miMenu->verificarClickSalir(mousePos)) {
                window->close();
            }
        }
    } else {
        clickProcesado = false;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) mostrarConfig = false;
}

void MenuState::draw() {
    window->setView(window->getDefaultView());
    miMenu->dibujar(*window);

    if (mostrarConfig) {
        sf::RectangleShape fondo({500.f, 350.f});
        fondo.setPosition({390.f, 185.f});
        fondo.setFillColor(sf::Color(20, 20, 20, 240));
        fondo.setOutlineThickness(3);
        window->draw(fondo);

        auto drawBar = [&](std::string name, float val, float y, bool sel) {
            sf::Text t(miMenu->getFuente(), name + ": " + std::to_string((int)val), 20);
            t.setPosition({420.f, y});
            t.setFillColor(sel ? sf::Color::Yellow : sf::Color::White);
            window->draw(t);

            sf::RectangleShape b({300.f, 10.f});
            b.setPosition({420.f, y + 40.f});
            b.setFillColor(sf::Color(100, 100, 100));
            window->draw(b);

            sf::RectangleShape p({(val / 100.f) * 300.f, 10.f});
            p.setPosition({420.f, y + 40.f});
            p.setFillColor(sel ? sf::Color::Yellow : sf::Color::Red);
            window->draw(p);
        };

        drawBar("GENERAL", game->getVolGeneral(), 220.f, seleccionConfig == 0);
        drawBar("MUSICA", game->getVolMusica(), 300.f, seleccionConfig == 1);
        drawBar("EFECTOS", game->getVolEfectos(), 380.f, seleccionConfig == 2);
    }
}