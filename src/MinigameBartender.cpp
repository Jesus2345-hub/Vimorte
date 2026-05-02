#include "MinigameBartender.hpp"
#include <iostream>
#include <algorithm>

MinigameBartender::MinigameBartender() : m_gen(m_rd()) {
    m_background.setFillColor(sf::Color(0, 0, 0, 180));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(180, 150, 50));
    
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (!m_fontLoaded) {
        std::cerr << "Error cargando fuente en MinigameBartender" << std::endl;
        return;
    }
    
    inicializarColores();
    inicializarUI();
}

void MinigameBartender::inicializarColores() {
    m_nombresColores = {
        "Rojo", "Azul", "Amarillo", "Blanco",
        "Naranja", "Morado", "Verde", "Rosa", "Celeste", "Crema"
    };
    
    m_coloresSFML = {
        sf::Color(255, 0, 0),     // Rojo
        sf::Color(0, 0, 255),     // Azul
        sf::Color(255, 255, 0),   // Amarillo
        sf::Color(255, 255, 255), // Blanco
        sf::Color(255, 165, 0),   // Naranja
        sf::Color(128, 0, 128),   // Morado
        sf::Color(0, 255, 0),     // Verde
        sf::Color(255, 192, 203), // Rosa
        sf::Color(173, 216, 230), // Celeste
        sf::Color(255, 253, 208)  // Crema
    };
}

void MinigameBartender::setPosition(const sf::Vector2f& pos) {
    m_position = pos;
    m_background.setPosition(pos);
}

void MinigameBartender::setSize(const sf::Vector2f& size) {
    m_size = size;
    m_background.setSize(size);
    
    // Cargar fondo
    if (m_fondoTexture.loadFromFile("assets/images/niveles/nivel2/bar.png")) {
        m_fondoSprite = std::make_unique<sf::Sprite>(m_fondoTexture);
        float escalaX = m_size.x / m_fondoTexture.getSize().x;
        float escalaY = m_size.y / m_fondoTexture.getSize().y;
        float escala = std::min(escalaX, escalaY);
        m_fondoSprite->setScale(sf::Vector2f(escala, escala));
        sf::FloatRect bounds = m_fondoSprite->getLocalBounds();
        m_fondoSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_fondoSprite->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + m_size.y / 2.f
        ));
    }
    
    // Cargar copa (más pequeña)
    if (m_copaTexture.loadFromFile("assets/images/niveles/nivel2/copa_vacia.png")) {
        m_copaSprite = std::make_unique<sf::Sprite>(m_copaTexture);
       
        sf::FloatRect bounds = m_copaSprite->getLocalBounds();
        m_copaSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_copaSprite->setPosition(sf::Vector2f(
            m_position.x + m_size.x * 0.5f,
            m_position.y + m_size.y * 0.72f  // Más abajo
        ));
    }
}

