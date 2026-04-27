#include "PauseState.hpp"
#include "Game.hpp"
#include "SaveSelectState.hpp"
#include "MenuState.hpp"
#include <iostream>

PauseState::PauseState(sf::RenderWindow* window, Game* game) 
    : State(window, game), mostrarConfig(false), seleccionConfig(0) 
{
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente en pausa" << std::endl;
    }
    
    m_background.setSize({1280.f, 720.f});
    m_background.setFillColor(sf::Color(0, 0, 0, 180));
    
    // Panel central
    m_panel.setSize({500.f, 550.f});
    m_panel.setFillColor(sf::Color(30, 30, 50, 240));
    m_panel.setOutlineThickness(4);
    m_panel.setOutlineColor(sf::Color::White);
    m_panel.setPosition({390.f, 85.f});
    
    // Título
    m_title = std::make_unique<sf::Text>(m_font, "PAUSA", 50);
    m_title->setFillColor(sf::Color::Yellow);
    sf::FloatRect titleBounds = m_title->getLocalBounds();
    m_title->setOrigin({titleBounds.size.x / 2.f, 0.f});
    m_title->setPosition({640.f, 110.f});

    // Botón Continuar
    m_resumeText = std::make_unique<sf::Text>(m_font, "CONTINUAR", 30);
    m_resumeText->setFillColor(sf::Color::White);
    sf::FloatRect resumeBounds = m_resumeText->getLocalBounds();
    m_resumeText->setOrigin({resumeBounds.size.x / 2.f, 0.f});
    m_resumeText->setPosition({640.f, 200.f});

    // Botón Guardar Partida
    m_saveText = std::make_unique<sf::Text>(m_font, "GUARDAR PARTIDA", 30);
    m_saveText->setFillColor(sf::Color::White);
    sf::FloatRect saveBounds = m_saveText->getLocalBounds();
    m_saveText->setOrigin({saveBounds.size.x / 2.f, 0.f});
    m_saveText->setPosition({640.f, 270.f});

    // Botón Cargar Partida
    m_loadText = std::make_unique<sf::Text>(m_font, "CARGAR PARTIDA", 30);
    m_loadText->setFillColor(sf::Color::White);
    sf::FloatRect loadBounds = m_loadText->getLocalBounds();
    m_loadText->setOrigin({loadBounds.size.x / 2.f, 0.f});
    m_loadText->setPosition({640.f, 340.f});

    // Botón Ajustes
    m_configBtn = std::make_unique<sf::Text>(m_font, "AJUSTES", 30);
    m_configBtn->setFillColor(sf::Color::White);
    sf::FloatRect configBounds = m_configBtn->getLocalBounds();
    m_configBtn->setOrigin({configBounds.size.x / 2.f, 0.f});
    m_configBtn->setPosition({640.f, 410.f});

    // Botón Menú Principal
    m_menuText = std::make_unique<sf::Text>(m_font, "MENU PRINCIPAL", 30);
    m_menuText->setFillColor(sf::Color::White);
    sf::FloatRect menuBounds = m_menuText->getLocalBounds();
    m_menuText->setOrigin({menuBounds.size.x / 2.f, 0.f});
    m_menuText->setPosition({640.f, 480.f});

    // Botón Salir
    m_exitText = std::make_unique<sf::Text>(m_font, "SALIR DEL JUEGO", 30);
    m_exitText->setFillColor(sf::Color::White);
    sf::FloatRect exitBounds = m_exitText->getLocalBounds();
    m_exitText->setOrigin({exitBounds.size.x / 2.f, 0.f});
    m_exitText->setPosition({640.f, 550.f});
}

