#include "MinigameColorMix.hpp"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstdint>

// CONSTRUCTOR
MinigameColorMix::MinigameColorMix()
    : m_isActive(false), m_gameWon(false), m_tieneColor(false),
      m_backgroundSprite(nullptr), m_size(900.f, 600.f), m_tituloObjetivos(nullptr),
      m_misionCumplida(nullptr), m_radiosOriginales(), m_textoObjetivoActual(nullptr)
{
    // Inicializar semilla aleatoria
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    m_background.setFillColor(sf::Color(0, 0, 0, 200));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(100, 100, 100));

    m_coloresDisponibles = {
        sf::Color::Red,
        sf::Color::Blue,
        sf::Color::Yellow
    };

    // --- 5 colores objetivo (todos requieren mezcla) ---
    m_coloresObjetivo = {
        sf::Color(255, 165, 0),   // naranja
        sf::Color(0, 255, 0),     // verde
        sf::Color(128, 0, 128),   // morado
        sf::Color(255, 83, 0),    // rojo anaranjado
        sf::Color(0, 128, 128)    // verde azulado
    };

    m_completados.resize(m_coloresObjetivo.size(), false);
}

void MinigameColorMix::generarOrdenAleatorio() {
    m_ordenObjetivos.clear();
    for (size_t i = 0; i < m_coloresObjetivo.size(); i++) {
        if (!m_completados[i]) {
            m_ordenObjetivos.push_back(i);
        }
    }
    
    // Barajar aleatoriamente
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_ordenObjetivos.begin(), m_ordenObjetivos.end(), g);
    
    m_objetivoActual = 0;
}

void MinigameColorMix::actualizarTextoObjetivo() {
    if (m_textoObjetivoActual && m_objetivoActual < m_ordenObjetivos.size()) {
        size_t idx = m_ordenObjetivos[m_objetivoActual];
        sf::Color colorObj = m_coloresObjetivo[idx];
        
        std::string nombreColor;
        if (colorObj == sf::Color(255, 165, 0)) nombreColor = "NARANJA";
        else if (colorObj == sf::Color(0, 255, 0)) nombreColor = "VERDE";
        else if (colorObj == sf::Color(128, 0, 128)) nombreColor = "MORADO";
        else if (colorObj == sf::Color(255, 83, 0)) nombreColor = "ROJO ANARANJADO";
        else if (colorObj == sf::Color(0, 128, 128)) nombreColor = "VERDE AZULADO";
        
        m_textoObjetivoActual->setString("Crea: " + nombreColor + 
            " (" + std::to_string(m_objetivoActual + 1) + "/" + 
            std::to_string(m_ordenObjetivos.size()) + ")");
    }
}

void MinigameColorMix::crearParticulas(sf::Vector2f pos, sf::Color color) {
    for (int i = 0; i < 20; i++) {
        Particula p;
        float radius = 2.f + (std::rand() % 5);
        p.shape.setRadius(radius);
        p.shape.setFillColor(color);
        p.shape.setOrigin({radius, radius});
        p.shape.setPosition(pos);
        
        float angle = (std::rand() % 360) * 3.14159f / 180.f;
        float speed = 50.f + (std::rand() % 150);
        p.velocity = {
            std::cos(angle) * speed,
            std::sin(angle) * speed - 100.f
        };
        p.life = 1.0f + (std::rand() % 100) / 100.0f;
        m_particulas.push_back(p);
    }
}

void MinigameColorMix::activate() {
    m_isActive = true;
    m_gameWon = false;
    m_tieneColor = false;
    m_colorActual = sf::Color::White;
    m_papel.setFillColor(sf::Color::White);
    m_papelAnimScale = 1.0f;
    m_tiempoCompletado = 0.0f;
    m_mostrandoCompletado = false;
    m_particulas.clear();
    
    // Resetear completados
    std::fill(m_completados.begin(), m_completados.end(), false);
    
    // Restaurar círculos objetivo
    for (size_t i = 0; i < m_objetivosUI.size(); i++) {
        m_objetivosUI[i].setRadius(m_radiosOriginales[i]);
        m_objetivosUI[i].setFillColor(m_coloresObjetivo[i]);
    }
    
    // Generar orden aleatorio
    generarOrdenAleatorio();
    actualizarTextoObjetivo();
}

