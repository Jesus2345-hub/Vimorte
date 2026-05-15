#include "MiniGameCook.hpp"
#include <iostream>
#include <algorithm>
#include <random>
#include <cmath>

MiniGameCook::MiniGameCook(Inventory* inventory)
    : m_estadoActual(EstadoMinijuego::ESPERANDO)
    , m_estanteActual(nullptr)
    , m_indiceSeleccionado(-1)
    , m_mostrarUIEstante(false)
    , m_platoEntregado(false)
    , m_misionCompletada(false)
    , m_inventory(inventory)
    , m_font(nullptr)
    , m_tituloEstante(nullptr)
    , m_textoInstrucciones(nullptr)
    , m_platoRequerido("", {}, "", sf::Color::White)
{
    m_font = std::make_unique<sf::Font>();
    if (m_font->openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        m_tituloEstante = std::make_unique<sf::Text>(*m_font);
        m_tituloEstante->setCharacterSize(28);
        m_tituloEstante->setFillColor(sf::Color::Yellow);
        
        m_textoInstrucciones = std::make_unique<sf::Text>(*m_font);
        m_textoInstrucciones->setCharacterSize(18);
        m_textoInstrucciones->setFillColor(sf::Color::White);
    }
    
    inicializarIngredientes();
    inicializarRecetas();
    cargarTexturas();
}

void MiniGameCook::inicializarIngredientes()
{
    Estante estanteCarnes;
    estanteCarnes.nombre = "Carnes";
    estanteCarnes.visible = true;
    estanteCarnes.ingredientes = {
        Ingrediente("Carne de Res", "carne", sf::Color(139, 69, 19), "assets/images/niveles/centinela2/carne_res.png"),
        Ingrediente("Pollo", "carne", sf::Color(255, 228, 196), "assets/images/niveles/centinela2/pollo.png"),
        Ingrediente("Cerdo", "carne", sf::Color(255, 182, 193), "assets/images/niveles/centinela2/cerdo.png"),
        Ingrediente("Cordero", "carne", sf::Color(160, 82, 45), "assets/images/niveles/centinela2/cordero.png")
    };
    m_estantes.push_back(estanteCarnes);
    
    Estante estanteVerduras;
    estanteVerduras.nombre = "Verduras";
    estanteVerduras.visible = true;
    estanteVerduras.ingredientes = {
        Ingrediente("Tomate", "verdura", sf::Color(255, 99, 71), "assets/images/niveles/centinela2/tomate.png"),
        Ingrediente("Lechuga", "verdura", sf::Color(124, 252, 0), "assets/images/niveles/centinela2/lechuga.png"),
        Ingrediente("Cebolla", "verdura", sf::Color(255, 215, 0), "assets/images/niveles/centinela2/cebolla.png"),
        Ingrediente("Zanahoria", "verdura", sf::Color(255, 140, 0), "assets/images/niveles/centinela2/zanahoria.png"),
        Ingrediente("Papa", "verdura", sf::Color(222, 184, 135), "assets/images/niveles/centinela2/papa.png")
    };
    m_estantes.push_back(estanteVerduras);
    
    Estante estanteOtros;
    estanteOtros.nombre = "Lacteos y Embutidos";
    estanteOtros.visible = true;
    estanteOtros.ingredientes = {
        Ingrediente("Huevo", "otros", sf::Color(255, 255, 200), "assets/images/niveles/centinela2/huevo.png"),
        Ingrediente("Leche", "otros", sf::Color(255, 255, 255), "assets/images/niveles/centinela2/leche.png"),
        Ingrediente("Queso", "otros", sf::Color(255, 215, 0), "assets/images/niveles/centinela2/queso.png"),
        Ingrediente("Jamon", "otros", sf::Color(255, 182, 193), "assets/images/niveles/centinela2/jamon.png"),
        Ingrediente("Salchicha", "otros", sf::Color(205, 92, 92), "assets/images/niveles/centinela2/salchicha.png"),
        Ingrediente("Tocino", "otros", sf::Color(165, 42, 42), "assets/images/niveles/centinela2/tocino.png")
    };
    m_estantes.push_back(estanteOtros);
}

