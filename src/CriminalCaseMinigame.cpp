#include "CriminalCaseMinigame.hpp"
#include <iostream>
#include <memory>
#include <sstream>
#include <random>
#include <fstream>
#include <chrono>

// Constructor
CriminalCaseMinigame::CriminalCaseMinigame()
    : m_active(false)
    , m_todasEvidencias(false)
    , m_culpableEncontrado(false)
    , m_completed(false)
    , m_debugMode(true)
    , m_fontLoaded(false)
    , m_waitingForNarrative(false)
    , m_gameState(CriminalGameState::BUSCANDO_EVIDENCIAS)
    , m_setActualObjetos(-1)
    , m_setActualSospechosos(-1)
    , m_setActualDialogos(-1)
    , m_dialogoActualIndex(0)
    , m_inventory(nullptr)
    , m_rng(std::chrono::steady_clock::now().time_since_epoch().count())
    , m_backgroundTexture(nullptr)
    , m_background(nullptr)
    , m_font(nullptr)
    , m_mensajeText(nullptr)
    , m_listaText(nullptr)
    , m_instruccionText(nullptr)
    , m_dialogoText(nullptr)
    , m_personaText(nullptr)
    , m_instruccionesNarrativas(nullptr)
    , m_tituloEleccion(nullptr)
    , m_mensajeAdvertencia(nullptr)
{
    m_mensajeTemp.tiempoRestante = 0.0f;
    m_position = sf::Vector2f(0, 0);
    m_size = sf::Vector2f(800, 600);
    m_tamanioBase = sf::Vector2f(800, 600);
    cargarFuente();
}

// Métodos para agregar sets
void CriminalCaseMinigame::agregarSetObjetos(const std::vector<ObjetoBuscar>& objetos) {
    m_poolObjetos.push_back(objetos);
}

void CriminalCaseMinigame::agregarSetSospechosos(const std::vector<Sospechoso>& sospechosos) {
    m_poolSospechosos.push_back(sospechosos);
}

void CriminalCaseMinigame::agregarSetDialogos(const std::vector<DialogoNarrativo>& dialogos) {
   m_poolDialogos.push_back(dialogos);
}

void CriminalCaseMinigame::limpiarPools() {
    m_poolObjetos.clear();
    m_poolSospechosos.clear();
    m_poolDialogos.clear();
}

void CriminalCaseMinigame::cargarFuente() {
    if (!m_fontLoaded) {
        m_font = std::make_unique<sf::Font>();
        if (!m_font->openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
            std::cerr << "Error cargando fuente" << std::endl;
            return;
        }
        m_fontLoaded = true;
        
        // Estilo de la versión simple (con outline)
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
        m_instruccionText->setCharacterSize(8);
        m_instruccionText->setFillColor(sf::Color::White);
        m_instruccionText->setOutlineColor(sf::Color::Black);
        m_instruccionText->setOutlineThickness(1.0f);
        m_instruccionText->setString("Click en los objetos para recolectar pistas | ESC para salir");     
        
        
        m_dialogoText = std::make_unique<sf::Text>(*m_font);
        m_dialogoText->setCharacterSize(18);
        m_dialogoText->setFillColor(sf::Color::White);
        m_dialogoText->setOutlineColor(sf::Color::Black);
        m_dialogoText->setOutlineThickness(1.0f);
        
        m_personaText = std::make_unique<sf::Text>(*m_font);
        m_personaText->setCharacterSize(24);
        m_personaText->setFillColor(sf::Color::Yellow);
        m_personaText->setOutlineColor(sf::Color::Black);
        m_personaText->setOutlineThickness(1.0f);
        m_personaText->setStyle(sf::Text::Bold);
        
        m_instruccionesNarrativas = std::make_unique<sf::Text>(*m_font);
        m_instruccionesNarrativas->setCharacterSize(14);
        m_instruccionesNarrativas->setFillColor(sf::Color(150, 150, 150));
        m_instruccionesNarrativas->setOutlineColor(sf::Color::Black);
        m_instruccionesNarrativas->setOutlineThickness(0.5f);
        m_instruccionesNarrativas->setString("ENTER: Siguiente | BACKSPACE: Anterior");
        
        m_tituloEleccion = std::make_unique<sf::Text>(*m_font);
        m_tituloEleccion->setCharacterSize(32);
        m_tituloEleccion->setFillColor(sf::Color::Red);
        m_tituloEleccion->setOutlineColor(sf::Color::Black);
        m_tituloEleccion->setOutlineThickness(2.0f);
        m_tituloEleccion->setStyle(sf::Text::Bold);
        m_tituloEleccion->setString("QUIEN ES EL CULPABLE");
        
        m_mensajeAdvertencia = std::make_unique<sf::Text>(*m_font);
        m_mensajeAdvertencia->setCharacterSize(16);
        m_mensajeAdvertencia->setFillColor(sf::Color(255, 200, 100));
        m_mensajeAdvertencia->setOutlineColor(sf::Color::Black);
        m_mensajeAdvertencia->setOutlineThickness(1.0f);
        m_mensajeAdvertencia->setString("Elige sabiamente. Si encarcelas al inocente, el verdadero culpable escapara con tus pruebas");
    }
}

