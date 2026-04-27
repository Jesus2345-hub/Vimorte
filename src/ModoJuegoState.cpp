#include "ModoJuegoState.hpp"
// Asegúrate de tener este include al inicio de Nivel1State.cpp:
#include "MuerteCentinelaState.hpp"
#include "PauseState.hpp"
#include "Game.hpp"
#include "Lobby.hpp"
#include <iostream>

ModoJuegoState::ModoJuegoState(sf::RenderWindow* window, Game* game, 
                               const std::string& nombre, int slotId)
    : State(window, game), m_nombreJugador(nombre), m_slotId(slotId) {
    
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "❌ Error cargando fuente en ModoJuegoState" << std::endl;
    }
    
    // Fondo
    m_background.setSize(sf::Vector2f(1280.f, 720.f));
    m_background.setFillColor(sf::Color(10, 10, 20, 255));
    
    // Panel central
    m_panel.setSize(sf::Vector2f(900.f, 580.f));
    m_panel.setPosition(sf::Vector2f(190.f, 70.f));
    m_panel.setFillColor(sf::Color(30, 30, 50, 240));
    m_panel.setOutlineThickness(3.f);
    m_panel.setOutlineColor(sf::Color::Yellow);
    
    // Título
    m_title = std::make_unique<sf::Text>(m_font, "ELIGE TU CAMINO", 40);
    m_title->setFillColor(sf::Color::Yellow);
    sf::FloatRect titleBounds = m_title->getLocalBounds();
    m_title->setOrigin(sf::Vector2f(titleBounds.size.x / 2.f, 0.f));
    m_title->setPosition(sf::Vector2f(640.f, 100.f));
    
    // Descripción
    m_description = std::make_unique<sf::Text>(m_font,
        "Esta eleccion afectara a TODA tu partida. Elige sabiamente.\n"
        "No podras cambiar esta decision mas adelante.", 18);
    m_description->setFillColor(sf::Color::White);
    sf::FloatRect descBounds = m_description->getLocalBounds();
    m_description->setOrigin(sf::Vector2f(descBounds.size.x / 2.f, 0.f));
    m_description->setPosition(sf::Vector2f(640.f, 160.f));
    
    // === OPCIÓN 1: MODO HISTORIA (CAMINO AGRADABLE) ===
    m_botonAgradable.setSize(sf::Vector2f(380.f, 180.f));
    m_botonAgradable.setPosition(sf::Vector2f(240.f, 260.f));
    m_botonAgradable.setFillColor(sf::Color(0, 80, 0, 200));
    m_botonAgradable.setOutlineThickness(3.f);
    m_botonAgradable.setOutlineColor(sf::Color::Green);
    
    m_opcionAgradableTitulo = std::make_unique<sf::Text>(m_font, "MODO HISTORIA", 28);
    m_opcionAgradableTitulo->setFillColor(sf::Color::Green);
    sf::FloatRect titulo1Bounds = m_opcionAgradableTitulo->getLocalBounds();
    m_opcionAgradableTitulo->setOrigin(sf::Vector2f(titulo1Bounds.size.x / 2.f, 0.f));
    m_opcionAgradableTitulo->setPosition(sf::Vector2f(430.f, 275.f));
    
    m_opcionAgradableDesc = std::make_unique<sf::Text>(m_font,
        "• Tus decisiones NO son permanentes\n"
        "• Si fallas en un desafio, puedes volver a intentarlo\n"
        "• El juego se guarda antes de cada momento importante\n"
        "• Podras explorar todos los caminos sin miedo\n"
        "• Ideal para disfrutar la historia y descubrir secretos", 13);
    m_opcionAgradableDesc->setFillColor(sf::Color(200, 255, 200));
    m_opcionAgradableDesc->setPosition(sf::Vector2f(255.f, 330.f));
    
    // === OPCIÓN 2: MODO SUPERVIVENCIA (CAMINO CON CONSECUENCIAS) ===
    m_botonConsecuencias.setSize(sf::Vector2f(380.f, 180.f));
    m_botonConsecuencias.setPosition(sf::Vector2f(660.f, 260.f));
    m_botonConsecuencias.setFillColor(sf::Color(80, 0, 0, 200));
    m_botonConsecuencias.setOutlineThickness(3.f);
    m_botonConsecuencias.setOutlineColor(sf::Color::Red);
    
    m_opcionConsecuenciasTitulo = std::make_unique<sf::Text>(m_font, "MODO SUPERVIVENCIA", 28);
    m_opcionConsecuenciasTitulo->setFillColor(sf::Color::Red);
    sf::FloatRect titulo2Bounds = m_opcionConsecuenciasTitulo->getLocalBounds();
    m_opcionConsecuenciasTitulo->setOrigin(sf::Vector2f(titulo2Bounds.size.x / 2.f, 0.f));
    m_opcionConsecuenciasTitulo->setPosition(sf::Vector2f(850.f, 275.f));
    
    m_opcionConsecuenciasDesc = std::make_unique<sf::Text>(m_font,
        "• Cada decision que tomes sera PARA SIEMPRE\n"
        "• Si fallas en un desafio... no hay segunda oportunidad\n"
        "• Tus errores afectaran el final de la historia\n"
        "• Solo los mas valientes veran el verdadero desenlace\n"
        "• Experiencia intensa con consecuencias reales", 13);
    m_opcionConsecuenciasDesc->setFillColor(sf::Color(255, 200, 200));
    m_opcionConsecuenciasDesc->setPosition(sf::Vector2f(675.f, 330.f));
    
    // Texto de advertencia
    sf::Text advertencia(m_font, 
        "Jugador: " + m_nombreJugador + " | Esta eleccion es PERMANENTE para esta partida", 14);
    advertencia.setFillColor(sf::Color(150, 150, 150));
    advertencia.setPosition(sf::Vector2f(250.f, 520.f));
    
    std::cout << "🎮 Estado de eleccion de modo creado para: " << m_nombreJugador << std::endl;
}

