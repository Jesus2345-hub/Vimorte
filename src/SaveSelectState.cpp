#include "SaveSelectState.hpp"
#include "Lobby.hpp"
#include "Game.hpp"
#include "ModoJuegoState.hpp" 
#include <iostream>
#include <algorithm>

SaveSelectState::SaveSelectState(sf::RenderWindow* window, Game* game, bool soloCarga) 
    : State(window, game), saveManager(game->getSaveManager()), 
      m_modoNuevaPartida(false), m_soloCarga(soloCarga), m_slotSeleccionado(-1),
      m_slotSeleccionadoParaEliminar(-1) {
    
    // Cargar fuente
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "❌ Error cargando fuente en SaveSelectState" << std::endl;
    }
    
    // Configurar fondo
    m_background.setSize(sf::Vector2f(1280.f, 720.f));
    m_background.setFillColor(sf::Color(20, 20, 30, 255));
    
    // Configurar panel
    m_panel.setSize(sf::Vector2f(700.f, 600.f));
    m_panel.setPosition(sf::Vector2f(290.f, 60.f));
    m_panel.setFillColor(sf::Color(30, 30, 50, 240));
    m_panel.setOutlineThickness(3.f);
    m_panel.setOutlineColor(sf::Color(100, 100, 150));
    
    // Título
    std::string titulo = "SELECCIONAR PARTIDA";
    m_title = std::make_unique<sf::Text>(m_font, titulo, 40);
    m_title->setFillColor(sf::Color::Yellow);
    sf::FloatRect titleBounds = m_title->getLocalBounds();
    m_title->setOrigin(sf::Vector2f(titleBounds.size.x / 2.f, titleBounds.size.y / 2.f));
    m_title->setPosition(sf::Vector2f(640.f, 100.f));
    
    // Instrucciones
    std::string instrucciones = "Click: Seleccionar slot | Doble Click: Cargar/Nueva | ELIMINAR: Borrar seleccionado | ESC: Volver";
    m_instructionText = std::make_unique<sf::Text>(m_font, instrucciones, 13);
    m_instructionText->setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect instBounds = m_instructionText->getLocalBounds();
    m_instructionText->setOrigin(sf::Vector2f(instBounds.size.x / 2.f, instBounds.size.y / 2.f));
    m_instructionText->setPosition(sf::Vector2f(640.f, 680.f));
    
    // Botón eliminar
    m_btnEliminar.setSize(sf::Vector2f(150.f, 40.f));
    m_btnEliminar.setPosition(sf::Vector2f(850.f, 620.f));
    m_btnEliminar.setFillColor(sf::Color(150, 0, 0, 200));
    m_btnEliminar.setOutlineThickness(2.f);
    m_btnEliminar.setOutlineColor(sf::Color::Red);
    
    m_btnEliminarText = std::make_unique<sf::Text>(m_font, "ELIMINAR", 18);
    m_btnEliminarText->setFillColor(sf::Color::White);
    sf::FloatRect btnBounds = m_btnEliminarText->getLocalBounds();
    m_btnEliminarText->setOrigin(sf::Vector2f(btnBounds.size.x / 2.f, btnBounds.size.y / 2.f));
    m_btnEliminarText->setPosition(sf::Vector2f(925.f, 640.f));
    
    // Texto de slot seleccionado
    m_selectedSlotText = std::make_unique<sf::Text>(m_font, "Ningún slot seleccionado", 14);
    m_selectedSlotText->setFillColor(sf::Color(150, 150, 150));
    m_selectedSlotText->setPosition(sf::Vector2f(340.f, 150.f));
    
    // Input text para nueva partida
    m_inputText = std::make_unique<sf::Text>(m_font, "", 24);
    m_inputText->setFillColor(sf::Color::White);
    
    actualizarUI();
    
    std::cout << "✅ SaveSelectState inicializado" << std::endl;
}