void CriminalCaseMinigame::setBaseSize(const sf::Vector2f& baseSize) {
    m_tamanioBase = baseSize;
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
        objEscalado.encontrado = m_objetosOriginales[i].encontrado;
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
        sosEscalado.acusado = m_sospechososOriginales[i].acusado;
        m_sospechosos.push_back(sosEscalado);
    }
    
    updateListaTexto();
}

void CriminalCaseMinigame::actualizarFondo() {
    if (m_fondoPath.empty()) return;
    
    if (!m_backgroundTexture) {
        m_backgroundTexture = std::make_unique<sf::Texture>();
    }
    
    if (!m_backgroundTexture->loadFromFile(m_fondoPath)) {
        std::cerr << "Error cargando fondo: " << m_fondoPath << std::endl;
        return;
    }
    
    m_background = std::make_unique<sf::Sprite>(*m_backgroundTexture);
    
    // Asegurar que m_size tiene valores válidos
    if (m_size.x <= 0 || m_size.y <= 0) {
        std::cerr << "WARNING: m_size inválido en actualizarFondo: " << m_size.x << "x" << m_size.y << std::endl;
        return;
    }
    
    sf::Vector2u textureSize = m_backgroundTexture->getSize();
    if (textureSize.x > 0 && textureSize.y > 0) {
        float escalaX = m_size.x / static_cast<float>(textureSize.x);
        float escalaY = m_size.y / static_cast<float>(textureSize.y);
        m_background->setScale(sf::Vector2f(escalaX, escalaY));
    }
    m_background->setPosition(m_position);
}

void CriminalCaseMinigame::setPosition(const sf::Vector2f& pos) {
    m_position = pos;
    if (m_background) {
        m_background->setPosition(pos);
    }
}

void CriminalCaseMinigame::setSize(const sf::Vector2f& size) {
    if (size.x <= 0 || size.y <= 0) return;
    m_size = size;
    actualizarFondo();
    escalarAreas();
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
}

void CriminalCaseMinigame::reinit(const std::string& fondoPath,
                                    const std::vector<ObjetoBuscar>& objetos,
                                    const std::vector<Sospechoso>& sospechosos) {
    m_fondoPath = fondoPath;
    m_objetosOriginales = objetos;
    m_sospechososOriginales = sospechosos;
    
    actualizarFondo();
    
    m_objetos = m_objetosOriginales;
    m_sospechosos = m_sospechososOriginales;
    
    escalarAreas();
}