void PauseState::update(float dt) {
    sf::Vector2f mouseF = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
    
    m_resumeHover = !mostrarConfig && m_resumeText->getGlobalBounds().contains(mouseF);
    m_saveHover = !mostrarConfig && m_saveText->getGlobalBounds().contains(mouseF);
    m_loadHover = !mostrarConfig && m_loadText->getGlobalBounds().contains(mouseF);
    m_configHover = !mostrarConfig && m_configBtn->getGlobalBounds().contains(mouseF);
    m_menuHover = !mostrarConfig && m_menuText->getGlobalBounds().contains(mouseF);
    m_exitHover = !mostrarConfig && m_exitText->getGlobalBounds().contains(mouseF);

    m_resumeText->setFillColor(m_resumeHover ? sf::Color::Yellow : sf::Color::White);
    m_saveText->setFillColor(m_saveHover ? sf::Color::Green : sf::Color::White);
    m_loadText->setFillColor(m_loadHover ? sf::Color::Cyan : sf::Color::White);
    m_configBtn->setFillColor(m_configHover ? sf::Color::Yellow : sf::Color::White);
    m_menuText->setFillColor(m_menuHover ? sf::Color::Yellow : sf::Color::White);
    m_exitText->setFillColor(m_exitHover ? sf::Color::Red : sf::Color::White);

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

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
            mostrarConfig = false;
        }
    }

    static bool clickProcesado = false;
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        if (!clickProcesado) {
            clickProcesado = true;
            if (!mostrarConfig) {
                if (m_resumeHover) {
                    game->popState();
                }
                if (m_saveHover) {
                    if (game->tienePartidaActiva()) {
                        game->guardarPartidaActual();
                        std::cout << "💾 Partida guardada exitosamente" << std::endl;
                    } else {
                        std::cout << "⚠️ No hay partida activa para guardar" << std::endl;
                    }
                }
                if (m_loadHover) {
                    game->pushState(std::make_unique<SaveSelectState>(window, game, true));
                }
                if (m_configHover) {
                    mostrarConfig = true;
                }
                if (m_menuHover) {
                    if (game->tienePartidaActiva()) {
                        game->guardarPartidaActual();
                    }
                    game->returnToMenu();
                }
                if (m_exitHover) {
                    if (game->tienePartidaActiva()) {
                        game->guardarPartidaActual();
                    }
                    window->close();
                }
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
    
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    float centerX = winW / 2.f;
    float centerY = winH / 2.f;
    
    m_background.setSize(sf::Vector2f(winW, winH));
    window->draw(m_background);
    
    float panelW = winW * 0.39f;
    float panelH = winH * 0.76f;
    m_panel.setSize(sf::Vector2f(panelW, panelH));
    m_panel.setPosition(sf::Vector2f(centerX - panelW/2.f, winH * 0.12f));
    window->draw(m_panel);
    
    auto drawCenteredText = [&](std::unique_ptr<sf::Text>& text, float y) {
        if (text) {
            sf::FloatRect bounds = text->getLocalBounds();
            text->setOrigin(sf::Vector2f(bounds.size.x/2.f, 0.f));
            text->setPosition(sf::Vector2f(centerX, y));
            window->draw(*text);
        }
    };
    
    float btnY = winH * 0.28f;
    float btnSpacing = winH * 0.097f;
    
    drawCenteredText(m_resumeText, btnY);
    drawCenteredText(m_saveText, btnY + btnSpacing);
    drawCenteredText(m_loadText, btnY + btnSpacing * 2);
    drawCenteredText(m_configBtn, btnY + btnSpacing * 3);
    drawCenteredText(m_menuText, btnY + btnSpacing * 4);
    drawCenteredText(m_exitText, btnY + btnSpacing * 5);
    
    if (m_title) {
        sf::FloatRect tb = m_title->getLocalBounds();
        m_title->setOrigin(sf::Vector2f(tb.size.x/2.f, 0.f));
        m_title->setPosition(sf::Vector2f(centerX, winH * 0.15f));
        window->draw(*m_title);
    }

    if (mostrarConfig) {
        float configW = panelW;
        float configH = winH * 0.49f;
        sf::RectangleShape fondoAjustes(sf::Vector2f(configW, configH));
        fondoAjustes.setPosition(sf::Vector2f(centerX - configW/2.f, winH * 0.26f));
        fondoAjustes.setFillColor(sf::Color(20, 20, 20, 255));
        fondoAjustes.setOutlineThickness(3);
        fondoAjustes.setOutlineColor(sf::Color::Red);
        window->draw(fondoAjustes);

        auto drawBar = [&](const std::string& name, float val, float y, bool sel) {
            sf::Text t(m_font, name + ": " + std::to_string((int)val), 18);
            t.setPosition(sf::Vector2f(centerX - configW/2.f + 30.f, y));
            t.setFillColor(sel ? sf::Color::Yellow : sf::Color::White);
            window->draw(t);

            sf::RectangleShape fondoBarra(sf::Vector2f(configW - 80.f, 10.f));
            fondoBarra.setPosition(sf::Vector2f(centerX - configW/2.f + 30.f, y + 35.f));
            fondoBarra.setFillColor(sf::Color(100, 100, 100));
            window->draw(fondoBarra);

            sf::RectangleShape progreso(sf::Vector2f((val / 100.f) * (configW - 80.f), 10.f));
            progreso.setPosition(sf::Vector2f(centerX - configW/2.f + 30.f, y + 35.f));
            progreso.setFillColor(sel ? sf::Color::Yellow : sf::Color::Red);
            window->draw(progreso);
        };

        drawBar("GENERAL", game->getVolGeneral(), winH * 0.31f, seleccionConfig == 0);
        drawBar("MUSICA", game->getVolMusica(), winH * 0.42f, seleccionConfig == 1);
        drawBar("EFECTOS", game->getVolEfectos(), winH * 0.53f, seleccionConfig == 2);
    }
}