void ModoJuegoState::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
    
    m_hoverAgradable = m_botonAgradable.getGlobalBounds().contains(mousePos);
    m_hoverConsecuencias = m_botonConsecuencias.getGlobalBounds().contains(mousePos);
    
    m_botonAgradable.setFillColor(m_hoverAgradable ? sf::Color(0, 130, 0, 200) : sf::Color(0, 80, 0, 200));
    m_botonAgradable.setOutlineColor(m_hoverAgradable ? sf::Color::White : sf::Color::Green);
    m_botonConsecuencias.setFillColor(m_hoverConsecuencias ? sf::Color(130, 0, 0, 200) : sf::Color(80, 0, 0, 200));
    m_botonConsecuencias.setOutlineColor(m_hoverConsecuencias ? sf::Color::White : sf::Color::Red);
    
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Left) {
            if (m_hoverAgradable) {
                // Elegir camino agradable
                game->getSaveManager().getCurrentProgress().modoElegido = 
                    GameProgressData::ModoJuego::CAMINO_AGRADABLE;
                game->getSaveManager().guardarProgresoActual();
                game->changeState(std::make_unique<LobbyState>(window, game));
                std::cout << "🌟 Jugador '" << m_nombreJugador << "' eligio MODO HISTORIA" << std::endl;
            }
            else if (m_hoverConsecuencias) {
                // Elegir camino con consecuencias
                game->getSaveManager().getCurrentProgress().modoElegido = 
                    GameProgressData::ModoJuego::CAMINO_CON_CONSECUENCIAS;
                game->getSaveManager().guardarProgresoActual();
                game->changeState(std::make_unique<LobbyState>(window, game));
                std::cout << "💀 Jugador '" << m_nombreJugador << "' eligio MODO SUPERVIVENCIA" << std::endl;
            }
        }
    }
    
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            game->popState();
        }
    }
}

void ModoJuegoState::update(float dt) {
    // No necesita actualización
}

void ModoJuegoState::draw() {
    if (!window) return;
    
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    float centerX = winW / 2.f;
    float centerY = winH / 2.f;
    
    m_background.setSize(sf::Vector2f(winW, winH));
    window->draw(m_background);
    
    float panelW = winW * 0.7f;
    float panelH = winH * 0.8f;
    m_panel.setSize(sf::Vector2f(panelW, panelH));
    m_panel.setPosition(sf::Vector2f(centerX - panelW/2.f, winH * 0.1f));
    window->draw(m_panel);
    
    if (m_title) {
        sf::FloatRect tb = m_title->getLocalBounds();
        m_title->setOrigin(sf::Vector2f(tb.size.x/2.f, 0.f));
        m_title->setPosition(sf::Vector2f(centerX, winH * 0.14f));
        window->draw(*m_title);
    }
    
    if (m_description) {
        sf::FloatRect db = m_description->getLocalBounds();
        m_description->setOrigin(sf::Vector2f(db.size.x/2.f, 0.f));
        m_description->setPosition(sf::Vector2f(centerX, winH * 0.22f));
        window->draw(*m_description);
    }
    
    float btnW = panelW * 0.42f;
    float btnH = panelH * 0.45f;
    float btnY = winH * 0.36f;
    float gap = panelW * 0.04f;
    
    m_botonAgradable.setSize(sf::Vector2f(btnW, btnH));
    m_botonAgradable.setPosition(sf::Vector2f(centerX - btnW - gap/2.f, btnY));
    window->draw(m_botonAgradable);
    
    m_botonConsecuencias.setSize(sf::Vector2f(btnW, btnH));
    m_botonConsecuencias.setPosition(sf::Vector2f(centerX + gap/2.f, btnY));
    window->draw(m_botonConsecuencias);
    
    if (m_opcionAgradableTitulo) {
        sf::FloatRect tb = m_opcionAgradableTitulo->getLocalBounds();
        m_opcionAgradableTitulo->setOrigin(sf::Vector2f(tb.size.x/2.f, 0.f));
        m_opcionAgradableTitulo->setPosition(sf::Vector2f(centerX - btnW/2.f - gap/2.f, btnY + 15.f));
        window->draw(*m_opcionAgradableTitulo);
    }
    
    if (m_opcionAgradableDesc) {
        m_opcionAgradableDesc->setPosition(sf::Vector2f(centerX - btnW - gap/2.f + 15.f, btnY + 55.f));
        window->draw(*m_opcionAgradableDesc);
    }
    
    if (m_opcionConsecuenciasTitulo) {
        sf::FloatRect tb = m_opcionConsecuenciasTitulo->getLocalBounds();
        m_opcionConsecuenciasTitulo->setOrigin(sf::Vector2f(tb.size.x/2.f, 0.f));
        m_opcionConsecuenciasTitulo->setPosition(sf::Vector2f(centerX + btnW/2.f + gap/2.f, btnY + 15.f));
        window->draw(*m_opcionConsecuenciasTitulo);
    }
    
    if (m_opcionConsecuenciasDesc) {
        m_opcionConsecuenciasDesc->setPosition(sf::Vector2f(centerX + gap/2.f + 15.f, btnY + 55.f));
        window->draw(*m_opcionConsecuenciasDesc);
    }
}