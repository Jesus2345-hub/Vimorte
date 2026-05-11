#include "MinigamePatron.hpp"
#include <iostream>
#include <algorithm>

MinigamePatron::MinigamePatron() 
    : m_gen(std::random_device{}())
{
    // Cargar fuente
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    
    if (m_fontLoaded) {
        m_textoInstrucciones = std::make_unique<sf::Text>(m_font);
        m_textoInstrucciones->setCharacterSize(22);
        m_textoInstrucciones->setFillColor(sf::Color::White);
        m_textoInstrucciones->setString("REPRODUCE EL PATRON DE LA PARED");
        
        m_textoEstado = std::make_unique<sf::Text>(m_font);
        m_textoEstado->setCharacterSize(18);
        m_textoEstado->setFillColor(sf::Color::Yellow);
        
        m_textoPatron = std::make_unique<sf::Text>(m_font);
        m_textoPatron->setCharacterSize(16);
        m_textoPatron->setFillColor(sf::Color(200, 200, 200));
        m_textoPatron->setString("Patron en la pared:");
    }
    
    // Intentar cargar imagen de fondo
    if (m_texturaFondo.loadFromFile("assets/images/niveles/nivel3/fondo_patron.png")) {
        m_spriteFondo = std::make_unique<sf::Sprite>(m_texturaFondo);
        m_fondoCargado = true;
    }
    
    m_background.setFillColor(sf::Color(10, 10, 20, 230));
    m_panel.setFillColor(sf::Color::Transparent);
    m_panel.setOutlineColor(sf::Color::Transparent);
    m_panel.setOutlineThickness(0.f);
    
    inicializarBotones();
}

void MinigamePatron::setSize(const sf::Vector2f& size) {
    m_size = size;
    m_background.setSize(size);
    m_panel.setSize(sf::Vector2f(size.x * 0.85f, size.y * 0.75f));
    
    if (m_spriteFondo && m_fondoCargado) {
        float escalaX = size.x / m_texturaFondo.getSize().x;
        float escalaY = size.y / m_texturaFondo.getSize().y;
        float escala = std::min(escalaX, escalaY);
        m_spriteFondo->setScale(sf::Vector2f(escala, escala));
        sf::FloatRect bounds = m_spriteFondo->getLocalBounds();
        m_spriteFondo->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    }
    
    actualizarPosiciones();  // ← AÑADIR
}

void MinigamePatron::setPosition(const sf::Vector2f& position) {
    m_position = position;
    m_background.setPosition(position);
    m_panel.setPosition(sf::Vector2f(
        position.x + (m_size.x - m_panel.getSize().x) / 2.f,
        position.y + (m_size.y - m_panel.getSize().y) / 2.f + 40.f
    ));
    
    if (m_spriteFondo && m_fondoCargado) {
        m_spriteFondo->setPosition(sf::Vector2f(
            position.x + m_size.x / 2.f,
            position.y + m_size.y / 2.f
        ));
    }
    
    actualizarPosiciones();  // ← AÑADIR
}

void MinigamePatron::activate() {
    m_active = true;
    m_completed = false;
    m_patronJugador.clear();
    m_pasoActual = 0;
    m_progresoJugador.clear();
    
    inicializarBotones();
    actualizarPosiciones();  // ← AÑADIR
}

void MinigamePatron::deactivate() {
    m_active = false;
}

sf::Color MinigamePatron::obtenerColor(int indice) const {
    switch (indice) {
        case 0: return sf::Color::Red;
        case 1: return sf::Color::Blue;
        case 2: return sf::Color::Green;
        case 3: return sf::Color::Yellow;
        case 4: return sf::Color(255, 165, 0);    // Naranja
        case 5: return sf::Color(148, 0, 211);    // Púrpura
        case 6: return sf::Color::White;
        case 7: return sf::Color(255, 105, 180);  // Rosa
        default: return sf::Color::White;
    }
}

std::string MinigamePatron::obtenerNombreColor(int indice) const {
    switch (indice) {
        case 0: return "ROJO";
        case 1: return "AZUL";
        case 2: return "VERDE";
        case 3: return "AMARILLO";
        case 4: return "NARANJA";
        case 5: return "PURPURA";
        case 6: return "BLANCO";
        case 7: return "ROSA";
        default: return "?";
    }
}