void CriminalCaseMinigame::generarNuevoCaso() {
    if (m_poolObjetos.empty() || m_poolSospechosos.empty()) {
        std::cerr << "Error: No hay suficientes datos en los pools" << std::endl;
        return;
    }
    
    std::uniform_int_distribution<int> distObjetos(0, static_cast<int>(m_poolObjetos.size()) - 1);
    std::uniform_int_distribution<int> distSospechosos(0, static_cast<int>(m_poolSospechosos.size()) - 1);
    std::uniform_int_distribution<int> distDialogos(0, static_cast<int>(m_poolDialogos.size()) - 1);
    
    m_setActualObjetos = distObjetos(m_rng);
    m_setActualSospechosos = distSospechosos(m_rng);
    m_setActualDialogos = distDialogos(m_rng);  // IMPORTANTE: Seleccionar diálogos también
    
    // Cargar los datos seleccionados
    m_objetosOriginales = m_poolObjetos[m_setActualObjetos];
    m_sospechososOriginales = m_poolSospechosos[m_setActualSospechosos];
    
    // Cargar diálogos del mismo índice o del correspondiente
    if (m_setActualDialogos < static_cast<int>(m_poolDialogos.size()) && 
        !m_poolDialogos[m_setActualDialogos].empty()) {
        m_dialogosActuales = m_poolDialogos[m_setActualDialogos];
        std::cout << "Diálogos cargados: " << m_dialogosActuales.size() << " diálogos" << std::endl;
    } else if (m_setActualSospechosos < static_cast<int>(m_poolDialogos.size()) && 
               !m_poolDialogos[m_setActualSospechosos].empty()) {
        // Fallback: usar diálogos del mismo set que sospechosos
        m_dialogosActuales = m_poolDialogos[m_setActualSospechosos];
        std::cout << "Diálogos cargados (fallback): " << m_dialogosActuales.size() << std::endl;
    } else {
        // Si no hay diálogos, crear unos por defecto
        m_dialogosActuales.clear();
        m_dialogosActuales.emplace_back("Testigo", 
            "He visto lo sucedido esa noche. Las pistas no mienten.");
        m_dialogosActuales.emplace_back("Alguien más", 
            "Todos tenemos algo que decir. Escucha con atención.");
        std::cout << "Diálogos por defecto creados" << std::endl;
    }
    
    // Resetear estados de objetos
    for (auto& obj : m_objetosOriginales) {
        obj.encontrado = false;
    }
    for (auto& sos : m_sospechososOriginales) {
        sos.acusado = false;
    }
    
    m_objetos = m_objetosOriginales;
    m_sospechosos = m_sospechososOriginales;
    
    escalarAreas();
    
    // RESET COMPLETO DEL ESTADO DEL JUEGO
    m_todasEvidencias = false;
    m_culpableEncontrado = false;
    m_completed = false;
    m_gameState = CriminalGameState::BUSCANDO_EVIDENCIAS;
    m_dialogoActualIndex = 0;
    m_waitingForNarrative = false;
    m_mensajeTemp.tiempoRestante = 0.0f;
    
    // Limpiar el mensaje temporal
    m_mensajeTemp.texto = "";
    
    std::cout << "Nuevo caso generado - Diálogos: " << m_dialogosActuales.size() 
              << " | Estado: BUSCANDO_EVIDENCIAS" << std::endl;
}
void CriminalCaseMinigame::activate() {
    if (m_completed) {
        // Si está completado, resetear antes de reactivar
        resetCompletamente();
    }
    
    // RESET COMPLETO DEL ESTADO
    m_active = false;  // Temporalmente desactivar para reset
    m_completed = false;
    m_todasEvidencias = false;
    m_culpableEncontrado = false;
    m_gameState = CriminalGameState::BUSCANDO_EVIDENCIAS;
    m_dialogoActualIndex = 0;
    m_waitingForNarrative = false;
    m_mensajeTemp.tiempoRestante = 0.0f;
    m_mensajeTemp.texto = "";
    
    // Regenerar el caso para asegurar diálogos frescos
    if (!m_poolObjetos.empty()) {
        generarNuevoCaso();
    }
    
    actualizarFondo();
    escalarAreas();
    updateListaTexto();
    
    m_active = true;
    mostrarMensaje("Encuentra todas las pistas", 2.0f);
    
    std::cout << "Minijuego activado - GameState: " << static_cast<int>(m_gameState) 
              << " | Diálogos: " << m_dialogosActuales.size() << std::endl;
}
void CriminalCaseMinigame::resetCompletamente() {
    // Resetear todos los estados
    m_active = false;
    m_completed = false;
    m_todasEvidencias = false;
    m_culpableEncontrado = false;
    m_gameState = CriminalGameState::BUSCANDO_EVIDENCIAS;
    m_dialogoActualIndex = 0;
    m_waitingForNarrative = false;
    m_mensajeTemp.tiempoRestante = 0.0f;
    m_mensajeTemp.texto = "";
    
    // Limpiar inventario de items del minijuego
    if (m_inventory) {
        for (const auto& obj : m_objetos) {
            if (obj.encontrado) {
                for (int i = 0; i < 20; i++) {
                    Item* item = m_inventory->getItem(i);
                    if (item && item->name == obj.nombre) {
                        m_inventory->removeItem(i);
                        break;
                    }
                }
            }
        }
    }
    
    // Resetear objetos y sospechosos
    for (auto& obj : m_objetosOriginales) {
        obj.encontrado = false;
    }
    for (auto& sos : m_sospechososOriginales) {
        sos.acusado = false;
    }
    
    m_objetos = m_objetosOriginales;
    m_sospechosos = m_sospechososOriginales;
    
    // Regenerar caso para diálogos nuevos
    if (!m_poolObjetos.empty()) {
        generarNuevoCaso();
    }
    
    escalarAreas();
    updateListaTexto();
    
    std::cout << "Reset completo del minijuego" << std::endl;
}

