#include "Estados/ModoJuegoState.hpp"
#include "Estados/MuerteCentinelaState.hpp"
#include "Estados/PauseState.hpp"
#include "Configuracion/Game.hpp"
#include "Administradores/Lobby.hpp"
#include <iostream>
#include <sstream>

ModoJuegoState::ModoJuegoState(sf::RenderWindow* window, Game* game, 
                               const std::string& nombre, int slotId)
    : State(window, game), m_nombreJugador(nombre), m_slotId(slotId) {
    
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente en ModoJuegoState" << std::endl;
    }
    
    updateScale();
    
    // Fondo
    m_background.setFillColor(sf::Color(10, 10, 20, 255));
    
    // Panel central
    m_panel.setFillColor(sf::Color(30, 30, 50, 240));
    m_panel.setOutlineColor(sf::Color::Yellow);
    
    rebuildTexts();
     // INICIALIZAR COLORES DE BOTONES AQUÍ
    m_botonAgradable.setFillColor(sf::Color(0, 80, 0, 200));
    m_botonAgradable.setOutlineColor(sf::Color::Green);
    m_botonAgradable.setOutlineThickness(getScaledValue(3.f));
    
    m_botonConsecuencias.setFillColor(sf::Color(80, 0, 0, 200));
    m_botonConsecuencias.setOutlineColor(sf::Color::Red);
    m_botonConsecuencias.setOutlineThickness(getScaledValue(3.f));
    
    std::cout << "Estado de eleccion de modo creado para: " << m_nombreJugador << std::endl;
}

void ModoJuegoState::updateScale() {
    if (!window) return;
    
    sf::Vector2u winSize = window->getSize();
    m_scaleX = static_cast<float>(winSize.x) / BASE_WIDTH;
    m_scaleY = static_cast<float>(winSize.y) / BASE_HEIGHT;
}

int ModoJuegoState::getScaledFontSize(int baseSize) {
    float scale = std::min(m_scaleX, m_scaleY);
    return std::max(8, static_cast<int>(baseSize * scale));
}

float ModoJuegoState::getScaledValue(float baseValue) {
    float scale = std::min(m_scaleX, m_scaleY);
    return baseValue * scale;
}