void MiniGameCook::inicializarRecetas()
{
    m_recetasDisponibles = {
        Plato("Hamburguesa", 
              {"Carne de Res", "Lechuga", "Tomate", "Queso"},
              "Jugosa hamburguesa con queso derretido",
              sf::Color(205, 133, 63)),
        
        Plato("Pollo Asado",
              {"Pollo", "Papa", "Zanahoria", "Cebolla"},
              "Pollo asado con verduras al horno",
              sf::Color(255, 140, 0)),
        
        Plato("Tortilla",
              {"Huevo", "Papa", "Cebolla"},
              "Clasica tortilla de papas",
              sf::Color(255, 215, 0)),
        
        Plato("Sandwich de Jamon",
              {"Jamon", "Queso", "Lechuga", "Tomate"},
              "Sandwich fresco de jamon y queso",
              sf::Color(222, 184, 135)),
        
        Plato("Huevos con Tocino",
              {"Huevo", "Tocino"},
              "Huevos estrellados con tocino crujiente",
              sf::Color(255, 182, 193)),
        
        Plato("Estofado de Cordero",
              {"Cordero", "Leche", "Salchicha", "Cerdo"},
              "Estofado de cordero con salchichas en salsa de leche",
              sf::Color(160, 82, 45))
    };
    
    // Mezclar para orden aleatorio
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_recetasDisponibles.begin(), m_recetasDisponibles.end(), g);
    
    if (!m_recetasDisponibles.empty()) {
        m_platoRequerido = m_recetasDisponibles[0];
    }
}

void MiniGameCook::cargarTexturas()
{
    std::vector<std::pair<std::string, std::string>> texturasCargar = {
        {"Carne de Res", "assets/images/niveles/centinela2/carne.png"},
        {"Pollo", "assets/images/niveles/centinela2/pollo.png"},
        {"Cerdo", "assets/images/niveles/centinela2/cerdo.png"},
        {"Cordero", "assets/images/niveles/centinela2/cordero.png"},
        {"Tomate", "assets/images/niveles/centinela2/tomate.png"},
        {"Lechuga", "assets/images/niveles/centinela2/lechuga.png"},
        {"Cebolla", "assets/images/niveles/centinela2/cebolla.png"},
        {"Zanahoria", "assets/images/niveles/centinela2/zanahoria.png"},
        {"Papa", "assets/images/niveles/centinela2/papa.png"},
        {"Huevo", "assets/images/niveles/centinela2/huevo.png"},
        {"Leche", "assets/images/niveles/centinela2/leche.png"},
        {"Queso", "assets/images/niveles/centinela2/queso.png"},
        {"Jamon", "assets/images/niveles/centinela2/jamon.png"},
        {"Salchicha", "assets/images/niveles/centinela2/salchicha.png"},
        {"Tocino", "assets/images/niveles/centinela2/tocino.png"}
    };
    
    for (const auto& [nombre, ruta] : texturasCargar) {
        sf::Texture tex;
        if (tex.loadFromFile(ruta)) {
            tex.setSmooth(true);
            m_texturasIngredientes[nombre] = tex;
        }
    }
}

void MiniGameCook::reiniciarMision()
{
    m_misionCompletada = false;
    m_platoEntregado = false;
    
    // Seleccionar un nuevo plato aleatorio de los disponibles
    if (!m_recetasDisponibles.empty()) {
        int indice = rand() % m_recetasDisponibles.size();
        m_platoRequerido = m_recetasDisponibles[indice];
    }
    
    m_estadoActual = EstadoMinijuego::ESPERANDO;
    m_mostrarUIEstante = false;
    m_estanteActual = nullptr;
}


void MiniGameCook::actualizarAreaEstante(const std::string& nombre, const sf::FloatRect& area)
{
    for (auto& estante : m_estantes) {
        if (estante.nombre == nombre) {
            estante.areaInteraccion = area;
            std::cout << "DEBUG: Area actualizada para " << nombre << " -> pos: " 
                      << area.position.x << "," << area.position.y << std::endl;
            break;
        }
    }
}

void MiniGameCook::agregarEstante(const std::string& nombre, const sf::FloatRect& area)
{
    for (auto& estante : m_estantes) {
        if (estante.nombre == nombre) {
            estante.areaInteraccion = area;
            break;
        }
    }
}

void MiniGameCook::agregarIngredienteAEstante(const std::string& nombreEstante, const Ingrediente& ingrediente)
{
    for (auto& estante : m_estantes) {
        if (estante.nombre == nombreEstante) {
            estante.ingredientes.push_back(ingrediente);
            break;
        }
    }
}