void CriminalCaseMinigame::deactivate() {
    m_active = false;
    m_mensajeTemp.tiempoRestante = 0.0f;
}

void CriminalCaseMinigame::iniciarFaseNarrativa() {
    if (m_dialogosActuales.empty()) {
        std::cout << "ADVERTENCIA: No hay diálogos cargados, pasando directamente a elección" << std::endl;
        m_gameState = CriminalGameState::ELECCION_FINAL;
        mostrarMensaje("Sin testimonios disponibles. Decide quién es el culpable.", 2.0f);
        return;
    }
    
    m_gameState = CriminalGameState::NARRATIVA;
    m_dialogoActualIndex = 0;
    
    m_fondoNarrativo.setSize(m_size);
    m_fondoNarrativo.setFillColor(sf::Color(0, 0, 0, 220));
    m_fondoNarrativo.setPosition(m_position);
    
    // Hacer el cuadro de diálogo MÁS GRANDE
    float cuadroAncho = m_size.x * 0.95f;   
    float cuadroAlto = m_size.y * 0.60f;   
    
    m_cuadroDialogo.setSize(sf::Vector2f(cuadroAncho, cuadroAlto));
    m_cuadroDialogo.setFillColor(sf::Color(30, 30, 40, 240));
    m_cuadroDialogo.setOutlineColor(sf::Color(200, 180, 100));
    m_cuadroDialogo.setOutlineThickness(4.f);
    m_cuadroDialogo.setPosition(sf::Vector2f(
        m_position.x + (m_size.x - cuadroAncho) / 2,
        m_position.y +(m_size.y - cuadroAlto)/2  
    ));
    
    std::cout << "Fase narrativa iniciada - Diálogos disponibles: " << m_dialogosActuales.size() << std::endl;
}


void CriminalCaseMinigame::verificarCompletado() {
    bool todosEncontrados = true;
    for (const auto& obj : m_objetos) {
        if (!obj.encontrado) {
            todosEncontrados = false;
            break;
        }
    }
    
    if (todosEncontrados && m_gameState == CriminalGameState::BUSCANDO_EVIDENCIAS) {
        m_todasEvidencias = true;
        iniciarFaseNarrativa();
        mostrarMensaje("Has reunido todas las pistas. Ahora escucha los testimonios...", 5.0f);
    }
}


