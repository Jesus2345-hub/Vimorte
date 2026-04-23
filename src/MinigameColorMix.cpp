#include "MinigameColorMix.hpp"

// CONSTRUCTOR
MinigameColorMix::MinigameColorMix()
    : m_isActive(false), m_gameWon(false), m_tieneColor(false),
      m_backgroundSprite(nullptr), m_size(900.f, 600.f), m_tituloObjetivos(nullptr),
      m_misionCumplida(nullptr), m_radiosOriginales()  // Inicializar vector de radios
{
    m_background.setFillColor(sf::Color(0, 0, 0, 200));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(100, 100, 100));

    m_coloresDisponibles = {
        sf::Color::Red,
        sf::Color::Blue,
        sf::Color::Yellow
    };

    // --- 5 colores objetivo (todos requieren mezcla) ---
    // Naranja (Rojo+Amarillo), Verde (Amarillo+Azul), Morado (Rojo+Azul)
    // Rojo-anaranjado (Rojo+Naranja), Verde-azulado (Azul+Verde)
    m_coloresObjetivo = {
        sf::Color(255, 165, 0),   // naranja
        sf::Color(0, 255, 0),     // verde
        sf::Color(128, 0, 128),   // morado
        sf::Color(255, 83, 0),    // rojo anaranjado (mezcla rojo+naranja)
        sf::Color(0, 128, 128)    // verde azulado (mezcla azul+verde)
    };

    m_completados.resize(m_coloresObjetivo.size(), false);
}

