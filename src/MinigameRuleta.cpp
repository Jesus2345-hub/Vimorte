#include "MinigameRuleta.hpp"
#include <iostream>
#include <algorithm>

MinigameRuleta::MinigameRuleta() 
    : m_isActive(false), m_gen(m_rd())
{
    // Fondo oscuro
    m_background.setFillColor(sf::Color(0, 0, 0, 220));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(180, 150, 50));
    
    // Cargar fuente
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (!m_fontLoaded) {
        std::cerr << "Error cargando fuente en MinigameRuleta" << std::endl;
        return;
    }
    
    inicializarUI();
}

void MinigameRuleta::setPosition(const sf::Vector2f& pos) {
    m_position = pos;
    m_background.setPosition(pos);
}

void MinigameRuleta::setSize(const sf::Vector2f& size) {
    m_size = size;
    m_background.setSize(size);
    
    // Cargar textura de la ruleta
    if (m_ruletaTexture.loadFromFile("assets/images/niveles/nivel2/ruleta.jpg")) {
        m_ruletaSprite = std::make_unique<sf::Sprite>(m_ruletaTexture);
        
        float escala = 0.7f;
        m_ruletaSprite->setScale(sf::Vector2f(escala, escala));
        
        sf::FloatRect bounds = m_ruletaSprite->getLocalBounds();
        m_ruletaSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_ruletaSprite->setPosition(sf::Vector2f(
            m_position.x + m_size.x * 0.3f,
            m_position.y + m_size.y * 0.45f
        ));
    }
}

