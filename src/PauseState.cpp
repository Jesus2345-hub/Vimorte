#include "PauseState.hpp"
#include "Game.hpp"
#include <iostream>
#include <algorithm>

PauseState::PauseState(sf::RenderWindow* window, Game* game) 
    : State(window, game), mostrarConfig(false), seleccionConfig(0) 
{
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente en pausa" << std::endl;
    }
    
    m_background.setSize({1280.f, 720.f});
    m_background.setFillColor(sf::Color(0, 0, 0, 180));
    
    // Panel central un poco más grande para que quepan todos los botones
    m_panel.setSize({500.f, 500.f});
    m_panel.setFillColor(sf::Color(30, 30, 50, 240));
    m_panel.setOutlineThickness(4);
    m_panel.setOutlineColor(sf::Color::White);
    m_panel.setPosition({390.f, 110.f});
    
    // Inicialización del Título (Esto faltaba en tu constructor)
    m_title = std::make_unique<sf::Text>(m_font, "PAUSA", 50);
    m_title->setFillColor(sf::Color::Yellow);
    m_title->setPosition({640.f - m_title->getGlobalBounds().size.x / 2.f, 140.f});

    // Botón Continuar
    m_resumeText = std::make_unique<sf::Text>(m_font, "CONTINUAR", 35);
    m_resumeText->setPosition({640.f - m_resumeText->getGlobalBounds().size.x / 2.f, 230.f});

    // Botón Ajustes
    m_configBtn = std::make_unique<sf::Text>(m_font, "AJUSTES", 35);
    m_configBtn->setPosition({640.f - m_configBtn->getGlobalBounds().size.x / 2.f, 305.f});

    // Botón Menú Principal
    m_menuText = std::make_unique<sf::Text>(m_font, "MENU PRINCIPAL", 35);
    m_menuText->setPosition({640.f - m_menuText->getGlobalBounds().size.x / 2.f, 380.f});

    // Botón Salir
    m_exitText = std::make_unique<sf::Text>(m_font, "SALIR DEL JUEGO", 35);
    m_exitText->setPosition({640.f - m_exitText->getGlobalBounds().size.x / 2.f, 455.f});
}

void PauseState::update(float dt) {
    sf::Vector2f mouseF = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
    
    bool resumeHover = !mostrarConfig && m_resumeText->getGlobalBounds().contains(mouseF);
    bool configHover = !mostrarConfig && m_configBtn->getGlobalBounds().contains(mouseF);
    bool menuHover   = !mostrarConfig && m_menuText->getGlobalBounds().contains(mouseF);
    bool exitHover   = !mostrarConfig && m_exitText->getGlobalBounds().contains(mouseF);

    m_resumeText->setFillColor(resumeHover ? sf::Color::Yellow : sf::Color::White);
    m_configBtn->setFillColor(configHover ? sf::Color::Yellow : sf::Color::White);
    m_menuText->setFillColor(menuHover ? sf::Color::Yellow : sf::Color::White);
    m_exitText->setFillColor(exitHover ? sf::Color::Red : sf::Color::White);

    if (mostrarConfig) {
        static bool teclaNavegacionPresionada = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) && !teclaNavegacionPresionada) {
            seleccionConfig = (seleccionConfig + 1) % 3;
            teclaNavegacionPresionada = true;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) && !teclaNavegacionPresionada) {
            seleccionConfig = (seleccionConfig - 1 + 3) % 3;
            teclaNavegacionPresionada = true;
        } else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            teclaNavegacionPresionada = false;
        }

        float mod = 0.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) mod = 0.5f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) mod = -0.5f;

        if (mod != 0.f) {
            if (seleccionConfig == 0) game->setVolGeneral(std::clamp(game->getVolGeneral() + mod, 0.f, 100.f));
            if (seleccionConfig == 1) game->setVolMusica(std::clamp(game->getVolMusica() + mod, 0.f, 100.f));
            if (seleccionConfig == 2) game->setVolEfectos(std::clamp(game->getVolEfectos() + mod, 0.f, 100.f));
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
            mostrarConfig = false;
        }
    }

    static bool clickProcesado = false;
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        if (!clickProcesado) {
            clickProcesado = true;
            if (!mostrarConfig) {
                if (resumeHover) game->popState();
                if (configHover) mostrarConfig = true; 
                if (menuHover)   game->returnToMenu();
                if (exitHover)   window->close();
            }
        }
    } else {
        clickProcesado = false;
    }

    static bool escPausaPresionado = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        if (!escPausaPresionado && !mostrarConfig) {
            escPausaPresionado = true;
            game->popState();
        }
    } else {
        escPausaPresionado = false;
    }
}

void PauseState::draw()
{
    if (!window) return;
    
    window->draw(m_background);
    window->draw(m_panel);
    
    // Dibujar textos (ahora todos están inicializados)
    if (m_title) window->draw(*m_title);
    if (m_resumeText) window->draw(*m_resumeText);
    if (m_configBtn) window->draw(*m_configBtn);
    if (m_menuText) window->draw(*m_menuText);
    if (m_exitText) window->draw(*m_exitText);

    if (mostrarConfig) {
        sf::RectangleShape fondoAjustes({500.f, 350.f});
        fondoAjustes.setPosition({390.f, 185.f});
        fondoAjustes.setFillColor(sf::Color(20, 20, 20, 255)); 
        fondoAjustes.setOutlineThickness(3);
        fondoAjustes.setOutlineColor(sf::Color::Red);
        window->draw(fondoAjustes);

        auto drawBar = [&](std::string name, float val, float y, bool sel) {
            sf::Text t(m_font, name + ": " + std::to_string((int)val), 20);
            t.setPosition({420.f, y});
            t.setFillColor(sel ? sf::Color::Yellow : sf::Color::White);
            window->draw(t);

            sf::RectangleShape fondoBarra({300.f, 10.f});
            fondoBarra.setPosition({420.f, y + 40.f});
            fondoBarra.setFillColor(sf::Color(100, 100, 100));
            window->draw(fondoBarra);

            sf::RectangleShape progreso({(val / 100.f) * 300.f, 10.f});
            progreso.setPosition({420.f, y + 40.f});
            progreso.setFillColor(sel ? sf::Color::Yellow : sf::Color::Red);
            window->draw(progreso);
        };

        drawBar("GENERAL", game->getVolGeneral(), 220.f, seleccionConfig == 0);
        drawBar("MUSICA", game->getVolMusica(), 300.f, seleccionConfig == 1);
        drawBar("EFECTOS", game->getVolEfectos(), 380.f, seleccionConfig == 2);
    }
}