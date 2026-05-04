#include "CriminalCaseMinigame.hpp"
#include <iostream>
#include <sstream>
#include <iostream>

CriminalCaseMinigame::CriminalCaseMinigame()
    : m_active(false)
    , m_todasEvidencias(false)
    , m_culpableEncontrado(false)
    , m_completed(false)
    , m_debugMode(true)
    , m_fontLoaded(false)
    , m_inventory(nullptr)
    , m_position(0.f, 0.f)
    , m_size(800.f, 600.f)
    , m_backgroundTexture(nullptr)
    , m_background(nullptr)
    , m_font(nullptr)
    , m_mensajeText(nullptr)
    , m_listaText(nullptr)
    , m_instruccionText(nullptr)
    , m_fondoPath("")
{
    m_font = std::make_unique<sf::Font>();
    cargarFuente();
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
    m_listaText->setCharacterSize(20);
    m_listaText->setFillColor(sf::Color::White);
    m_listaText->setOutlineColor(sf::Color::Black);
    m_listaText->setOutlineThickness(1.0f);
    
    m_instruccionText = std::make_unique<sf::Text>(*m_font);
    m_instruccionText->setCharacterSize(14);
    m_instruccionText->setFillColor(sf::Color::Black);
    m_instruccionText->setString("Click izquierdo para investigar | ESC para salir");
}

void CriminalCaseMinigame::setPosition(const sf::Vector2f& pos) {
    m_position = pos;
    if (m_background) {
        m_background->setPosition(pos);
    }
    std::cout << "setPosition llamado - Nueva posición: (" << m_position.x << ", " << m_position.y << ")" << std::endl;
}

void CriminalCaseMinigame::setSize(const sf::Vector2f& size) {
    m_size = size;
    
    if (!m_objetosOriginales.empty()) {
        escalarAreas();
        if (m_background && m_backgroundTexture) {
            sf::Vector2u textureSize = m_backgroundTexture->getSize();
            if (textureSize.x > 0 && textureSize.y > 0) {
                m_background->setScale(sf::Vector2f(
                    m_size.x / static_cast<float>(textureSize.x),
                    m_size.y / static_cast<float>(textureSize.y)
                ));
            }
            m_background->setPosition(m_position);
        }
        updateListaTexto();
    }
}

void CriminalCaseMinigame::setBaseSize(const sf::Vector2f& baseSize) {
    m_tamanioBase = baseSize;
}

void CriminalCaseMinigame::actualizarFondo() {
    if (m_fondoPath.empty()) return;
    
    m_backgroundTexture = std::make_unique<sf::Texture>();
    
    if (!m_backgroundTexture->loadFromFile(m_fondoPath)) {
        std::cerr << "Error cargando fondo: " << m_fondoPath << std::endl;
        return;
    }
    
    m_background = std::make_unique<sf::Sprite>(*m_backgroundTexture);
    
    sf::Vector2u textureSize = m_backgroundTexture->getSize();
    if (textureSize.x > 0 && textureSize.y > 0) {
        float escalaX = m_size.x / static_cast<float>(textureSize.x);
        float escalaY = m_size.y / static_cast<float>(textureSize.y);
        m_background->setScale(sf::Vector2f(escalaX, escalaY));
    }
    m_background->setPosition(m_position);
}

void CriminalCaseMinigame::init(const std::string& fondoPath,
                                 std::vector<ObjetoBuscar> objetos,
                                 std::vector<Sospechoso> sospechosos) {
    m_fondoPath = fondoPath;
    m_objetosOriginales = objetos;
    m_sospechososOriginales = sospechosos;
    
    actualizarFondo();
    
    m_objetos = m_objetosOriginales;
    m_sospechosos = m_sospechososOriginales;
    
    escalarAreas();
    updateListaTexto();
}

void CriminalCaseMinigame::reinit(const std::string& fondoPath,
                                   const std::vector<ObjetoBuscar>& objetos,
                                   const std::vector<Sospechoso>& sospechosos)
{
    m_objetosOriginales = objetos;
    m_sospechososOriginales = sospechosos;
    m_fondoPath = fondoPath;
    
    actualizarFondo();
    
    m_objetos = m_objetosOriginales;
    m_sospechosos = m_sospechososOriginales;
    
    escalarAreas();
    updateListaTexto();
}