void MinigameRuleta::inicializarUI() {
    if (!m_fontLoaded) return;
    
    // Título
    m_tituloText = std::make_unique<sf::Text>(m_font, "RULETA VIMORTE", 30);
    m_tituloText->setFillColor(sf::Color::Yellow);
    m_tituloText->setStyle(sf::Text::Bold);
    
    // Instrucciones
    m_instruccionesText = std::make_unique<sf::Text>(m_font, 
        "1. Elige tipo de apuesta | 2. Selecciona numero/color | 3. GIRA\n"
        "Apuesta minima: $5 | ESC para salir",
    12);
    m_instruccionesText->setFillColor(sf::Color(180, 180, 180));
    
    // Texto de apuesta
    m_apuestaText = std::make_unique<sf::Text>(m_font, "Apuesta: $5", 20);
    m_apuestaText->setFillColor(sf::Color::White);
    
    // Texto de resultado
    m_resultadoText = std::make_unique<sf::Text>(m_font, "", 28);
    m_resultadoText->setFillColor(sf::Color::White);
    
    // Texto de dinero
    m_dineroText = std::make_unique<sf::Text>(m_font, "", 22);
    m_dineroText->setFillColor(sf::Color::Yellow);
    
    // ===== BOTONES PRINCIPALES =====
    float btnX = m_position.x + m_size.x * 0.55f;
    float btnYStart = m_position.y + 80.f;
    float btnWidth = 250.f;
    float btnHeight = 40.f;
    float btnSpacing = 55.f;
    
    // Botón Número Exacto
    m_btnNumeroExacto.setSize(sf::Vector2f(btnWidth, btnHeight));
    m_btnNumeroExacto.setPosition(sf::Vector2f(btnX, btnYStart));
    m_btnNumeroExacto.setFillColor(sf::Color(100, 0, 0, 200));
    m_btnNumeroExacto.setOutlineThickness(2.f);
    m_btnNumeroExacto.setOutlineColor(sf::Color::Red);
    
    m_textoBtnExacto = std::make_unique<sf::Text>(m_font, "NUMERO EXACTO (35:1)", 16);
    m_textoBtnExacto->setFillColor(sf::Color::White);
    
    // Botón Cuarto
    m_btnCuarto.setSize(sf::Vector2f(btnWidth, btnHeight));
    m_btnCuarto.setPosition(sf::Vector2f(btnX, btnYStart + btnSpacing));
    m_btnCuarto.setFillColor(sf::Color(0, 80, 0, 200));
    m_btnCuarto.setOutlineThickness(2.f);
    m_btnCuarto.setOutlineColor(sf::Color::Green);
    
    m_textoBtnCuarto = std::make_unique<sf::Text>(m_font, "CUARTO (3:1)", 16);
    m_textoBtnCuarto->setFillColor(sf::Color::White);
    
    // Botón Color
    m_btnColor.setSize(sf::Vector2f(btnWidth, btnHeight));
    m_btnColor.setPosition(sf::Vector2f(btnX, btnYStart + btnSpacing * 2));
    m_btnColor.setFillColor(sf::Color(0, 0, 100, 200));
    m_btnColor.setOutlineThickness(2.f);
    m_btnColor.setOutlineColor(sf::Color::Blue);
    
    m_textoBtnColor = std::make_unique<sf::Text>(m_font, "COLOR (2:1)", 16);
    m_textoBtnColor->setFillColor(sf::Color::White);
    
    // ===== BOTONES DE APUESTA (+/-) =====
    float apuestaY = btnYStart + btnSpacing * 3.5f;
    
    m_btnDisminuir.setSize(sf::Vector2f(40.f, 40.f));
    m_btnDisminuir.setPosition(sf::Vector2f(btnX, apuestaY));
    m_btnDisminuir.setFillColor(sf::Color(150, 0, 0, 200));
    m_btnDisminuir.setOutlineThickness(1.f);
    m_btnDisminuir.setOutlineColor(sf::Color::Red);
    
    m_textoBtnDisminuir = std::make_unique<sf::Text>(m_font, "-", 20);
    m_textoBtnDisminuir->setFillColor(sf::Color::White);
    
    m_btnAumentar.setSize(sf::Vector2f(40.f, 40.f));
    m_btnAumentar.setPosition(sf::Vector2f(btnX + 60.f, apuestaY));
    m_btnAumentar.setFillColor(sf::Color(0, 150, 0, 200));
    m_btnAumentar.setOutlineThickness(1.f);
    m_btnAumentar.setOutlineColor(sf::Color::Green);
    
    m_textoBtnAumentar = std::make_unique<sf::Text>(m_font, "+", 20);
    m_textoBtnAumentar->setFillColor(sf::Color::White);
    
    // ===== BOTÓN GIRAR =====
    m_btnGirar.setSize(sf::Vector2f(btnWidth, 50.f));
    m_btnGirar.setPosition(sf::Vector2f(btnX, apuestaY + 60.f));
    m_btnGirar.setFillColor(sf::Color(200, 150, 0, 200));
    m_btnGirar.setOutlineThickness(3.f);
    m_btnGirar.setOutlineColor(sf::Color::Yellow);
    
    m_textoBtnGirar = std::make_unique<sf::Text>(m_font, "¡GIRAR!", 24);
    m_textoBtnGirar->setFillColor(sf::Color::White);
    
    // ===== SELECTOR DE NÚMEROS (0-36) =====
    for (int i = 0; i <= 36; i++) {
        sf::RectangleShape btn(sf::Vector2f(38.f, 30.f));
        btn.setFillColor(sf::Color(50, 50, 50, 200));
        btn.setOutlineThickness(1.f);
        btn.setOutlineColor(sf::Color(100, 100, 100));
        m_botonesNumeros.push_back(btn);
        
        auto texto = std::make_unique<sf::Text>(m_font, std::to_string(i), 14);
        texto->setFillColor(sf::Color::White);
        m_textosNumeros.push_back(std::move(texto));
        m_hoverNumeros.push_back(false);
    }
    
    // ===== SELECTOR DE CUARTOS =====
    float cuartoY = m_position.y + 300.f;
    
    m_btnCuarto1.setSize(sf::Vector2f(200.f, 35.f));
    m_btnCuarto1.setFillColor(sf::Color(50, 50, 50, 200));
    m_btnCuarto1.setOutlineThickness(1.f);
    m_btnCuarto1.setOutlineColor(sf::Color(100, 100, 100));
    
    m_btnCuarto2.setSize(sf::Vector2f(200.f, 35.f));
    m_btnCuarto2.setFillColor(sf::Color(50, 50, 50, 200));
    m_btnCuarto2.setOutlineThickness(1.f);
    m_btnCuarto2.setOutlineColor(sf::Color(100, 100, 100));
    
    m_btnCuarto3.setSize(sf::Vector2f(200.f, 35.f));
    m_btnCuarto3.setFillColor(sf::Color(50, 50, 50, 200));
    m_btnCuarto3.setOutlineThickness(1.f);
    m_btnCuarto3.setOutlineColor(sf::Color(100, 100, 100));
    
    m_textoCuarto1 = std::make_unique<sf::Text>(m_font, "1er Cuarto (1-12)", 16);
    m_textoCuarto1->setFillColor(sf::Color::White);
    m_textoCuarto2 = std::make_unique<sf::Text>(m_font, "2do Cuarto (13-24)", 16);
    m_textoCuarto2->setFillColor(sf::Color::White);
    m_textoCuarto3 = std::make_unique<sf::Text>(m_font, "3er Cuarto (25-36)", 16);
    m_textoCuarto3->setFillColor(sf::Color::White);
    
    // ===== SELECTOR DE COLORES =====
    m_btnRojo.setSize(sf::Vector2f(200.f, 35.f));
    m_btnRojo.setFillColor(sf::Color(180, 0, 0, 200));
    m_btnRojo.setOutlineThickness(1.f);
    m_btnRojo.setOutlineColor(sf::Color::Red);
    
    m_btnNegro.setSize(sf::Vector2f(200.f, 35.f));
    m_btnNegro.setFillColor(sf::Color(30, 30, 30, 200));
    m_btnNegro.setOutlineThickness(1.f);
    m_btnNegro.setOutlineColor(sf::Color::White);
    
    m_btnVerde.setSize(sf::Vector2f(200.f, 35.f));
    m_btnVerde.setFillColor(sf::Color(0, 100, 0, 200));
    m_btnVerde.setOutlineThickness(1.f);
    m_btnVerde.setOutlineColor(sf::Color::Green);
    
    m_textoRojo = std::make_unique<sf::Text>(m_font, "ROJO (2:1)", 16);
    m_textoRojo->setFillColor(sf::Color::White);
    m_textoNegro = std::make_unique<sf::Text>(m_font, "NEGRO (2:1)", 16);
    m_textoNegro->setFillColor(sf::Color::White);
    m_textoVerde = std::make_unique<sf::Text>(m_font, "VERDE (36:1)", 16);
    m_textoVerde->setFillColor(sf::Color::White);
}