void MiniGameCook::update(float dt, const sf::Vector2f& playerPos)
{
   
    
    if (m_mostrarUIEstante && m_estanteActual) {
        sf::Vector2f centroEstante(
            m_estanteActual->areaInteraccion.position.x + m_estanteActual->areaInteraccion.size.x / 2,
            m_estanteActual->areaInteraccion.position.y + m_estanteActual->areaInteraccion.size.y / 2
        );
        float distancia = std::sqrt(
            std::pow(playerPos.x - centroEstante.x, 2) + 
            std::pow(playerPos.y - centroEstante.y, 2)
        );
        if (distancia > 100.0f) {
            m_mostrarUIEstante = false;
            m_estanteActual = nullptr;
            m_indiceSeleccionado = -1;
        }
    }
}

std::string MiniGameCook::getEstanteCerca(const sf::Vector2f& playerPos) const
{
    sf::FloatRect playerRect(playerPos - sf::Vector2f(15.f, 15.f), sf::Vector2f(30.f, 30.f));
    
    for (const auto& estante : m_estantes) {
        if (estante.areaInteraccion.findIntersection(playerRect).has_value()) {
            return estante.nombre;
        }
    }
    return "";
}

bool MiniGameCook::estaCercaCocina(const sf::Vector2f& playerPos) const
{
    sf::FloatRect playerRect(playerPos - sf::Vector2f(5.f, 5.f), sf::Vector2f(10.f, 10.f));
    return m_areaCocina.findIntersection(playerRect).has_value();
}

bool MiniGameCook::estaCercaEntrega(const sf::Vector2f& playerPos) const
{
    sf::FloatRect playerRect(playerPos - sf::Vector2f(5.f, 5.f), sf::Vector2f(10.f, 10.f));
    return m_areaEntrega.findIntersection(playerRect).has_value();
}

bool MiniGameCook::estaCercaMenu(const sf::Vector2f& playerPos) const
{
    sf::FloatRect playerRect(playerPos - sf::Vector2f(5.f, 5.f), sf::Vector2f(10.f, 10.f));
    return m_areaMenu.findIntersection(playerRect).has_value();
}

bool MiniGameCook::tieneIngredientes(const Plato& plato)
{
    if (!m_inventory) return false;
    
    std::vector<std::string> ingredientesFaltantes = plato.ingredientesNecesarios;
    
    for (int i = 0; i < 15; i++) {
        Item* item = m_inventory->getItem(i);
        if (item) {
            auto it = std::find(ingredientesFaltantes.begin(), ingredientesFaltantes.end(), item->name);
            if (it != ingredientesFaltantes.end()) {
                ingredientesFaltantes.erase(it);
            }
        }
    }
    
    return ingredientesFaltantes.empty();
}

bool MiniGameCook::consumirIngredientes(const Plato& plato)
{
    if (!m_inventory) return false;
    
    for (const auto& ingrediente : plato.ingredientesNecesarios) {
        for (int i = 0; i < 15; i++) {
            Item* item = m_inventory->getItem(i);
            if (item && item->name == ingrediente) {
                m_inventory->removeItem(i);
                break;
            }
        }
    }
    return true;
}

void MiniGameCook::agregarPlatoAlInventario(const Plato& plato)
{
    if (!m_inventory) return;
    
    Item platoItem(plato.nombre, plato.color, "assets/images/niveles/centinela2/plato.png");
    m_inventory->addItem(platoItem);
}

