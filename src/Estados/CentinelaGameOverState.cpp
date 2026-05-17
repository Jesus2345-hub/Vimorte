#include "Estados/CentinelaGameOverState.hpp"
#include "Configuracion/Game.hpp"
#include "Estructuras/LevelTree.hpp"
#include "Estados/VideoFinalState.hpp"
#include <iostream>
#include <cmath>

CentinelaGameOverState::CentinelaGameOverState(sf::RenderWindow* window, Game* game, 
                                                bool modoSupervivencia, 
                                                const std::string& nivelOriginalId)
    : State(window, game)
    , m_modoSupervivencia(modoSupervivencia)
    , m_nivelOriginalId(nivelOriginalId)
    , m_finalMostrado(false)
    , m_hoverAceptarFinalMalo(false)
{
    // Fuente
    if (!m_fuente.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente en CentinelaGameOverState" << std::endl;
    }
    
    // Fondo negro
    m_fondo.setSize(sf::Vector2f(1280.f, 720.f));
    m_fondo.setFillColor(sf::Color::Black);
    
    // Panel central
    m_panel.setSize(sf::Vector2f(900.f, 500.f));
    m_panel.setFillColor(sf::Color(20, 20, 40, 240));
    m_panel.setOutlineThickness(3.f);
    m_panel.setOutlineColor(sf::Color(255, 215, 0));
    m_panel.setOrigin({450.f, 250.f});
    m_panel.setPosition({640.f, 360.f});
    
    // Título (diferente según modo)
    std::string tituloTexto = "DERROTA";
    m_titulo = std::make_unique<sf::Text>(m_fuente, tituloTexto, 70);
    m_titulo->setFillColor(sf::Color::Red);
    m_titulo->setStyle(sf::Text::Style::Bold);
    sf::FloatRect tb = m_titulo->getLocalBounds();
    m_titulo->setOrigin({tb.size.x / 2.f, tb.size.y / 2.f});
    m_titulo->setPosition({640.f, 140.f});
    
    // Sombra del título
    m_tituloSombra = std::make_unique<sf::Text>(m_fuente, tituloTexto, 70);
    m_tituloSombra->setFillColor(sf::Color(100, 0, 0));
    m_tituloSombra->setStyle(sf::Text::Style::Bold);
    m_tituloSombra->setOrigin({tb.size.x / 2.f, tb.size.y / 2.f});
    m_tituloSombra->setPosition({642.f, 142.f});
    
    // Subtítulo según modo
    if (m_modoSupervivencia) {
        m_subtitulo = std::make_unique<sf::Text>(m_fuente, 
            "Has sido derrotado por el centinela en modo supervivencia\nRegresa al nivel y encuentra otra ruta", 22);
    } else {
        m_subtitulo = std::make_unique<sf::Text>(m_fuente, 
            "Has sido derrotado por el centinela en modo historia\nTienes la oportunidad de Reintentar o aceptar tu destino?", 22);
    }
    m_subtitulo->setFillColor(sf::Color(200, 200, 200));
    sf::FloatRect stb = m_subtitulo->getLocalBounds();
    m_subtitulo->setOrigin({stb.size.x / 2.f, stb.size.y / 2.f});
    m_subtitulo->setPosition({640.f, 230.f});
    
    float posYActual = 320.f;
    float espaciado = 85.f;
    
    // ========== MODO HISTORIA ==========
    if (!m_modoSupervivencia) {
        // Botón REINTENTAR CENTINELA
        m_reintentarCentinelaText = std::make_unique<sf::Text>(m_fuente, "REINTENTAR CENTINELA", 26);
        m_reintentarCentinelaText->setFillColor(sf::Color::White);
        m_reintentarCentinelaText->setStyle(sf::Text::Style::Bold);
        
        m_botonReintentarCentinela.setSize(sf::Vector2f(400.f, 65.f));
        m_botonReintentarCentinela.setFillColor(sf::Color(220, 50, 50));
        m_botonReintentarCentinela.setOutlineThickness(3.f);
        m_botonReintentarCentinela.setOutlineColor(sf::Color::White);
        m_botonReintentarCentinela.setOrigin({200.f, 32.5f});
        m_botonReintentarCentinela.setPosition({640.f, posYActual});
        
        sf::FloatRect br = m_reintentarCentinelaText->getLocalBounds();
        m_reintentarCentinelaText->setOrigin({br.size.x / 2.f, br.size.y / 2.f});
        m_reintentarCentinelaText->setPosition({640.f, posYActual});
        
        posYActual += espaciado;
        
        // NUEVO: Botón ACEPTAR FINAL MALO
        m_aceptarFinalMaloText = std::make_unique<sf::Text>(m_fuente, "ACEPTAR FINAL MALO", 26);
        m_aceptarFinalMaloText->setFillColor(sf::Color::White);
        m_aceptarFinalMaloText->setStyle(sf::Text::Style::Bold);
        
        m_botonAceptarFinalMalo.setSize(sf::Vector2f(400.f, 65.f));
        m_botonAceptarFinalMalo.setFillColor(sf::Color(80, 40, 0));
        m_botonAceptarFinalMalo.setOutlineThickness(3.f);
        m_botonAceptarFinalMalo.setOutlineColor(sf::Color(255, 150, 50));
        m_botonAceptarFinalMalo.setOrigin({200.f, 32.5f});
        m_botonAceptarFinalMalo.setPosition({640.f, posYActual});
        
        sf::FloatRect bam = m_aceptarFinalMaloText->getLocalBounds();
        m_aceptarFinalMaloText->setOrigin({bam.size.x / 2.f, bam.size.y / 2.f});
        m_aceptarFinalMaloText->setPosition({640.f, posYActual});
        
        posYActual += espaciado;
        
    } 
    // ========== MODO SUPERVIVENCIA ==========
    else {
        // Botón REGRESAR AL NIVEL
        m_regresarNivelText = std::make_unique<sf::Text>(m_fuente, "REGRESAR AL NIVEL", 38);
        m_regresarNivelText->setFillColor(sf::Color::White);
        m_regresarNivelText->setStyle(sf::Text::Style::Bold);
        
        m_botonRegresarNivel.setSize(sf::Vector2f(400.f, 65.f));
        m_botonRegresarNivel.setFillColor(sf::Color(50, 100, 150));
        m_botonRegresarNivel.setOutlineThickness(3.f);
        m_botonRegresarNivel.setOutlineColor(sf::Color::White);
        m_botonRegresarNivel.setOrigin({200.f, 32.5f});
        m_botonRegresarNivel.setPosition({640.f, posYActual});
        
        sf::FloatRect br2 = m_regresarNivelText->getLocalBounds();
        m_regresarNivelText->setOrigin({br2.size.x / 2.f, br2.size.y / 2.f});
        m_regresarNivelText->setPosition({640.f, posYActual});
        
        posYActual += espaciado;
        
        // Botón MENÚ PRINCIPAL
        m_menuText = std::make_unique<sf::Text>(m_fuente, "MENU PRINCIPAL", 38);
        m_menuText->setFillColor(sf::Color::White);
        m_menuText->setStyle(sf::Text::Style::Bold);
        
        m_botonMenu.setSize(sf::Vector2f(350.f, 65.f));
        m_botonMenu.setFillColor(sf::Color(60, 60, 100));
        m_botonMenu.setOutlineThickness(3.f);
        m_botonMenu.setOutlineColor(sf::Color::White);
        m_botonMenu.setOrigin({175.f, 32.5f});
        m_botonMenu.setPosition({640.f, posYActual});
        
        sf::FloatRect bm = m_menuText->getLocalBounds();
        m_menuText->setOrigin({bm.size.x / 2.f, bm.size.y / 2.f});
        m_menuText->setPosition({640.f, posYActual});
    }
    
    m_tiempoAnimacion = 0.f;
}

