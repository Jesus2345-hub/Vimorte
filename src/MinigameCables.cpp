#include "MinigameCables.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

MinigameCables::MinigameCables() 
    : m_gen(std::random_device{}())
    , m_dist(-1.0f, 1.0f)
{
    // Intentar cargar imagen de fondo
    if (m_texturaFondo.loadFromFile("assets/images/niveles/nivel3/fondo_cables.png")) {
        m_spriteFondo = std::make_unique<sf::Sprite>(m_texturaFondo);
        m_fondoCargado = true;
    } else {
        m_fondoCargado = false;
    }
    
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    
    if (m_fontLoaded) {
        m_textoInstrucciones = std::make_unique<sf::Text>(m_font);
        m_textoInstrucciones->setCharacterSize(22);
        m_textoInstrucciones->setFillColor(sf::Color::White);
        m_textoInstrucciones->setString("¡CORTA LOS CABLES EN EL ORDEN CORRECTO!");
        
        m_textoEstado = std::make_unique<sf::Text>(m_font);
        m_textoEstado->setCharacterSize(18);
        m_textoEstado->setFillColor(sf::Color::Yellow);
        
        m_textoErrores = std::make_unique<sf::Text>(m_font);
        m_textoErrores->setCharacterSize(20);
        m_textoErrores->setFillColor(sf::Color::Red);
        
        m_textoPista1 = std::make_unique<sf::Text>(m_font);
        m_textoPista1->setCharacterSize(15);
        m_textoPista1->setFillColor(sf::Color::Cyan);
        
        m_textoPista2 = std::make_unique<sf::Text>(m_font);
        m_textoPista2->setCharacterSize(15);
        m_textoPista2->setFillColor(sf::Color::Cyan);
    }
    
    // Fondo de respaldo (solo si no hay imagen)
    m_background.setFillColor(sf::Color(20, 5, 5, 230));
    
    // Panel semi-transparente SOLO para el sub-panel de cables
    m_panel.setFillColor(sf::Color::Transparent);  // SIN FONDO
    m_panel.setOutlineColor(sf::Color::Transparent); // SIN BORDE
    m_panel.setOutlineThickness(0.f);
    
    m_tijeraHoja1.setFillColor(sf::Color(192, 192, 192));
    m_tijeraHoja2.setFillColor(sf::Color(169, 169, 169));
    m_tijeraPivote.setFillColor(sf::Color(100, 100, 100));
    m_tijeraPivote.setRadius(5.f);
    
    m_overlayGameOver.setFillColor(sf::Color(255, 0, 0, 150));
}

void MinigameCables::setSize(const sf::Vector2f& size) {
    m_size = size;
    m_background.setSize(size);
    m_panel.setSize(sf::Vector2f(size.x * 0.85f, size.y * 0.75f));
    
    // Escalar sprite para que encaje exactamente en el recuadro
    if (m_spriteFondo && m_fondoCargado) {
        float escalaX = size.x / m_texturaFondo.getSize().x;
        float escalaY = size.y / m_texturaFondo.getSize().y;
        // Usar std::min para que no se salga del recuadro
        float escala = std::min(escalaX, escalaY);
        m_spriteFondo->setScale(sf::Vector2f(escala, escala));
        
        sf::FloatRect bounds = m_spriteFondo->getLocalBounds();
        m_spriteFondo->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    }
}

void MinigameCables::setPosition(const sf::Vector2f& position) {
    m_position = position;
    m_background.setPosition(position);
    m_panel.setPosition(sf::Vector2f(
        position.x + (m_size.x - m_panel.getSize().x) / 2.f,
        position.y + (m_size.y - m_panel.getSize().y) / 2.f + 40.f
    ));
    
    // Centrar sprite
    if (m_spriteFondo && m_fondoCargado) {
        m_spriteFondo->setPosition(sf::Vector2f(
            position.x + m_size.x / 2.f,
            position.y + m_size.y / 2.f
        ));
    }
}

void MinigameCables::activate() {
    m_active = true;
    m_completed = false;
    m_cablesCortados = 0;
    m_errores = 0;
    m_mostrarTijeras = false;
    m_gameOver = false;
    m_tiempoGameOver = 0.0f;
    m_tiempoParpadeo = 0.0f;
    m_parpadeoVisible = true;
    inicializarCables();
}

