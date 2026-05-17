#include "Estados/PauseState.hpp"
#include "Configuracion/Game.hpp"
#include "Estados/SaveSelectState.hpp"
#include "Estados/MenuState.hpp"
#include <iostream>

PauseState::PauseState(sf::RenderWindow* window, Game* game) 
    : State(window, game), mostrarConfig(false), seleccionConfig(0), 
      mostrarGuia(false), m_closeGuideHover(false)
{
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente en pausa" << std::endl;
    }
    
    m_background.setSize({1280.f, 720.f});
    m_background.setFillColor(sf::Color(0, 0, 0, 180));
    
    // Panel central
    m_panel.setSize({500.f, 600.f});  // Un poco más alto para el botón GUÍA
    m_panel.setFillColor(sf::Color(30, 30, 50, 240));
    m_panel.setOutlineThickness(4);
    m_panel.setOutlineColor(sf::Color::White);
    m_panel.setPosition({390.f, 60.f});
    
    // Título
    m_title = std::make_unique<sf::Text>(m_font, "PAUSA", 50);
    m_title->setFillColor(sf::Color::Yellow);
    sf::FloatRect titleBounds = m_title->getLocalBounds();
    m_title->setOrigin({titleBounds.size.x / 2.f, 0.f});
    m_title->setPosition({640.f, 85.f});

    // Botón Continuar
    m_resumeText = std::make_unique<sf::Text>(m_font, "CONTINUAR", 30);
    m_resumeText->setFillColor(sf::Color::White);
    sf::FloatRect resumeBounds = m_resumeText->getLocalBounds();
    m_resumeText->setOrigin({resumeBounds.size.x / 2.f, 0.f});
    m_resumeText->setPosition({640.f, 175.f});

    // Botón Guardar Partida
    m_saveText = std::make_unique<sf::Text>(m_font, "GUARDAR PARTIDA", 30);
    m_saveText->setFillColor(sf::Color::White);
    sf::FloatRect saveBounds = m_saveText->getLocalBounds();
    m_saveText->setOrigin({saveBounds.size.x / 2.f, 0.f});
    m_saveText->setPosition({640.f, 235.f});

    // Botón Cargar Partida
    m_loadText = std::make_unique<sf::Text>(m_font, "CARGAR PARTIDA", 30);
    m_loadText->setFillColor(sf::Color::White);
    sf::FloatRect loadBounds = m_loadText->getLocalBounds();
    m_loadText->setOrigin({loadBounds.size.x / 2.f, 0.f});
    m_loadText->setPosition({640.f, 295.f});

    // Botón Ajustes
    m_configBtn = std::make_unique<sf::Text>(m_font, "AJUSTES", 30);
    m_configBtn->setFillColor(sf::Color::White);
    sf::FloatRect configBounds = m_configBtn->getLocalBounds();
    m_configBtn->setOrigin({configBounds.size.x / 2.f, 0.f});
    m_configBtn->setPosition({640.f, 355.f});

    // Botón Guía (NUEVO)
    m_guideText = std::make_unique<sf::Text>(m_font, "GUIA DE CONTROLES", 30);
    m_guideText->setFillColor(sf::Color::White);
    sf::FloatRect guideBounds = m_guideText->getLocalBounds();
    m_guideText->setOrigin({guideBounds.size.x / 2.f, 0.f});
    m_guideText->setPosition({640.f, 415.f});

    // Botón Menú Principal
    m_menuText = std::make_unique<sf::Text>(m_font, "MENU PRINCIPAL", 30);
    m_menuText->setFillColor(sf::Color::White);
    sf::FloatRect menuBounds = m_menuText->getLocalBounds();
    m_menuText->setOrigin({menuBounds.size.x / 2.f, 0.f});
    m_menuText->setPosition({640.f, 475.f});

    // Botón Salir
    m_exitText = std::make_unique<sf::Text>(m_font, "SALIR DEL JUEGO", 30);
    m_exitText->setFillColor(sf::Color::White);
    sf::FloatRect exitBounds = m_exitText->getLocalBounds();
    m_exitText->setOrigin({exitBounds.size.x / 2.f, 0.f});
    m_exitText->setPosition({640.f, 535.f});
    
    inicializarGuia();
}