void CentinelaGameOverState::update(float dt)
{
    m_tiempoAnimacion += dt;
    
    if (!m_modoSupervivencia) {
        // MODO HISTORIA
        if (m_hoverReintentar) {
            m_botonReintentarCentinela.setFillColor(sf::Color(255, 80, 80));
            m_reintentarCentinelaText->setFillColor(sf::Color::Yellow);
        } else {
            m_botonReintentarCentinela.setFillColor(sf::Color(200, 40, 40));
            m_reintentarCentinelaText->setFillColor(sf::Color::White);
        }
        
        if (m_hoverAceptarFinalMalo) {
            m_botonAceptarFinalMalo.setFillColor(sf::Color(130, 70, 20));
            m_aceptarFinalMaloText->setFillColor(sf::Color::Yellow);
        } else {
            m_botonAceptarFinalMalo.setFillColor(sf::Color(80, 40, 0));
            m_aceptarFinalMaloText->setFillColor(sf::Color::White);
        }
    } else {
        // MODO SUPERVIVENCIA
        if (m_hoverRegresar) {
            m_botonRegresarNivel.setFillColor(sf::Color(100, 150, 200));
            m_regresarNivelText->setFillColor(sf::Color::Yellow);
        } else {
            m_botonRegresarNivel.setFillColor(sf::Color(50, 100, 150));
            m_regresarNivelText->setFillColor(sf::Color::White);
        }
        
        if (m_hoverMenu) {
            m_botonMenu.setFillColor(sf::Color(100, 100, 150));
            m_menuText->setFillColor(sf::Color::Yellow);
        } else {
            m_botonMenu.setFillColor(sf::Color(60, 60, 100));
            m_menuText->setFillColor(sf::Color::White);
        }
    }
}

