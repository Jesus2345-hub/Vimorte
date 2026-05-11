#include "MinigameMemoria.hpp"
#include <iostream>
#include <algorithm>

MinigameMemoria::MinigameMemoria() 
    : m_gen(std::random_device{}())
    , m_intervaloBotones(0.9f)      // Más lento: 0.9s entre botones (antes 0.6)
    , m_duracionIluminado(0.6f)     // Más tiempo brillando: 0.6s (antes 0.4)
{
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    
    if (m_fontLoaded) {
        m_textoInstrucciones = std::make_unique<sf::Text>(m_font);
        m_textoInstrucciones->setCharacterSize(22);
        m_textoInstrucciones->setFillColor(sf::Color::White);
        m_textoInstrucciones->setString("MEMORIZA LA SECUENCIA");
        
        m_textoEstado = std::make_unique<sf::Text>(m_font);
        m_textoEstado->setCharacterSize(18);
        m_textoEstado->setFillColor(sf::Color::Yellow);
        
        m_textoRonda = std::make_unique<sf::Text>(m_font);
        m_textoRonda->setCharacterSize(16);
        m_textoRonda->setFillColor(sf::Color(200, 200, 200));
    }
    
    if (m_texturaFondo.loadFromFile("assets/images/niveles/nivel3/fondo_memoria.png")) {
        m_spriteFondo = std::make_unique<sf::Sprite>(m_texturaFondo);
        m_fondoCargado = true;
    }
    
    m_background.setFillColor(sf::Color(10, 10, 30, 230));
    m_panel.setFillColor(sf::Color::Transparent);
    m_panel.setOutlineColor(sf::Color::Transparent);
    m_panel.setOutlineThickness(0.f);
}

void MinigameMemoria::setSize(const sf::Vector2f& size) {
    m_size = size;
    m_background.setSize(size);
    
    if (m_spriteFondo && m_fondoCargado) {
        float escalaX = size.x / m_texturaFondo.getSize().x;
        float escalaY = size.y / m_texturaFondo.getSize().y;
        float escala = std::min(escalaX, escalaY);
        m_spriteFondo->setScale(sf::Vector2f(escala, escala));
        sf::FloatRect bounds = m_spriteFondo->getLocalBounds();
        m_spriteFondo->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    }
}

void MinigameMemoria::setPosition(const sf::Vector2f& position) {
    m_position = position;
    m_background.setPosition(position);
    
    if (m_spriteFondo && m_fondoCargado) {
        m_spriteFondo->setPosition(sf::Vector2f(
            position.x + m_size.x / 2.f,
            position.y + m_size.y / 2.f
        ));
    }
    
    // Reposicionar botones
    if (!m_botones.empty()) {
        float btnSize = 100.f;
        float spacing = 30.f;
        float totalWidth = 4 * btnSize + 3 * spacing;
        float startX = m_position.x + m_size.x / 2.f - totalWidth / 2.f;
        float startY = m_position.y + m_size.y / 2.f - btnSize / 2.f;
        
        for (size_t i = 0; i < m_botones.size(); i++) {
            m_botones[i].forma.setPosition(sf::Vector2f(startX + i * (btnSize + spacing), startY));
        }
    }
}

void MinigameMemoria::activate() {
    m_active = true;
    m_completed = false;
    m_rondaActual = 0;
    m_longitudSecuencia = 3;
    m_secuencia.clear();
    m_secuenciaJugador.clear();
    m_pasoActual = 0;
    m_estado = Estado::MOSTRANDO_SECUENCIA;
    m_indiceMostrando = 0;
    m_tiempoEntreBotones = 0.0f;
    m_tiempoMensaje = 0.0f;
    
    inicializarBotones();
    generarSecuencia();
}

void MinigameMemoria::deactivate() {
    m_active = false;
}

sf::Color MinigameMemoria::obtenerColorBoton(int indice) const {
    switch (indice) {
        case 0: return sf::Color(200, 50, 50);    // Rojo
        case 1: return sf::Color(50, 150, 50);    // Verde
        case 2: return sf::Color(50, 50, 200);    // Azul
        case 3: return sf::Color(200, 180, 50);   // Amarillo
        default: return sf::Color::White;
    }
}