void CriminalCaseMinigame::escalarAreas() {
    if (m_tamanioBase.x == 0 || m_tamanioBase.y == 0) {
        std::cerr << "ERROR: m_tamanioBase no configurado" << std::endl;
        return;
    }
    
    float escalaX = m_size.x / m_tamanioBase.x;
    float escalaY = m_size.y / m_tamanioBase.y;
    
    m_objetos.clear();
    for (size_t i = 0; i < m_objetosOriginales.size(); ++i) {
        ObjetoBuscar objEscalado = m_objetosOriginales[i];
        objEscalado.area = sf::FloatRect(
            sf::Vector2f(m_objetosOriginales[i].area.position.x * escalaX,
                        m_objetosOriginales[i].area.position.y * escalaY),
            sf::Vector2f(m_objetosOriginales[i].area.size.x * escalaX,
                        m_objetosOriginales[i].area.size.y * escalaY)
        );
        m_objetos.push_back(objEscalado);
    }
    
    m_sospechosos.clear();
    for (size_t i = 0; i < m_sospechososOriginales.size(); ++i) {
        Sospechoso sosEscalado = m_sospechososOriginales[i];
        sosEscalado.area = sf::FloatRect(
            sf::Vector2f(m_sospechososOriginales[i].area.position.x * escalaX,
                        m_sospechososOriginales[i].area.position.y * escalaY),
            sf::Vector2f(m_sospechososOriginales[i].area.size.x * escalaX,
                        m_sospechososOriginales[i].area.size.y * escalaY)
        );
        m_sospechosos.push_back(sosEscalado);
    }
}

void CriminalCaseMinigame::activate()
{
    if (m_completed) return;
    
    // Forzar la posición y tamaño correctos desde el nivel
    std::cout << "=== ACTIVANDO MINIJUEGO ===" << std::endl;
    std::cout << "Posición ANTES de forzar: (" << m_position.x << ", " << m_position.y << ")" << std::endl;
    
    // NO hagas nada aquí - la posición ya debería estar correcta desde setPosition()
    
    m_active = true;
    m_todasEvidencias = false;
    m_culpableEncontrado = false;
    
    // Imprimir las áreas ESCALADAS actuales
    std::cout << "\n=== ÁREAS ESCALADAS ACTUALES ===" << std::endl;
    for (size_t i = 0; i < m_objetos.size(); ++i) {
        std::cout << "OBJETO " << i << ": " << m_objetos[i].nombre << std::endl;
        std::cout << "  Area: (" << m_objetos[i].area.position.x << ", " << m_objetos[i].area.position.y 
                  << ", " << m_objetos[i].area.size.x << ", " << m_objetos[i].area.size.y << ")" << std::endl;
    }
    
    for (auto& obj : m_objetos) obj.encontrado = false;
    for (auto& s : m_sospechosos) s.acusado = false;
    
    updateListaTexto();
    mostrarMensaje("Encuentra las 10 pistas en la playa", 3.0f);
}