void MinigameCables::deactivate() {
    m_active = false;
}

void MinigameCables::crearCable(sf::Color color, const std::string& nombre, int numero, float x, float y) {
    Cable cable;
    cable.color = color;
    cable.nombreColor = nombre;
    cable.numero = numero;
    cable.cortado = false;
    cable.animandoCorte = false;
    cable.tiempoAnimacion = 0.0f;
    
    // Cable más grande: 250px de largo, 14px de grosor
    cable.formaCable = std::make_unique<sf::RectangleShape>(sf::Vector2f(350.f, 20.f));
    cable.formaCable->setFillColor(color);
    cable.formaCable->setPosition(sf::Vector2f(x, y));
    cable.formaCable->setOutlineColor(sf::Color(255, 255, 255, 120));
    cable.formaCable->setOutlineThickness(1.5f);
    
    cable.puntaIzquierda = std::make_unique<sf::RectangleShape>(sf::Vector2f(250.f, 14.f));
    cable.puntaIzquierda->setFillColor(color);
    cable.puntaIzquierda->setPosition(sf::Vector2f(x, y));
    
    cable.puntaDerecha = std::make_unique<sf::RectangleShape>(sf::Vector2f(250.f, 14.f));
    cable.puntaDerecha->setFillColor(color);
    cable.puntaDerecha->setPosition(sf::Vector2f(x, y));
    
    // Número más grande
    if (m_fontLoaded) {
        cable.textoNumero = std::make_unique<sf::Text>(m_font);
        cable.textoNumero->setCharacterSize(18);
        cable.textoNumero->setFillColor(sf::Color::White);
        cable.textoNumero->setString(std::to_string(numero));
        cable.textoNumero->setPosition(sf::Vector2f(x - 40.f, y - 2.f));
    }
    
    m_cables.push_back(std::move(cable));
}

void MinigameCables::inicializarCables() {
    m_cables.clear();
    m_ordenCorrecto.clear();
    
    // Calcular posición centrada de los cables
    float cablesWidth = 400.f;
    float cablesHeight = 260.f; //mas alto
    float cablesX = m_position.x + (m_size.x - cablesWidth) / 2.4f;
    float cablesY = m_position.y + m_size.y - cablesHeight - 150.f;
    
    std::vector<sf::Color> colores = {sf::Color::Red, sf::Color::Blue, sf::Color::Green, sf::Color::Yellow};
    std::vector<std::string> nombres = {"ROJO", "AZUL", "VERDE", "AMARILLO"};
    
    float startY = cablesY + 22.f;
    float spacing = 50.f; //espacio entre cables
    
    for (int i = 0; i < 4; i++) {
        float x = cablesX + 80.f;  // Más margen izquierdo para números
        float y = startY + i * spacing;
        crearCable(colores[i], nombres[i], i + 1, x, y);
    }
    

    
    if (!m_ordenPredefinido.empty() && m_ordenPredefinido.size() == 4) {
        m_ordenCorrecto = m_ordenPredefinido;
    } else {
        m_ordenCorrecto = {0, 1, 2, 3};
        std::shuffle(m_ordenCorrecto.begin(), m_ordenCorrecto.end(), m_gen);
    }
    
    std::string pistasCombinadas = "";
    
    if (!m_pista1.empty()) {
        pistasCombinadas = m_pista1;
    }
    
    if (!m_pista2.empty()) {
        if (!pistasCombinadas.empty()) {
            pistasCombinadas += " | ";
        }
        pistasCombinadas += m_pista2;
    }
    
    if (!pistasCombinadas.empty() && m_textoPista1) {
        m_textoPista1->setString("Pistas: " + pistasCombinadas);
        m_textoPista1->setFillColor(sf::Color::Cyan);
    } else if (m_textoPista1) {
        m_textoPista1->setString("Sin pistas. Busca en el nivel!");
        m_textoPista1->setFillColor(sf::Color(150, 150, 150));
    }
    
    if (m_textoPista2) {
        m_textoPista2->setString("");
    }
    
    actualizarRayosElectricos(0.0f);
}