void MiniGameCook::draw(sf::RenderWindow& window)
{
    sf::Vector2u windowSize = window.getSize();
    
    if (m_mostrarUIEstante && m_estanteActual && m_font) {
        sf::RectangleShape fondo(sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
        fondo.setFillColor(sf::Color(0, 0, 0, 200));
        window.draw(fondo);
        
        sf::RectangleShape panel(sf::Vector2f(500.f, 350.f));
        panel.setFillColor(sf::Color(30, 30, 50, 240));
        panel.setOutlineThickness(2.f);
        panel.setOutlineColor(sf::Color(150, 100, 50));
        panel.setOrigin(sf::Vector2f(250.f, 175.f));
        panel.setPosition(sf::Vector2f(static_cast<float>(windowSize.x) / 2.f, static_cast<float>(windowSize.y) / 2.f));
        window.draw(panel);
        
        if (m_tituloEstante) {
            m_tituloEstante->setString("=== " + m_estanteActual->nombre + " ===");
            sf::FloatRect bounds = m_tituloEstante->getLocalBounds();
            m_tituloEstante->setOrigin(sf::Vector2f(bounds.size.x / 2.f, 0.f));
            m_tituloEstante->setPosition(sf::Vector2f(static_cast<float>(windowSize.x) / 2.f, static_cast<float>(windowSize.y) / 2.f - 130.f));
            window.draw(*m_tituloEstante);
        }
        
        if (m_textoInstrucciones) {
            m_textoInstrucciones->setString("Presiona el numero del ingrediente para obtenerlo | F para salir");
            sf::FloatRect bounds = m_textoInstrucciones->getLocalBounds();
            m_textoInstrucciones->setOrigin(sf::Vector2f(bounds.size.x / 2.f, 0.f));
            m_textoInstrucciones->setPosition(sf::Vector2f(static_cast<float>(windowSize.x) / 2.f, static_cast<float>(windowSize.y) / 2.f + 140.f));
            window.draw(*m_textoInstrucciones);
        }
        
        float startY = static_cast<float>(windowSize.y) / 2.f - 80.f;
        for (size_t i = 0; i < m_estanteActual->ingredientes.size() && i < 9; i++) {
            sf::Text textoIng(*m_font);
            textoIng.setString(std::to_string(i + 1) + ". " + m_estanteActual->ingredientes[i].nombre);
            textoIng.setCharacterSize(20);
            textoIng.setFillColor(sf::Color::White);
            textoIng.setPosition(sf::Vector2f(static_cast<float>(windowSize.x) / 2.f - 200.f, startY + static_cast<float>(i) * 30.f));
            window.draw(textoIng);
        }
    }
    
}

void MiniGameCook::handleEvent(const sf::Event& event, sf::RenderWindow& window, const sf::Vector2f& playerPos)
{
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        
        if (m_mostrarUIEstante && m_estanteActual) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                m_mostrarUIEstante = false;
                m_estanteActual = nullptr;
                m_indiceSeleccionado = -1;
                return;
            }
            
            int numSeleccionado = -1;
            switch (keyPressed->code) {
                case sf::Keyboard::Key::Num1: numSeleccionado = 0; break;
                case sf::Keyboard::Key::Num2: numSeleccionado = 1; break;
                case sf::Keyboard::Key::Num3: numSeleccionado = 2; break;
                case sf::Keyboard::Key::Num4: numSeleccionado = 3; break;
                case sf::Keyboard::Key::Num5: numSeleccionado = 4; break;
                case sf::Keyboard::Key::Num6: numSeleccionado = 5; break;
                case sf::Keyboard::Key::Num7: numSeleccionado = 6; break;
                case sf::Keyboard::Key::Num8: numSeleccionado = 7; break;
                case sf::Keyboard::Key::Num9: numSeleccionado = 8; break;
                default: break;
            }
            
           if (numSeleccionado >= 0 && numSeleccionado < (int)m_estanteActual->ingredientes.size()) {
                const auto& ing = m_estanteActual->ingredientes[numSeleccionado];
                Item nuevoItem(ing.nombre, ing.color, ing.rutaImagen);
                
                auto it = m_texturasIngredientes.find(ing.nombre);
                if (it != m_texturasIngredientes.end() && nuevoItem.textura) {
                    *nuevoItem.textura = it->second;
                    nuevoItem.sprite = std::make_unique<sf::Sprite>(*nuevoItem.textura);
                }
                
                if (m_inventory && m_inventory->tryCollectItem(ing.nombre, ing.color)) {
                    if (m_mensajeCallback) {
                        m_mensajeCallback("Obtuviste: " + ing.nombre, 2.0f, sf::Color::Green);
                    }
                } else {
                    if (m_mensajeCallback) {
                        m_mensajeCallback("Inventario lleno!", 2.0f, sf::Color::Red);
                    }
                }
                
                m_mostrarUIEstante = false;
                m_estanteActual = nullptr;
                return;
            }
            return;
        }
        
        if (keyPressed->code == sf::Keyboard::Key::F) 
        {
            
           if (estaCercaMenu(playerPos) && !m_misionCompletada) {
                std::string platoReq = m_platoRequerido.nombre;
                std::string descripcion = m_platoRequerido.descripcion;
                
                std::string ingredientes = "Ingredientes: ";
                for (size_t i = 0; i < m_platoRequerido.ingredientesNecesarios.size(); i++) {
                    if (i > 0) ingredientes += ", ";
                    ingredientes += m_platoRequerido.ingredientesNecesarios[i];
                }
                
                std::string mensaje = "=== PLATO REQUERIDO ===\n\n" + platoReq + "\n\n" + ingredientes + "\n\n" + descripcion;
                if (m_mensajeCallback) {
                    m_mensajeCallback(mensaje, 5.0f, sf::Color::Yellow);
                }
                return;
            }
            
            std::string estanteCerca = getEstanteCerca(playerPos);
            if (!estanteCerca.empty() && !m_mostrarUIEstante) {
                for (auto& estante : m_estantes) {
                    if (estante.nombre == estanteCerca) {
                        m_estanteActual = &estante;
                        m_mostrarUIEstante = true;
                        m_indiceSeleccionado = -1;
                        return;
                    }
                }
            }
            // ========== COCINA ==========
            if (estaCercaCocina(playerPos) && !m_misionCompletada) {
                if (tieneIngredientes(m_platoRequerido)) {
                    if (consumirIngredientes(m_platoRequerido)) {
                        agregarPlatoAlInventario(m_platoRequerido);
                        if (m_mensajeCallback) {
                            m_mensajeCallback("Cocinaste: " + m_platoRequerido.nombre + "!", 2.0f, sf::Color::Green);
                        }
                    }
                } else {
                    if (m_mensajeCallback) {
                        m_mensajeCallback("Te faltan ingredientes para " + m_platoRequerido.nombre, 2.0f, sf::Color::Yellow);
                    }
                }
                return;
            }

            
            if (estaCercaEntrega(playerPos) && !m_misionCompletada) {
                bool platoEncontrado = false;
                for (int i = 0; i < 15; i++) {
                    Item* item = m_inventory->getItem(i);
                    if (item && item->name == m_platoRequerido.nombre) {
                        platoEncontrado = true;
                        m_inventory->removeItem(i);
                        break;
                    }
                }
                
                if (platoEncontrado) {
                    if (m_mensajeCallback) {
                        m_mensajeCallback("Excelente! El alienigena amo tu " + m_platoRequerido.nombre + "!", 3.0f, sf::Color::Green);
                    }
                    m_misionCompletada = true;
                    m_platoEntregado = true;
                } else {
                    if (m_mensajeCallback) {
                        m_mensajeCallback("Necesitas entregar: " + m_platoRequerido.nombre, 2.0f, sf::Color::Yellow);
                    }
                }
                return;
            }
        }
    }
}