void MinigamePatron::inicializarBotones() {
    m_botonesColores.clear();
    
    // 10 colores mezclados: ROJO(0), AZUL(1), VERDE(2) + distractores
    std::vector<int> indices = {0, 1, 2, 3, 4, 5, 6, 7, 4, 6};
    
    // Mezclar aleatoriamente
    std::shuffle(indices.begin(), indices.end(), m_gen);
    
    float botonRadius = 28.f;
    float spacing = 65.f;
    int numBotones = static_cast<int>(indices.size());
    float totalWidth = numBotones * spacing;
    float startX = m_position.x + m_size.x / 2.f - totalWidth / 2.f + spacing / 2.f;
    float startY = m_position.y + m_size.y - 110.f;
    
    for (size_t i = 0; i < indices.size(); i++) {
        BotonColor boton;
        boton.circulo.setRadius(botonRadius);
        boton.circulo.setOrigin(sf::Vector2f(botonRadius, botonRadius));
        boton.circulo.setPosition(sf::Vector2f(startX + i * spacing, startY));
        boton.circulo.setOutlineThickness(3.f);
        boton.circulo.setOutlineColor(sf::Color(60, 60, 60));
        
        boton.color = obtenerColor(indices[i]);
        boton.nombre = obtenerNombreColor(indices[i]);
        boton.indiceColor = indices[i];  // ← Guardar índice
        boton.circulo.setFillColor(boton.color);
        
        m_botonesColores.push_back(boton);
    }
    
    // Círculos de progreso vacíos (ya no mostramos el patrón)
    m_muestraPatron.clear();
}

void MinigamePatron::seleccionarColor(int indice) {
    if (m_completed) return;
    if (indice < 0 || indice >= static_cast<int>(m_botonesColores.size())) return;
    
    int colorSeleccionado = m_botonesColores[indice].indiceColor;
    
    // Verificar si es el color correcto para el paso actual
    if (m_pasoActual < static_cast<int>(m_patronObjetivo.size()) && 
        colorSeleccionado == m_patronObjetivo[m_pasoActual]) {
        
        // ¡Correcto!
        m_patronJugador.push_back(colorSeleccionado);
        m_pasoActual++;
        
        // Añadir círculo de progreso
        sf::CircleShape circulo(20.f);
        circulo.setFillColor(obtenerColor(colorSeleccionado));
        circulo.setOutlineThickness(2.f);
        circulo.setOutlineColor(sf::Color::Green);
        circulo.setOrigin(sf::Vector2f(20.f, 20.f));
        
        float progY = m_panel.getPosition().y + 130.f;
        float progSpacing = 55.f;
        float progStartX = m_position.x + m_size.x / 2.f - (3 * progSpacing) / 2.f + progSpacing / 2.f;
        circulo.setPosition(sf::Vector2f(progStartX + (m_patronJugador.size() - 1) * progSpacing, progY));
        
        m_progresoJugador.push_back(circulo);
        
        if (m_textoEstado) {
            m_textoEstado->setString("CORRECTO! " + std::to_string(m_pasoActual) + "/3");
            m_textoEstado->setFillColor(sf::Color::Green);
        }
        
        // Oscurecer botón ya usado
        m_botonesColores[indice].circulo.setFillColor(sf::Color(60, 60, 60));
        m_botonesColores[indice].circulo.setOutlineColor(sf::Color::Green);
        
        verificarPatron();
        
    } else {
        // Error: reiniciar
        m_patronJugador.clear();
        m_pasoActual = 0;
        m_progresoJugador.clear();
        
        // Restaurar todos los botones
        for (auto& boton : m_botonesColores) {
            boton.circulo.setFillColor(boton.color);
            boton.circulo.setOutlineColor(sf::Color(60, 60, 60));
        }
        
        if (m_textoEstado) {
            m_textoEstado->setString("ERROR! Intenta de nuevo.");
            m_textoEstado->setFillColor(sf::Color::Red);
        }
    }
}

void MinigamePatron::verificarPatron() {
    if (m_pasoActual >= 3) {
        m_completed = true;
        if (m_textoEstado) {
            m_textoEstado->setString("PATRON COMPLETADO! Segunda pista desbloqueada.");
            m_textoEstado->setFillColor(sf::Color::Green);
        }
        if (m_onComplete) m_onComplete();
    }
}

