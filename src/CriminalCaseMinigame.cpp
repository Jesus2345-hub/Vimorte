#include "CriminalCaseMinigame.hpp"
#include <iostream>
#include <sstream>

CriminalCaseMinigame::CriminalCaseMinigame()
    : m_active(false)
    , m_todasEvidencias(false)
    , m_culpableEncontrado(false)
    , m_completed(false)
    , m_debugMode(true)  // Temporalmente true para ajustar coordenadas
    , m_fontLoaded(false)
    , m_inventory(nullptr)
    , m_ultimoObjetoEncontrado(-1)
    , m_position(0.f, 0.f)
    , m_size(800.f, 600.f)
    , m_backgroundTexture(nullptr)
    , m_background(nullptr)
    , m_font(nullptr)
    , m_mensajeText(nullptr)
    , m_listaText(nullptr)
    , m_instruccionText(nullptr)
{
    m_font = std::make_unique<sf::Font>();
    cargarFuente();
}

void CriminalCaseMinigame::setPosition(const sf::Vector2f& pos) {
    m_position = pos;
    if (m_background) {
        m_background->setPosition(pos);
    }
}

void CriminalCaseMinigame::setSize(const sf::Vector2f& size) {
    m_size = size;
}

void CriminalCaseMinigame::cargarFuente() {
    if (!m_font->openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente para CriminalCaseMinigame" << std::endl;
        m_fontLoaded = false;
        return;
    }
    m_fontLoaded = true;
    
    m_mensajeText = std::make_unique<sf::Text>(*m_font);
    m_mensajeText->setCharacterSize(24);
    m_mensajeText->setFillColor(sf::Color::Yellow);
    m_mensajeText->setOutlineColor(sf::Color::Black);
    m_mensajeText->setOutlineThickness(2.0f);
    
    m_listaText = std::make_unique<sf::Text>(*m_font);
    m_listaText->setCharacterSize(16);
    m_listaText->setFillColor(sf::Color::White);
    m_listaText->setOutlineColor(sf::Color::Black);
    m_listaText->setOutlineThickness(1.0f);
    
    m_instruccionText = std::make_unique<sf::Text>(*m_font);
    m_instruccionText->setCharacterSize(14);
    m_instruccionText->setFillColor(sf::Color::Black);
    m_instruccionText->setString("Click izquierdo para investigar | ESC para salir");
}

void CriminalCaseMinigame::init(const std::string& fondoPath,
                                 std::vector<ObjetoBuscar> objetos,
                                 std::vector<Sospechoso> sospechosos) {
    // Cargar solo el fondo
    m_backgroundTexture = std::make_unique<sf::Texture>();
    
    if (!m_backgroundTexture->loadFromFile(fondoPath)) {
        std::cerr << "Error cargando fondo: " << fondoPath << std::endl;
    } else {
        m_background = std::make_unique<sf::Sprite>(*m_backgroundTexture);
        
        sf::Vector2u textureSize = m_backgroundTexture->getSize();
        if (textureSize.x > 0 && textureSize.y > 0) {
            m_background->setScale(sf::Vector2f(
                m_size.x / textureSize.x,
                m_size.y / textureSize.y
            ));
        }
        m_background->setPosition(m_position);
    }
    
    m_objetos = std::move(objetos);
    m_sospechosos = std::move(sospechosos);
}

void CriminalCaseMinigame::activate() {
    if (m_completed) return;
    
    m_active = true;
    m_todasEvidencias = false;
    m_culpableEncontrado = false;
    
    // Resetear estados
    for (auto& obj : m_objetos) {
        obj.encontrado = false;
    }
    for (auto& s : m_sospechosos) {
        s.acusado = false;
    }
    
    updateListaTexto();
    mostrarMensaje("Encuentra las 10 pistas en la playa", 3.0f);
    std::cout << "Minijuego Criminal Case activado" << std::endl;
}

void CriminalCaseMinigame::deactivate() {
    m_active = false;
    m_mensajeTemp.tiempoRestante = 0.0f;
    std::cout << "Minijuego Criminal Case desactivado" << std::endl;
}

void CriminalCaseMinigame::centrarTexto(sf::Text& text, float x, float y) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    text.setPosition(sf::Vector2f(x, y));
}

void CriminalCaseMinigame::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (!m_active || m_completed) return;
    
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos = window.mapPixelToCoords(
                sf::Vector2i(mousePressed->position.x, mousePressed->position.y)
            );
            
            // Convertir a coordenadas relativas al minijuego
            mousePos.x -= m_position.x;
            mousePos.y -= m_position.y;
            
            // ===== MOSTRAR COORDENADAS EN CONSOLA =====
            std::cout << "CLICK EN COORDENADAS: (" << mousePos.x << ", " << mousePos.y << ")" << std::endl;
            
            // Mostrar coordenadas para debug (útil para ajustar áreas)
            if (m_debugMode) {
                std::cout << "Click en: (" << mousePos.x << ", " << mousePos.y << ")" << std::endl;
            }
            
            // FASE 1: Buscar evidencias
            if (!m_todasEvidencias) {
                for (size_t i = 0; i < m_objetos.size(); ++i) {
                    auto& obj = m_objetos[i];
                    if (!obj.encontrado && obj.area.contains(mousePos)) {
                        obj.encontrado = true;
                        m_ultimoObjetoEncontrado = i;
                        m_verdeClock.restart();
                        
                        mostrarMensaje( obj.nombre + "\n" + obj.descripcion, 3.0f);
                        updateListaTexto();
                        
                        // Añadir al inventario
                        if (m_inventory) {
                            Item nuevoItem;
                            nuevoItem.name = obj.nombre;
                            nuevoItem.color = sf::Color(255, 215, 0);  // Dorado
                            nuevoItem.rutaImagen = "";
                            m_inventory->addItem(nuevoItem);
                            std::cout << "Evidencia encontrada: " << obj.nombre << std::endl;
                        }
                        
                        if (contarObjetosEncontrados() == static_cast<int>(m_objetos.size())) {
                            m_todasEvidencias = true;
                            mostrarMensaje("TODAS LAS PISTAS ENCONTRADAS\nAhora interroga a los sospechosos", 4.0f);
                            updateListaTexto();
                        }
                        break;
                    }
                }
            }
            // FASE 2: Interrogar sospechosos
            else {
                for (auto& sos : m_sospechosos) {
                    if (!sos.acusado && sos.area.contains(mousePos)) {
                        sos.acusado = true;
                        
                        if (sos.esElCulpable) {
                            mostrarMensaje("CASO RESUELTO\n" + sos.nombre + " es el culpable.\n" + sos.descripcion, 5.0f);
                            m_culpableEncontrado = true;
                            m_completed = true;
                            verificarCompletado();
                        } else {
                            mostrarMensaje(sos.nombre + " es inocente.\n" + sos.descripcion, 2.5f);
                        }
                        updateListaTexto();
                        break;
                    }
                }
            }
        }
    }
}