void CriminalCaseMinigame::procesarAcusacion(int sospechosoIndex) {
    if (sospechosoIndex < 0 || sospechosoIndex >= static_cast<int>(m_sospechosos.size())) return;
    
    Sospechoso& sospechoso = m_sospechosos[sospechosoIndex];
    
    if (sospechoso.esElCulpable) {
        mostrarMensaje("Correcto " + sospechoso.nombre + " es el culpable. Caso cerrado", 3.0f);
        m_completed = true;
        m_active = false;
        
        if (m_onCompleteCallback) {
            m_onCompleteCallback(true);
        }
    } else {
        std::string mensajePenalizacion = "ERROR. Has acusado a " + sospechoso.nombre + 
                              ".\nEl verdadero culpable escapo con todas las pruebas.\n" +
                              "Las pistas encontradas han desaparecido...";
        mostrarMensaje(mensajePenalizacion, 4.0f);
        
        if (m_inventory) {
            for (const auto& obj : m_objetos) {
                if (obj.encontrado) {
                    for (int i = 0; i < 20; i++) {
                        Item* item = m_inventory->getItem(i);
                        if (item && item->name == obj.nombre) {
                            m_inventory->removeItem(i);
                            break;
                        }
                    }
                }
            }
        }
        
        m_active = false;
        
        // IMPORTANTE: Resetear el estado para la próxima vez
        m_gameState = CriminalGameState::BUSCANDO_EVIDENCIAS;
        m_todasEvidencias = false;
        m_dialogoActualIndex = 0;
        
        if (m_onCompleteCallback) {
            m_onCompleteCallback(false);
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
    if (!m_listaText || !m_fontLoaded) return;
    
    std::stringstream ss;
    ss << "PISTAS POR ENCONTRAR:\n";
    for (const auto& obj : m_objetos) {
        ss << (obj.encontrado ? "o " : "x ");
        ss << obj.nombre << "\n";
    }
    
    ss << "\nEVIDENCIAS: " << contarObjetosEncontrados() << "/" << m_objetos.size();
    m_listaText->setString(ss.str());
    
    // TAMAÑO MÁS GRANDE: de 14-28 a 20-36
    float escalaTexto = std::min(m_size.x, m_size.y) / 800.0f;
    int nuevoTamano = static_cast<int>(20 * escalaTexto);  // AUMENTADO de 14 a 20
    if (nuevoTamano < 16) nuevoTamano = 16;  // Mínimo más alto
    if (nuevoTamano > 36) nuevoTamano = 36;  // Máximo más alto
    
    m_listaText->setCharacterSize(nuevoTamano);
    m_listaText->setOutlineThickness(1.5f);  // Outline más grueso para mejor legibilidad
    
    // Posición esquina superior izquierda
    float offsetX = 20.0f * escalaTexto;
    float offsetY = 20.0f * escalaTexto;
    m_listaText->setPosition(sf::Vector2f(m_position.x + offsetX, m_position.y + offsetY));
}

void CriminalCaseMinigame::mostrarMensaje(const std::string& msg, float duracion) {
    m_mensajeTemp.texto = msg;
    m_mensajeTemp.tiempoRestante = duracion;
    if (m_mensajeText) {
        m_mensajeText->setString(msg);
    }
}

void CriminalCaseMinigame::centrarTexto(sf::Text& text, float x, float y) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    text.setPosition(sf::Vector2f(x, y));
}

void CriminalCaseMinigame::cargarFondoDialogo(DialogoNarrativo& dialogo) {
    // Solo cargar si hay una ruta de fondo y no está cargado
    if (!dialogo.fondoPath.empty() && !dialogo.fondoCargado) {
        dialogo.fondoTexture = std::make_shared<sf::Texture>();
        
        if (dialogo.fondoTexture->loadFromFile(dialogo.fondoPath)) {
            dialogo.fondoCargado = true;
            std::cout << "Fondo cargado: " << dialogo.fondoPath << std::endl;
        } else {
            std::cerr << "Error cargando fondo: " << dialogo.fondoPath << std::endl;
            dialogo.fondoCargado = false;
        }
    }
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
            
            //  IMPRESIÓN DE COORDENADAS 
            std::cout << "\n=== COORDENADAS PARA CONFIGURACIÓN ===" << std::endl;
            std::cout << "Rectángulo: sf::FloatRect(sf::Vector2f(" 
                      << originalCoords.x << "f, " << originalCoords.y << "f), ";
            std::cout << "sf::Vector2f(30f, 30f))" << std::endl;
            std::cout << "=======================================" << std::endl;
            std::cout << "Copia esto: (" << originalCoords.x << ", " << originalCoords.y << ")" << std::endl;
            
        }
    }


    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2i mousePixel(mousePressed->position.x, mousePressed->position.y);
            sf::Vector2f localMousePos = sf::Vector2f(
                static_cast<float>(mousePixel.x) - m_position.x,
                static_cast<float>(mousePixel.y) - m_position.y
            );
            
            switch (m_gameState) {
                case CriminalGameState::BUSCANDO_EVIDENCIAS:
                    for (auto& objeto : m_objetos) {
                        if (!objeto.encontrado && objeto.area.contains(localMousePos)) {
                            objeto.encontrado = true;
                            mostrarMensaje(objeto.nombre + "\n" + objeto.descripcion, 2.0f);
                            updateListaTexto();
                            
                            if (m_inventory) {
                                Item nuevoItem;
                                nuevoItem.name = objeto.nombre;
                                nuevoItem.color = sf::Color(255, 215, 0);
                                m_inventory->addItem(nuevoItem);
                            }
                            
                            verificarCompletado();
                            break;
                        }
                    }
                    break;
                    
                case CriminalGameState::ELECCION_FINAL:
                    for (size_t i = 0; i < m_areasBotones.size(); i++) {
                        if (m_areasBotones[i].contains(localMousePos)) {
                            procesarAcusacion(static_cast<int>(i));
                            break;
                        }
                    }
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (m_gameState == CriminalGameState::NARRATIVA) {
            if (keyPressed->code == sf::Keyboard::Key::Enter) {
                if (m_dialogoActualIndex < static_cast<int>(m_dialogosActuales.size()) - 1) {
                    m_dialogoActualIndex++;
                } else {
                    m_gameState = CriminalGameState::ELECCION_FINAL;
                    mostrarMensaje("Has escuchado todos los testimonios. Quien es el culpable?", 9.0f);
                }
            } else if (keyPressed->code == sf::Keyboard::Key::Backspace) {
                if (m_dialogoActualIndex > 0) {
                    m_dialogoActualIndex--;
                }
            }
        }
        
        if (keyPressed->code == sf::Keyboard::Key::Escape && !m_completed) {
            deactivate();
        }
    }
    
}