void PauseState::inicializarGuia() {
    // Panel de la guía
    m_guidePanel.setSize({600.f, 520.f});
    m_guidePanel.setFillColor(sf::Color(20, 20, 40, 250));
    m_guidePanel.setOutlineThickness(4);
    m_guidePanel.setOutlineColor(sf::Color::Yellow);
    
    // Título de la guía
    m_guideTitle = std::make_unique<sf::Text>(m_font, " CONTROLES DEL JUEGO ", 24);
    m_guideTitle->setFillColor(sf::Color::Yellow);
    m_guideTitle->setStyle(sf::Text::Bold);
    
    // Entradas de la guía (tecla + descripción)
    struct ControlEntry {
        std::string tecla;
        std::string descripcion;
    };
    
    std::vector<ControlEntry> controles = {
        {"Up, Down, Left, Right/W A S D", "Movimiento del personaje"},
        {"E", "Abrir/Cerrar inventario"},
        {"F", "Avanzar al siguiente nivel"},
        {"F", "Abrir/Cerrar minijuego"},
        {"F", "Recoger objetos del suelo"},
        {"ESC", "Abrir menu de pausa"},
    };
    
    for (const auto& entry : controles) {
        auto texto = std::make_unique<sf::Text>(m_font, 
            entry.tecla + "  ->  " + entry.descripcion, 16);
        texto->setFillColor(sf::Color::White);
        m_guideEntries.push_back(std::move(texto));
    }
    
    // Texto para cerrar la guía
    m_closeGuideText = std::make_unique<sf::Text>(m_font, "[ESC] Cerrar", 18);
    m_closeGuideText->setFillColor(sf::Color(200, 200, 100));
    m_closeGuideText->setStyle(sf::Text::Bold);
}

void PauseState::actualizarGuia(const sf::Vector2f& mousePos) {
    if (mostrarGuia) {
        // Verificar hover sobre el botón de cerrar
        sf::FloatRect closeBounds = m_closeGuideText->getGlobalBounds();
        m_closeGuideHover = closeBounds.contains(mousePos);
        m_closeGuideText->setFillColor(m_closeGuideHover ? 
            sf::Color::Yellow : sf::Color(200, 200, 100));
    }
}

void PauseState::dibujarGuia() {
    if (!mostrarGuia) return;
    
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    float centerX = winW / 2.f;
    float centerY = winH / 2.f;
    
    // Panel principal
    m_guidePanel.setSize({650.f, 550.f});  
    m_guidePanel.setPosition({centerX - 325.f, centerY - 275.f});  
    window->draw(m_guidePanel);
    
    // Título - bajado un poco
    if (m_guideTitle) {
        sf::FloatRect tb = m_guideTitle->getLocalBounds();
        m_guideTitle->setOrigin({tb.size.x / 2.f, 0.f});
        m_guideTitle->setPosition({centerX, centerY - 210.f});  
        window->draw(*m_guideTitle);
    }
    
    // Línea separadora - bajada
    sf::RectangleShape separator({550.f, 2.f});  
    separator.setFillColor(sf::Color(100, 100, 150));
    separator.setPosition({centerX - 275.f, centerY - 170.f});  
    window->draw(separator);
    
   
    float startY = centerY - 140.f;  
    float lineHeight = 38.f;  
    
    for (size_t i = 0; i < m_guideEntries.size(); ++i) {
        if (m_guideEntries[i]) {
            float yPos = startY + (i * lineHeight);
            m_guideEntries[i]->setPosition({centerX - 280.f, yPos});  
            
            m_guideEntries[i]->setCharacterSize(17); 
            
            // Resaltar teclas importantes con color diferente
            std::string textoCompleto = m_guideEntries[i]->getString();
            if (textoCompleto.find("E") != std::string::npos && 
                textoCompleto.find("->") != std::string::npos) {
                if (textoCompleto.find("E  ->") != std::string::npos) {
                    m_guideEntries[i]->setFillColor(sf::Color(100, 255, 100));
                } else if (textoCompleto.find("F  ->") != std::string::npos) {
                    m_guideEntries[i]->setFillColor(sf::Color(100, 200, 255));
                } else if (textoCompleto.find("ESC") != std::string::npos) {
                    m_guideEntries[i]->setFillColor(sf::Color(255, 150, 100));
                } else {
                    m_guideEntries[i]->setFillColor(sf::Color(220, 220, 220));
                }
            } else {
                m_guideEntries[i]->setFillColor(sf::Color(200, 200, 200));
            }
            
            window->draw(*m_guideEntries[i]);
        }
    }
    
    // Boton cerrar
    if (m_closeGuideText) {
        m_closeGuideText->setCharacterSize(18);
        sf::FloatRect cb = m_closeGuideText->getLocalBounds();
        m_closeGuideText->setOrigin({cb.size.x / 2.f, 0.f});
        m_closeGuideText->setPosition({centerX, centerY + 230.f});  // Cambiado de +210 a +230
        window->draw(*m_closeGuideText);
    }
}