void CriminalCaseMinigame::deactivate() {
    m_active = false;
    m_mensajeTemp.tiempoRestante = 0.0f;
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
            
            sf::Vector2i mousePixel(mousePressed->position.x, mousePressed->position.y);
            
            // Coordenadas locales (dentro del minijuego)
            sf::Vector2f localMousePos = sf::Vector2f(
                static_cast<float>(mousePixel.x) - m_position.x,
                static_cast<float>(mousePixel.y) - m_position.y
            );
            
            // Coordenadas en el sistema ORIGINAL (antes del escalado)
            sf::Vector2f originalCoords;
            if (m_tamanioBase.x > 0 && m_tamanioBase.y > 0) {
                originalCoords.x = localMousePos.x * (m_tamanioBase.x / m_size.x);
                originalCoords.y = localMousePos.y * (m_tamanioBase.y / m_size.y);
            } else {
                originalCoords = localMousePos;
            }
            
            // ===== IMPRESIÓN SIMPLIFICADA SOLO COORDENADAS =====
            std::cout << "\n=== COORDENADAS PARA CONFIGURACIÓN ===" << std::endl;
            std::cout << "Rectángulo: sf::FloatRect(sf::Vector2f(" 
                      << originalCoords.x << "f, " << originalCoords.y << "f), ";
            
            // Sugerir un tamaño predeterminado (puedes cambiarlo)
            std::cout << "sf::Vector2f(30f, 30f))" << std::endl;
            std::cout << "=======================================" << std::endl;
            
            // Opcional: También mostrar en formato directo para copiar
            std::cout << "Copia esto: (" << originalCoords.x << ", " << originalCoords.y << ")" << std::endl;
            
            // Continuar con la lógica normal del juego...
            // FASE 1: Buscar evidencias
            if (!m_todasEvidencias) {
                for (size_t i = 0; i < m_objetos.size(); ++i) {
                    auto& obj = m_objetos[i];
                    if (!obj.encontrado && obj.area.contains(localMousePos)) {
                        std::cout << "Encontraste: " << obj.nombre << std::endl;
                        obj.encontrado = true;
                        mostrarMensaje(obj.nombre + "\n" + obj.descripcion, 3.0f);
                        updateListaTexto();
                        
                        if (m_inventory) {
                            Item nuevoItem;
                            nuevoItem.name = obj.nombre;
                            nuevoItem.color = sf::Color(255, 215, 0);
                            m_inventory->addItem(nuevoItem);
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
                    if (!sos.acusado && sos.area.contains(localMousePos)) {
                        std::cout << "Interrogaste a: " << sos.nombre << std::endl;
                        sos.acusado = true;
                        
                        if (sos.esElCulpable) {
                            std::cout << "*** ES EL CULPABLE ***" << std::endl;
                            mostrarMensaje("CASO RESUELTO\n" + sos.nombre + " es el culpable.\n" + sos.descripcion, 5.0f);
                            m_culpableEncontrado = true;
                            m_completed = true;
                            verificarCompletado();
                        } else {
                            std::cout << "Es inocente" << std::endl;
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
        if (m_onCompleteCallback) m_onCompleteCallback(true);
    }
}

int CriminalCaseMinigame::contarObjetosEncontrados() const {
    int count = 0;
    for (const auto& obj : m_objetos) if (obj.encontrado) count++;
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
        ss << "----- SOSPECHOSOS -----\n";
        int acusados = 0;
        for (const auto& s : m_sospechosos) if (s.acusado) acusados++;
        ss << "(" << acusados << "/" << m_sospechosos.size() << ")\n\n";
        
        for (const auto& sos : m_sospechosos) {
            if (sos.acusado) {
                ss << (sos.esElCulpable ? " o " : " x ");
            } else {
                ss << " o ";
            }
            ss << sos.nombre << "\n";
        }
    }
    
    m_listaText->setString(ss.str());
    
    float escalaTexto = std::min(m_size.x, m_size.y) / 800.0f;
    int nuevoTamano = static_cast<int>(16 * escalaTexto);
    if (nuevoTamano < 10) nuevoTamano = 10;
    if (nuevoTamano > 30) nuevoTamano = 30;
    m_listaText->setCharacterSize(nuevoTamano);
    
    float offsetX = 20.0f * escalaTexto;
    float offsetY = 80.0f * escalaTexto;
    m_listaText->setPosition(sf::Vector2f(m_position.x + offsetX, m_position.y + offsetY));
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
    
    if (m_background) window.draw(*m_background);
    
    if (m_debugMode) {
        for (const auto& obj : m_objetos) {
            if (!obj.encontrado) {
                sf::RectangleShape rect(sf::Vector2f(obj.area.size.x, obj.area.size.y));
                rect.setPosition(sf::Vector2f(m_position.x + obj.area.position.x, 
                                               m_position.y + obj.area.position.y));
                rect.setFillColor(sf::Color(255, 255, 0, 100));
                rect.setOutlineThickness(2.f);
                rect.setOutlineColor(sf::Color::Yellow);
                window.draw(rect);
            }
        }
        
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
    
    if (m_fontLoaded) {
        float escalaTexto = std::min(m_size.x, m_size.y) / 800.0f;
        
        int tamanoInstruccion = static_cast<int>(14 * escalaTexto);
        if (tamanoInstruccion < 10) tamanoInstruccion = 10;
        if (tamanoInstruccion > 30) tamanoInstruccion = 30;
        m_instruccionText->setCharacterSize(tamanoInstruccion);
        m_instruccionText->setPosition(sf::Vector2f(m_position.x + 20.0f * escalaTexto, 
                                                     m_position.y + 20.0f * escalaTexto));
        window.draw(*m_instruccionText);
        
        window.draw(*m_listaText);
        
        if (!m_mensajeText->getString().isEmpty()) {
            sf::Vector2u winSize = window.getSize();
            centrarTexto(*m_mensajeText, winSize.x / 2.f, winSize.y - 100.f);
            window.draw(*m_mensajeText);
        }
    }
}