void MinigamePatron::reiniciarPatron() {
    m_patronJugador.clear();
    m_pasoActual = 0;
    m_progresoJugador.clear();
}

void MinigamePatron::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_active || m_completed) return;
    
    if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseButton->button == sf::Mouse::Button::Left) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
            
            for (size_t i = 0; i < m_botonesColores.size(); i++) {
                if (m_botonesColores[i].circulo.getGlobalBounds().contains(worldPos)) {
                    seleccionarColor(static_cast<int>(i));
                    break;
                }
            }
        }
    }
}

void MinigamePatron::actualizarPosiciones() {
    if (m_botonesColores.empty()) return;
    
    float botonRadius = 28.f;
    float spacing = 65.f;
    int numBotones = static_cast<int>(m_botonesColores.size());
    float totalWidth = numBotones * spacing;
    float startX = m_position.x + m_size.x / 2.f - totalWidth / 2.f + spacing / 2.f;
    float startY = m_position.y + m_size.y - 110.f;
    
    for (size_t i = 0; i < m_botonesColores.size(); i++) {
        m_botonesColores[i].circulo.setPosition(sf::Vector2f(startX + i * spacing, startY));
    }
    
    // Actualizar progreso
    float progY = m_panel.getPosition().y + 130.f;
    float progSpacing = 55.f;
    float progStartX = m_position.x + m_size.x / 2.f - (3 * progSpacing) / 2.f + progSpacing / 2.f;
    
    for (size_t i = 0; i < m_progresoJugador.size(); i++) {
        m_progresoJugador[i].setPosition(sf::Vector2f(progStartX + i * progSpacing, progY));
    }
}

void MinigamePatron::update(float dt) {
    if (!m_active) return;
    
    // Actualizar hover
    // (No tenemos acceso a la posición del mouse aquí, se maneja en handleEvent)
}

void MinigamePatron::draw(sf::RenderWindow& window) {
    if (!m_active) return;
    
    // Fondo
    if (m_spriteFondo && m_fondoCargado) {
        window.draw(*m_spriteFondo);
    } else {
        window.draw(m_background);
    }
    
    // Instrucciones
    if (m_textoInstrucciones) {
        sf::FloatRect bounds = m_textoInstrucciones->getLocalBounds();
        m_textoInstrucciones->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f - bounds.size.x / 2.f,
            m_position.y + 8.f
        ));
        window.draw(*m_textoInstrucciones);
    }
    
    // Texto recordatorio
    if (m_textoPatron) {
        m_textoPatron->setString("Reproduce el patron de la pared");
        sf::FloatRect bounds = m_textoPatron->getLocalBounds();
        m_textoPatron->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f - bounds.size.x / 2.f,
            m_position.y + 50.f
        ));
        window.draw(*m_textoPatron);
    }
    
    // Espacios para los 3 pasos (círculos vacíos + progreso)
    float progY = m_position.y + 120.f;
    float progSpacing = 55.f;
    float progStartX = m_position.x + m_size.x / 2.f - (3 * progSpacing) / 2.f + progSpacing / 2.f;
    
    for (int i = 0; i < 3; i++) {
        sf::CircleShape circulo(20.f);
        circulo.setOrigin(sf::Vector2f(20.f, 20.f));
        circulo.setPosition(sf::Vector2f(progStartX + i * progSpacing, progY));
        
        if (i < static_cast<int>(m_progresoJugador.size())) {
            // Ya seleccionado
            circulo.setFillColor(m_progresoJugador[i].getFillColor());
            circulo.setOutlineThickness(2.f);
            circulo.setOutlineColor(sf::Color::Green);
        } else {
            // Vacío
            circulo.setFillColor(sf::Color(30, 30, 30));
            circulo.setOutlineThickness(2.f);
            circulo.setOutlineColor(sf::Color(80, 80, 80));
        }
        window.draw(circulo);
    }
    
    // Botones de colores
    for (const auto& boton : m_botonesColores) {
        window.draw(boton.circulo);
    }
    
    // Estado
    if (m_textoEstado) {
        sf::FloatRect bounds = m_textoEstado->getLocalBounds();
        m_textoEstado->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f - bounds.size.x / 2.f,
            m_position.y + m_size.y - 50.f
        ));
        window.draw(*m_textoEstado);
    }
}