void MinigameColorMix::deactivate() {
    m_isActive = false;
    m_particulas.clear();
}

void MinigameColorMix::initUI()
{
    // ---------- FONDO ----------
    m_background.setSize(m_size);
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel1/lab_background.jpg")) {
        m_backgroundSprite = std::make_unique<sf::Sprite>(m_backgroundTexture);
        m_backgroundSprite->setPosition(m_position);

        sf::Vector2u texSize = m_backgroundTexture.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            m_backgroundSprite->setScale({
                m_size.x / static_cast<float>(texSize.x),
                m_size.y / static_cast<float>(texSize.y)
            });
        }
    } else {
        m_backgroundSprite = nullptr;
    }

    // ---------- BOTELLAS ----------
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
            b->animScale = 1.0f;
            b->animating = false;
            m_botellas.push_back(std::move(b));
        }
    }
    
    // ---------- PAPEL ----------
    m_papel.setSize({250.f, 250.f});
    m_papel.setFillColor(sf::Color::White);
    m_papel.setPosition({
        m_position.x + m_size.x / 2.f - 125.f - 30.f,
        m_position.y + m_size.y / 2.f - 125.f - 90.f
    });

    // ---------- BOTÓN RESET ----------
    m_botonReset.setSize({150.f, 50.f});
    m_botonReset.setFillColor(sf::Color(80, 80, 80));
    m_botonReset.setPosition({
        m_position.x + 80.f,
        m_position.y + m_size.y - 40.f
    });

    // ---------- OBJETIVOS ----------
    m_objetivosUI.clear();
    m_radiosOriginales.clear();
    float startX = m_position.x + m_size.x - 180.f;
    float startY = m_position.y + 140.f;
    float stepY = 55.f;
    for (size_t i = 0; i < m_coloresObjetivo.size(); i++) {
        sf::CircleShape c(22.f);
        c.setFillColor(m_coloresObjetivo[i]);
        c.setPosition({startX, startY + i * stepY});
        m_objetivosUI.push_back(c);
        m_radiosOriginales.push_back(22.f);
    }

    // ---------- FUENTE ----------
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente en MinigameColorMix" << std::endl;
    }
    
    // Título
    m_tituloObjetivos = std::make_unique<sf::Text>(m_font);
    m_tituloObjetivos->setString("Colores que debes crear");
    m_tituloObjetivos->setCharacterSize(16);
    m_tituloObjetivos->setFillColor(sf::Color::Yellow);
    sf::FloatRect tituloBounds = m_tituloObjetivos->getLocalBounds();
    m_tituloObjetivos->setOrigin({tituloBounds.size.x / 2.f, 0.f});
    m_tituloObjetivos->setPosition({
        startX + 22.f,
        m_position.y + 80.f
    });

    // Texto del botón reset
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

    // Instrucción ESC
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

    // Misión cumplida
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

    // Texto objetivo actual
    m_textoObjetivoActual = std::make_unique<sf::Text>(m_font);
    m_textoObjetivoActual->setCharacterSize(20);
    m_textoObjetivoActual->setFillColor(sf::Color::White);
    m_textoObjetivoActual->setStyle(sf::Text::Bold);
    m_textoObjetivoActual->setPosition({
        m_position.x + 50.f,
        m_position.y + 50.f
    });
}