void MinigameColorMix::initUI()
{
    // ---------- FONDO ----------
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel1/lab_background.jpg")) {
        m_backgroundSprite = std::make_unique<sf::Sprite>(m_backgroundTexture);
        m_backgroundSprite->setPosition(m_position);

        sf::Vector2u texSize = m_backgroundTexture.getSize();
        sf::Vector2f rectSize = m_background.getSize(); 

        if (texSize.x > 0 && texSize.y > 0) {
            m_backgroundSprite->setScale({
                rectSize.x / static_cast<float>(texSize.x),
                rectSize.y / static_cast<float>(texSize.y)
            });
        }
    } else {
        m_backgroundSprite = nullptr;
    }

    // ---------- BOTELLAS (posición original) ----------
    m_botellas.clear();
    std::vector<std::string> rutas = {
        "assets/images/niveles/nivel1/red.png",
        "assets/images/niveles/nivel1/blue.png",
        "assets/images/niveles/nivel1/yellow.png"
    };

    std::vector<sf::Color> colores = {
        sf::Color::Red,
        sf::Color::Blue,
        sf::Color::Yellow
    };

    for (int i = 0; i < 3; i++) {
        auto b = std::make_unique<Botella>();
        b->texture = std::make_unique<sf::Texture>();

        if (b->texture->loadFromFile(rutas[i])) {
            b->sprite = std::make_unique<sf::Sprite>(*b->texture);
            b->sprite->setScale({0.2f, 0.2f});
            b->sprite->setPosition({
                m_position.x + 150.f + i * 140.f,
                m_position.y + 350.f
            });
            b->color = colores[i];
            m_botellas.push_back(std::move(b));
        }
    }
    
    // ---------- PAPEL (movido más arriba e izquierda) ----------
    m_papel.setSize({250.f, 250.f});
    m_papel.setFillColor(sf::Color::White);
    // Antes: centro +50 X, -50 Y. Ahora: más arriba (restar 90) y más izquierda (restar 30)
    m_papel.setPosition({
        m_position.x + m_size.x / 2.f - 125.f - 30.f,   // más izquierda
        m_position.y + m_size.y / 2.f - 125.f - 90.f    // más arriba
    });

    // ---------- BOTÓN RESET (esquina inferior izquierda) ----------
    m_botonReset.setSize({150.f, 50.f});
    m_botonReset.setFillColor(sf::Color(80, 80, 80));
    m_botonReset.setPosition({
        m_position.x + 80.f,
        m_position.y + m_size.y - 40.f
    });

    // ---------- OBJETIVOS (movidos más a la izquierda) ----------
    m_objetivosUI.clear();
    m_radiosOriginales.clear();
    float startX = m_position.x + m_size.x - 180.f;   // antes -60, ahora -180 (más izquierda)
    float startY = m_position.y + 140.f;
    float stepY = 55.f;  // espaciado para 5 colores
    for (size_t i = 0; i < m_coloresObjetivo.size(); i++) {
        sf::CircleShape c(22.f);
        c.setFillColor(m_coloresObjetivo[i]);
        c.setPosition({startX, startY + i * stepY});
        // Sin outline por defecto (se elimina el borde)
        m_objetivosUI.push_back(c);
        m_radiosOriginales.push_back(22.f);   // guardar radio original
    }

    // ---------- TEXTO "Colores que debes crear" (centrado sobre los círculos) ----------
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente en MinigameColorMix" << std::endl;
    }
    m_tituloObjetivos = std::make_unique<sf::Text>(m_font);
    m_tituloObjetivos->setString("Colores que debes crear");
    m_tituloObjetivos->setCharacterSize(16);
    m_tituloObjetivos->setFillColor(sf::Color::Yellow);
    sf::FloatRect tituloBounds = m_tituloObjetivos->getLocalBounds();
    m_tituloObjetivos->setOrigin({tituloBounds.size.x / 2.f, 0.f});
    // Centrar el título sobre el primer círculo (su centro es startX + 22)
    m_tituloObjetivos->setPosition({
        startX + 22.f,
        m_position.y + 80.f
    });

    // ---------- TEXTO DEL BOTÓN RESET ----------
    m_botonResetText = std::make_unique<sf::Text>(m_font);
    m_botonResetText->setString("LIMPIAR HOJA");
    m_botonResetText->setCharacterSize(18);
    m_botonResetText->setFillColor(sf::Color::White);
    sf::FloatRect tb = m_botonResetText->getLocalBounds();
    m_botonResetText->setOrigin({tb.size.x / 2.f, tb.size.y / 2.f});
    m_botonResetText->setPosition({
        m_botonReset.getPosition().x + m_botonReset.getSize().x / 2.f,
        m_botonReset.getPosition().y + m_botonReset.getSize().y / 2.f
    });

    // ---------- TEXTO "Presiona ESC para salir" ----------
    m_instruccionEscText = std::make_unique<sf::Text>(m_font);
    m_instruccionEscText->setString("Presiona ESC para salir");
    m_instruccionEscText->setCharacterSize(14);
    m_instruccionEscText->setFillColor(sf::Color(200, 200, 200));
    sf::FloatRect eb = m_instruccionEscText->getLocalBounds();
    m_instruccionEscText->setOrigin({eb.size.x / 2.f, 0.f});
    m_instruccionEscText->setPosition({
        m_position.x + m_size.x / 2.f,
        m_position.y + m_size.y - 20.f
    });

    // ---------- TEXTO "Misión cumplida" ----------
    m_misionCumplida = std::make_unique<sf::Text>(m_font);
    m_misionCumplida->setString("MISION CUMPLIDA...");
    m_misionCumplida->setCharacterSize(40);
    m_misionCumplida->setFillColor(sf::Color::Green);
    m_misionCumplida->setStyle(sf::Text::Bold);
    sf::FloatRect mcBounds = m_misionCumplida->getLocalBounds();
    m_misionCumplida->setOrigin({mcBounds.size.x / 2.f, mcBounds.size.y / 2.f});
    m_misionCumplida->setPosition({
        m_position.x + m_size.x / 2.f,
        m_position.y + m_size.y / 2.f - 50.f
    });
}

void MinigameColorMix::update(float dt) {
    (void)dt;
}

void MinigameColorMix::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_isActive) return;
    if (m_gameWon) return;

    if (auto* mousePtr = event.getIf<sf::Event::MouseButtonPressed>()) {
        auto mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        for (auto& b : m_botellas) {
            if (b->sprite && b->sprite->getGlobalBounds().contains(mousePos)) {
                mezclarColor(b->color);
            }
        }

        if (m_botonReset.getGlobalBounds().contains(mousePos)) {
            resetPapel();
        }
    }
}