void MinigameBartender::inicializarUI() {
    if (!m_fontLoaded) return;
    
    // Título
    m_tituloText = std::make_unique<sf::Text>(m_font, "BARTENDER VIMORTE", 28);
    m_tituloText->setFillColor(sf::Color::Yellow);
    m_tituloText->setStyle(sf::Text::Bold);
    
    // Dinero
    m_dineroText = std::make_unique<sf::Text>(m_font, "", 22);
    m_dineroText->setFillColor(sf::Color::Yellow);
    
    // Bebidas servidas
    m_bebidasText = std::make_unique<sf::Text>(m_font, "", 18);
    m_bebidasText->setFillColor(sf::Color::White);
    
    // Errores
    m_erroresText = std::make_unique<sf::Text>(m_font, "", 18);
    m_erroresText->setFillColor(sf::Color::Red);
    
    // Resultado
    m_resultadoText = std::make_unique<sf::Text>(m_font, "", 30);
    m_resultadoText->setFillColor(sf::Color::Cyan);
    
    // Robot (texto)
    m_textoRobot = std::make_unique<sf::Text>(m_font, "ROBOT: Prepara esta bebida", 20);
    m_textoRobot->setFillColor(sf::Color::White);
    
    // Pedido
    m_textoPedido = std::make_unique<sf::Text>(m_font, "", 24);
    m_textoPedido->setFillColor(sf::Color::Yellow);
    m_textoPedido->setStyle(sf::Text::Bold);
    
    // Instrucciones
    m_textoInstrucciones = std::make_unique<sf::Text>(m_font,
        "Click en botellas para mezclar | Doble click misma botella = reset | ESC para salir", 12);
    m_textoInstrucciones->setFillColor(sf::Color(180, 180, 180));
    
        // Crear 4 botellas (invisibles, solo áreas clickeables)
    std::vector<ColorBebida> tipos = {
        ColorBebida::ROJO, ColorBebida::AZUL, 
        ColorBebida::AMARILLO, ColorBebida::BLANCO
    };
    
    for (size_t i = 0; i < tipos.size(); i++) {
        Botella b;
        b.boton.setSize(sf::Vector2f(50.f, 80.f));
        b.boton.setFillColor(sf::Color::Transparent);  // INVISIBLE
        b.boton.setOutlineThickness(0.f);               // SIN BORDE
        b.color = obtenerColorSFML(tipos[i]);
        b.tipo = tipos[i];
        b.texto = nullptr;  // Sin texto
        m_botellas.push_back(std::move(b));
    }
    
}

void MinigameBartender::activate() {
    m_isActive = true;
    m_bebidasServidas = 0;
    m_errores = 0;
    m_mostrandoResultado = false;
    resetearMezcla();
    generarNuevoPedido();
}

void MinigameBartender::deactivate() {
    m_isActive = false;
}

sf::Color MinigameBartender::obtenerColorSFML(ColorBebida tipo) {
    return m_coloresSFML[static_cast<int>(tipo)];
}

sf::Color MinigameBartender::mezclarColores(sf::Color a, sf::Color b) {
    // Rojo + Amarillo = Naranja
    if ((a == sf::Color::Red && b == sf::Color::Yellow) ||
        (a == sf::Color::Yellow && b == sf::Color::Red))
        return sf::Color(255, 165, 0);
    
    // Rojo + Azul = Morado
    if ((a == sf::Color::Red && b == sf::Color::Blue) ||
        (a == sf::Color::Blue && b == sf::Color::Red))
        return sf::Color(128, 0, 128);
    
    // Amarillo + Azul = Verde
    if ((a == sf::Color::Yellow && b == sf::Color::Blue) ||
        (a == sf::Color::Blue && b == sf::Color::Yellow))
        return sf::Color(0, 255, 0);
    
    // Rojo + Blanco = Rosa
    if ((a == sf::Color::Red && b == sf::Color::White) ||
        (a == sf::Color::White && b == sf::Color::Red))
        return sf::Color(255, 192, 203);
    
    // Azul + Blanco = Celeste
    if ((a == sf::Color::Blue && b == sf::Color::White) ||
        (a == sf::Color::White && b == sf::Color::Blue))
        return sf::Color(173, 216, 230);
    
    // Amarillo + Blanco = Crema
    if ((a == sf::Color::Yellow && b == sf::Color::White) ||
        (a == sf::Color::White && b == sf::Color::Yellow))
        return sf::Color(255, 253, 208);
    
    // Si son iguales, devolver ese color
    if (a == b) return a;
    
    // Default: promedio
    return sf::Color(
        (a.r + b.r) / 2,
        (a.g + b.g) / 2,
        (a.b + b.b) / 2
    );
}

bool MinigameBartender::coloresIguales(const sf::Color& a, const sf::Color& b) {
    return std::abs(static_cast<int>(a.r) - b.r) < 20 &&
           std::abs(static_cast<int>(a.g) - b.g) < 20 &&
           std::abs(static_cast<int>(a.b) - b.b) < 20;
}

void MinigameBartender::generarNuevoPedido() {
    // Solo combinaciones (índices 4 al 9): Naranja, Morado, Verde, Rosa, Celeste, Crema
    std::uniform_int_distribution<int> dist(4, 9);
    int tipo = dist(m_gen);
    m_pedidoActual = static_cast<ColorBebida>(tipo);
    m_colorPedido = m_coloresSFML[tipo];
    m_nombrePedido = m_nombresColores[tipo];
}