void MinigameCables::actualizarRayosElectricos(float dt) {
    m_tiempoRayos += dt;
    
    if (m_tiempoRayos > 0.1f || m_rayosElectricos.empty()) {
        m_tiempoRayos = 0.0f;
        m_rayosElectricos.clear();
        
        // Rayos en toda la pantalla, no solo en el panel
        int numRayos = 8 + (m_gen() % 6);
        
        for (int i = 0; i < numRayos; i++) {
            float x1 = m_position.x + static_cast<float>(m_gen() % static_cast<int>(m_size.x));
            float y1 = m_position.y + static_cast<float>(m_gen() % static_cast<int>(m_size.y));
            float x2 = x1 + (m_dist(m_gen) * 80.f);
            float y2 = y1 + (m_dist(m_gen) * 80.f);
            
            sf::Color colorRayo = (m_gen() % 2 == 0) ? 
                sf::Color(100, 150, 255, 150) : sf::Color(150, 200, 255, 100);
            
            sf::Vertex v1;
            v1.position = sf::Vector2f(x1, y1);
            v1.color = colorRayo;
            
            sf::Vertex v2;
            v2.position = sf::Vector2f(x2, y2);
            v2.color = colorRayo;
            
            m_rayosElectricos.push_back(v1);
            m_rayosElectricos.push_back(v2);
        }
    }
}

void MinigameCables::actualizarParpadeo(float dt) {
    if (m_errores >= m_maxErrores - 1) {
        m_tiempoParpadeo += dt;
        if (m_tiempoParpadeo > 0.3f) {
            m_tiempoParpadeo = 0.0f;
            m_parpadeoVisible = !m_parpadeoVisible;
        }
    }
}

void MinigameCables::crearChispas(Cable& cable) {
    float cx = cable.formaCable->getPosition().x + 125.f;  // Centro del cable (250/2)
    float cy = cable.formaCable->getPosition().y + 7.f;     // Centro vertical (14/2)
    
    for (int i = 0; i < 15; i++) {
        sf::CircleShape chispa(2.5f);
        chispa.setFillColor(sf::Color(255, 255, 100));
        chispa.setPosition(sf::Vector2f(cx, cy));
        cable.chispas.push_back(chispa);
    }
    cable.tiempoChispas = 0.7f;
}

void MinigameCables::cortarCable(int indice) {
    if (indice < 0 || indice >= static_cast<int>(m_cables.size())) return;
    if (m_cables[indice].cortado || m_cables[indice].animandoCorte) return;
    if (m_gameOver) return;
    
    if (m_ordenCorrecto[m_cablesCortados] == indice) {
        m_cables[indice].animandoCorte = true;
        m_cables[indice].tiempoAnimacion = 0.0f;
        m_cables[indice].cortado = true;
        
        crearChispas(m_cables[indice]);
        
        m_cablesCortados++;
        
        if (m_textoEstado) {
            m_textoEstado->setString("CORRECTO! " + std::to_string(m_cablesCortados) + "/4 cables cortados");
            m_textoEstado->setFillColor(sf::Color::Green);
        }
        
        m_mostrarTijeras = true;
        m_tiempoTijeras = 0.0f;
        m_posicionTijeras = sf::Vector2f(
    m_cables[indice].formaCable->getPosition().x + 125.f,  // Centro
    m_cables[indice].formaCable->getPosition().y
);
        
    } else {
        m_errores++;
        
        if (m_textoEstado) {
            m_textoEstado->setString("ERROR! Orden incorrecto.");
            m_textoEstado->setFillColor(sf::Color::Red);
        }
        if (m_textoErrores) {
            m_textoErrores->setString("Errores: " + std::to_string(m_errores) + "/" + std::to_string(m_maxErrores));
        }
        
        if (m_errores >= m_maxErrores) {
            if (m_textoEstado) {
                m_textoEstado->setString("HAS FALLADO! Demasiados errores...");
            }
            m_gameOver = true;
            m_tiempoGameOver = 0.0f;
            if (m_onFail) m_onFail();
        }
    }
    
    verificarVictoria();
}