void CriminalCaseMinigame::mostrarMensaje(const std::string& msg, float duracion) {
    m_mensajeTemp.texto = msg;
    m_mensajeTemp.tiempoRestante = duracion;
    m_mensajeText->setString(msg);
    m_mensajeClock.restart();
}

void CriminalCaseMinigame::verificarCompletado() {
    if (m_todasEvidencias && m_culpableEncontrado && !m_completed) {
        m_active = false;
        m_completed = true;
        if (m_onCompleteCallback) {
            m_onCompleteCallback(true);
        }
    }
}

int CriminalCaseMinigame::contarObjetosEncontrados() const {
    int count = 0;
    for (const auto& obj : m_objetos) {
        if (obj.encontrado) count++;
    }
    return count;
}

void CriminalCaseMinigame::updateListaTexto() {
    if (!m_fontLoaded) return;
    
    std::stringstream ss;
    
    if (!m_todasEvidencias) {
        ss << "---- PISTAS POR ENCONTRAR ---\n";
        ss << "(" << contarObjetosEncontrados() << "/" << m_objetos.size() << ")\n\n";
        for (const auto& obj : m_objetos) {
            ss << (obj.encontrado ? " o " : " x ") << obj.nombre << "\n";
        }
    } else {
        ss << "---SOSPECHOSOS ----\n";
        int acusados = 0;
        for (const auto& s : m_sospechosos) if (s.acusado) acusados++;
        ss << "(" << acusados << "/" << m_sospechosos.size() << ")\n\n";
        
        for (const auto& sos : m_sospechosos) {
            if (sos.acusado) {
                ss << (sos.esElCulpable ? " O " : " X ");
            } else {
                ss << "○ ";
            }
            ss << sos.nombre << "\n";
        }
    }
    
    m_listaText->setString(ss.str());
    m_listaText->setPosition(sf::Vector2f(m_position.x + 20.f, m_position.y + 80.f));
}

void CriminalCaseMinigame::update(float dt) {
    if (!m_active) return;
    
    if (m_mensajeTemp.tiempoRestante > 0.0f) {
        m_mensajeTemp.tiempoRestante -= dt;
        if (m_mensajeTemp.tiempoRestante <= 0.0f) {
            m_mensajeText->setString("");
        }
    }
}

void CriminalCaseMinigame::draw(sf::RenderWindow& window) {
    if (!m_active) return;
    
    // Dibujar fondo
    if (m_background) {
        window.draw(*m_background);
    }
    
    // SOLO en modo debug: mostrar áreas interactivas
    if (m_debugMode) {
        // Áreas de objetos (amarillo) - para que puedas ajustar las coordenadas
        for (const auto& obj : m_objetos) {
            if (!obj.encontrado) {
                sf::RectangleShape rect(sf::Vector2f(obj.area.size.x, obj.area.size.y));
                rect.setPosition(sf::Vector2f(m_position.x + obj.area.position.x, 
                                               m_position.y + obj.area.position.y));
                rect.setFillColor(sf::Color(255, 255, 0, 100));
                rect.setOutlineThickness(2.f);
                rect.setOutlineColor(sf::Color::Yellow);
                // window.draw(rect);
            }
        }
        
        // Áreas de sospechosos (rojo)
        if (m_todasEvidencias) {
            for (const auto& sos : m_sospechosos) {
                if (!sos.acusado) {
                    sf::RectangleShape rect(sf::Vector2f(sos.area.size.x, sos.area.size.y));
                    rect.setPosition(sf::Vector2f(m_position.x + sos.area.position.x,
                                                   m_position.y + sos.area.position.y));
                    rect.setFillColor(sf::Color(255, 0, 0, 100));
                    rect.setOutlineThickness(2.f);
                    rect.setOutlineColor(sf::Color::Red);
                    window.draw(rect);
                }
            }
        }
    }
    
    // Dibujar textos
    if (m_fontLoaded) {
        m_instruccionText->setPosition(sf::Vector2f(m_position.x + 20.f, m_position.y + 20.f));
        window.draw(*m_instruccionText);
        window.draw(*m_listaText);
        
        if (!m_mensajeText->getString().isEmpty()) {
            sf::Vector2u winSize = window.getSize();
            centrarTexto(*m_mensajeText, winSize.x / 2.f, winSize.y - 100.f);
            window.draw(*m_mensajeText);
        }
    }
}