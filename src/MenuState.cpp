#include "MenuState.hpp"
#include "Lobby.hpp"
#include "SaveSelectState.hpp"
#include "AdminMenuState.hpp"
#include "Game.hpp"  
#include <iostream>
#include <cctype>

MenuState::MenuState(sf::RenderWindow* window, Game* game) 
    : State(window, game), mostrarConfig(false), seleccionConfig(0),
      m_adminInput(""), m_adminMode(false), m_adminHover(false)
{
    window->setView(window->getDefaultView());
    miMenu = std::make_unique<Menu>(static_cast<float>(window->getSize().x), static_cast<float>(window->getSize().y));

    // Configurar botón admin
    m_adminButton.setSize(sf::Vector2f(80.f, 40.f));
    m_adminButton.setPosition(sf::Vector2f(10.f, 10.f));
    m_adminButton.setFillColor(sf::Color(100, 0, 0, 200));
    m_adminButton.setOutlineThickness(2.f);
    m_adminButton.setOutlineColor(sf::Color::Red);
    
    m_adminText = std::make_unique<sf::Text>(miMenu->getFuente(), "ADMIN", 16);
    m_adminText->setFillColor(sf::Color::White);
    sf::FloatRect textBounds = m_adminText->getLocalBounds();
    m_adminText->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
    m_adminText->setPosition(sf::Vector2f(50.f, 30.f));

    // Música del menú
    game->cambiarMusica("assets/sounds/menu.ogg");
}

void MenuState::handleEvent(const sf::Event& event) {
    // Detectar teclas para "admin"
    if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
        char c = static_cast<char>(textEntered->unicode);
        if (std::isalpha(static_cast<unsigned char>(c))) {
            m_adminInput += std::tolower(static_cast<unsigned char>(c));
            // Mantener solo últimos 5 caracteres
            if (m_adminInput.length() > 5) {
                m_adminInput = m_adminInput.substr(m_adminInput.length() - 5);
            }
            
            if (m_adminInput == "admin") {
                m_adminMode = true;
                m_adminInput = "";
                std::cout << "🔐 Modo administrador activado" << std::endl;
            }
        }
    }
}

void MenuState::update(float dt) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
    miMenu->actualizar(mousePos);

    // Si está en modo admin, mostrar botón extra
    if (m_adminMode) {
        sf::Vector2f mouseF = window->mapPixelToCoords(mousePos);
        m_adminHover = m_adminButton.getGlobalBounds().contains(mouseF);
        m_adminButton.setFillColor(m_adminHover ? sf::Color(150, 0, 0, 200) : sf::Color(100, 0, 0, 200));
    }

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

        if (seleccionConfig == 0) {
            float nuevoVol = game->getVolGeneral() + mod;
            if (nuevoVol < 0.f) nuevoVol = 0.f;
            if (nuevoVol > 100.f) nuevoVol = 100.f;
            game->setVolGeneral(nuevoVol);
        }
        if (seleccionConfig == 1) {
            float nuevoVol = game->getVolMusica() + mod;
            if (nuevoVol < 0.f) nuevoVol = 0.f;
            if (nuevoVol > 100.f) nuevoVol = 100.f;
            game->setVolMusica(nuevoVol);
        }
        if (seleccionConfig == 2) {
            float nuevoVol = game->getVolEfectos() + mod;
            if (nuevoVol < 0.f) nuevoVol = 0.f;
            if (nuevoVol > 100.f) nuevoVol = 100.f;
            game->setVolEfectos(nuevoVol);
        }
    }

    static bool clickProcesado = false;
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        if (!clickProcesado) {
            clickProcesado = true;
            
            // Botón admin
            if (m_adminMode && m_adminHover) {
                game->pushState(std::make_unique<AdminMenuState>(window, game));
                return;
            }
            
            // PRIORIDAD 1: Configuración (La tuerca)
            if (miMenu->verificarClickConfig(mousePos)) {
                mostrarConfig = !mostrarConfig;
            }
            // PRIORIDAD 2: Jugar
            else if (!mostrarConfig && miMenu->verificarClickJugar(mousePos)) {
                std::cout << "🎮 JUGAR - Abriendo selector de slots unificado" << std::endl;
                game->pushState(std::make_unique<SaveSelectState>(window, game, false));
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

    // Dibujar botón admin si está activado
    if (m_adminMode) {
        window->draw(m_adminButton);
        if (m_adminText) window->draw(*m_adminText);
    }

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