void MinigameColorMix::draw(sf::RenderWindow& window) {
    if (!m_isActive) return;

    if (m_backgroundSprite && m_backgroundTexture.getSize().x > 0) {
        window.draw(*m_backgroundSprite);
    } else {
        m_background.setPosition(m_position);
        window.draw(m_background);
    }

    // ---------- DIBUJAR OBJETIVOS (círculos de colores) ----------
    // Ya no se usa outline verde; los círculos ya están modificados en verificarColor()
    for (size_t i = 0; i < m_objetivosUI.size(); i++) {
        window.draw(m_objetivosUI[i]);
    }

    if (m_tituloObjetivos) window.draw(*m_tituloObjetivos);

    for (auto& b : m_botellas) {
        if (b->sprite) window.draw(*b->sprite);
    }

    window.draw(m_papel);
    window.draw(m_botonReset);
    if (m_botonResetText) window.draw(*m_botonResetText);
    if (m_instruccionEscText) window.draw(*m_instruccionEscText);

    if (m_gameWon && m_misionCumplida) window.draw(*m_misionCumplida);
}

// ==================== FUNCIONES DE MEZCLA ====================
sf::Color MinigameColorMix::mezclar(const sf::Color& a, const sf::Color& b) {
    // Mezcla por promedio RGB (funciona para nuestros propósitos)
    if ((a == sf::Color::Red && b == sf::Color::Yellow) ||
        (a == sf::Color::Yellow && b == sf::Color::Red)) {
        return sf::Color(255, 165, 0); // naranja
    }
    if ((a == sf::Color::Blue && b == sf::Color::Yellow) ||
        (a == sf::Color::Yellow && b == sf::Color::Blue)) {
        return sf::Color(0, 255, 0); // verde
    }
    if ((a == sf::Color::Red && b == sf::Color::Blue) ||
        (a == sf::Color::Blue && b == sf::Color::Red)) {
        return sf::Color(128, 0, 128); // morado
    }
    // Mezclas con secundarios (rojo+naranja, azul+verde)
    if ((a == sf::Color::Red && b == sf::Color(255, 165, 0)) ||
        (a == sf::Color(255, 165, 0) && b == sf::Color::Red)) {
        return sf::Color(255, 83, 0); // rojo anaranjado
    }
    if ((a == sf::Color::Blue && b == sf::Color(0, 255, 0)) ||
        (a == sf::Color(0, 255, 0) && b == sf::Color::Blue)) {
        return sf::Color(0, 128, 128); // verde azulado
    }
    if (a == b) return a;
    return sf::Color((a.r + b.r) / 2, (a.g + b.g) / 2, (a.b + b.b) / 2);
}

void MinigameColorMix::mezclarColor(const sf::Color& nuevo) {
    if (!m_tieneColor) {
        m_colorActual = nuevo;
        m_tieneColor = true;
    } else {
        m_colorActual = mezclar(m_colorActual, nuevo);
    }
    m_papel.setFillColor(m_colorActual);
    verificarColor();
}

void MinigameColorMix::resetPapel() {
    m_tieneColor = false;
    m_papel.setFillColor(sf::Color::White);
}

bool MinigameColorMix::coloresIguales(const sf::Color& a, const sf::Color& b) {
    return std::abs(static_cast<int>(a.r) - b.r) < 15 &&
           std::abs(static_cast<int>(a.g) - b.g) < 15 &&
           std::abs(static_cast<int>(a.b) - b.b) < 15;
}

// ---------- VERIFICAR COLOR Y ACTUALIZAR CÍRCULO OBJETIVO ----------
void MinigameColorMix::verificarColor() {
    for (size_t i = 0; i < m_coloresObjetivo.size(); i++) {
        if (!m_completados[i] && coloresIguales(m_colorActual, m_coloresObjetivo[i])) {
            m_completados[i] = true;
            // Al completar el color, reducir el círculo a un pequeño punto gris
            m_objetivosUI[i].setRadius(5.f);
            m_objetivosUI[i].setFillColor(sf::Color(150, 150, 150));
            std::cout << "✅ Color correcto!" << std::endl;
            resetPapel();
            break;
        }
    }
    // Verificar si todos los colores están completados
    if (std::all_of(m_completados.begin(), m_completados.end(), [](bool v){ return v; })) {
        if (!m_gameWon) {
            m_gameWon = true;
            std::cout << "MISION CUMPLIDA Has creado todos los colores" << std::endl;
        }
    }
}