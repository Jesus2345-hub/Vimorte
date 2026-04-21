#include "PauseState.hpp"
#include "Game.hpp"
#include "SaveSelectState.hpp"
#include <iostream>
// Quitar #include <algorithm> si solo lo usas para clamp

PauseState::PauseState(sf::RenderWindow* window, Game* game) 
    : State(window, game), mostrarConfig(false), seleccionConfig(0) 
{
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente en pausa" << std::endl;
    }
    
    m_background.setSize({1280.f, 720.f});
    m_background.setFillColor(sf::Color(0, 0, 0, 180));
    
    // Panel central - más grande para todos los botones
    m_panel.setSize({500.f, 550.f});
    m_panel.setFillColor(sf::Color(30, 30, 50, 240));
    m_panel.setOutlineThickness(4);
    m_panel.setOutlineColor(sf::Color::White);
    m_panel.setPosition({390.f, 85.f});
    
    // Título
    m_title = std::make_unique<sf::Text>(m_font, "PAUSA", 50);
    m_title->setFillColor(sf::Color::Yellow);
    m_title->setPosition({640.f - m_title->getGlobalBounds().size.x / 2.f, 110.f});

    // Botón Continuar
    m_resumeText = std::make_unique<sf::Text>(m_font, "CONTINUAR", 30);
    m_resumeText->setPosition({640.f - m_resumeText->getGlobalBounds().size.x / 2.f, 200.f});

    // Botón Guardar Partida
    m_saveText = std::make_unique<sf::Text>(m_font, "GUARDAR PARTIDA", 30);
    m_saveText->setPosition({640.f - m_saveText->getGlobalBounds().size.x / 2.f, 270.f});

    // Botón Cargar Partida
    m_loadText = std::make_unique<sf::Text>(m_font, "CARGAR PARTIDA", 30);
    m_loadText->setPosition({640.f - m_loadText->getGlobalBounds().size.x / 2.f, 340.f});

    // Botón Ajustes
    m_configBtn = std::make_unique<sf::Text>(m_font, "AJUSTES", 30);
    m_configBtn->setPosition({640.f - m_configBtn->getGlobalBounds().size.x / 2.f, 410.f});

    // Botón Menú Principal
    m_menuText = std::make_unique<sf::Text>(m_font, "MENU PRINCIPAL", 30);
    m_menuText->setPosition({640.f - m_menuText->getGlobalBounds().size.x / 2.f, 480.f});

    // Botón Salir
    m_exitText = std::make_unique<sf::Text>(m_font, "SALIR DEL JUEGO", 30);
    m_exitText->setPosition({640.f - m_exitText->getGlobalBounds().size.x / 2.f, 550.f});
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
    
    window->draw(m_background);
    window->draw(m_panel);
    
    if (m_title) window->draw(*m_title);
    if (m_resumeText) window->draw(*m_resumeText);
    if (m_saveText) window->draw(*m_saveText);
    if (m_loadText) window->draw(*m_loadText);
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