void MinigameMemoria::inicializarBotones() {
    m_botones.clear();
    
    float btnSize = 100.f;
    float spacing = 30.f;
    float totalWidth = 4 * btnSize + 3 * spacing;
    float startX = m_position.x + m_size.x / 2.f - totalWidth / 2.f;
    float startY = m_position.y + m_size.y / 2.f - btnSize / 2.f;
    
    for (int i = 0; i < 4; i++) {
        BotonReactor boton;
        boton.forma.setSize(sf::Vector2f(btnSize, btnSize));
        boton.forma.setPosition(sf::Vector2f(startX + i * (btnSize + spacing), startY));
        boton.forma.setOutlineThickness(3.f);
        boton.forma.setOutlineColor(sf::Color(60, 60, 60));
        boton.colorNormal = obtenerColorBoton(i);
        boton.colorBrillante = sf::Color(
            std::min(255, boton.colorNormal.r + 100),
            std::min(255, boton.colorNormal.g + 100),
            std::min(255, boton.colorNormal.b + 100)
        );
        boton.forma.setFillColor(boton.colorNormal);
        boton.iluminado = false;
        boton.tiempoIluminado = 0.0f;
        boton.indice = i;
        
        m_botones.push_back(boton);
    }
}

void MinigameMemoria::generarSecuencia() {
    m_secuencia.clear();
    std::uniform_int_distribution<int> dist(0, 3);
    
    int anterior = -1;  // Para evitar repeticiones
    
    for (int i = 0; i < m_longitudSecuencia; i++) {
        int nuevo;
        do {
            nuevo = dist(m_gen);
        } while (nuevo == anterior);  // Repetir si sale igual al anterior
        
        m_secuencia.push_back(nuevo);
        anterior = nuevo;
    }
    
    m_indiceMostrando = 0;
    m_estado = Estado::MOSTRANDO_SECUENCIA;
    m_tiempoEntreBotones = 0.0f;
    
    // Apagar todos los botones
    for (auto& boton : m_botones) {
        boton.iluminado = false;
        boton.forma.setFillColor(boton.colorNormal);
    }
    
    std::cout << "[MEMORIA] Ronda " << (m_rondaActual + 1) 
              << " - Secuencia (longitud " << m_longitudSecuencia << "): ";
    for (int s : m_secuencia) std::cout << s << " ";
    std::cout << std::endl;
}

void MinigameMemoria::mostrarSiguienteBoton() {
    if (m_indiceMostrando >= static_cast<int>(m_secuencia.size())) {
        // Terminó de mostrar, ahora el jugador debe repetir
        m_estado = Estado::ESPERANDO_JUGADOR;
        m_secuenciaJugador.clear();
        m_pasoActual = 0;
        
        // Apagar todos los botones
        for (auto& boton : m_botones) {
            boton.iluminado = false;
            boton.forma.setFillColor(boton.colorNormal);
        }
        return;
    }
    
    // APAGAR TODOS los botones primero (para crear un "apagón" entre números)
    for (auto& boton : m_botones) {
        boton.iluminado = false;
        boton.forma.setFillColor(boton.colorNormal);
    }
    
    // Pequeña pausa de oscuridad antes de iluminar el siguiente
    // (esto se maneja con m_tiempoEntreBotones en update)
    
    // Iluminar botón actual
    int indice = m_secuencia[m_indiceMostrando];
    m_botones[indice].iluminado = true;
    m_botones[indice].forma.setFillColor(m_botones[indice].colorBrillante);
    m_botones[indice].tiempoIluminado = 0.0f;
    
    m_indiceMostrando++;
}

void MinigameMemoria::botonPresionado(int indice) {
    if (m_estado != Estado::ESPERANDO_JUGADOR) return;
    
    // Iluminar botón brevemente
    m_botones[indice].iluminado = true;
    m_botones[indice].forma.setFillColor(m_botones[indice].colorBrillante);
    m_botones[indice].tiempoIluminado = 0.0f;
    
    // Verificar
    if (indice == m_secuencia[m_pasoActual]) {
        m_secuenciaJugador.push_back(indice);
        m_pasoActual++;
        
        if (m_pasoActual >= static_cast<int>(m_secuencia.size())) {
            // ¡Ronda completada!
            avanzarRonda();
        }
    } else {
        // Error
        m_estado = Estado::ERROR;
        m_tiempoMensaje = 0.0f;
        
        if (m_textoEstado) {
            m_textoEstado->setString("ERROR! Reiniciando...");
            m_textoEstado->setFillColor(sf::Color::Red);
        }
    }
}