void CriminalCaseMinigame::update(float dt) {
    if (!m_active) return;
    
    if (m_mensajeTemp.tiempoRestante > 0.0f) {
        m_mensajeTemp.tiempoRestante -= dt;
        if (m_mensajeTemp.tiempoRestante <= 0.0f && m_mensajeText) {
            m_mensajeText->setString("");
        }
    }
}

void CriminalCaseMinigame::dibujarPantallaNarrativa(sf::RenderWindow& window) {
    window.draw(m_fondoNarrativo);
    
    if (m_dialogoActualIndex >= 0 && m_dialogoActualIndex < static_cast<int>(m_dialogosActuales.size())) {
        auto& dialogo = m_dialogosActuales[m_dialogoActualIndex];
        
        cargarFondoDialogo(dialogo);
        
        // 1. Dibujar el marco principal del cuadro de diálogo
        window.draw(m_cuadroDialogo);
        
        // 2. Dibujar la imagen del personaje (más grande, ocupa lado izquierdo)
        if (dialogo.fondoCargado && dialogo.fondoTexture) {
            sf::Sprite fondoPersonaje(*dialogo.fondoTexture);
            sf::Vector2f textureSize(static_cast<float>(dialogo.fondoTexture->getSize().x),
                                     static_cast<float>(dialogo.fondoTexture->getSize().y));
            
            // La imagen ocupará el 45% del ancho y casi toda la altura
            float anchoImagen = m_cuadroDialogo.getSize().x * 0.45f;
            float altoImagen = m_cuadroDialogo.getSize().y - 20;  // Margen pequeño
            
            float scaleX = anchoImagen / textureSize.x;
            float scaleY = altoImagen / textureSize.y;
            
            fondoPersonaje.setScale(sf::Vector2f(scaleX, scaleY));
            fondoPersonaje.setPosition(sf::Vector2f(
                m_cuadroDialogo.getPosition().x + 10,
                m_cuadroDialogo.getPosition().y + 10
            ));
            window.draw(fondoPersonaje);
        }
        
        // 3. Recuadro para el texto (lado derecho, separado)
        float anchoTexto = m_cuadroDialogo.getSize().x * 0.48f;
        float altoTexto = m_cuadroDialogo.getSize().y - 70;
        float inicioTextoX = m_cuadroDialogo.getPosition().x + m_cuadroDialogo.getSize().x - anchoTexto - 15;
        float inicioTextoY = m_cuadroDialogo.getPosition().y + 15;
        
        sf::RectangleShape recuadroTexto;
        recuadroTexto.setSize(sf::Vector2f(anchoTexto, altoTexto));
        recuadroTexto.setFillColor(sf::Color(0, 0, 0, 200));  // Negro semitransparente
        recuadroTexto.setOutlineColor(sf::Color(200, 180, 100));  // Borde dorado
        recuadroTexto.setOutlineThickness(2.f);
        recuadroTexto.setPosition(sf::Vector2f(inicioTextoX, inicioTextoY));
        window.draw(recuadroTexto);
        
        // 4. Nombre del personaje (dentro del recuadro, arriba)
        if (m_personaText) {
            m_personaText->setString(dialogo.persona);
            m_personaText->setCharacterSize(28);  // Más grande
            m_personaText->setFillColor(sf::Color(255, 215, 0));  // Dorado
            m_personaText->setStyle(sf::Text::Bold);
            
            sf::FloatRect bounds = m_personaText->getLocalBounds();
            m_personaText->setPosition(sf::Vector2f(
                inicioTextoX + (anchoTexto - bounds.size.x) / 2,  // Centrado
                inicioTextoY + 15
            ));
            window.draw(*m_personaText);
        }
        
        // 5. Línea separadora debajo del nombre
        sf::RectangleShape lineaSeparadora;
        lineaSeparadora.setSize(sf::Vector2f(anchoTexto - 40, 2.f));
        lineaSeparadora.setFillColor(sf::Color(200, 180, 100));
        lineaSeparadora.setPosition(sf::Vector2f(
            inicioTextoX + 20,
            inicioTextoY + 55
        ));
        window.draw(lineaSeparadora);
        
        // 6. Texto del diálogo
        if (m_dialogoText) {
            m_dialogoText->setString(dialogo.texto);
            m_dialogoText->setCharacterSize(18);
            m_dialogoText->setFillColor(sf::Color::White);
            m_dialogoText->setPosition(sf::Vector2f(
                inicioTextoX + 20,
                inicioTextoY + 70
            ));
            window.draw(*m_dialogoText);
        }
        
        // 7. Instrucciones (abajo a la derecha, fuera del recuadro)
        if (m_instruccionesNarrativas) {
            std::string instrucciones = (m_dialogoActualIndex == static_cast<int>(m_dialogosActuales.size()) - 1) 
                ? "ENTER: Acusar culpable | BACKSPACE: Anterior"
                : "ENTER: Siguiente dialogo | BACKSPACE: Anterior";
            m_instruccionesNarrativas->setString(instrucciones);
            m_instruccionesNarrativas->setCharacterSize(14);
            m_instruccionesNarrativas->setFillColor(sf::Color(180, 180, 180));
            
            sf::FloatRect bounds = m_instruccionesNarrativas->getLocalBounds();
            m_instruccionesNarrativas->setPosition(sf::Vector2f(
                m_cuadroDialogo.getPosition().x + m_cuadroDialogo.getSize().x - bounds.size.x - 20,
                m_cuadroDialogo.getPosition().y + m_cuadroDialogo.getSize().y - 35
            ));
            window.draw(*m_instruccionesNarrativas);
        }
    }
}