// ==================== COCINA MINIGAME STATE ====================

CocinaMinigameState::CocinaMinigameState(Inventory* inventory)
    : m_miniGame(inventory)
    , m_activo(true)
    , m_font(nullptr)
    , m_textoFeedback(nullptr)
    , m_fontLoaded(false)
{
    m_font = std::make_unique<sf::Font>();
    if (m_font->openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        m_fontLoaded = true;
        m_textoFeedback = std::make_unique<sf::Text>(*m_font);
        m_textoFeedback->setCharacterSize(20);
        m_textoFeedback->setFillColor(sf::Color::White);
    }
}

void CocinaMinigameState::setAreas(const sf::FloatRect& cocina, const sf::FloatRect& entrega, const sf::FloatRect& menu)
{
    m_miniGame.setAreaCocina(cocina);
    m_miniGame.setAreaEntrega(entrega);
    m_miniGame.setAreaMenu(menu);
}

void CocinaMinigameState::agregarEstante(const std::string& nombre, const sf::FloatRect& area)
{
    m_miniGame.agregarEstante(nombre, area);
}

void CocinaMinigameState::update(float dt, const sf::Vector2f& playerPos)
{
    if (m_activo) {
        m_miniGame.update(dt, playerPos);
    }
}

void CocinaMinigameState::handleEvent(const sf::Event& event, sf::RenderWindow& window, const sf::Vector2f& playerPos)
{
    if (m_activo) {
        m_miniGame.handleEvent(event, window, playerPos);
    }
}

void CocinaMinigameState::draw(sf::RenderWindow& window)
{
    if (m_activo) {
        m_miniGame.draw(window);
    }
}