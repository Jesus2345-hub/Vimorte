#include "CriminalCaseMinigame.hpp"
#include <memory>
#include <sstream>
#include <random>
#include <fstream>
#include <chrono>

// Constructor
CriminalCaseMinigame::CriminalCaseMinigame()
    : m_active(false), 
    m_todasEvidencias(false), 
    m_culpableEncontrado(false), 
    m_completed(false), m_debugMode(true), 
    m_fontLoaded(false), 
    m_waitingForNarrative(false), 
    m_gameState(CriminalGameState::BUSCANDO_EVIDENCIAS), 
    m_setActualObjetos(-1), 
    m_setActualSospechosos(-1), 
    m_setActualDialogos(-1), 
    m_dialogoActualIndex(0), 
    m_inventory(nullptr), 
    m_rng(std::chrono::steady_clock::now().time_since_epoch().count()), 
    m_backgroundTexture(nullptr), 
    m_background(nullptr), 
    m_font(nullptr), 
    m_mensajeText(nullptr), 
    m_listaText(nullptr), 
    m_instruccionText(nullptr), 
    m_dialogoText(nullptr), 
    m_personaText(nullptr), 
    m_instruccionesNarrativas(nullptr), 
    m_tituloEleccion(nullptr), 
    m_mensajeAdvertencia(nullptr),
    m_mensajeErrorActivo(false)
{
    m_mensajeTemp.tiempoRestante = 0.0f;
    m_position = sf::Vector2f(0, 0);
    m_size = sf::Vector2f(800, 600);
    m_tamanioBase = sf::Vector2f(800, 600);
    cargarFuente();
}

// Métodos para agregar sets
void CriminalCaseMinigame::agregarSetObjetos(const std::vector<ObjetoBuscar> &objetos)
{
    m_poolObjetos.push_back(objetos);
}

void CriminalCaseMinigame::agregarSetSospechosos(const std::vector<Sospechoso> &sospechosos)
{
    m_poolSospechosos.push_back(sospechosos);
}

void CriminalCaseMinigame::agregarSetDialogos(const std::vector<DialogoNarrativo> &dialogos)
{
    m_poolDialogos.push_back(dialogos);
}

void CriminalCaseMinigame::limpiarPools()
{
    m_poolObjetos.clear();
    m_poolSospechosos.clear();
    m_poolDialogos.clear();
}

void CriminalCaseMinigame::limpiarInventarioCaso()
{
    if (!m_inventory) return;
    
    std::cout << "=== LIMPIANDO INVENTARIO DEL CASO ACTUAL ===" << std::endl;
    
    for (const auto &obj : m_objetos)
    {
        for (int i = 0; i < 20; i++)
        {
            Item *item = m_inventory->getItem(i);
            if (item && item->name == obj.nombre)
            {
                m_inventory->removeItem(i);
                std::cout << "Eliminado: " << obj.nombre << std::endl;
                break;
            }
        }
    }
}

void CriminalCaseMinigame::mostrarMensajeConFondo(const std::string& msg, float duracion, sf::Color color)
{
    m_mensajeTemp.texto = msg;
    m_mensajeTemp.tiempoRestante = duracion;
    m_mensajeTemp.color = color;  
    m_mensajeErrorActivo = true;
    if (m_mensajeText)
    {
        m_mensajeText->setString(msg);
        m_mensajeText->setFillColor(color);
    }
    std::cout << "MENSAJE ERROR (CON FONDO): " << msg << std::endl; // Debug
}

void CriminalCaseMinigame::recalcularAreasBotones()
{
    if (m_sospechosos.empty()) return;
    
    m_areasBotones.clear();
    
    float proporcionAncho = 0.55f;  
    float proporcionAlto = 0.50f;   
    
    float botonAncho = m_size.x * proporcionAncho;
    float botonAlto = m_size.y * proporcionAlto;
    
    // Límites mínimos y máximos basados en porcentaje, no en píxeles fijos
    float minAncho = m_size.x * 0.40f;
    float maxAncho = m_size.y * 0.70f;
    float minAlto = m_size.y * 0.35f;
    float maxAlto = m_size.y * 0.55f;
    
    if (botonAncho < minAncho) botonAncho = minAncho;
    if (botonAncho > maxAncho) botonAncho = maxAncho;
    if (botonAlto < minAlto) botonAlto = minAlto;
    if (botonAlto > maxAlto) botonAlto = maxAlto;
    
    // Calcular distribución horizontal
    float espacioTotal = botonAncho * m_sospechosos.size();
    float espacioEntre = (m_size.x - espacioTotal) / (m_sospechosos.size() + 1);
    
    // Espacio mínimo relativo
    float minEspacio = m_size.x * 0.02f;
    if (espacioEntre < minEspacio) {
        espacioEntre = minEspacio;
        // Recalcular ancho si es necesario
        botonAncho = (m_size.x - espacioEntre * (m_sospechosos.size() + 1)) / m_sospechosos.size();
    }
    
    float inicioX = m_position.x + espacioEntre;
    float y = m_position.y + (m_size.y * 0.35f);  // 35% desde arriba (relativo)
    
    for (size_t i = 0; i < m_sospechosos.size(); i++) {
        float x = inicioX + i * (botonAncho + espacioEntre);
        m_areasBotones.push_back(sf::FloatRect(sf::Vector2f(x, y), sf::Vector2f(botonAncho, botonAlto)));
    }
}