void MinigameBartender::mezclarColor(ColorBebida color) {
    if (!m_tienePrimerColor) {
        m_primerColor = color;
        m_colorMezcla = obtenerColorSFML(color);
        m_tienePrimerColor = true;
    } else {
        sf::Color color1 = obtenerColorSFML(m_primerColor);
        sf::Color color2 = obtenerColorSFML(color);
        m_colorMezcla = mezclarColores(color1, color2);
        verificarPedido();
    }
}

void MinigameBartender::resetearMezcla() {
    m_tienePrimerColor = false;
    m_colorMezcla = sf::Color::Transparent;
}

void MinigameBartender::verificarPedido() {
    if (coloresIguales(m_colorMezcla, m_colorPedido)) {
        // Correcto
        m_bebidasServidas++;
        if (m_dineroJugador) *m_dineroJugador += 1;
        
        m_mensajeResultado = "CORRECTO! +$1";
        m_resultadoText->setFillColor(sf::Color::Green);
        m_mostrandoResultado = true;
        m_tiempoResultado = 1.5f;
        
        resetearMezcla();
        generarNuevoPedido();
    } else {
        // Error
        m_errores++;
        
        m_mensajeResultado = "INCORRECTO!";
        m_resultadoText->setFillColor(sf::Color::Red);
        m_mostrandoResultado = true;
        m_tiempoResultado = 1.5f;
        
        resetearMezcla();
        
        if (m_errores >= m_maxErrores) {
            m_mensajeResultado = "GAME OVER! Bebidas: " + std::to_string(m_bebidasServidas);
        }
    }
}

void MinigameBartender::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_isActive) return;
    if (m_errores >= m_maxErrores) return;
    if (m_mostrandoResultado) return;
    
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    
    // Hover de botellas
    for (auto& b : m_botellas) {
        b.hover = b.boton.getGlobalBounds().contains(mousePos);
        b.boton.setOutlineColor(b.hover ? sf::Color::Yellow : sf::Color::White);
    }
    
    // Click
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            for (auto& b : m_botellas) {
                if (b.hover) {
                    // Si click misma botella = reset
                    if (m_tienePrimerColor && m_primerColor == b.tipo) {
                        resetearMezcla();
                    } else {
                        mezclarColor(b.tipo);
                    }
                    break;
                }
            }
        }
    }
}

void MinigameBartender::update(float dt) {
    if (!m_isActive) return;
    
    if (m_mostrandoResultado) {
        m_tiempoResultado -= dt;
        if (m_tiempoResultado <= 0.0f) {
            m_mostrandoResultado = false;
        }
    }
}