void SaveSelectState::actualizarUI() {
    slots = saveManager.getSlotsDisponibles();
    
    m_slotTexts.clear();
    m_slotBoxes.clear();
    m_slotHover.clear();
    
    float startY = 200.f;
    float spacing = 90.f;
    
    for (size_t i = 0; i < slots.size(); i++) {
        // Crear caja para el slot
        sf::RectangleShape box(sf::Vector2f(600.f, 70.f));
        box.setPosition(sf::Vector2f(340.f, startY + i * spacing));
        box.setFillColor(sf::Color(40, 40, 60, 200));
        box.setOutlineThickness(2.f);
        box.setOutlineColor(sf::Color(100, 100, 100));
        m_slotBoxes.push_back(box);
        
        // Crear texto del slot
        auto text = std::make_unique<sf::Text>(m_font);
        std::string displayText = "SLOT " + std::to_string(i + 1) + ": ";
        
        if (slots[i].nombrePartida != "[VACIO]") {
            displayText += slots[i].nombrePartida + " - Nivel " + 
                          std::to_string(slots[i].nivelActual) + " - " +
                          slots[i].fechaGuardado;
            
            if (slots[i].tieneCentinela) {
                displayText += " [CENTINELA]";
            }
        } else {
            displayText += "[VACIO]";
        }
        
        text->setString(displayText);
        text->setCharacterSize(16);
        text->setFillColor(sf::Color::White);
        text->setPosition(sf::Vector2f(360.f, startY + i * spacing + 20.f));
        m_slotTexts.push_back(std::move(text));
        
        m_slotHover.push_back(false);
    }
}

void SaveSelectState::seleccionarSlot(int slotId) {
    m_slotSeleccionadoParaEliminar = slotId;
    
    // Actualizar texto de selección
    std::string seleccionText = "Slot seleccionado: " + std::to_string(slotId + 1);
    if (slots[slotId].nombrePartida != "[VACIO]") {
        seleccionText += " - " + slots[slotId].nombrePartida;
    } else {
        seleccionText += " [VACIO]";
    }
    m_selectedSlotText->setString(seleccionText);
    
    std::cout << "📍 Slot " << (slotId + 1) << " seleccionado" << std::endl;
}

void SaveSelectState::ejecutarAccionSlot(int slotId) {
    // Vacío = nueva partida, ocupado = cargar
    if (slots[slotId].nombrePartida == "[VACIO]") {
        iniciarNuevaPartida(slotId);
    } else {
        cargarPartidaExistente(slotId);
    }
}

void SaveSelectState::iniciarNuevaPartida(int slotId) {
    if (m_modoNuevaPartida && m_slotSeleccionado == slotId) {
        if (!m_nombreInput.empty()) {
            if (saveManager.crearNuevaPartida(slotId, m_nombreInput)) {
                std::cout << "✅ Nueva partida creada: " << m_nombreInput << std::endl;
                
                // En lugar de ir directo al Lobby, mostrar elección de modo
                game->changeState(std::make_unique<ModoJuegoState>(window, game, m_nombreInput, slotId));
            }
        }
    } else {
        m_modoNuevaPartida = true;
        m_slotSeleccionado = slotId;
        m_nombreInput = "";
        std::cout << "📝 Ingrese nombre para nueva partida en slot " << (slotId + 1) << std::endl;
    }
}


void SaveSelectState::cargarPartidaExistente(int slotId) {
    game->cargarPartidaYContinuar(slotId);
}

void SaveSelectState::eliminarPartidaSeleccionada() {
    if (m_slotSeleccionadoParaEliminar >= 0 && 
        m_slotSeleccionadoParaEliminar < (int)slots.size() &&
        slots[m_slotSeleccionadoParaEliminar].nombrePartida != "[VACIO]") {
        
        int slotAEliminar = m_slotSeleccionadoParaEliminar;
        saveManager.eliminarPartida(slotAEliminar);
        actualizarUI();
        
        // Resetear selección
        m_slotSeleccionadoParaEliminar = -1;
        m_selectedSlotText->setString("Ningún slot seleccionado");
        
        std::cout << "🗑️ Partida eliminada del slot " << (slotAEliminar + 1) << std::endl;
    } else {
        std::cout << "⚠️ No hay un slot válido seleccionado para eliminar" << std::endl;
    }
}