void PauseState::update(float dt) {
    sf::Vector2f mouseF = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
    
    // Solo actualizar hovers si no está mostrando la guía ni los ajustes
    if (!mostrarGuia && !mostrarConfig) {
        m_resumeHover = m_resumeText->getGlobalBounds().contains(mouseF);
        m_saveHover = m_saveText->getGlobalBounds().contains(mouseF);
        m_loadHover = m_loadText->getGlobalBounds().contains(mouseF);
        m_configHover = m_configBtn->getGlobalBounds().contains(mouseF);
        m_guideHover = m_guideText->getGlobalBounds().contains(mouseF);
        m_menuHover = m_menuText->getGlobalBounds().contains(mouseF);
        m_exitHover = m_exitText->getGlobalBounds().contains(mouseF);

        m_resumeText->setFillColor(m_resumeHover ? sf::Color::Yellow : sf::Color::White);
        m_saveText->setFillColor(m_saveHover ? sf::Color::Green : sf::Color::White);
        m_loadText->setFillColor(m_loadHover ? sf::Color::Cyan : sf::Color::White);
        m_configBtn->setFillColor(m_configHover ? sf::Color::Yellow : sf::Color::White);
        m_guideText->setFillColor(m_guideHover ? sf::Color::Yellow : sf::Color::White);
        m_menuText->setFillColor(m_menuHover ? sf::Color::Yellow : sf::Color::White);
        m_exitText->setFillColor(m_exitHover ? sf::Color::Red : sf::Color::White);
    } else if (mostrarGuia) {
        actualizarGuia(mouseF);
    }

    // Configuración (ajustes)
    if (mostrarConfig && !mostrarGuia) {
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
    }

    static bool clickProcesado = false;
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        if (!clickProcesado) {
            clickProcesado = true;
            
            // Si está en la guía, verificar click en cerrar
            if (mostrarGuia) {
                if (m_closeGuideHover) {
                    mostrarGuia = false;
                }
                return;
            }
            
            if (mostrarConfig) {
                // En ajustes, click fuera cierra
                float winW = static_cast<float>(window->getSize().x);
                float winH = static_cast<float>(window->getSize().y);
                sf::FloatRect configPanel({winW/2.f - 250.f, winH/2.f - 170.f}, {500.f, 340.f});
                if (!configPanel.contains(mouseF)) {
                    mostrarConfig = false;
                }
                return;
            }
            
            // Menú principal de pausa
            if (m_resumeHover) {
                game->popState();
            }
            else if (m_saveHover) {
                if (game->tienePartidaActiva()) {
                    game->guardarPartidaActual();
                    std::cout << "Partida guardada exitosamente" << std::endl;
                } else {
                    std::cout << "No hay partida activa para guardar" << std::endl;
                }
            }
            else if (m_loadHover) {
                game->pushState(std::make_unique<SaveSelectState>(window, game, true));
            }
            else if (m_configHover) {
                mostrarConfig = true;
                seleccionConfig = 0;
            }
            else if (m_guideHover) {
                mostrarGuia = true;
            }
            else if (m_menuHover) {
                if (game->tienePartidaActiva()) {
                    game->guardarPartidaActual();
                }
                game->returnToMenu();
            }
            else if (m_exitHover) {
                if (game->tienePartidaActiva()) {
                    game->guardarPartidaActual();
                }
                window->close();
            }
        }
    } else {
        clickProcesado = false;
    }

    // Tecla ESC para cerrar menús
    static bool escPresionado = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        if (!escPresionado) {
            escPresionado = true;
            if (mostrarGuia) {
                mostrarGuia = false;
            } else if (mostrarConfig) {
                mostrarConfig = false;
            } else {
                game->popState();
            }
        }
    } else {
        escPresionado = false;
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
    
    // Si está mostrando la guía, dibujarla y no dibujar el menú principal
    if (mostrarGuia) {
        dibujarGuia();
        return;
    }
    
    float panelW = winW * 0.39f;
    float panelH = winH * 0.76f;
    m_panel.setSize(sf::Vector2f(panelW, panelH));
    m_panel.setPosition(sf::Vector2f(centerX - panelW/2.f, winH * 0.08f));
    window->draw(m_panel);
    
    auto drawCenteredText = [&](std::unique_ptr<sf::Text>& text, float y) {
        if (text) {
            sf::FloatRect bounds = text->getLocalBounds();
            text->setOrigin(sf::Vector2f(bounds.size.x/2.f, 0.f));
            text->setPosition(sf::Vector2f(centerX, y));
            window->draw(*text);
        }
    };
    
    float btnY = winH * 0.24f;
    float btnSpacing = winH * 0.064f;
    
    drawCenteredText(m_resumeText, btnY);
    drawCenteredText(m_saveText, btnY + btnSpacing);
    drawCenteredText(m_loadText, btnY + btnSpacing * 2);
    drawCenteredText(m_configBtn, btnY + btnSpacing * 3);
    drawCenteredText(m_guideText, btnY + btnSpacing * 4);
    drawCenteredText(m_menuText, btnY + btnSpacing * 5);
    drawCenteredText(m_exitText, btnY + btnSpacing * 6);
    
    if (m_title) {
        sf::FloatRect tb = m_title->getLocalBounds();
        m_title->setOrigin(sf::Vector2f(tb.size.x/2.f, 0.f));
        m_title->setPosition(sf::Vector2f(centerX, winH * 0.12f));
        window->draw(*m_title);
    }

    // Panel de ajustes (configuración)
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
        
        // Indicador de cómo salir
        sf::Text exitHint(m_font, "Presiona ESC para volver", 14);
        exitHint.setFillColor(sf::Color(150, 150, 150));
        sf::FloatRect hintBounds = exitHint.getLocalBounds();
        exitHint.setOrigin({hintBounds.size.x / 2.f, 0.f});
        exitHint.setPosition({centerX, winH * 0.72f});
        window->draw(exitHint);
    }
}