void MinigameMemoria::avanzarRonda() {
    m_rondaActual++;
    
    if (m_rondaActual >= m_rondasMaximas) {
        // ¡Victoria!
        m_completed = true;
        if (m_textoEstado) {
            m_textoEstado->setString("SECUENCIA COMPLETADA!");
            m_textoEstado->setFillColor(sf::Color::Green);
        }
        if (m_onComplete) m_onComplete();
    } else {
        m_longitudSecuencia++;
        m_estado = Estado::CORRECTO;
        m_tiempoMensaje = 0.0f;
        
        if (m_textoEstado) {
            m_textoEstado->setString("CORRECTO! Siguiente ronda...");
            m_textoEstado->setFillColor(sf::Color::Green);
        }
        if (m_textoRonda) {
            m_textoRonda->setString("Ronda " + std::to_string(m_rondaActual + 1) + "/" + std::to_string(m_rondasMaximas));
        }
    }
}

void MinigameMemoria::reiniciar() {
    m_rondaActual = 0;
    m_longitudSecuencia = 3;
    m_secuencia.clear();
    m_secuenciaJugador.clear();
    m_pasoActual = 0;
    m_estado = Estado::MOSTRANDO_SECUENCIA;
    m_indiceMostrando = 0;
    m_tiempoEntreBotones = 0.0f;
    
    for (auto& boton : m_botones) {
        boton.iluminado = false;
        boton.forma.setFillColor(boton.colorNormal);
    }
    
    if (m_textoRonda) {
        m_textoRonda->setString("Ronda 1/" + std::to_string(m_rondasMaximas));
    }
    
    generarSecuencia();
}

void MinigameMemoria::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_active || m_completed) return;
    
    if (m_estado == Estado::ESPERANDO_JUGADOR) {
        if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButton->button == sf::Mouse::Button::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
                
                for (size_t i = 0; i < m_botones.size(); i++) {
                    if (m_botones[i].forma.getGlobalBounds().contains(worldPos)) {
                        botonPresionado(static_cast<int>(i));
                        break;
                    }
                }
            }
        }
    }
}

void MinigameMemoria::update(float dt) {
    if (!m_active) return;
    
    // Actualizar iluminación de botones
    for (auto& boton : m_botones) {
        if (boton.iluminado) {
            boton.tiempoIluminado += dt;
            if (boton.tiempoIluminado > m_duracionIluminado) {
                boton.iluminado = false;
                if (m_estado == Estado::ESPERANDO_JUGADOR || m_estado == Estado::CORRECTO) {
                    boton.forma.setFillColor(boton.colorNormal);
                }
            }
        }
    }
    
    // Lógica de mostrar secuencia
if (m_estado == Estado::MOSTRANDO_SECUENCIA) {
    m_tiempoEntreBotones += dt;
    
    // Fase 0: Pausa inicial antes de empezar
    if (m_indiceMostrando == 0 && m_tiempoEntreBotones > 1.0f) {
        mostrarSiguienteBoton();
        m_tiempoEntreBotones = 0.0f;
    }
    // Entre botones: esperar el intervalo completo (incluye tiempo de brillo + apagón)
    else if (m_indiceMostrando > 0 && m_tiempoEntreBotones > m_intervaloBotones) {
        // Verificar si el botón actual ya brilló suficiente
        int indiceActual = m_secuencia[m_indiceMostrando - 1];
        if (m_botones[indiceActual].tiempoIluminado >= m_duracionIluminado) {
            // Apagar botón y mostrar siguiente
            m_botones[indiceActual].iluminado = false;
            m_botones[indiceActual].forma.setFillColor(m_botones[indiceActual].colorNormal);
            
            m_tiempoEntreBotones = 0.0f;
            mostrarSiguienteBoton();
        }
    }
    
    // Si ya se mostraron todos, verificar si el último botón terminó de brillar
    if (m_indiceMostrando >= static_cast<int>(m_secuencia.size()) && m_estado == Estado::MOSTRANDO_SECUENCIA) {
        int ultimoIndice = m_secuencia.back();
        if (m_botones[ultimoIndice].tiempoIluminado >= m_duracionIluminado) {
            // Apagar todo y pasar al jugador
            for (auto& boton : m_botones) {
                boton.iluminado = false;
                boton.forma.setFillColor(boton.colorNormal);
            }
            m_estado = Estado::ESPERANDO_JUGADOR;
            m_secuenciaJugador.clear();
            m_pasoActual = 0;
            
            if (m_textoEstado) {
                m_textoEstado->setString("TU TURNO! Repite la secuencia");
                m_textoEstado->setFillColor(sf::Color::Yellow);
            }
        }
    }
}
    
    // Mensaje de correcto -> pasar a siguiente ronda
    if (m_estado == Estado::CORRECTO) {
        m_tiempoMensaje += dt;
        if (m_tiempoMensaje > 2.0f) {  // 2 segundos para leer el mensaje
            m_tiempoMensaje = 0.0f;
            generarSecuencia();
        }
    }
    
    // Mensaje de error -> reiniciar
    if (m_estado == Estado::ERROR) {
        m_tiempoMensaje += dt;
        if (m_tiempoMensaje > 2.0f) {  // 2 segundos para leer el error
            m_tiempoMensaje = 0.0f;
            reiniciar();
        }
    }
}