std::vector<std::string> ModoJuegoState::wrapText(const std::string& text, int maxWidth, int fontSize) {
    std::vector<std::string> lines;
    std::stringstream ss(text);
    std::string word;
    std::string currentLine;
    
    // Crear un texto temporal para medir
    sf::Text tempText(m_font, "", fontSize);
    
    while (ss >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        tempText.setString(testLine);
        sf::FloatRect bounds = tempText.getLocalBounds();
        
        if (bounds.size.x > maxWidth && !currentLine.empty()) {
            lines.push_back(currentLine);
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    
    return lines;
}

void ModoJuegoState::rebuildTexts() {
    if (!window) return;
    
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    float panelW = winW * 0.7f;
    float maxTextWidth = static_cast<int>(panelW - getScaledValue(60.f));
    
    // Título
    m_title = std::make_unique<sf::Text>(m_font, "ELIGE TU CAMINO", getScaledFontSize(40));
    m_title->setFillColor(sf::Color::Yellow);
    
    // Descripción
    m_description = std::make_unique<sf::Text>(m_font,
        "Esta eleccion afectara a TODA tu partida. Elige sabiamente.\n"
        "No podras cambiar esta decision mas adelante.", getScaledFontSize(18));
    m_description->setFillColor(sf::Color::White);
    
    // Advertencia
    m_advertencia = std::make_unique<sf::Text>(m_font, 
        "Jugador: " + m_nombreJugador + " | Esta eleccion es PERMANENTE para esta partida", 
        getScaledFontSize(14));
    m_advertencia->setFillColor(sf::Color(150, 150, 150));
    
    // Títulos de botones
    m_opcionAgradableTitulo = std::make_unique<sf::Text>(m_font, "MODO HISTORIA", getScaledFontSize(24));
    m_opcionAgradableTitulo->setFillColor(sf::Color::Green);
    
    m_opcionConsecuenciasTitulo = std::make_unique<sf::Text>(m_font, "MODO SUPERVIVENCIA", getScaledFontSize(24));
    m_opcionConsecuenciasTitulo->setFillColor(sf::Color::Red);
    
    // Texto descriptivo del MODO HISTORIA (con wrapping)
    std::string textoAgradable = 
        "- Tus decisiones NO\nson permanentes\n"
        "- Si fallas en un desafio,\npuedes volver a intentarlo\n"
        "- El juego se guarda antes\nde cada momento importante\n"
        "- Podras explorar todos los\ncaminos sin miedo\n"
        "- Ideal para disfrutar la\nhistoria y descubrir secretos";
    
    // Texto descriptivo del MODO SUPERVIVENCIA (con wrapping)
    std::string textoConsecuencias = 
        "- Cada decision que tomes\nsera PARA SIEMPRE\n"
        "- Si fallas en un desafio...\nno hay segunda oportunidad\n"
        "- Tus errores afectaran el\nfinal de la historia\n"
        "- Solo los mas valientes\nveran el verdadero desenlace\n"
        "- Experiencia intensa con\nconsecuencias reales";
    
    // Limpiar líneas anteriores
    m_opcionAgradableLineas.clear();
    m_opcionConsecuenciasLineas.clear();
    
    // Crear líneas para cada texto
    int descFontSize = getScaledFontSize(18);
    
    // Dividir el texto por saltos de línea existentes y luego aplicar wrap
    std::stringstream ssAgradable(textoAgradable);
    std::string line;
    while (std::getline(ssAgradable, line)) {
        auto wrappedLines = wrapText(line, maxTextWidth, descFontSize);
        for (const auto& wrappedLine : wrappedLines) {
            sf::Text textLine(m_font, wrappedLine, descFontSize);
            textLine.setFillColor(sf::Color(200, 255, 200));
            m_opcionAgradableLineas.push_back(textLine);
        }
    }
    
    std::stringstream ssConsecuencias(textoConsecuencias);
    while (std::getline(ssConsecuencias, line)) {
        auto wrappedLines = wrapText(line, maxTextWidth, descFontSize);
        for (const auto& wrappedLine : wrappedLines) {
            sf::Text textLine(m_font, wrappedLine, descFontSize);
            textLine.setFillColor(sf::Color(255, 200, 200));
            m_opcionConsecuenciasLineas.push_back(textLine);
        }
    }
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
                game->getSaveManager().getCurrentProgress().modoElegido = 
                    GameProgressData::ModoJuego::CAMINO_AGRADABLE;
                game->getSaveManager().guardarProgresoActual();
                game->changeState(std::make_unique<LobbyState>(window, game));
                std::cout << "Jugador '" << m_nombreJugador << "' eligio MODO HISTORIA" << std::endl;
            }
            else if (m_hoverConsecuencias) {
                game->getSaveManager().getCurrentProgress().modoElegido = 
                    GameProgressData::ModoJuego::CAMINO_CON_CONSECUENCIAS;
                game->getSaveManager().guardarProgresoActual();
                game->changeState(std::make_unique<LobbyState>(window, game));
                std::cout << "Jugador '" << m_nombreJugador << "' eligio MODO SUPERVIVENCIA" << std::endl;
            }
        }
    }
}

void ModoJuegoState::update(float dt) {
    static sf::Vector2u lastSize = window->getSize();
    sf::Vector2u currentSize = window->getSize();
    if (lastSize != currentSize) {
        updateScale();
        rebuildTexts();
        lastSize = currentSize;
    }
}