void MinigameColorMix::update(float dt) {
    if (!m_isActive) return;
    
    // Actualizar animaciones de botellas
    for (auto& b : m_botellas) {
        if (b->animating) {
            b->animScale += dt * 4.0f;
            if (b->animScale >= 1.3f) {
                b->animScale = 1.0f;
                b->animating = false;
            }
            if (b->sprite) {
                b->sprite->setScale({0.2f * b->animScale, 0.2f * b->animScale});
            }
        }
    }
    
    // Animación del papel al mezclar
    if (m_tieneColor && m_papelAnimScale < 1.1f) {
        m_papelAnimScale += dt * 2.0f;
        if (m_papelAnimScale > 1.1f) m_papelAnimScale = 1.1f;
    } else if (!m_tieneColor && m_papelAnimScale > 1.0f) {
        m_papelAnimScale -= dt * 2.0f;
        if (m_papelAnimScale < 1.0f) m_papelAnimScale = 1.0f;
    }
    
    // Actualizar partículas
    for (auto it = m_particulas.begin(); it != m_particulas.end();) {
        it->life -= dt;
        if (it->life <= 0.0f) {
            it = m_particulas.erase(it);
        } else {
            it->shape.move(it->velocity * dt);
            it->velocity.y += 200.f * dt; // gravedad
            
            // Desvanecer partícula
            sf::Color c = it->shape.getFillColor();
            c.a = static_cast<uint8_t>(255 * it->life);
            it->shape.setFillColor(c);
            ++it;
        }
    }
    
    // Actualizar animación de completado
    if (m_mostrandoCompletado) {
        m_tiempoCompletado -= dt;
        if (m_tiempoCompletado <= 0.0f) {
            m_mostrandoCompletado = false;
        }
    }
}

void MinigameColorMix::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_isActive) return;
    if (m_gameWon) return;

    if (auto* mousePtr = event.getIf<sf::Event::MouseButtonPressed>()) {
        auto mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        for (auto& b : m_botellas) {
            if (b->sprite && b->sprite->getGlobalBounds().contains(mousePos)) {
                // Activar animación
                b->animScale = 1.0f;
                b->animating = true;
                mezclarColor(b->color);
            }
        }

        if (m_botonReset.getGlobalBounds().contains(mousePos)) {
            resetPapel();
            m_papelAnimScale = 1.0f;
        }
    }
}

void MinigameColorMix::draw(sf::RenderWindow& window) {
    if (!m_isActive) return;

    // Dibujar fondo
    m_background.setPosition(m_position);
    m_background.setSize(m_size);
    
    if (m_backgroundSprite && m_backgroundTexture.getSize().x > 0) {
        m_backgroundSprite->setPosition(m_position);
        sf::Vector2u texSize = m_backgroundTexture.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            m_backgroundSprite->setScale({
                m_size.x / static_cast<float>(texSize.x),
                m_size.y / static_cast<float>(texSize.y)
            });
        }
        window.draw(*m_backgroundSprite);
    } else {
        window.draw(m_background);
    }

    // Dibujar objetivos
    for (size_t i = 0; i < m_objetivosUI.size(); i++) {
        window.draw(m_objetivosUI[i]);
    }

    if (m_tituloObjetivos) window.draw(*m_tituloObjetivos);
    if (m_textoObjetivoActual) window.draw(*m_textoObjetivoActual);

    // Dibujar botellas
    for (auto& b : m_botellas) {
        if (b->sprite) window.draw(*b->sprite);
    }

    // Dibujar papel con animación
    sf::RectangleShape papelAnimado = m_papel;
    sf::Vector2f papelCenter = {
        m_papel.getPosition().x + m_papel.getSize().x / 2.f,
        m_papel.getPosition().y + m_papel.getSize().y / 2.f
    };
    papelAnimado.setOrigin({m_papel.getSize().x / 2.f, m_papel.getSize().y / 2.f});
    papelAnimado.setPosition(papelCenter);
    papelAnimado.setScale({m_papelAnimScale, m_papelAnimScale});
    window.draw(papelAnimado);
    
    // Dibujar botón reset
    window.draw(m_botonReset);
    if (m_botonResetText) window.draw(*m_botonResetText);
    if (m_instruccionEscText) window.draw(*m_instruccionEscText);

    // Dibujar partículas
    for (const auto& p : m_particulas) {
        window.draw(p.shape);
    }

    // Mostrar mensaje de color completado
    if (m_mostrandoCompletado) {
        sf::Text completadoText(m_font);
        completadoText.setString("¡COLOR CREADO!");
        completadoText.setCharacterSize(24);
        completadoText.setFillColor(sf::Color::Green);
        completadoText.setStyle(sf::Text::Bold);
        sf::FloatRect bounds = completadoText.getLocalBounds();
        completadoText.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
        completadoText.setPosition({
            m_position.x + m_size.x / 2.f,
            m_position.y + m_size.y / 2.f + 50.f
        });
        window.draw(completadoText);
    }

    // Dibujar texto de misión cumplida con overlay
    if (m_gameWon && m_misionCumplida) {
        sf::RectangleShape overlay(m_size);
        overlay.setPosition(m_position);
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);
        window.draw(*m_misionCumplida);
    }
}