void CriminalCaseMinigame::dibujarPantallaEleccion(sf::RenderWindow& window) {
    if (!m_fontLoaded) return;
    
    sf::RectangleShape fondo(m_size);
    fondo.setFillColor(sf::Color(0, 0, 0, 220));
    fondo.setPosition(m_position);
    window.draw(fondo);
    
    if (m_tituloEleccion) {
        sf::FloatRect bounds = m_tituloEleccion->getLocalBounds();
        m_tituloEleccion->setPosition(sf::Vector2f(
            m_position.x + (m_size.x - bounds.size.x) / 2,
            m_position.y + 40
        ));
        window.draw(*m_tituloEleccion);
    }
    
    if (m_mensajeAdvertencia) {
        sf::FloatRect bounds = m_mensajeAdvertencia->getLocalBounds();
        m_mensajeAdvertencia->setPosition(sf::Vector2f(
            m_position.x + (m_size.x - bounds.size.x) / 2,
            m_position.y + 90
        ));
        window.draw(*m_mensajeAdvertencia);
    }
    
    m_areasBotones.clear();
    float botonAncho = 220.f;
    float botonAlto = 140.f;
    float espacioTotal = botonAncho * m_sospechosos.size();
    float inicioX = m_position.x + (m_size.x - espacioTotal) / 2;
    float y = m_position.y + m_size.y - botonAlto - 50;
    
    for (size_t i = 0; i < m_sospechosos.size(); i++) {
        float x = inicioX + i * botonAncho;
        m_areasBotones.push_back(sf::FloatRect(sf::Vector2f(x, y), sf::Vector2f(botonAncho, botonAlto)));
        
        sf::RectangleShape boton(sf::Vector2f(botonAncho, botonAlto));
        boton.setPosition(sf::Vector2f(x, y));
        boton.setFillColor(sf::Color(40, 40, 70, 230));
        boton.setOutlineColor(sf::Color::White);
        boton.setOutlineThickness(2.f);
        window.draw(boton);
        
        sf::Text nombreText(*m_font);
        nombreText.setString(m_sospechosos[i].nombre);
        nombreText.setCharacterSize(20);
        nombreText.setFillColor(sf::Color::White);
        nombreText.setOutlineColor(sf::Color::Black);
        nombreText.setOutlineThickness(1.0f);
        nombreText.setStyle(sf::Text::Bold);
        sf::FloatRect nameBounds = nombreText.getLocalBounds();
        nombreText.setPosition(sf::Vector2f(x + (botonAncho - nameBounds.size.x) / 2, y + 15));
        window.draw(nombreText);
        
        sf::Text descText(*m_font);
        std::string desc = m_sospechosos[i].descripcion;
        if (desc.length() > 30) desc = desc.substr(0, 27) + "...";
        descText.setString(desc);
        descText.setCharacterSize(13);
        descText.setFillColor(sf::Color(200, 200, 200));
        descText.setOutlineColor(sf::Color::Black);
        descText.setOutlineThickness(0.5f);
        sf::FloatRect descBounds = descText.getLocalBounds();
        descText.setPosition(sf::Vector2f(x + (botonAncho - descBounds.size.x) / 2, y + 50));
        window.draw(descText);
        
        sf::RectangleShape botonAcusar(sf::Vector2f(botonAncho - 40, 35));
        botonAcusar.setPosition(sf::Vector2f(x + 20, y + botonAlto - 45));
        botonAcusar.setFillColor(sf::Color(150, 40, 40));
        botonAcusar.setOutlineColor(sf::Color::Yellow);
        botonAcusar.setOutlineThickness(1.f);
        window.draw(botonAcusar);
        
        sf::Text acusarText(*m_font);
        acusarText.setString("ACUSAR");
        acusarText.setCharacterSize(16);
        acusarText.setFillColor(sf::Color::White);
        acusarText.setOutlineColor(sf::Color::Black);
        acusarText.setOutlineThickness(1.0f);
        acusarText.setStyle(sf::Text::Bold);
        sf::FloatRect acusarBounds = acusarText.getLocalBounds();
        acusarText.setPosition(sf::Vector2f(x + botonAncho / 2 - acusarBounds.size.x / 2, y + botonAlto - 38));
        window.draw(acusarText);
    }
}