void CentinelaGameOverState::handleEvent(const sf::Event& event)
{
    sf::Vector2f mouse = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
    
    if (!m_modoSupervivencia) {
        // MODO HISTORIA
        m_hoverReintentar = m_botonReintentarCentinela.getGlobalBounds().contains(mouse);
        m_hoverAceptarFinalMalo = m_botonAceptarFinalMalo.getGlobalBounds().contains(mouse);
    } else {
        // MODO SUPERVIVENCIA
        m_hoverRegresar = m_botonRegresarNivel.getGlobalBounds().contains(mouse);
        m_hoverMenu = m_botonMenu.getGlobalBounds().contains(mouse);
    }
    
    if (const auto* click = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (click->button == sf::Mouse::Button::Left)
        {
            if (!m_modoSupervivencia) {
                // MODO HISTORIA
                if (m_hoverReintentar) {
                    reiniciarCentinela();
                }
                else if (m_hoverAceptarFinalMalo) {
                    aceptarFinalMalo();
                }
            } else {
                // MODO SUPERVIVENCIA
                if (m_hoverRegresar) {
                    regresarANivel();
                }
                else if (m_hoverMenu) {
                    irMenuPrincipal();
                }
            }
        }
    }
}

void CentinelaGameOverState::reiniciarCentinela()
{
    std::cout << "Reintentando centinela..." << std::endl;
    
    // Obtener el nodo actual (el centinela)
    LevelNode* nodoActual = game->getLevelTree().getCurrentNode();
    
    // Verificar si es un centinela
    if (nodoActual && nodoActual->type == LevelType::CENTINELA)
    {
        // Simplemente recrear el mismo estado del centinela
        auto nuevoEstado = game->getLevelTree().createCurrentState(window, game);
        if (nuevoEstado)
        {
            game->changeState(std::move(nuevoEstado));
        }
        else
        {
            game->returnToMenu();
        }
    }
    else
    {
        std::cerr << "Error: No se encuentra el centinela para reiniciar" << std::endl;
        game->returnToMenu();
    }
}