void SaveSelectState::handleEvent(const sf::Event& event) {
    if (m_modoNuevaPartida) {
        if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
            // Solo permitir caracteres imprimibles y limitar longitud
            if (textEntered->unicode >= 32 && textEntered->unicode < 128 && m_nombreInput.length() < 20) {
                m_nombreInput += static_cast<char>(textEntered->unicode);
            }
        }
        
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Backspace && !m_nombreInput.empty()) {
                m_nombreInput.pop_back();
            }
            if (keyEvent->code == sf::Keyboard::Key::Enter && !m_nombreInput.empty()) {
                iniciarNuevaPartida(m_slotSeleccionado);
            }
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                m_modoNuevaPartida = false;
                m_slotSeleccionado = -1;
            }
        }
        return;
    }
    
    sf::Vector2f mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
    
    // Actualizar hover
    for (size_t i = 0; i < m_slotBoxes.size(); i++) {
        m_slotHover[i] = m_slotBoxes[i].getGlobalBounds().contains(mousePos);
        
        // Color diferente según estado
        if (i == (size_t)m_slotSeleccionadoParaEliminar) {
            // Slot seleccionado: borde verde
            m_slotBoxes[i].setOutlineColor(sf::Color::Green);
            m_slotBoxes[i].setOutlineThickness(3.f);
        } else if (m_slotHover[i]) {
            // Slot con hover: borde amarillo
            m_slotBoxes[i].setOutlineColor(sf::Color::Yellow);
            m_slotBoxes[i].setOutlineThickness(2.f);
        } else {
            // Slot normal
            m_slotBoxes[i].setOutlineColor(sf::Color(100, 100, 100));
            m_slotBoxes[i].setOutlineThickness(2.f);
        }
    }
    
    // Actualizar hover del botón eliminar
    if (m_btnEliminarText) {
        m_btnEliminarHover = m_btnEliminar.getGlobalBounds().contains(mousePos);
        m_btnEliminar.setFillColor(m_btnEliminarHover ? sf::Color(200, 0, 0, 200) : sf::Color(150, 0, 0, 200));
    }
    
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            // PRIMERO: Verificar si se hizo click en el botón eliminar
            if (m_btnEliminarHover) {
                eliminarPartidaSeleccionada();
                return;
            }
            
            // SEGUNDO: Verificar click en slots
            for (size_t i = 0; i < m_slotBoxes.size(); i++) {
                if (m_slotHover[i]) {
                    // Si ya estaba seleccionado, ejecutar acción (doble click implícito)
                    if (m_slotSeleccionadoParaEliminar == (int)i) {
                        ejecutarAccionSlot(i);
                    } else {
                        // Si no, solo seleccionarlo
                        seleccionarSlot(i);
                    }
                    break;
                }
            }
        }
    }
    
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            game->popState();
        }
        
        // Eliminar partida con SUPR
        if (keyEvent->code == sf::Keyboard::Key::Delete) {
            eliminarPartidaSeleccionada();
        }
        
        // Enter para ejecutar acción en slot seleccionado
        if (keyEvent->code == sf::Keyboard::Key::Enter && m_slotSeleccionadoParaEliminar >= 0) {
            ejecutarAccionSlot(m_slotSeleccionadoParaEliminar);
        }
    }
}

void SaveSelectState::update(float dt) {
    // No necesita actualizaciones por frame
}

void SaveSelectState::dibujarTecladoVirtual(sf::RenderWindow& window) {
    // Placeholder para teclado virtual si se necesita
}

void SaveSelectState::draw() {
    if (!window) return;
    
    window->draw(m_background);
    window->draw(m_panel);
    
    if (m_title) window->draw(*m_title);
    if (m_instructionText) window->draw(*m_instructionText);
    if (m_selectedSlotText) window->draw(*m_selectedSlotText);
    
    // Dibujar slots
    for (size_t i = 0; i < m_slotBoxes.size(); i++) {
        window->draw(m_slotBoxes[i]);
        if (m_slotTexts[i]) window->draw(*m_slotTexts[i]);
    }
    
    // Dibujar botón eliminar
    window->draw(m_btnEliminar);
    if (m_btnEliminarText) window->draw(*m_btnEliminarText);
    
    // Dibujar input de nombre si estamos creando nueva partida
    if (m_modoNuevaPartida) {
        sf::RectangleShape inputBg(sf::Vector2f(400.f, 50.f));
        inputBg.setPosition(sf::Vector2f(440.f, 500.f));
        inputBg.setFillColor(sf::Color(0, 0, 0, 200));
        inputBg.setOutlineThickness(2.f);
        inputBg.setOutlineColor(sf::Color::Yellow);
        window->draw(inputBg);
        
        m_inputText->setString("Nombre: " + m_nombreInput + (m_nombreInput.empty() ? "_" : ""));
        m_inputText->setPosition(sf::Vector2f(460.f, 512.f));
        window->draw(*m_inputText);
        
        sf::Text helpText(m_font, "Presiona ENTER para confirmar, ESC para cancelar", 14);
        helpText.setFillColor(sf::Color(150, 150, 150));
        helpText.setPosition(sf::Vector2f(460.f, 560.f));
        window->draw(helpText);
    }
}