void MinigameMemoria::draw(sf::RenderWindow& window) {
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
            m_position.y + 15.f
        ));
        window.draw(*m_textoInstrucciones);
    }
    
    // Ronda
    if (m_textoRonda) {
        sf::FloatRect bounds = m_textoRonda->getLocalBounds();
        m_textoRonda->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f - bounds.size.x / 2.f,
            m_position.y + 50.f
        ));
        window.draw(*m_textoRonda);
    }
    
    // Botones del reactor
    for (const auto& boton : m_botones) {
        window.draw(boton.forma);
        
        // Número en el botón
        if (m_fontLoaded) {
            sf::Text numero(m_font);
            numero.setString(std::to_string(boton.indice + 1));
            numero.setCharacterSize(30);
            numero.setFillColor(sf::Color::White);
            sf::FloatRect b = numero.getLocalBounds();
            numero.setOrigin(sf::Vector2f(b.size.x / 2.f, b.size.y / 2.f));
            numero.setPosition(sf::Vector2f(
                boton.forma.getPosition().x + boton.forma.getSize().x / 2.f,
                boton.forma.getPosition().y + boton.forma.getSize().y / 2.f
            ));
            window.draw(numero);
        }
    }
    
    // Estado (ESPERANDO / CORRECTO / ERROR)
    if (m_textoEstado) {
        sf::FloatRect bounds = m_textoEstado->getLocalBounds();
        m_textoEstado->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f - bounds.size.x / 2.f,
            m_position.y + m_size.y - 60.f
        ));
        window.draw(*m_textoEstado);
    }
    
   // Indicador de estado del juego
std::string estadoStr = "";
sf::Color colorIndicador = sf::Color::White;

if (m_estado == Estado::MOSTRANDO_SECUENCIA) {
    estadoStr = "OBSERVA LA SECUENCIA...";
    colorIndicador = sf::Color(255, 200, 50);
} else if (m_estado == Estado::ESPERANDO_JUGADOR) {
    estadoStr = "TU TURNO! Repite";
    colorIndicador = sf::Color(50, 255, 50);
}

if (!estadoStr.empty() && m_fontLoaded) {
    sf::Text indicador(m_font);
    indicador.setString(estadoStr);
    indicador.setCharacterSize(18);
    indicador.setFillColor(colorIndicador);
    sf::FloatRect b = indicador.getLocalBounds();
    indicador.setOrigin(sf::Vector2f(b.size.x / 2.f, 0.f));
    indicador.setPosition(sf::Vector2f(
        m_position.x + m_size.x / 2.f,
        m_botones[0].forma.getPosition().y - 60.f
    ));
    window.draw(indicador);
}
}