void MinigameRuleta::activate() {
    m_isActive = true;
    m_tipoApuesta = TipoApuesta::NINGUNA;
    m_numeroElegido = -1;
    m_cuartoElegido = -1;
    m_colorElegido = ColorElegido::NINGUNO;
    m_apuesta = 5;
    m_mostrandoResultado = false;
    m_mostrandoTableroNumeros = false;
    m_mostrandoSelectorCuarto = false;
    m_mostrandoSelectorColor = false;
}

void MinigameRuleta::deactivate() {
    m_isActive = false;
}

bool MinigameRuleta::esRojo(int numero) {
    // Números rojos en la ruleta clásica
    int rojos[] = {1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36};
    for (int r : rojos) {
        if (r == numero) return true;
    }
    return false;
}

bool MinigameRuleta::esNegro(int numero) {
    if (numero == 0) return false;
    return !esRojo(numero);
}

int MinigameRuleta::obtenerCuarto(int numero) {
    if (numero == 0) return -1;
    if (numero <= 12) return 0;
    if (numero <= 24) return 1;
    return 2;
}

void MinigameRuleta::girarRuleta() {
    // Verificar que haya apuesta válida
    if (m_tipoApuesta == TipoApuesta::NINGUNA) return;
    if (!m_dineroJugador) return;
    if (*m_dineroJugador < m_apuesta) return;
    
    // Verificar selección según tipo
    if (m_tipoApuesta == TipoApuesta::NUMERO_EXACTO && m_numeroElegido < 0) return;
    if (m_tipoApuesta == TipoApuesta::CUARTO && m_cuartoElegido < 0) return;
    if (m_tipoApuesta == TipoApuesta::COLOR && m_colorElegido == ColorElegido::NINGUNO) return;
    
    // Cobrar apuesta
    *m_dineroJugador -= m_apuesta;
    
    // Generar número ganador (0-36)
    std::uniform_int_distribution<int> dist(0, 36);
    m_numeroGanador = dist(m_gen);
    
    // Calcular ganancia
    int ganancia = m_apuesta; // Se devuelve la apuesta base
    
    switch (m_tipoApuesta) {
        case TipoApuesta::NUMERO_EXACTO:
            if (m_numeroElegido == m_numeroGanador) {
                ganancia = m_apuesta * 35; // 35:1
                *m_dineroJugador += ganancia;
                m_mensajeResultado = "GANASTE! $" + std::to_string(ganancia) + 
                    "\nNumero: " + std::to_string(m_numeroGanador);
            } else {
                m_mensajeResultado = "Perdiste... Numero: " + std::to_string(m_numeroGanador) +
                    "\nTu numero: " + std::to_string(m_numeroElegido);
            }
            break;
            
        case TipoApuesta::CUARTO: {
            int cuartoGanador = obtenerCuarto(m_numeroGanador);
            if (cuartoGanador == m_cuartoElegido) {
                ganancia = m_apuesta * 3; // 3:1
                *m_dineroJugador += ganancia;
                m_mensajeResultado = "GANASTE! $" + std::to_string(ganancia) +
                    "\nNumero: " + std::to_string(m_numeroGanador);
            } else if (m_numeroGanador == 0) {
                // El 0 no pertenece a ningún cuarto
                m_mensajeResultado = "Cayo 0 verde. Perdiste.";
            } else {
                m_mensajeResultado = "Perdiste... Numero: " + std::to_string(m_numeroGanador);
            }
            break;
        }
        
        case TipoApuesta::COLOR: {
            ColorElegido colorGanador;
            if (m_numeroGanador == 0) {
                colorGanador = ColorElegido::VERDE;
            } else if (esRojo(m_numeroGanador)) {
                colorGanador = ColorElegido::ROJO;
            } else {
                colorGanador = ColorElegido::NEGRO;
            }
            
            if (colorGanador == m_colorElegido) {
                if (m_colorElegido == ColorElegido::VERDE) {
                    ganancia = m_apuesta * 36; // 36:1 para verde
                } else {
                    ganancia = m_apuesta * 2; // 2:1 para rojo/negro
                }
                *m_dineroJugador += ganancia;
                m_mensajeResultado = "GANASTE! $" + std::to_string(ganancia) +
                    "\nNumero: " + std::to_string(m_numeroGanador);
            } else {
                m_mensajeResultado = "Perdiste... Numero: " + std::to_string(m_numeroGanador);
            }
            break;
        }
        
        default:
            break;
    }
    
    m_mostrandoResultado = true;
    m_tiempoResultado = 3.0f;
    
    // Resetear selección
    m_tipoApuesta = TipoApuesta::NINGUNA;
    m_numeroElegido = -1;
    m_cuartoElegido = -1;
    m_colorElegido = ColorElegido::NINGUNO;
    m_mostrandoTableroNumeros = false;
    m_mostrandoSelectorCuarto = false;
    m_mostrandoSelectorColor = false;
    
    std::cout << "Ruleta gira... Numero: " << m_numeroGanador << std::endl;
}