void MinigameCables::actualizarAnimacionCorte(Cable& cable, float dt) {
    if (!cable.animandoCorte) return;
    
    cable.tiempoAnimacion += dt;
    float progreso = cable.tiempoAnimacion / 0.6f;
    
    if (progreso >= 1.0f) {
        cable.animandoCorte = false;
        cable.formaCable->setFillColor(sf::Color(50, 50, 50));
        cable.formaCable->setOutlineColor(sf::Color::Green);
        return;
    }
    
    float separacion = progreso * 40.f;
    float x = cable.formaCable->getPosition().x;
    float y = cable.formaCable->getPosition().y;
    
    cable.puntaIzquierda->setPosition(sf::Vector2f(x - separacion, y));
    cable.puntaDerecha->setPosition(sf::Vector2f(x + separacion, y));
    cable.formaCable->setFillColor(sf::Color::Transparent);
    
    if (cable.tiempoChispas > 0) {
        cable.tiempoChispas -= dt;
        float cx = x + 75.f;
        float cy = y + 5.f;
        
        for (auto& chispa : cable.chispas) {
            float angle = (m_gen() % 360) * 3.14159f / 180.f;
            float speed = (m_gen() % 120) * cable.tiempoChispas;
            chispa.move(sf::Vector2f(std::cos(angle) * speed * dt, 
                                     std::sin(angle) * speed * dt - 50.f * dt));
            
            sf::Color colorChispa = chispa.getFillColor();
            colorChispa.a = static_cast<uint8_t>(255 * cable.tiempoChispas / 0.7f);
            chispa.setFillColor(colorChispa);
        }
    }
}

void MinigameCables::verificarVictoria() {
    if (m_cablesCortados >= 4) {
        m_completed = true;
        if (m_textoEstado) {
            m_textoEstado->setString("BOMBA DESACTIVADA!");
            m_textoEstado->setFillColor(sf::Color::Green);
        }
        if (m_onComplete) m_onComplete();
    }
}

void MinigameCables::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_active || m_completed || m_gameOver) return;
    
    if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseButton->button == sf::Mouse::Button::Left) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
            
            for (size_t i = 0; i < m_cables.size(); i++) {
                if (!m_cables[i].cortado && !m_cables[i].animandoCorte &&
                    m_cables[i].formaCable->getGlobalBounds().contains(worldPos)) {
                    cortarCable(static_cast<int>(i));
                    break;
                }
            }
        }
    }
    
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code >= sf::Keyboard::Key::Num1 && keyPressed->code <= sf::Keyboard::Key::Num4) {
            int indice = static_cast<int>(keyPressed->code) - static_cast<int>(sf::Keyboard::Key::Num1);
            if (indice < static_cast<int>(m_cables.size())) {
                cortarCable(indice);
            }
        }
    }
}

void MinigameCables::dibujarTijeras(sf::RenderWindow& window) {
    if (!m_mostrarTijeras) return;
    if (m_tiempoTijeras > 0.5f) {
        m_mostrarTijeras = false;
        return;
    }
    
    float x = m_posicionTijeras.x;
    float y = m_posicionTijeras.y;
    float apertura = std::sin(m_tiempoTijeras * 20.f) * 15.f;
    
    m_tijeraHoja1.setSize(sf::Vector2f(45.f, 7.f));
    m_tijeraHoja1.setPosition(sf::Vector2f(x - 45.f, y - 12.f));
    m_tijeraHoja1.setRotation(sf::degrees(-25.f + apertura));
    window.draw(m_tijeraHoja1);
    
    m_tijeraHoja2.setSize(sf::Vector2f(45.f, 7.f));
    m_tijeraHoja2.setPosition(sf::Vector2f(x - 45.f, y + 5.f));
    m_tijeraHoja2.setRotation(sf::degrees(25.f - apertura));
    window.draw(m_tijeraHoja2);
    
    m_tijeraPivote.setPosition(sf::Vector2f(x - 5.f, y - 5.f));
    window.draw(m_tijeraPivote);
}

void MinigameCables::update(float dt) {
    if (!m_active) return;
    
    if (m_gameOver) {
        m_tiempoGameOver += dt;
        if (m_tiempoGameOver > 2.0f) {
            deactivate();
        }
        return;
    }
    
    actualizarRayosElectricos(dt);
    actualizarParpadeo(dt);
    
    if (m_mostrarTijeras) {
        m_tiempoTijeras += dt;
    }
    
    for (auto& cable : m_cables) {
        if (cable.animandoCorte) {
            actualizarAnimacionCorte(cable, dt);
        }
    }
}