// ==================== FUNCIONES DE MEZCLA ====================
sf::Color MinigameColorMix::mezclar(const sf::Color& a, const sf::Color& b) {
    // Mezclas específicas
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
    // Mezclas con secundarios
    if ((a == sf::Color::Red && b == sf::Color(255, 165, 0)) ||
        (a == sf::Color(255, 165, 0) && b == sf::Color::Red)) {
        return sf::Color(255, 83, 0); // rojo anaranjado
    }
    if ((a == sf::Color::Blue && b == sf::Color(0, 255, 0)) ||
        (a == sf::Color(0, 255, 0) && b == sf::Color::Blue)) {
        return sf::Color(0, 128, 128); // verde azulado
    }
    if (a == b) return a;
    // Mezcla genérica
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
    m_colorActual = sf::Color::White;
    m_papel.setFillColor(sf::Color::White);
}

bool MinigameColorMix::coloresIguales(const sf::Color& a, const sf::Color& b) {
    return std::abs(static_cast<int>(a.r) - b.r) < 15 &&
           std::abs(static_cast<int>(a.g) - b.g) < 15 &&
           std::abs(static_cast<int>(a.b) - b.b) < 15;
}

void MinigameColorMix::verificarColor() {
    if (m_objetivoActual >= m_ordenObjetivos.size()) return;
    
    size_t idxObjetivo = m_ordenObjetivos[m_objetivoActual];
    
    if (!m_completados[idxObjetivo] && coloresIguales(m_colorActual, m_coloresObjetivo[idxObjetivo])) {
        m_completados[idxObjetivo] = true;
        
        // Reducir el círculo completado
        m_objetivosUI[idxObjetivo].setRadius(5.f);
        m_objetivosUI[idxObjetivo].setFillColor(sf::Color(150, 150, 150));
        
        // Crear partículas en el papel
        sf::Vector2f papelCenter = {
            m_papel.getPosition().x + m_papel.getSize().x / 2.f,
            m_papel.getPosition().y + m_papel.getSize().y / 2.f
        };
        crearParticulas(papelCenter, m_coloresObjetivo[idxObjetivo]);
        
        // Mostrar animación de completado
        m_mostrandoCompletado = true;
        m_tiempoCompletado = 1.5f;
        m_ultimoCompletado = idxObjetivo;
        
        std::cout << "✅ Color correcto! (" << (m_objetivoActual + 1) << "/" << m_ordenObjetivos.size() << ")" << std::endl;
        
        // Avanzar al siguiente objetivo
        m_objetivoActual++;
        actualizarTextoObjetivo();
        
        resetPapel();
        m_papelAnimScale = 1.0f;
        
        // Verificar si todos están completados
        if (std::all_of(m_completados.begin(), m_completados.end(), [](bool v){ return v; })) {
            if (!m_gameWon) {
                m_gameWon = true;
                
                // Crear muchas partículas de celebración
                sf::Vector2f center = {
                    m_position.x + m_size.x / 2.f,
                    m_position.y + m_size.y / 2.f
                };
                for (int i = 0; i < 5; i++) {
                    crearParticulas(center, sf::Color::Green);
                    crearParticulas(center, sf::Color::Yellow);
                    crearParticulas(center, sf::Color::Cyan);
                }
                
                std::cout << "MISION CUMPLIDA! Has creado todos los colores" << std::endl;
            }
        }
    }
}