void MinigameBartender::draw(sf::RenderWindow& window) {
    if (!m_isActive) return;
    
    // Fondo
    if (m_fondoSprite) {
        m_fondoSprite->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + m_size.y / 2.f
        ));
        window.draw(*m_fondoSprite);
    } else {
        window.draw(m_background);
    }
    
    // Factor de escala
    float escala = std::min(m_size.x / 800.f, m_size.y / 600.f);
    
    // Título
    if (m_tituloText) {
        sf::FloatRect tb = m_tituloText->getLocalBounds();
        m_tituloText->setOrigin(sf::Vector2f(tb.size.x / 2.f, 0.f));
        m_tituloText->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + 10.f
        ));
        window.draw(*m_tituloText);
    }
    
    // Robot y pedido
    if (m_textoRobot) {
        m_textoRobot->setPosition(sf::Vector2f(m_position.x + 20.f, m_position.y + 20.f));
        window.draw(*m_textoRobot);
    }
    
            if (m_textoPedido) {
        float tamanoColor = 120.f * escala;
        sf::RectangleShape colorPedido(sf::Vector2f(tamanoColor, tamanoColor));
        colorPedido.setFillColor(m_colorPedido);
        colorPedido.setOutlineThickness(3.f * escala);
        colorPedido.setOutlineColor(sf::Color::White);
        colorPedido.setPosition(sf::Vector2f(
            m_position.x + 46.f * escala, 
            m_position.y + 50.f * escala
        ));
        window.draw(colorPedido);
    }
    
       // Copa
    if (m_copaSprite) {
        float escalaCopa = 0.15f * escala;  // Mantiene 0.15f proporcional
        m_copaSprite->setScale(sf::Vector2f(escalaCopa, escalaCopa));
        m_copaSprite->setPosition(sf::Vector2f(
            m_position.x + m_size.x * 0.1f,
            m_position.y + m_size.y * 0.8f
        ));
        window.draw(*m_copaSprite);
        
        // Líquido en la copa
        if (m_colorMezcla != sf::Color::Transparent) {
            sf::CircleShape liquido(22.f * escala);  // Más chico (antes 22.f)
            liquido.setFillColor(sf::Color(m_colorMezcla.r, m_colorMezcla.g, m_colorMezcla.b, 180));
            liquido.setScale(sf::Vector2f(1.f, 0.6f));
            liquido.setOrigin(sf::Vector2f(12.f * escala, 12.f * escala));
            liquido.setPosition(sf::Vector2f(
                m_position.x + m_size.x * 0.1f,
                m_position.y + m_size.y * 0.75f  // Ajustado a la copa
            ));
            window.draw(liquido);
        }
    }
    
     // Botellas (invisibles, solo para debug si quieres)
    for (size_t i = 0; i < m_botellas.size(); i++) {
        float bx = m_position.x + m_size.x * 0.58f + i * 110.f * escala;
        float by = m_position.y + m_size.y * 0.55f;
        
        m_botellas[i].boton.setSize(sf::Vector2f(60.f * escala, 80.f * escala));
        m_botellas[i].boton.setPosition(sf::Vector2f(bx, by));
        
       
    }
    
    // Dinero
    if (m_dineroText && m_dineroJugador) {
        m_dineroText->setString("Dinero: $" + std::to_string(*m_dineroJugador));
        m_dineroText->setPosition(sf::Vector2f(m_position.x + m_size.x - 200.f, m_position.y + 10.f));
        window.draw(*m_dineroText);
    }
    
    // Bebidas
    if (m_bebidasText) {
        m_bebidasText->setString("Bebidas: " + std::to_string(m_bebidasServidas));
        m_bebidasText->setPosition(sf::Vector2f(m_position.x + m_size.x - 200.f, m_position.y + 40.f));
        window.draw(*m_bebidasText);
    }
    
    // Errores
    if (m_erroresText) {
        m_erroresText->setString("Errores: " + std::to_string(m_errores) + "/" + std::to_string(m_maxErrores));
        m_erroresText->setPosition(sf::Vector2f(m_position.x + m_size.x - 200.f, m_position.y + 65.f));
        window.draw(*m_erroresText);
    }
    
    // Instrucciones
    if (m_textoInstrucciones) {
        m_textoInstrucciones->setPosition(sf::Vector2f(
            m_position.x + 15.f,
            m_position.y + m_size.y - 40.f
        ));
        window.draw(*m_textoInstrucciones);
    }
    
    // Resultado
    if (m_mostrandoResultado && m_resultadoText) {
        m_resultadoText->setString(m_mensajeResultado);
        sf::FloatRect bounds = m_resultadoText->getLocalBounds();
        
        // Fondo para el mensaje
        sf::RectangleShape fondoMensaje(sf::Vector2f(bounds.size.x + 40.f, bounds.size.y + 30.f));
        fondoMensaje.setFillColor(sf::Color(0, 0, 0, 200));
        fondoMensaje.setOutlineThickness(2.f);
        fondoMensaje.setOutlineColor(sf::Color::Yellow);
        fondoMensaje.setOrigin(sf::Vector2f((bounds.size.x + 40.f) / 2.f, (bounds.size.y + 30.f) / 2.f));
        fondoMensaje.setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + m_size.y / 2.f
        ));
        window.draw(fondoMensaje);
        
        m_resultadoText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_resultadoText->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + m_size.y / 2.f
        ));
        window.draw(*m_resultadoText);
    }
}