void CriminalCaseMinigame::cargarFuente()
{
    if (!m_fontLoaded)
    {
        m_font = std::make_unique<sf::Font>();
        if (!m_font->openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf"))
        {
            std::cout << "Error cargando fuente" << std::endl;
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
        m_mensajeAdvertencia->setCharacterSize(22);
        m_mensajeAdvertencia->setFillColor(sf::Color(255, 200, 100));
        m_mensajeAdvertencia->setOutlineColor(sf::Color::Black);
        m_mensajeAdvertencia->setOutlineThickness(1.0f);
        m_mensajeAdvertencia->setString("Elige sabiamente. Si encarcelas al inocente, el verdadero culpable escapara con tus pruebas");
    }
}

void CriminalCaseMinigame::setBaseSize(const sf::Vector2f &baseSize)
{
    m_tamanioBase = baseSize;
}

void CriminalCaseMinigame::escalarAreas()
{
    if (m_tamanioBase.x == 0 || m_tamanioBase.y == 0) return;

    float escalaX = m_size.x / m_tamanioBase.x;
    float escalaY = m_size.y / m_tamanioBase.y;

    m_objetos.clear();
    for (size_t i = 0; i < m_objetosOriginales.size(); ++i)
    {
        ObjetoBuscar objEscalado;
        objEscalado.nombre = m_objetosOriginales[i].nombre;
        objEscalado.descripcion = m_objetosOriginales[i].descripcion;
        objEscalado.encontrado = m_objetosOriginales[i].encontrado;
        objEscalado.rutaImagen = m_objetosOriginales[i].rutaImagen;
        objEscalado.area = sf::FloatRect(
            sf::Vector2f(
                m_objetosOriginales[i].area.position.x * escalaX,
                m_objetosOriginales[i].area.position.y * escalaY
            ),
            sf::Vector2f(
                m_objetosOriginales[i].area.size.x * escalaX,
                m_objetosOriginales[i].area.size.y * escalaY
            )
        );
        m_objetos.push_back(objEscalado);
    }

    m_sospechosos.clear();
    for (size_t i = 0; i < m_sospechososOriginales.size(); ++i)
    {
        Sospechoso sosEscalado;
        sosEscalado.nombre = m_sospechososOriginales[i].nombre;
        sosEscalado.descripcion = m_sospechososOriginales[i].descripcion;
        sosEscalado.acusado = m_sospechososOriginales[i].acusado;
        sosEscalado.esElCulpable = m_sospechososOriginales[i].esElCulpable;  // ← CRÍTICO
        sosEscalado.area = sf::FloatRect(
            sf::Vector2f(
                m_sospechososOriginales[i].area.position.x * escalaX,
                m_sospechososOriginales[i].area.position.y * escalaY
            ),
            sf::Vector2f(
                m_sospechososOriginales[i].area.size.x * escalaX,
                m_sospechososOriginales[i].area.size.y * escalaY
            )
        );
        m_sospechosos.push_back(sosEscalado);
    }

    updateListaTexto();
}

void CriminalCaseMinigame::generarNuevoCasoCompleto()
{
    if (m_poolObjetos.empty() || m_poolSospechosos.empty())
    {
        std::cout<< "Error: No hay suficientes datos en los pools" << std::endl;
        return;
    }

    std::uniform_int_distribution<int> distSets(0, static_cast<int>(m_poolObjetos.size()) - 1);
    int nuevoSet = distSets(m_rng);
    
    m_setActualObjetos = nuevoSet;
    m_setActualSospechosos = nuevoSet;
    m_setActualDialogos = nuevoSet;

    // Cargar COPIA FRESCA de los datos originales
    m_objetosOriginales = m_poolObjetos[m_setActualObjetos];
    m_sospechososOriginales = m_poolSospechosos[m_setActualSospechosos];
    
    // Resetear estados encontrados/acusados
    for (auto &obj : m_objetosOriginales) { obj.encontrado = false; }
    for (auto &sos : m_sospechososOriginales) { sos.acusado = false; }

    // Sincronizar vectores actuales
    m_objetos = m_objetosOriginales;
    m_sospechosos = m_sospechososOriginales;

    // Cargar diálogos
    if (m_setActualDialogos < static_cast<int>(m_poolDialogos.size()) &&
        !m_poolDialogos[m_setActualDialogos].empty())
    {
        m_dialogosActuales = m_poolDialogos[m_setActualDialogos];
        std::cout << "Dialogos cargados: " << m_dialogosActuales.size() << std::endl;
    }
    else
    {
        m_dialogosActuales.clear();
        m_dialogosActuales.emplace_back("Testigo", "Las pistas no mienten. Escucha con atencion.");
        m_dialogosActuales.emplace_back("Alguien mas", "Todos tenemos algo que decir.");
        std::cout << "Dialogos por defecto cargados" << std::endl; // DEBUG
    }

    // Resetear estados del juego
    m_gameState = CriminalGameState::BUSCANDO_EVIDENCIAS;
    m_todasEvidencias = false;
    m_waitingForNarrative = false;
    m_dialogoActualIndex = 0;
    m_completed = false;
        
    // Re-escalar áreas y actualizar UI
    escalarAreas();
    updateListaTexto();
}

void CriminalCaseMinigame::actualizarFondo()
{
    if (m_fondoPath.empty())
        return;

    if (!m_backgroundTexture)
    {
        m_backgroundTexture = std::make_unique<sf::Texture>();
    }

    if (!m_backgroundTexture->loadFromFile(m_fondoPath))
    {
        std::cout<< "Error cargando fondo: " << m_fondoPath << std::endl;
        return;
    }

    m_background = std::make_unique<sf::Sprite>(*m_backgroundTexture);

    // Asegurar que m_size tiene valores válidos
    if (m_size.x <= 0 || m_size.y <= 0)
    {
        std::cout<< "WARNING: m_size invalido en actualizarFondo: " << m_size.x << "x" << m_size.y << std::endl;
        return;
    }

    sf::Vector2u textureSize = m_backgroundTexture->getSize();
    if (textureSize.x > 0 && textureSize.y > 0)
    {
        float escalaX = m_size.x / static_cast<float>(textureSize.x);
        float escalaY = m_size.y / static_cast<float>(textureSize.y);
        m_background->setScale(sf::Vector2f(escalaX, escalaY));
    }
    m_background->setPosition(m_position);
}

void CriminalCaseMinigame::cargarFondoOnly(const std::string &fondoPath)
{
    m_fondoPath = fondoPath;
    actualizarFondo();
}

void CriminalCaseMinigame::setPosition(const sf::Vector2f &pos)
{
    m_position = pos;
    if (m_background)
    {
        m_background->setPosition(pos);
    }
}

void CriminalCaseMinigame::setSize(const sf::Vector2f &size)
{
    if (size.x <= 0 || size.y <= 0)
        return;
    
    m_size = size;
    
    actualizarFondo();
    
    // Siempre reescalar áreas
    if (m_tamanioBase.x > 0 && m_tamanioBase.y > 0) {
        escalarAreas();
    }
    
    if (m_gameState == CriminalGameState::ELECCION_FINAL) {
        recalcularAreasBotones();
    }
    
    // Si estamos en modo narrativa, reescalar elementos visuales
    if (m_gameState == CriminalGameState::NARRATIVA) {
        float escalaRef = std::min(m_size.x, m_size.y) / 800.0f;
        
        m_fondoNarrativo.setSize(m_size);
        m_fondoNarrativo.setPosition(m_position);
        
        float cuadroAncho = m_size.x * 0.95f;
        float cuadroAlto = m_size.y * 0.60f;
        
        m_cuadroDialogo.setSize(sf::Vector2f(cuadroAncho, cuadroAlto));
        m_cuadroDialogo.setPosition(sf::Vector2f(
            m_position.x + (m_size.x - cuadroAncho) / 2,
            m_position.y + (m_size.y - cuadroAlto) / 2));
        
        // Actualizar tamaños de texto en narrativa
        if (m_personaText) {
            int tamano = static_cast<int>(28 * escalaRef);
            if (tamano < 18) tamano = 18;
            if (tamano > 42) tamano = 42;
            m_personaText->setCharacterSize(tamano);
        }
        
        if (m_dialogoText) {
            int tamano = static_cast<int>(18 * escalaRef);
            if (tamano < 14) tamano = 14;
            if (tamano > 32) tamano = 32;
            m_dialogoText->setCharacterSize(tamano);
        }
    }
}

void CriminalCaseMinigame::init(const std::string &fondoPath,
                                std::vector<ObjetoBuscar> objetos,
                                std::vector<Sospechoso> sospechosos)
{
    m_fondoPath = fondoPath;
    m_objetosOriginales = objetos;
    m_sospechososOriginales = sospechosos;

    actualizarFondo();

    m_objetos = m_objetosOriginales;
    m_sospechosos = m_sospechososOriginales;

    escalarAreas();
}

void CriminalCaseMinigame::reinit(const std::string &fondoPath,
                                  const std::vector<ObjetoBuscar> &objetos,
                                  const std::vector<Sospechoso> &sospechosos)
{
    m_fondoPath = fondoPath;
    m_objetosOriginales = objetos;
    m_sospechososOriginales = sospechosos;

    actualizarFondo();

    m_objetos = m_objetosOriginales;
    m_sospechosos = m_sospechososOriginales;

    escalarAreas();
}

void CriminalCaseMinigame::generarNuevoCaso()
{
    if (m_poolObjetos.empty() || m_poolSospechosos.empty())
    {
        std::cout<< "Error: No hay suficientes datos en los pools" << std::endl;
        return;
    }

    std::uniform_int_distribution<int> distSets(0, static_cast<int>(m_poolObjetos.size()) - 1);
    int mismoSet = distSets(m_rng);
    
    m_setActualObjetos = mismoSet;
    m_setActualSospechosos = mismoSet;
    m_setActualDialogos = mismoSet;

    //  COPIAR DATOS FRESCOS 
    m_objetosOriginales.clear();
    m_sospechososOriginales.clear();
    
    m_objetosOriginales = m_poolObjetos[m_setActualObjetos];
    m_sospechososOriginales = m_poolSospechosos[m_setActualSospechosos];

    // Resetear estados 
    for (auto &obj : m_objetosOriginales)
    {
        obj.encontrado = false;
    }
    for (auto &sos : m_sospechososOriginales)
    {
        sos.acusado = false;
    }

    // Cargar diálogos
    if (m_setActualDialogos < static_cast<int>(m_poolDialogos.size()) && !m_poolDialogos[m_setActualDialogos].empty())
    {
        m_dialogosActuales = m_poolDialogos[m_setActualDialogos];
    }
    else
    {
        m_dialogosActuales.clear();
        m_dialogosActuales.emplace_back("Testigo", "He visto lo sucedido esa noche. Las pistas no mienten.");
        m_dialogosActuales.emplace_back("Alguien mas", "Todos tenemos algo que decir. Escucha con atención.");
    }

    escalarAreas();

    // Resetear estado del juego
    m_todasEvidencias = false;
    m_culpableEncontrado = false;
    m_completed = false;
    m_gameState = CriminalGameState::BUSCANDO_EVIDENCIAS;
    m_dialogoActualIndex = 0;
    m_waitingForNarrative = false;
    m_mensajeTemp.tiempoRestante = 0.0f;
    m_mensajeTemp.texto = "";

    // DEBUG: Verificar el culpable
    std::cout << "=== NUEVO CASO GENERADO ===" << std::endl;
    for (const auto &s : m_sospechosos)
    {
        std::cout << "Sospechoso: " << s.nombre << " - Culpable: " << (s.esElCulpable ? "SI" : "NO") << std::endl;
    }
}
void CriminalCaseMinigame::activate()
{
    if (m_completed)
    {
        resetCompletamente();
    }

    // RESET COMPLETO DEL ESTADO
    m_active = false;
    m_completed = false;
    m_todasEvidencias = false;
    m_culpableEncontrado = false;
    m_gameState = CriminalGameState::BUSCANDO_EVIDENCIAS;
    m_dialogoActualIndex = 0;
    m_waitingForNarrative = false;
    m_mensajeTemp.tiempoRestante = 0.0f;
    m_mensajeTemp.texto = "";

    // Regenerar el caso para asegurar diálogos frescos
    if (!m_poolObjetos.empty())
    {
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
void CriminalCaseMinigame::resetCompletamente()
{
    // Resetear todos los estados
    m_active = false;
    m_completed = false;
    m_todasEvidencias = false;
    m_culpableEncontrado = false;
    m_gameState = CriminalGameState::BUSCANDO_EVIDENCIAS;
    m_dialogoActualIndex = 0;
    m_waitingForNarrative = false;
    m_mensajeTemp.tiempoRestante = 0.0f;
    m_mensajeErrorActivo = false;

    // Recargar desde los pools usando los mismos sets
    if (m_setActualObjetos >= 0 && m_setActualObjetos < static_cast<int>(m_poolObjetos.size()))
    {
        m_objetosOriginales = m_poolObjetos[m_setActualObjetos];
        for (auto &obj : m_objetosOriginales)
        {
            obj.encontrado = false;
        }
    }

    if (m_setActualSospechosos >= 0 && m_setActualSospechosos < static_cast<int>(m_poolSospechosos.size()))
    {
        m_sospechososOriginales = m_poolSospechosos[m_setActualSospechosos];
        for (auto &sos : m_sospechososOriginales)
        {
            sos.acusado = false;
        }
    }

    if (m_setActualDialogos >= 0 && m_setActualDialogos < static_cast<int>(m_poolDialogos.size()))
    {
        m_dialogosActuales = m_poolDialogos[m_setActualDialogos];
    }

    m_objetos = m_objetosOriginales;
    m_sospechosos = m_sospechososOriginales;

    escalarAreas();
    updateListaTexto();
}

void CriminalCaseMinigame::deactivate()
{
    if (!m_completed && m_active)
    {
        limpiarInventarioCaso();
    }
    
    m_active = false;
    m_mensajeTemp.tiempoRestante = 0.0f;
}

void CriminalCaseMinigame::iniciarFaseNarrativa()
{
    if (m_dialogosActuales.empty())
    {
        std::cout << "ADVERTENCIA: No hay dialogos cargados, pasando directamente a eleccion" << std::endl;
        m_gameState = CriminalGameState::ELECCION_FINAL;
        mostrarMensaje("Sin testimonios disponibles. Decide quien es el culpable.", 2.0f);
        return;
    }

    m_gameState = CriminalGameState::NARRATIVA;
    m_dialogoActualIndex = 0;

    mostrarMensaje("Escucha los testimonios de los Sospechosos", 3.0f);

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
        m_position.y + (m_size.y - cuadroAlto) / 2));

    std::cout << "Fase narrativa iniciada - Dialogos disponibles: " << m_dialogosActuales.size() << std::endl;
}


void CriminalCaseMinigame::verificarCompletado()
{
    bool todosEncontrados = true;
    for (const auto &obj : m_objetos)
    {
        if (!obj.encontrado)
        {
            todosEncontrados = false;
            break;
        }
    }

    if (todosEncontrados && m_gameState == CriminalGameState::BUSCANDO_EVIDENCIAS)
    {
        m_todasEvidencias = true;
        iniciarFaseNarrativa();
    }
}
void CriminalCaseMinigame::reiniciarCasoCompleto()
{
    // Resetear objetos encontrados
    for (auto &obj : m_objetosOriginales)
    {
        obj.encontrado = false;
    }
    for (auto &obj : m_objetos)
    {
        obj.encontrado = false;
    }

    // Resetear acusaciones de sospechosos
    for (auto &sos : m_sospechososOriginales)
    {
        sos.acusado = false;
    }
    for (auto &sos : m_sospechosos)
    {
        sos.acusado = false;
    }

    // Recargar desde los pools originales
    if (m_setActualObjetos >= 0 && m_setActualObjetos < static_cast<int>(m_poolObjetos.size()))
    {
        m_objetosOriginales = m_poolObjetos[m_setActualObjetos];
        for (auto &obj : m_objetosOriginales)
        {
            obj.encontrado = false;
        }
    }

    if (m_setActualSospechosos >= 0 && m_setActualSospechosos < static_cast<int>(m_poolSospechosos.size()))
    {
        m_sospechososOriginales = m_poolSospechosos[m_setActualSospechosos];
        for (auto &sos : m_sospechososOriginales)
        {
            sos.acusado = false;
            std::cout << "Sospechoso: " << sos.nombre << " - Culpable: " << (sos.esElCulpable ? "SI" : "NO") << std::endl;
        }
    }

    // Recargar diálogos
    if (m_setActualDialogos >= 0 && m_setActualDialogos < static_cast<int>(m_poolDialogos.size()))
    {
        m_dialogosActuales = m_poolDialogos[m_setActualDialogos];
    }

    // Sincronizar vectores actuales
    m_objetos = m_objetosOriginales;
    m_sospechosos = m_sospechososOriginales;
    m_mensajeErrorActivo = false;

    // Re-escalar áreas
    escalarAreas();

    //  Actualizar la UI 
    updateListaTexto();
}
void CriminalCaseMinigame::procesarAcusacion(int sospechosoIndex)
{
    if (sospechosoIndex < 0 || sospechosoIndex >= static_cast<int>(m_sospechosos.size()))
        return;

    const Sospechoso& sospechosoAcusado = m_sospechosos[sospechosoIndex];
    
    std::string nombreCulpable = "";
    for (const auto& s : m_sospechososOriginales) {
        if (s.esElCulpable) {
            nombreCulpable = s.nombre;
            break;
        }
    }

    if (sospechosoAcusado.nombre == nombreCulpable)
    {
        std::cout << "CORRECTO! " << sospechosoAcusado.nombre << " es el culpable." << std::endl;
        m_completed = true;
        m_active = false;
        if (m_onCompleteCallback) m_onCompleteCallback(true);
    }
    else
    {
        std::cout << "INCORRECTO! " << sospechosoAcusado.nombre << " NO es el culpable." << std::endl;
        
        // GUARDAR el mensaje
        std::string mensajeGuardado = "INCORRECTO! " + sospechosoAcusado.nombre + " es inocente.\n";
        mensajeGuardado += "El verdadero culpable ha escapado con las pruebas...\n";
        mensajeGuardado += "Ahora debes investigar un NUEVO caso.";
        
        limpiarInventarioCaso();

        generarNuevoCasoCompleto();
        
        m_mensajeTemp.texto = mensajeGuardado;
        m_mensajeTemp.tiempoRestante = 5.0f;
        m_mensajeTemp.color = sf::Color::Red;
        m_mensajeErrorActivo = true;
        
        if (m_mensajeText)
        {
            m_mensajeText->setString(mensajeGuardado);
            m_mensajeText->setFillColor(sf::Color::Red);
        }
        
        m_active = true;
        m_gameState = CriminalGameState::BUSCANDO_EVIDENCIAS;

        if (m_onCompleteCallback)
        {
            m_onCompleteCallback(false);
        }
    }
}

int CriminalCaseMinigame::contarObjetosEncontrados() const
{
    int count = 0;
    for (const auto &obj : m_objetos)
    {
        if (obj.encontrado)
            count++;
    }
    return count;
}

void CriminalCaseMinigame::updateListaTexto()
{
    if (!m_listaText || !m_fontLoaded)
        return;

    std::stringstream ss;
    ss << "PISTAS POR ENCONTRAR:\n";
    for (const auto &obj : m_objetos)
    {
        ss << (obj.encontrado ? "o " : "x ");
        ss << obj.nombre << "\n";
    }

    ss << "\nEVIDENCIAS: " << contarObjetosEncontrados() << "/" << m_objetos.size();
    m_listaText->setString(ss.str());

    // TAMAÑO MÁS GRANDE: de 14-28 a 20-36
    float escalaTexto = std::min(m_size.x, m_size.y) / 800.0f;
    int nuevoTamano = static_cast<int>(20 * escalaTexto); 
    if (nuevoTamano < 16)
        nuevoTamano = 16; 
    if (nuevoTamano > 36)
        nuevoTamano = 36; 

    m_listaText->setCharacterSize(nuevoTamano);
    m_listaText->setOutlineThickness(1.5f); 

    // Posición esquina superior izquierda
    float offsetX = 20.0f * escalaTexto;
    float offsetY = 20.0f * escalaTexto;
    m_listaText->setPosition(sf::Vector2f(m_position.x + offsetX, m_position.y + offsetY));
}

void CriminalCaseMinigame::mostrarMensaje(const std::string &msg, float duracion)
{
    m_mensajeTemp.texto = msg;
    m_mensajeTemp.tiempoRestante = duracion;
    m_mensajeErrorActivo = false;
    if (m_mensajeText)
    {
        m_mensajeText->setString(msg);
        m_mensajeText->setFillColor(sf::Color::Yellow);
    }
    std::cout << "MENSAJE NORMAL: " << msg << std::endl; // Debug
}

void CriminalCaseMinigame::centrarTexto(sf::Text &text, float x, float y)
{
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    text.setPosition(sf::Vector2f(x, y));
}

void CriminalCaseMinigame::cargarFondoDialogo(DialogoNarrativo &dialogo)
{
    // Solo cargar si hay una ruta de fondo y no está cargado
    if (!dialogo.fondoPath.empty() && !dialogo.fondoCargado)
    {
        dialogo.fondoTexture = std::make_shared<sf::Texture>();

        if (dialogo.fondoTexture->loadFromFile(dialogo.fondoPath))
        {
            dialogo.fondoCargado = true;
            std::cout << "Fondo cargado: " << dialogo.fondoPath << std::endl;
        }
        else
        {
            std::cout<< "Error cargando fondo: " << dialogo.fondoPath << std::endl;
            dialogo.fondoCargado = false;
        }
    }
}

void CriminalCaseMinigame::handleEvent(const sf::Event &event, sf::RenderWindow &window)
{
    if (!m_active || m_completed)
        return;

    
    if (const auto *mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mousePressed->button == sf::Mouse::Button::Left)
        {
            sf::Vector2i mousePixel(mousePressed->position.x, mousePressed->position.y);

            // Coordenadas locales (dentro del minijuego)
            sf::Vector2f localMousePos = sf::Vector2f(
                static_cast<float>(mousePixel.x) - m_position.x,
                static_cast<float>(mousePixel.y) - m_position.y);

            // Coordenadas en el sistema ORIGINAL (antes del escalado) - solo para debug
            sf::Vector2f originalCoords;
            if (m_tamanioBase.x > 0 && m_tamanioBase.y > 0)
            {
                originalCoords.x = localMousePos.x * (m_tamanioBase.x / m_size.x);
                originalCoords.y = localMousePos.y * (m_tamanioBase.y / m_size.y);
            }
            else
            {
                originalCoords = localMousePos;
            }

            //  LÓGICA PRINCIPAL SEGÚN EL ESTADO 
            switch (m_gameState)
            {
            case CriminalGameState::BUSCANDO_EVIDENCIAS:
                for (auto &objeto : m_objetos)
                {
                    if (!objeto.encontrado && objeto.area.contains(localMousePos))
                    {
                        objeto.encontrado = true;
                        mostrarMensaje(objeto.nombre + "\n" + objeto.descripcion, 2.0f);
                        updateListaTexto();

                       if (m_inventory) 
                    {
                        std::cout << " RECOLECTANDO OBJETO " << std::endl;
                        std::cout << "Nombre: " << objeto.nombre << std::endl;
                        std::cout << "Ruta de imagen: " << objeto.rutaImagen << std::endl;
                        
                        Item nuevoItem;
                        nuevoItem.name = objeto.nombre;
                        nuevoItem.color = sf::Color(255, 215, 0);
                        nuevoItem.rutaImagen = objeto.rutaImagen;
                        
                        if (!objeto.rutaImagen.empty()) {
                            std::cout << "Intentando cargar: " << objeto.rutaImagen << std::endl;
                            nuevoItem.textura = std::make_shared<sf::Texture>();
                            if (nuevoItem.textura->loadFromFile(objeto.rutaImagen)) {
                                nuevoItem.sprite = std::make_unique<sf::Sprite>(*nuevoItem.textura);
                                std::cout << "IMAGEN CARGADA" << std::endl;
                            } else {
                                std::cout << "ERROR: No se pudo cargar " << objeto.rutaImagen << std::endl;
                                nuevoItem.textura = nullptr;
                            }
                        } else {
                            std::cout << "Ruta de imagen VACIA" << std::endl;
                        }
                        
                        m_inventory->addItem(nuevoItem);
                    }

                        verificarCompletado();
                        break;
                    }
                }
                break;

            case CriminalGameState::ELECCION_FINAL:
                recalcularAreasBotones();
                
                for (size_t i = 0; i < m_areasBotones.size() && i < m_sospechosos.size(); i++) 
                {
                    if (m_areasBotones[i].contains(localMousePos)) 
                    {
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

    if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (m_gameState == CriminalGameState::NARRATIVA)
        {
            if (keyPressed->code == sf::Keyboard::Key::Enter)
            {
                if (m_dialogoActualIndex < static_cast<int>(m_dialogosActuales.size()) - 1)
                {
                    m_dialogoActualIndex++;
                }
                else
                {
                    m_gameState = CriminalGameState::ELECCION_FINAL;
                    mostrarMensaje("Has escuchado todos los testimonios. Quien es el culpable?", 9.0f);
                    recalcularAreasBotones();
                }
            }
            else if (keyPressed->code == sf::Keyboard::Key::Backspace)
            {
                if (m_dialogoActualIndex > 0)
                {
                    m_dialogoActualIndex--;
                }
            }
        }

        if (keyPressed->code == sf::Keyboard::Key::Escape && !m_completed)
        {
            deactivate();
        }
    }
}

void CriminalCaseMinigame::update(float dt)
{
    if (!m_active)
        return;

    if (m_mensajeTemp.tiempoRestante > 0.0f)
    {
        m_mensajeTemp.tiempoRestante -= dt;
        if (m_mensajeTemp.tiempoRestante <= 0.0f && m_mensajeText)
        {
            m_mensajeText->setString("");
            m_mensajeErrorActivo = false;
        }
    }
}

void CriminalCaseMinigame::dibujarPantallaNarrativa(sf::RenderWindow &window)
{
    float cuadroAncho = m_size.x * 0.95f;
    float cuadroAlto = m_size.y * 0.60f;
    
    m_cuadroDialogo.setSize(sf::Vector2f(cuadroAncho, cuadroAlto));
    m_cuadroDialogo.setPosition(sf::Vector2f(
        m_position.x + (m_size.x - cuadroAncho) / 2,
        m_position.y + (m_size.y - cuadroAlto) / 2));
    
    m_fondoNarrativo.setSize(m_size);
    m_fondoNarrativo.setPosition(m_position);
    
    window.draw(m_fondoNarrativo);

    if (m_dialogoActualIndex >= 0 && m_dialogoActualIndex < static_cast<int>(m_dialogosActuales.size()))
    {
        auto &dialogo = m_dialogosActuales[m_dialogoActualIndex];

        cargarFondoDialogo(dialogo);

        window.draw(m_cuadroDialogo);

        if (dialogo.fondoCargado && dialogo.fondoTexture)
        {
            sf::Sprite fondoPersonaje(*dialogo.fondoTexture);
            sf::Vector2f textureSize(static_cast<float>(dialogo.fondoTexture->getSize().x),
                                     static_cast<float>(dialogo.fondoTexture->getSize().y));

            float anchoImagen = m_cuadroDialogo.getSize().x * 0.45f;
            float altoImagen = m_cuadroDialogo.getSize().y - 20; 

            float scaleX = anchoImagen / textureSize.x;
            float scaleY = altoImagen / textureSize.y;

            fondoPersonaje.setScale(sf::Vector2f(scaleX, scaleY));
            fondoPersonaje.setPosition(sf::Vector2f(
                m_cuadroDialogo.getPosition().x + 10,
                m_cuadroDialogo.getPosition().y + 10));
            window.draw(fondoPersonaje);
        }

        float anchoTexto = m_cuadroDialogo.getSize().x * 0.48f;
        float altoTexto = m_cuadroDialogo.getSize().y - 70;
        float inicioTextoX = m_cuadroDialogo.getPosition().x + m_cuadroDialogo.getSize().x - anchoTexto - 15;
        float inicioTextoY = m_cuadroDialogo.getPosition().y + 15;

        sf::RectangleShape recuadroTexto;
        recuadroTexto.setSize(sf::Vector2f(anchoTexto, altoTexto));
        recuadroTexto.setFillColor(sf::Color(0, 0, 0, 200));     
        recuadroTexto.setOutlineColor(sf::Color(200, 180, 100)); 
        recuadroTexto.setOutlineThickness(2.f);
        recuadroTexto.setPosition(sf::Vector2f(inicioTextoX, inicioTextoY));
        window.draw(recuadroTexto);

        if (m_personaText)
        {
            m_personaText->setString(dialogo.persona);
            
            // Tamaño de fuente proporcional al tamaño de la ventana
            float escalaTexto = std::min(m_size.x, m_size.y) / 800.0f;
            int tamanoNombre = static_cast<int>(28 * escalaTexto);
            if (tamanoNombre < 18) tamanoNombre = 18;
            if (tamanoNombre > 42) tamanoNombre = 42;
            m_personaText->setCharacterSize(tamanoNombre);
            m_personaText->setFillColor(sf::Color(255, 215, 0)); 
            m_personaText->setStyle(sf::Text::Bold);

            sf::FloatRect bounds = m_personaText->getLocalBounds();
            m_personaText->setPosition(sf::Vector2f(
                inicioTextoX + (anchoTexto - bounds.size.x) / 2, 
                inicioTextoY + 15));
            window.draw(*m_personaText);
        }

        // 5. Línea separadora debajo del nombre
        sf::RectangleShape lineaSeparadora;
        lineaSeparadora.setSize(sf::Vector2f(anchoTexto - 40, 2.f));
        lineaSeparadora.setFillColor(sf::Color(200, 180, 100));
        lineaSeparadora.setPosition(sf::Vector2f(
            inicioTextoX + 20,
            inicioTextoY + 55));
        window.draw(lineaSeparadora);

        // 6. Texto del diálogo
        if (m_dialogoText)
        {
            m_dialogoText->setString(dialogo.texto);
            
            // Tamaño de fuente proporcional al tamaño de la ventana
            float escalaTexto = std::min(m_size.x, m_size.y) / 800.0f;
            int tamanoDialogo = static_cast<int>(18 * escalaTexto);
            if (tamanoDialogo < 14) tamanoDialogo = 14;
            if (tamanoDialogo > 32) tamanoDialogo = 32;
            m_dialogoText->setCharacterSize(tamanoDialogo);
            m_dialogoText->setFillColor(sf::Color::White);
            m_dialogoText->setPosition(sf::Vector2f(
                inicioTextoX + 20,
                inicioTextoY + 70));
            window.draw(*m_dialogoText);
        }

        // 7. Instrucciones (abajo a la derecha, fuera del recuadro)
        if (m_instruccionesNarrativas)
        {
            std::string instrucciones = (m_dialogoActualIndex == static_cast<int>(m_dialogosActuales.size()) - 1)
                                            ? "ENTER: Acusar culpable | BACKSPACE: Anterior"
                                            : "ENTER: Siguiente dialogo | BACKSPACE: Anterior";
            m_instruccionesNarrativas->setString(instrucciones);
            
            // Tamaño de fuente proporcional
            float escalaTexto = std::min(m_size.x, m_size.y) / 800.0f;
            int tamanoInstruccion = static_cast<int>(14 * escalaTexto);
            if (tamanoInstruccion < 12) tamanoInstruccion = 12;
            if (tamanoInstruccion > 24) tamanoInstruccion = 24;
            m_instruccionesNarrativas->setCharacterSize(tamanoInstruccion);
            m_instruccionesNarrativas->setFillColor(sf::Color(180, 180, 180));

            sf::FloatRect bounds = m_instruccionesNarrativas->getLocalBounds();
            m_instruccionesNarrativas->setPosition(sf::Vector2f(
                m_cuadroDialogo.getPosition().x + m_cuadroDialogo.getSize().x - bounds.size.x - 20,
                m_cuadroDialogo.getPosition().y + m_cuadroDialogo.getSize().y - 35));
            window.draw(*m_instruccionesNarrativas);
        }
    }
}

void CriminalCaseMinigame::dibujarPantallaEleccion(sf::RenderWindow &window)
{
    if (!m_fontLoaded)
        return;

    // Recalcular áreas de botones
    recalcularAreasBotones();
    
    // Factor de escala universal basado en el tamaño de la ventana
    float escalaReferencia = std::min(m_size.x, m_size.y) / 800.0f;
    
    sf::RectangleShape fondo(m_size);
    fondo.setFillColor(sf::Color(0, 0, 0, 220));
    fondo.setPosition(m_position);
    window.draw(fondo);

    // Título (tamaño proporcional)
    if (m_tituloEleccion)
    {
        int tamanoTitulo = static_cast<int>(36 * escalaReferencia);
        if (tamanoTitulo < 24) tamanoTitulo = 24;
        if (tamanoTitulo > 48) tamanoTitulo = 48;
        
        m_tituloEleccion->setCharacterSize(tamanoTitulo);
        sf::FloatRect bounds = m_tituloEleccion->getLocalBounds();
        m_tituloEleccion->setPosition(sf::Vector2f(
            m_position.x + (m_size.x - bounds.size.x) / 2,
            m_position.y + 40 * escalaReferencia));
        window.draw(*m_tituloEleccion);
    }

    // Mensaje de advertencia (tamaño proporcional)
    if (m_mensajeAdvertencia)
    {
        int tamanoAdvertencia = static_cast<int>(14 * escalaReferencia);
        if (tamanoAdvertencia < 12) tamanoAdvertencia = 12;
        if (tamanoAdvertencia > 24) tamanoAdvertencia = 24;
        
        m_mensajeAdvertencia->setCharacterSize(tamanoAdvertencia);
        sf::FloatRect bounds = m_mensajeAdvertencia->getLocalBounds();
        m_mensajeAdvertencia->setPosition(sf::Vector2f(
            m_position.x + (m_size.x - bounds.size.x) / 2,
            m_position.y + 80 * escalaReferencia));
        window.draw(*m_mensajeAdvertencia);
    }
    
    // Dibujar botones y textos de sospechosos
    for (size_t i = 0; i < m_sospechosos.size() && i < m_areasBotones.size(); i++)
    {
        sf::FloatRect areaBoton = m_areasBotones[i];
        float x = areaBoton.position.x;
        float y = areaBoton.position.y;
        float botonAncho = areaBoton.size.x;
        float botonAlto = areaBoton.size.y;

        // Fondo del botón
        sf::RectangleShape boton(sf::Vector2f(botonAncho, botonAlto));
        boton.setPosition(sf::Vector2f(x, y));
        boton.setFillColor(sf::Color(40, 40, 70, 230));
        boton.setOutlineColor(sf::Color::White);
        boton.setOutlineThickness(2.f * escalaReferencia);
        window.draw(boton);

        // Nombre del sospechoso (tamaño proporcional)
        sf::Text nombreText(*m_font);
        nombreText.setString(m_sospechosos[i].nombre);
        int tamanoNombre = static_cast<int>(26 * escalaReferencia);
        if (tamanoNombre < 18) tamanoNombre = 18;
        if (tamanoNombre > 40) tamanoNombre = 40;
        
        nombreText.setCharacterSize(tamanoNombre);
        nombreText.setFillColor(sf::Color::White);
        nombreText.setOutlineColor(sf::Color::Black);
        nombreText.setOutlineThickness(1.0f * escalaReferencia);
        nombreText.setStyle(sf::Text::Bold);
        
        sf::FloatRect nameBounds = nombreText.getLocalBounds();
        nombreText.setPosition(sf::Vector2f(
            x + (botonAncho - nameBounds.size.x) / 2, 
            y + 20 * escalaReferencia)); 
        window.draw(nombreText);

        //  LÍNEA SEPARADORA DESPUÉS DEL NOMBRE 
        sf::RectangleShape lineaSeparadora;
        lineaSeparadora.setSize(sf::Vector2f(botonAncho - 40 * escalaReferencia, 2.f * escalaReferencia));
        lineaSeparadora.setFillColor(sf::Color(200, 180, 100));
        lineaSeparadora.setPosition(sf::Vector2f(
            x + 20 * escalaReferencia,
            y + 55 * escalaReferencia));  
        window.draw(lineaSeparadora);

        //  DESCRIPCIÓN MULTILÍNEA (más abajo) 
        int tamanoDesc = static_cast<int>(17 * escalaReferencia);
        if (tamanoDesc < 14) tamanoDesc = 14;
        if (tamanoDesc > 26) tamanoDesc = 26;
        
        std::string descOriginal = m_sospechosos[i].descripcion;
        std::vector<std::string> lineas;
        
        // Dividir por saltos de línea manuales primero
        std::stringstream ss(descOriginal);
        std::string linea;
        while (std::getline(ss, linea, '\n')) {
            sf::Text testText(*m_font);
            testText.setCharacterSize(tamanoDesc);
            testText.setString(linea);
            
            float maxWidth = botonAncho - 40 * escalaReferencia;
            float currentWidth = testText.getLocalBounds().size.x;
            
            if (currentWidth <= maxWidth) {
                lineas.push_back(linea);
            } else {
                std::stringstream words(linea);
                std::string word;
                std::string currentLine = "";
                
                while (words >> word) {
                    testText.setString(currentLine + (currentLine.empty() ? "" : " ") + word);
                    if (testText.getLocalBounds().size.x <= maxWidth) {
                        currentLine += (currentLine.empty() ? "" : " ") + word;
                    } else {
                        if (!currentLine.empty()) {
                            lineas.push_back(currentLine);
                        }
                        currentLine = word;
                    }
                }
                if (!currentLine.empty()) {
                    lineas.push_back(currentLine);
                }
            }
        }
        
        float descY = y + 75 * escalaReferencia;  
        float lineSpacing = (tamanoDesc + 8) * escalaReferencia;
        
        // Limitar a máximo 3 líneas para no ocupar demasiado espacio
        size_t maxLines = std::min(lineas.size(), static_cast<size_t>(3));
        
        for (size_t j = 0; j < maxLines; j++) {
            sf::Text descText(*m_font);
            descText.setString(lineas[j]);
            descText.setCharacterSize(tamanoDesc);
            descText.setFillColor(sf::Color(220, 220, 220));
            descText.setOutlineColor(sf::Color::Black);
            descText.setOutlineThickness(0.8f * escalaReferencia);
            
            sf::FloatRect descBounds = descText.getLocalBounds();
            descText.setPosition(sf::Vector2f(
                x + (botonAncho - descBounds.size.x) / 2,
                descY));
            window.draw(descText);
            
            descY += lineSpacing;
        }

        // Botón "ACUSAR" 
        float botonAcusarAlto = 45 * escalaReferencia;
        float botonAcusarAncho = (botonAncho - 50 * escalaReferencia);
        float margenLateral = 25 * escalaReferencia;
        
        sf::RectangleShape botonAcusar(sf::Vector2f(botonAcusarAncho, botonAcusarAlto));
        botonAcusar.setPosition(sf::Vector2f(
            x + margenLateral, 
            y + botonAlto - botonAcusarAlto - 15 * escalaReferencia));
        botonAcusar.setFillColor(sf::Color(150, 40, 40));
        botonAcusar.setOutlineColor(sf::Color::Yellow);
        botonAcusar.setOutlineThickness(1.5f * escalaReferencia);
        window.draw(botonAcusar);

        // Texto "ACUSAR"
        sf::Text acusarText(*m_font);
        acusarText.setString("ACUSAR");
        int tamanoAcusar = static_cast<int>(16 * escalaReferencia);
        if (tamanoAcusar < 12) tamanoAcusar = 12;
        if (tamanoAcusar > 24) tamanoAcusar = 24;
        
        acusarText.setCharacterSize(tamanoAcusar);
        acusarText.setFillColor(sf::Color::White);
        acusarText.setOutlineColor(sf::Color::Black);
        acusarText.setOutlineThickness(1.0f * escalaReferencia);
        acusarText.setStyle(sf::Text::Bold);
        
        sf::FloatRect acusarBounds = acusarText.getLocalBounds();
        acusarText.setPosition(sf::Vector2f(
            x + botonAncho / 2 - acusarBounds.size.x / 2,
            y + botonAlto - botonAcusarAlto - 15 * escalaReferencia + (botonAcusarAlto - acusarBounds.size.y) / 2));
        window.draw(acusarText);
    }
}
void CriminalCaseMinigame::draw(sf::RenderWindow &window)
{
    if (!m_active)
        return;

    if (m_background)
    {
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
                // window.draw(rect);
            }
        }
    }
    
    switch (m_gameState)
    {
    case CriminalGameState::BUSCANDO_EVIDENCIAS:
        if (m_listaText)
            window.draw(*m_listaText);
        if (m_instruccionText)
        {
            float escalaTexto = std::min(m_size.x, m_size.y) / 800.0f;
            int tamanoInstruccion = static_cast<int>(20 * escalaTexto);
            if (tamanoInstruccion < 16)
                tamanoInstruccion = 16;
            if (tamanoInstruccion > 34)
                tamanoInstruccion = 34;
            m_instruccionText->setCharacterSize(tamanoInstruccion);
            m_instruccionText->setOutlineThickness(1.5f);

            m_instruccionText->setPosition(sf::Vector2f(
                m_position.x + 20.0f * escalaTexto,
                m_position.y + m_size.y - 45.0f * escalaTexto));
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
    
    // MENSAJES TEMPORALES 
    if (m_mensajeTemp.tiempoRestante > 0 && m_mensajeText && !m_mensajeTemp.texto.empty())
    {
        // Obtener el tamaño de la ventana en coordenadas de pantalla
        sf::Vector2u winSize = window.getSize();
        
        float centerX = winSize.x / 2.f;
        float centerY = static_cast<float>(winSize.y) - 150.f;
        
        sf::FloatRect textBounds = m_mensajeText->getLocalBounds();
        
        if (m_mensajeErrorActivo)
        {
            centerY = winSize.y / 2.f;
            
            sf::RectangleShape fondoMsg(sf::Vector2f(textBounds.size.x + 50, textBounds.size.y + 30));
            fondoMsg.setFillColor(sf::Color(0, 0, 0, 220));
            fondoMsg.setOutlineColor(m_mensajeText->getFillColor());
            fondoMsg.setOutlineThickness(2.f);
            
            // Convertir coordenadas de pantalla a coordenadas de mundo
            sf::Vector2f screenPos(centerX, centerY);
            sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y)));
            
            fondoMsg.setOrigin(sf::Vector2f(fondoMsg.getSize().x / 2.f, fondoMsg.getSize().y / 2.f));
            fondoMsg.setPosition(worldPos);
            window.draw(fondoMsg);
            
            m_mensajeText->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_mensajeText->setPosition(worldPos);
        }
        else
        {
            // Convertir coordenadas de pantalla a coordenadas de mundo
            sf::Vector2f screenPos(centerX, centerY);
            sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y)));
            
            m_mensajeText->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_mensajeText->setPosition(worldPos);
        }
        
        window.draw(*m_mensajeText);
    }
}

void CriminalCaseMinigame::resetGame()
{
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

void CriminalCaseMinigame::limpiarInventario()
{
    if (m_inventory)
    {
        for (const auto &obj : m_objetos)
        {
            if (obj.encontrado)
            {
                for (int i = 0; i < 20; i++)
                {
                    Item *item = m_inventory->getItem(i);
                    if (item && item->name == obj.nombre)
                    {
                        m_inventory->removeItem(i);
                        break;
                    }
                }
            }
        }
    }
}