void MinigameCables::draw(sf::RenderWindow& window) {
    if (!m_active) return;
    
    // ===== 1. FONDO (IMAGEN O COLOR) =====
    if (m_spriteFondo && m_fondoCargado) {
        window.draw(*m_spriteFondo);
    } else {
        window.draw(m_background);
    }
    
    // ===== 2. RAYOS ELÉCTRICOS =====
    window.draw(m_rayosElectricos.data(), m_rayosElectricos.size(), sf::PrimitiveType::Lines);
    
    // ===== 3. SUB-PANEL DE CABLES (TRANSPARENTE, SOLO BORDE) =====
    float panelX = m_panel.getPosition().x;
    float panelY = m_panel.getPosition().y;
    float panelW = m_panel.getSize().x;
    float panelH = m_panel.getSize().y;
    
    // Cambiar: centrar el panel de cables y hacerlo más grande
    float cablesWidth = 400.f;
    float cablesHeight = 200.f;
    float cablesX = m_position.x + (m_size.x - cablesWidth) / 2.f;  // Centrado horizontal
    float cablesY = m_position.y + m_size.y - cablesHeight - 40.f;  // Abajo
    
    sf::RectangleShape panelCables;
    panelCables.setSize(sf::Vector2f(cablesWidth, cablesHeight));
    panelCables.setPosition(sf::Vector2f(cablesX, cablesY));
    panelCables.setFillColor(sf::Color::Transparent);  // TRANSPARENTE
    panelCables.setOutlineColor(sf::Color(100, 100, 120, 80));
    panelCables.setOutlineThickness(1.f);
    window.draw(panelCables);
    
    // ===== 4. CABLES (MÁS GRANDES Y CENTRADOS) =====
    for (const auto& cable : m_cables) {
        if (cable.animandoCorte) {
            window.draw(*cable.puntaIzquierda);
            window.draw(*cable.puntaDerecha);
        } else {
            window.draw(*cable.formaCable);
        }
        
        for (const auto& chispa : cable.chispas) {
            if (chispa.getFillColor().a > 0) {
                window.draw(chispa);
            }
        }
        
        if (cable.textoNumero && !cable.animandoCorte) {
            window.draw(*cable.textoNumero);
        }
    }
    
    // ===== 5. TIJERAS =====
    dibujarTijeras(window);
    
    // ===== 6. TEXTOS =====
    if (m_textoInstrucciones) {
        sf::FloatRect bounds = m_textoInstrucciones->getLocalBounds();
        m_textoInstrucciones->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f - bounds.size.x / 2.f,
            m_position.y + 8.f
        ));
        window.draw(*m_textoInstrucciones);
    }
    
    if (m_textoEstado) {
        sf::FloatRect bounds = m_textoEstado->getLocalBounds();
        m_textoEstado->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f - bounds.size.x / 2.f,
            cablesY + cablesHeight + 10.f
        ));
        window.draw(*m_textoEstado);
    }
    
    if (m_textoErrores) {
        m_textoErrores->setPosition(sf::Vector2f(
            m_position.x + m_size.x - 150.f,
            m_position.y + 8.f
        ));
        window.draw(*m_textoErrores);
    }
    
    // Pistas
    if (m_textoPista1 && !m_textoPista1->getString().isEmpty()) {
        sf::FloatRect bounds = m_textoPista1->getLocalBounds();
        
        sf::RectangleShape fondoPista(sf::Vector2f(bounds.size.x + 20.f, bounds.size.y + 10.f));
        fondoPista.setFillColor(sf::Color(0, 0, 0, 200));
        fondoPista.setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f - bounds.size.x / 2.f - 10.f,
            m_position.y + m_size.y - 28.f
        ));
        window.draw(fondoPista);
        
        m_textoPista1->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f - bounds.size.x / 2.f,
            m_position.y + m_size.y - 25.f
        ));
        window.draw(*m_textoPista1);
    }
    
    // ===== 7. GAME OVER =====
    if (m_gameOver) {
        m_overlayGameOver.setSize(m_size);
        m_overlayGameOver.setPosition(m_position);
        window.draw(m_overlayGameOver);
    }
}