void MinigameRuleta::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_isActive || m_mostrandoResultado) return;
    
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    
    // Actualizar hovers
    m_hoverExacto = m_btnNumeroExacto.getGlobalBounds().contains(mousePos);
    m_hoverCuarto = m_btnCuarto.getGlobalBounds().contains(mousePos);
    m_hoverColor = m_btnColor.getGlobalBounds().contains(mousePos);
    m_hoverGirar = m_btnGirar.getGlobalBounds().contains(mousePos);
    m_hoverAumentar = m_btnAumentar.getGlobalBounds().contains(mousePos);
    m_hoverDisminuir = m_btnDisminuir.getGlobalBounds().contains(mousePos);
    
    // Actualizar colores de hover
    m_btnNumeroExacto.setFillColor(m_hoverExacto ? sf::Color(180, 0, 0, 200) : 
        (m_tipoApuesta == TipoApuesta::NUMERO_EXACTO ? sf::Color(150, 50, 50, 200) : sf::Color(100, 0, 0, 200)));
    m_btnCuarto.setFillColor(m_hoverCuarto ? sf::Color(0, 180, 0, 200) : 
        (m_tipoApuesta == TipoApuesta::CUARTO ? sf::Color(50, 150, 50, 200) : sf::Color(0, 80, 0, 200)));
    m_btnColor.setFillColor(m_hoverColor ? sf::Color(0, 0, 180, 200) : 
        (m_tipoApuesta == TipoApuesta::COLOR ? sf::Color(50, 50, 150, 200) : sf::Color(0, 0, 100, 200)));
    
    // Eventos de click
    if (event.is<sf::Event::MouseButtonPressed>()) {
        const auto& mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            
            // Botón Número Exacto
            if (m_hoverExacto) {
                m_tipoApuesta = TipoApuesta::NUMERO_EXACTO;
                m_mostrandoTableroNumeros = true;
                m_mostrandoSelectorCuarto = false;
                m_mostrandoSelectorColor = false;
                m_numeroElegido = -1;
                std::cout << "Seleccionado: Número Exacto" << std::endl;
            }
            
            // Botón Cuarto
            if (m_hoverCuarto) {
                m_tipoApuesta = TipoApuesta::CUARTO;
                m_mostrandoTableroNumeros = false;
                m_mostrandoSelectorCuarto = true;
                m_mostrandoSelectorColor = false;
                m_cuartoElegido = -1;
                std::cout << "Seleccionado: Cuarto" << std::endl;
            }
            
            // Botón Color
            if (m_hoverColor) {
                m_tipoApuesta = TipoApuesta::COLOR;
                m_mostrandoTableroNumeros = false;
                m_mostrandoSelectorCuarto = false;
                m_mostrandoSelectorColor = true;
                m_colorElegido = ColorElegido::NINGUNO;
                std::cout << "Seleccionado: Color" << std::endl;
            }
            
            // Botón Girar
            if (m_hoverGirar) {
                girarRuleta();
            }
            
            // Botones de apuesta
            if (m_hoverAumentar && m_apuesta < m_apuestaMaxima) {
                m_apuesta += 5;
                if (m_dineroJugador && m_apuesta > *m_dineroJugador) {
                    m_apuesta = *m_dineroJugador;
                }
            }
            if (m_hoverDisminuir && m_apuesta > m_apuestaMinima) {
                m_apuesta -= 5;
            }
            
            // Selector de números
            if (m_mostrandoTableroNumeros) {
                for (size_t i = 0; i < m_botonesNumeros.size(); i++) {
                    if (m_hoverNumeros[i]) {
                        m_numeroElegido = i;
                        std::cout << "Número elegido: " << i << std::endl;
                    }
                }
            }
            
            // Selector de cuartos
            if (m_mostrandoSelectorCuarto) {
                m_hoverCuarto1 = m_btnCuarto1.getGlobalBounds().contains(mousePos);
                m_hoverCuarto2 = m_btnCuarto2.getGlobalBounds().contains(mousePos);
                m_hoverCuarto3 = m_btnCuarto3.getGlobalBounds().contains(mousePos);
                
                if (m_hoverCuarto1) m_cuartoElegido = 0;
                if (m_hoverCuarto2) m_cuartoElegido = 1;
                if (m_hoverCuarto3) m_cuartoElegido = 2;
            }
            
            // Selector de colores
            if (m_mostrandoSelectorColor) {
                m_hoverRojo = m_btnRojo.getGlobalBounds().contains(mousePos);
                m_hoverNegro = m_btnNegro.getGlobalBounds().contains(mousePos);
                m_hoverVerde = m_btnVerde.getGlobalBounds().contains(mousePos);
                
                if (m_hoverRojo) m_colorElegido = ColorElegido::ROJO;
                if (m_hoverNegro) m_colorElegido = ColorElegido::NEGRO;
                if (m_hoverVerde) m_colorElegido = ColorElegido::VERDE;
            }
        }
    }
    
    // Hover para números
    if (m_mostrandoTableroNumeros) {
        for (size_t i = 0; i < m_botonesNumeros.size(); i++) {
            m_hoverNumeros[i] = m_botonesNumeros[i].getGlobalBounds().contains(mousePos);
        }
    }
}