// Aceptar el final malo y continuar con el video de final malo
void CentinelaGameOverState::aceptarFinalMalo()
{
    std::cout << "Aceptando final malo del centinela..." << std::endl;
    
    // Obtener el nodo actual (el centinela)
    LevelNode* nodoActual = game->getLevelTree().getCurrentNode();
    std::string finalMaloId;
    
    // Determinar el ID del final malo según el centinela actual
    if (nodoActual && nodoActual->id == "centinela1") {
        finalMaloId = "final_malo_centinela1";
    } else if (nodoActual && nodoActual->id == "centinela2") {
        finalMaloId = "final_malo_centinela2";
    } else if (nodoActual && nodoActual->id == "centinela3Camaras") {
        finalMaloId = "final_malo_centinela3";
    } else {
        std::cerr << "Error: Centinela no reconocido: " 
                  << (nodoActual ? nodoActual->id : "nullptr") << std::endl;
        game->returnToMenu();
        return;
    }
    
    // Saltar directamente al final malo
    if (game->getLevelTree().jumpToNode(finalMaloId))
    {
        auto finalMaloState = game->getLevelTree().createCurrentState(window, game);
        if (finalMaloState)
        {
            game->changeState(std::move(finalMaloState));
            return;
        }
    }
    
    std::cerr << "Error: No se pudo cargar el final malo: " << finalMaloId << std::endl;
    game->returnToMenu();
}

void CentinelaGameOverState::regresarANivel()
{
    std::cout << "Regresando al nivel original: " << m_nivelOriginalId << std::endl;
    
    if (game->getLevelTree().returnFromCentinela())
    {
        auto nuevoEstado = game->getLevelTree().createCurrentState(window, game);
        if (nuevoEstado)
        {
            game->changeState(std::move(nuevoEstado));
        }
        else
        {
            game->returnToMenu();
        }
    }
    else
    {
        LevelNode* nivelOriginal = game->getLevelTree().findNode(m_nivelOriginalId);
        
        if (nivelOriginal)
        {
            game->getLevelTree().jumpToNode(m_nivelOriginalId);
            auto nuevoEstado = game->getLevelTree().createCurrentState(window, game);
            if (nuevoEstado)
            {
                game->changeState(std::move(nuevoEstado));
            }
            else
            {
                game->returnToMenu();
            }
        }
        else
        {
            std::cerr << "Error: No se encuentra el nivel original" << std::endl;
            game->returnToMenu();
        }
    }
}

void CentinelaGameOverState::irMenuPrincipal()
{
    std::cout << "Volviendo al menú principal..." << std::endl;
    game->returnToMenu();
}

void CentinelaGameOverState::draw()
{
    sf::View vista(sf::Vector2f(640.f, 360.f), sf::Vector2f(1280.f, 720.f));
    window->setView(vista);
    
    // Fondo
    window->draw(m_fondo);
    
    // Panel
    window->draw(m_panel);
    
    // Líneas decorativas
    sf::RectangleShape lineaSup(sf::Vector2f(600.f, 3.f));
    lineaSup.setFillColor(sf::Color::Red);
    lineaSup.setOrigin({300.f, 0.f});
    lineaSup.setPosition({640.f, 80.f});
    window->draw(lineaSup);
    
    sf::RectangleShape lineaInf(sf::Vector2f(600.f, 3.f));
    lineaInf.setFillColor(sf::Color::Red);
    lineaInf.setOrigin({300.f, 0.f});
    lineaInf.setPosition({640.f, 640.f});
    window->draw(lineaInf);
    
    // Títulos
    if (m_tituloSombra) window->draw(*m_tituloSombra);
    if (m_titulo) window->draw(*m_titulo);
    if (m_subtitulo) window->draw(*m_subtitulo);
    
    // Dibujar según el modo
    if (m_modoSupervivencia) {
        // MODO SUPERVIVENCIA: REGRESAR AL NIVEL y MENÚ
        if (m_regresarNivelText) {
            window->draw(m_botonRegresarNivel);
            window->draw(*m_regresarNivelText);
        }
        if (m_menuText) {
            window->draw(m_botonMenu);
            window->draw(*m_menuText);
        }
    } 
    else {
        // MODO HISTORIA: Solo REINTENTAR y ACEPTAR FINAL MALO (sin menú)
        if (m_reintentarCentinelaText) {
            window->draw(m_botonReintentarCentinela);
            window->draw(*m_reintentarCentinelaText);
        }
        if (m_aceptarFinalMaloText) {
            window->draw(m_botonAceptarFinalMalo);
            window->draw(*m_aceptarFinalMaloText);
        }
    }
}