void CriminalCaseMinigame::draw(sf::RenderWindow& window) {
    if (!m_active) return;
    
    if (m_background) {
        window.draw(*m_background);
    }
    
    if (m_debugMode && m_gameState == CriminalGameState::BUSCANDO_EVIDENCIAS) {
        for (const auto& objeto : m_objetos) {
            if (!objeto.encontrado) {
                sf::RectangleShape rect(sf::Vector2f(objeto.area.size.x, objeto.area.size.y));
                rect.setPosition(sf::Vector2f(m_position.x + objeto.area.position.x, 
                                               m_position.y + objeto.area.position.y));
                rect.setFillColor(sf::Color(255, 255, 0, 100));
                rect.setOutlineThickness(2.f);
                rect.setOutlineColor(sf::Color::Yellow);
                window.draw(rect);
            }
        }
    }
    
    switch (m_gameState) {
        case CriminalGameState::BUSCANDO_EVIDENCIAS:
            if (m_listaText) window.draw(*m_listaText);
            if (m_instruccionText) 
            {
                float escalaTexto = std::min(m_size.x, m_size.y) / 800.0f;
                int tamanoInstruccion = static_cast<int>(20 * escalaTexto);  
                if (tamanoInstruccion < 16) tamanoInstruccion = 16;
                if (tamanoInstruccion > 34) tamanoInstruccion = 34;
                m_instruccionText->setCharacterSize(tamanoInstruccion);
                m_instruccionText->setOutlineThickness(1.5f);
                
                // Posición en la parte inferior
                m_instruccionText->setPosition(sf::Vector2f(
                    m_position.x + 20.0f * escalaTexto, 
                    m_position.y + m_size.y - 45.0f * escalaTexto  
                ));
                window.draw(*m_instruccionText);
            }
            break;
            
        case CriminalGameState::NARRATIVA:
            dibujarPantallaNarrativa(window);
            break;
            
        case CriminalGameState::ELECCION_FINAL:
            dibujarPantallaEleccion(window);
            break;
    }
    
    if (m_mensajeTemp.tiempoRestante > 0 && m_mensajeText) {
        m_mensajeText->setString(m_mensajeTemp.texto);
        sf::Vector2u winSize = window.getSize();
        centrarTexto(*m_mensajeText, static_cast<float>(winSize.x) / 2.f, 
                     static_cast<float>(winSize.y) - 150.f);
        window.draw(*m_mensajeText);
    }
}

void CriminalCaseMinigame::resetGame() {
    // Resetear todos los estados
    m_active = false;
    m_completed = false;
    m_todasEvidencias = false;
    m_culpableEncontrado = false;
    m_gameState = CriminalGameState::BUSCANDO_EVIDENCIAS;
    m_dialogoActualIndex = 0;
    m_waitingForNarrative = false;
    m_mensajeTemp.tiempoRestante = 0.0f;
    
    // Regenerar el caso
    generarNuevoCaso();
    
    // Actualizar UI
    updateListaTexto();
}

void CriminalCaseMinigame::limpiarInventario() {
    if (m_inventory) {
        for (const auto& obj : m_objetos) {
            if (obj.encontrado) {
                for (int i = 0; i < 20; i++) {
                    Item* item = m_inventory->getItem(i);
                    if (item && item->name == obj.nombre) {
                        m_inventory->removeItem(i);
                        break;
                    }
                }
            }
        }
    }
}