void MinigameRuleta::update(float dt) {
    if (!m_isActive) return;
    
    if (m_mostrandoResultado) {
        m_tiempoResultado -= dt;
        if (m_tiempoResultado <= 0.0f) {
            m_mostrandoResultado = false;
        }
    }
}

void MinigameRuleta::draw(sf::RenderWindow& window) {
    if (!m_isActive) return;
    
    // Fondo
    window.draw(m_background);
    
    // Ruleta (imagen)
    if (m_ruletaSprite) {
        window.draw(*m_ruletaSprite);
    }
    
    // Título
    if (m_tituloText) {
        m_tituloText->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f - 100.f,
            m_position.y + 20.f
        ));
        window.draw(*m_tituloText);
    }
    
    // Instrucciones
    if (m_instruccionesText) {
        m_instruccionesText->setPosition(sf::Vector2f(
            m_position.x + 30.f,
            m_position.y + m_size.y - 50.f
        ));
        window.draw(*m_instruccionesText);
    }
    
    // Dinero
    if (m_dineroText && m_dineroJugador) {
        m_dineroText->setString("Tu dinero: $" + std::to_string(*m_dineroJugador));
        m_dineroText->setPosition(sf::Vector2f(
            m_position.x + 30.f,
            m_position.y + 20.f
        ));
        window.draw(*m_dineroText);
    }
    
    // Texto de apuesta
    if (m_apuestaText) {
        m_apuestaText->setString("Apuesta: $" + std::to_string(m_apuesta));
        m_apuestaText->setPosition(sf::Vector2f(
            m_position.x + m_size.x * 0.55f,
            m_position.y + 250.f
        ));
        window.draw(*m_apuestaText);
    }
    
    // ===== DIBUJAR BOTONES PRINCIPALES =====
    window.draw(m_btnNumeroExacto);
    window.draw(m_btnCuarto);
    window.draw(m_btnColor);
    window.draw(m_btnDisminuir);
    window.draw(m_btnAumentar);
    window.draw(m_btnGirar);
    
    // Textos de botones
    auto centerTextInButton = [](sf::Text& text, const sf::RectangleShape& btn) {
        sf::FloatRect tb = text.getLocalBounds();
        text.setOrigin(sf::Vector2f(tb.size.x / 2.f, tb.size.y / 2.f));
        text.setPosition(sf::Vector2f(
            btn.getPosition().x + btn.getSize().x / 2.f,
            btn.getPosition().y + btn.getSize().y / 2.f
        ));
    };
    
    if (m_textoBtnExacto) {
        centerTextInButton(*m_textoBtnExacto, m_btnNumeroExacto);
        window.draw(*m_textoBtnExacto);
    }
    if (m_textoBtnCuarto) {
        centerTextInButton(*m_textoBtnCuarto, m_btnCuarto);
        window.draw(*m_textoBtnCuarto);
    }
    if (m_textoBtnColor) {
        centerTextInButton(*m_textoBtnColor, m_btnColor);
        window.draw(*m_textoBtnColor);
    }
    if (m_textoBtnDisminuir) {
        centerTextInButton(*m_textoBtnDisminuir, m_btnDisminuir);
        window.draw(*m_textoBtnDisminuir);
    }
    if (m_textoBtnAumentar) {
        centerTextInButton(*m_textoBtnAumentar, m_btnAumentar);
        window.draw(*m_textoBtnAumentar);
    }
    if (m_textoBtnGirar) {
        centerTextInButton(*m_textoBtnGirar, m_btnGirar);
        window.draw(*m_textoBtnGirar);
    }
    
    // ===== SELECTOR DE NÚMEROS =====
    if (m_mostrandoTableroNumeros) {
        float numX = m_position.x + 30.f;
        float numY = m_position.y + 80.f;
        
        for (size_t i = 0; i < m_botonesNumeros.size(); i++) {
            int col = i % 10;
            int row = i / 10;
            
            m_botonesNumeros[i].setPosition(sf::Vector2f(numX + col * 42.f, numY + row * 35.f));
            
            // Color según número
            if (i == 0) {
                m_botonesNumeros[i].setFillColor(sf::Color(0, 100, 0, 200)); // Verde
            } else if (esRojo(i)) {
                m_botonesNumeros[i].setFillColor(sf::Color(180, 0, 0, 200));
            } else {
                m_botonesNumeros[i].setFillColor(sf::Color(30, 30, 30, 200));
            }
            
            // Resaltar seleccionado
            if ((int)i == m_numeroElegido) {
                m_botonesNumeros[i].setOutlineColor(sf::Color::Yellow);
                m_botonesNumeros[i].setOutlineThickness(3.f);
            } else if (m_hoverNumeros[i]) {
                m_botonesNumeros[i].setOutlineColor(sf::Color::White);
                m_botonesNumeros[i].setOutlineThickness(2.f);
            } else {
                m_botonesNumeros[i].setOutlineColor(sf::Color(100, 100, 100));
                m_botonesNumeros[i].setOutlineThickness(1.f);
            }
            
            window.draw(m_botonesNumeros[i]);
            
            if (m_textosNumeros[i]) {
                sf::FloatRect tb = m_textosNumeros[i]->getLocalBounds();
                m_textosNumeros[i]->setOrigin(sf::Vector2f(tb.size.x / 2.f, tb.size.y / 2.f));
                m_textosNumeros[i]->setPosition(sf::Vector2f(
                    m_botonesNumeros[i].getPosition().x + 19.f,
                    m_botonesNumeros[i].getPosition().y + 15.f
                ));
                window.draw(*m_textosNumeros[i]);
            }
        }
    }
    
    // ===== SELECTOR DE CUARTOS =====
    if (m_mostrandoSelectorCuarto) {
        float cy = m_position.y + 300.f;
        float cx = m_position.x + 30.f;
        
        m_btnCuarto1.setPosition(sf::Vector2f(cx, cy));
        m_btnCuarto2.setPosition(sf::Vector2f(cx, cy + 40.f));
        m_btnCuarto3.setPosition(sf::Vector2f(cx, cy + 80.f));
        
        // Resaltar seleccionado
        m_btnCuarto1.setFillColor(m_cuartoElegido == 0 ? sf::Color(0, 150, 0, 200) : sf::Color(50, 50, 50, 200));
        m_btnCuarto2.setFillColor(m_cuartoElegido == 1 ? sf::Color(0, 150, 0, 200) : sf::Color(50, 50, 50, 200));
        m_btnCuarto3.setFillColor(m_cuartoElegido == 2 ? sf::Color(0, 150, 0, 200) : sf::Color(50, 50, 50, 200));
        
        window.draw(m_btnCuarto1);
        window.draw(m_btnCuarto2);
        window.draw(m_btnCuarto3);
        
        if (m_textoCuarto1) {
            centerTextInButton(*m_textoCuarto1, m_btnCuarto1);
            window.draw(*m_textoCuarto1);
        }
        if (m_textoCuarto2) {
            centerTextInButton(*m_textoCuarto2, m_btnCuarto2);
            window.draw(*m_textoCuarto2);
        }
        if (m_textoCuarto3) {
            centerTextInButton(*m_textoCuarto3, m_btnCuarto3);
            window.draw(*m_textoCuarto3);
        }
    }
    
    // ===== SELECTOR DE COLORES =====
    if (m_mostrandoSelectorColor) {
        float cy = m_position.y + 300.f;
        float cx = m_position.x + 30.f;
        
        m_btnRojo.setPosition(sf::Vector2f(cx, cy));
        m_btnNegro.setPosition(sf::Vector2f(cx, cy + 40.f));
        m_btnVerde.setPosition(sf::Vector2f(cx, cy + 80.f));
        
        // Resaltar seleccionado
        m_btnRojo.setOutlineThickness(m_colorElegido == ColorElegido::ROJO ? 3.f : 1.f);
        m_btnRojo.setOutlineColor(m_colorElegido == ColorElegido::ROJO ? sf::Color::Yellow : sf::Color::Red);
        
        m_btnNegro.setOutlineThickness(m_colorElegido == ColorElegido::NEGRO ? 3.f : 1.f);
        m_btnNegro.setOutlineColor(m_colorElegido == ColorElegido::NEGRO ? sf::Color::Yellow : sf::Color::White);
        
        m_btnVerde.setOutlineThickness(m_colorElegido == ColorElegido::VERDE ? 3.f : 1.f);
        m_btnVerde.setOutlineColor(m_colorElegido == ColorElegido::VERDE ? sf::Color::Yellow : sf::Color::Green);
        
        window.draw(m_btnRojo);
        window.draw(m_btnNegro);
        window.draw(m_btnVerde);
        
                if (m_textoRojo) {
            centerTextInButton(*m_textoRojo, m_btnRojo);
            window.draw(*m_textoRojo);
        }
        if (m_textoNegro) {
            centerTextInButton(*m_textoNegro, m_btnNegro);
            window.draw(*m_textoNegro);
        }
        if (m_textoVerde) {
            centerTextInButton(*m_textoVerde, m_btnVerde);
            window.draw(*m_textoVerde);
        }
    }
    
    // ===== RESULTADO =====
    if (m_mostrandoResultado && m_resultadoText) {
        m_resultadoText->setString(m_mensajeResultado);
        m_resultadoText->setFillColor(sf::Color::Yellow);
        
        // Centrar el texto
        sf::FloatRect bounds = m_resultadoText->getLocalBounds();
        m_resultadoText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_resultadoText->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + m_size.y / 2.f + 150.f
        ));
        
        window.draw(*m_resultadoText);
        
        // Mostrar el número ganador grande
        sf::Text numeroGanadorText(m_font, std::to_string(m_numeroGanador), 60);
        numeroGanadorText.setFillColor(sf::Color::Yellow);
        numeroGanadorText.setStyle(sf::Text::Bold);
        
        sf::FloatRect numBounds = numeroGanadorText.getLocalBounds();
        numeroGanadorText.setOrigin(sf::Vector2f(numBounds.size.x / 2.f, numBounds.size.y / 2.f));
        numeroGanadorText.setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + m_size.y / 2.f + 50.f
        ));
        
        window.draw(numeroGanadorText);
    }
}
       