void ModoJuegoState::draw() {
    if (!window) return;
    
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    float centerX = winW / 2.f;
    float centerY = winH / 2.f;
    
    // Fondo
    m_background.setSize(sf::Vector2f(winW, winH));
    window->draw(m_background);
    
    // Panel
    float panelW = winW * 0.7f;
    float panelH = winH * 0.8f;
    float panelX = centerX - panelW/2.f;
    float panelY = winH * 0.1f;
    
    m_panel.setSize(sf::Vector2f(panelW, panelH));
    m_panel.setPosition(sf::Vector2f(panelX, panelY));
    m_panel.setOutlineThickness(getScaledValue(3.f));
    window->draw(m_panel);
    
    // Título
    if (m_title) {
        sf::FloatRect tb = m_title->getLocalBounds();
        m_title->setOrigin(sf::Vector2f(tb.size.x/2.f, 0.f));
        m_title->setPosition(sf::Vector2f(centerX, panelY + getScaledValue(20.f)));
        window->draw(*m_title);
    }
    
    // Descripción
    if (m_description) {
        sf::FloatRect db = m_description->getLocalBounds();
        m_description->setOrigin(sf::Vector2f(db.size.x/2.f, 0.f));
        m_description->setPosition(sf::Vector2f(centerX, panelY + getScaledValue(80.f)));
        window->draw(*m_description);
    }
    
    // Botones
    float btnW = panelW * 0.42f;
    float btnH = panelH * 0.5f;
    float btnY = panelY + panelH * 0.35f;
    float gap = panelW * 0.04f;
    
    float btn1X = centerX - btnW - gap/2.f;
    float btn2X = centerX + gap/2.f;
    
    m_botonAgradable.setSize(sf::Vector2f(btnW, btnH));
    m_botonAgradable.setPosition(sf::Vector2f(btn1X, btnY));
    m_botonAgradable.setOutlineThickness(getScaledValue(3.f));
    window->draw(m_botonAgradable);
    
    m_botonConsecuencias.setSize(sf::Vector2f(btnW, btnH));
    m_botonConsecuencias.setPosition(sf::Vector2f(btn2X, btnY));
    m_botonConsecuencias.setOutlineThickness(getScaledValue(3.f));
    window->draw(m_botonConsecuencias);
    
    // Títulos de botones
    if (m_opcionAgradableTitulo) {
        sf::FloatRect tb = m_opcionAgradableTitulo->getLocalBounds();
        m_opcionAgradableTitulo->setOrigin(sf::Vector2f(tb.size.x/2.f, 0.f));
        m_opcionAgradableTitulo->setPosition(sf::Vector2f(btn1X + btnW/2.f, btnY + getScaledValue(10.f)));
        window->draw(*m_opcionAgradableTitulo);
    }
    
    if (m_opcionConsecuenciasTitulo) {
        sf::FloatRect tb = m_opcionConsecuenciasTitulo->getLocalBounds();
        m_opcionConsecuenciasTitulo->setOrigin(sf::Vector2f(tb.size.x/2.f, 0.f));
        m_opcionConsecuenciasTitulo->setPosition(sf::Vector2f(btn2X + btnW/2.f, btnY + getScaledValue(10.f)));
        window->draw(*m_opcionConsecuenciasTitulo);
    }
    
    // Líneas de texto (con wrapping)
    float lineY = btnY + getScaledValue(45.f);
    float lineHeight = getScaledValue(18.f);
    float leftMargin = getScaledValue(15.f);
    
    for (auto& line : m_opcionAgradableLineas) {
        line.setPosition(sf::Vector2f(btn1X + leftMargin, lineY));
        window->draw(line);
        lineY += lineHeight;
    }
    
    lineY = btnY + getScaledValue(45.f);
    for (auto& line : m_opcionConsecuenciasLineas) {
        line.setPosition(sf::Vector2f(btn2X + leftMargin, lineY));
        window->draw(line);
        lineY += lineHeight;
    }
    
    // Advertencia
    if (m_advertencia) {
        sf::FloatRect ab = m_advertencia->getLocalBounds();
        m_advertencia->setOrigin(sf::Vector2f(ab.size.x/2.f, 0.f));
        m_advertencia->setPosition(sf::Vector2f(centerX, panelY + panelH - getScaledValue(30.f)));
        window->draw(*m_advertencia);
    }
}