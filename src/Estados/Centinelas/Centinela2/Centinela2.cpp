#include "Estados/Niveles/Centinelas/Centinela2/Centinela2.hpp"
#include "Estados/PauseState.hpp"
#include "Estados/CentinelaGameOverState.hpp"
#include "Estados/MuerteCentinelaState.hpp"
#include "Configuracion/CoordenadasDebug.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

Centinela2State::Centinela2State(sf::RenderWindow *window, Game *game)
    : State(window, game),
      m_background(nullptr),
      m_fontLoaded(false),
      m_debugMode(false),
      m_tiempoRestante(180.0f),
      m_platosEntregados(0),
      m_platosRequeridos(6),
      m_juegoActivo(true),
      m_gameOver(false),
      m_muertes(0),
      m_activo(true),
      m_cercaCocina(false),
      m_cercaEntrega(false),
      m_cercaMenu(false),
      m_cercaCentinela(false),
      m_nivelCompletado(false),
      m_cercaBloqueInteractivo(false),
      m_mensajeEmergenteActivo(false),
      m_bloqueActualIndex(-1),
      m_tiempoFlotante(0.0f),
      m_dialogoDecisionActivo(false),
      m_opcionSeleccionada(0),
      m_estanteAbierto(false),
      m_tiempoBloqueoAscensor(0.0f)
{
    m_msjActual.texto = "";
    m_msjActual.tiempoRestante = 0.0f;
    m_msjActual.color = sf::Color::Yellow;
    
    // Configurar jugador
    m_player.loadAssets();
    m_player.setPosition(296.f, 361.f);
    m_player.setSpeed(300.0f);
    
    // Cargar fondo
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/centinela2/background.png"))
    {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x), static_cast<float>(textureSize.y));
    }
    else
    {
        std::cerr << "Error: No se pudo cargar background del centinela" << std::endl;
        m_worldSize = sf::Vector2f(1280.f, 720.f);
    }
    
    // Configurar cámara
    sf::Vector2u windowSize = window->getSize();
    m_camera = sf::View(sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f), sf::Vector2f(1280.f, 720.f));
    m_lastWindowSize = windowSize;
    
    // Configurar colisiones
    configurarColisiones();
    
    // Configurar minijuego de cocina
    m_cocinaMinigame = std::make_unique<CocinaMinigameState>(m_player.getInventory());
    configurarAreasCocina();
    configurarBloqueAscensor();
    
    m_cocinaMinigame->getMiniGame()->setMensajeCallback(
        [this](const std::string& texto, float duracion, sf::Color color) {
            mostrarMensajeFlotante(texto, duracion, color);
        }
    );
    // Cargar fuente
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (m_fontLoaded)
    {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setCharacterSize(20);
        m_textoInteraccion->setFillColor(sf::Color::White);
        m_textoInteraccion->setOutlineThickness(1.5f);
        m_textoInteraccion->setOutlineColor(sf::Color::Black);
        
        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(24);
        m_textoMensaje->setFillColor(sf::Color::Yellow);
        m_textoMensaje->setOutlineThickness(1.5f);
        m_textoMensaje->setOutlineColor(sf::Color::Black);
        m_textoMensaje->setStyle(sf::Text::Bold);
        
        m_mensajeFlotante = std::make_unique<sf::Text>(m_font);
        m_mensajeFlotante->setCharacterSize(22);
        m_mensajeFlotante->setFillColor(sf::Color::Yellow);
        m_mensajeFlotante->setOutlineThickness(2.0f);
        m_mensajeFlotante->setOutlineColor(sf::Color::Black);
        m_mensajeFlotante->setStyle(sf::Text::Bold);
        
        // UI del timer
        m_textoTimer = std::make_unique<sf::Text>(m_font);
        m_textoTimer->setCharacterSize(24);
        m_textoTimer->setFillColor(sf::Color::White);
        m_textoTimer->setOutlineThickness(1.0f);
        m_textoTimer->setOutlineColor(sf::Color::Black);
        
        m_textoContador = std::make_unique<sf::Text>(m_font);
        m_textoContador->setCharacterSize(20);
        m_textoContador->setFillColor(sf::Color::White);
        m_textoContador->setOutlineThickness(1.0f);
        m_textoContador->setOutlineColor(sf::Color::Black);
        
        m_textoPlatoActual = std::make_unique<sf::Text>(m_font);
        m_textoPlatoActual->setCharacterSize(18);
        m_textoPlatoActual->setFillColor(sf::Color(255, 215, 0));
        m_textoPlatoActual->setOutlineThickness(1.0f);
        m_textoPlatoActual->setOutlineColor(sf::Color::Black);
        
        m_timerBarra.setSize(sf::Vector2f(300.f, 20.f));
        m_timerBarra.setFillColor(sf::Color::Green);
        m_timerBarra.setOutlineThickness(1.f);
        m_timerBarra.setOutlineColor(sf::Color::White);
    }
    
    game->setIsInLevel(true);
    m_mensajeEmergenteActivo = true;
    m_bloqueActualIndex = 0;
}

void Centinela2State::configurarColisiones()
{
    m_mapaFisico.clear();
    
    m_mapaFisico.emplace_back(30.f, 12.f, m_worldSize.x - 60.f, 30.f);
    m_mapaFisico.emplace_back(30.f, 12.f, 20.f, m_worldSize.y - 40.f);
    m_mapaFisico.emplace_back(401.f,142.f , 13.f, 866.f);
    m_mapaFisico.emplace_back(548.f,595.f , 170.f, 294.f);
    m_mapaFisico.emplace_back(784.f,595.f , 170.f, 294.f);
    m_mapaFisico.emplace_back(1048.f,595.f , 170.f, 294.f);
    m_mapaFisico.emplace_back(65.f,818.f , 217.f, 148.f);
    m_mapaFisico.emplace_back(646.f,185.f , 371.f, 121.f);
    m_mapaFisico.emplace_back(1198.f, 312.f , 144.f, 253.f);
    m_mapaFisico.emplace_back(1281.f, 571.f , 80.f, 216.f);
    m_mapaFisico.emplace_back(48.f, 42.f , 1348.f, 204.f);
    m_mapaFisico.emplace_back(52.f, 204.f , 190.f, 206.f);
    m_mapaFisico.emplace_back(237.f, 227.f , 131.f, 78.f);
    m_mapaFisico.emplace_back(336.f, 322.f , 34.f, 88.f);
    m_mapaFisico.emplace_back(1295.f, 239.f , 22.f, 58.f);
    m_mapaFisico.emplace_back(380.f, 652.f , 20.f, 354.f);
    m_mapaFisico.emplace_back(416.f, 248.f , 10.f, 658.f);
    m_mapaFisico.emplace_back(30.f, 1066.f , 1412.f, 18.f);
    m_mapaFisico.emplace_back(1362.f, 788.f , 70.f, 285.f);
    m_mapaFisico.emplace_back(52.f, 411.f , 14.f, 322.f);
    m_mapaFisico.emplace_back(603.f, 306.f , 454.f, 141.f);
    m_mapaFisico.emplace_back(380.f, 482.f , 20.f, 168.f);
}


void Centinela2State::configurarAreasCocina()
{
    m_areaCocina = sf::FloatRect(sf::Vector2f(1104.f, 309.f), sf::Vector2f(117.f, 210.f));
    m_areaEntrega = sf::FloatRect(sf::Vector2f(575.f, 307.f), sf::Vector2f(475.f, 199.f));
    m_areaMenuPlato = sf::FloatRect(sf::Vector2f(66.f, 780.f), sf::Vector2f(223.f, 250.f));
    
    m_areaEstanteCarnes = sf::FloatRect(sf::Vector2f(549.f, 510.f), sf::Vector2f(190.f, 370.f));
    m_areaEstanteVerduras = sf::FloatRect(sf::Vector2f(783.f, 510.f), sf::Vector2f(190.f, 370.f));
    m_areaEstanteOtros = sf::FloatRect(sf::Vector2f(1042.f, 510.f), sf::Vector2f(190.f, 370.f));
    
    if (m_cocinaMinigame) {
        m_cocinaMinigame->setAreas(m_areaCocina, m_areaEntrega, m_areaMenuPlato);
        
        m_cocinaMinigame->getMiniGame()->actualizarAreaEstante("Carnes", m_areaEstanteCarnes);
        m_cocinaMinigame->getMiniGame()->actualizarAreaEstante("Verduras", m_areaEstanteVerduras);
        m_cocinaMinigame->getMiniGame()->actualizarAreaEstante("Lacteos y Embutidos", m_areaEstanteOtros);
    }
}
void Centinela2State::configurarBloqueAscensor()
{
    m_bloquesInteractivos.clear();
    
    m_bloquesInteractivos.push_back({
        sf::FloatRect(sf::Vector2f(254.f, 300.f), sf::Vector2f(100.f, 100.f)),
        "\n\n"
        "Este ascensor te llevara de regreso...\n\n"
        "Pero antes debes completar tu mision.\n"
        "Prepara 6 platos para los alienigenas.\n\n"
        "Usa los estantes de ingredientes (Carnes, Verduras, Lacteos)\n"
        "Cocina en la zona fuego\n"
        "Revisa el plato requerido en la zona de recepcion\n"
        "Entrega el plato a los jueces\n"
    
    });
}

void Centinela2State::actualizarPlatoRequerido()
{
    if (m_cocinaMinigame && m_cocinaMinigame->getMiniGame()) {
        m_cocinaMinigame->getMiniGame()->reiniciarMision();
        auto* miniGame = m_cocinaMinigame->getMiniGame();
        std::string platoNombre = miniGame->getPlatoRequeridoNombre();
        auto ingredientes = miniGame->getIngredientesRequeridos();
        
        std::string textoIngredientes = "Ingredientes: ";
        for (size_t i = 0; i < ingredientes.size(); i++) {
            if (i > 0) textoIngredientes += ", ";
            textoIngredientes += ingredientes[i];
        }
        
        std::string mensaje = "NUEVO PLATO: " + platoNombre + "\n\n" + textoIngredientes;
        mostrarMensajeFlotante(mensaje, 4.0f, sf::Color::Yellow);
    }
}

void Centinela2State::actualizarTimer(float dt)
{
    if (!m_juegoActivo || m_nivelCompletado || m_gameOver) return;
    
    m_tiempoRestante -= dt;
    
    float porcentaje = std::max(0.0f, m_tiempoRestante / 180.0f);
    m_timerBarra.setSize(sf::Vector2f(300.f * porcentaje, 20.f));
    
    if (porcentaje > 0.5f) m_timerBarra.setFillColor(sf::Color::Green);
    else if (porcentaje > 0.25f) m_timerBarra.setFillColor(sf::Color::Yellow);
    else m_timerBarra.setFillColor(sf::Color::Red);
    
    if (m_tiempoRestante <= 0.0f && !m_gameOver && !m_nivelCompletado) {
        m_tiempoRestante = 0.0f;
        m_gameOver = true;
        m_juegoActivo = false;
        
        // Usar el nuevo sistema de game over
        cargarGameOver();
    }
}

void Centinela2State::verificarEntregaPlato()
{
    if (!m_juegoActivo || m_nivelCompletado || m_gameOver) return;
    
    MiniGameCook* miniGame = m_cocinaMinigame->getMiniGame();
    if (!miniGame) return;
    
    if (miniGame->isMisionCompletada()) {
        m_platosEntregados++;
        mostrarMensajeFlotante("Plato entregado! " + std::to_string(m_platosEntregados) + "/" + std::to_string(m_platosRequeridos), 2.0f, sf::Color::Green);
        
        if (m_platosEntregados >= m_platosRequeridos) {
            m_nivelCompletado = true;
            m_juegoActivo = true;
            
            // Actualizar el mensaje del bloque para mostrar victoria
            if (!m_bloquesInteractivos.empty()) {
                m_bloquesInteractivos[0].mensaje = 
                    "VICTORIA\n\n"
                    "Has preparado los 6 platos!\n"
                    "Los alienigenas estan impresionados.\n\n"
                    "Ahora tienes una decision que tomar:\n"
                    "Escaparas del laboratorio?\n"
                    "O te uniras a sus experimentos humanos?\n\n"
                    "Presiona F para tomar tu decision.";
            }
            
            mostrarMensajeFlotante("MISION COMPLETADA! Ve al ascensor y presiona F", 3.0f, sf::Color::Green);
            return;
        }
        
        miniGame->reiniciarMision();
        actualizarPlatoRequerido();
    }
}

void Centinela2State::mostrarMensaje(const std::string &texto, float duracion, sf::Color color)
{
    if (!m_textoMensaje) return;
    m_msjActual.texto = texto;
    m_msjActual.tiempoRestante = duracion;
    m_msjActual.color = color;
    m_textoMensaje->setString(texto);
    m_textoMensaje->setFillColor(color);
    
    sf::FloatRect bounds = m_textoMensaje->getLocalBounds();
    m_textoMensaje->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
}

void Centinela2State::mostrarMensajeFlotante(const std::string& texto, float duracion, sf::Color color)
{
    if (!m_mensajeFlotante) return;
    m_mensajeFlotante->setString(texto);
    m_mensajeFlotante->setFillColor(color);
    sf::FloatRect bounds = m_mensajeFlotante->getLocalBounds();
    m_mensajeFlotante->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    m_tiempoFlotante = duracion;
    m_clockFlotante.restart();
}

void Centinela2State::mostrarDialogoDecision()
{
    m_dialogoDecisionActivo = true;
    m_opcionSeleccionada = 0;
    
    if (!m_fontLoaded) return;
    
    m_textoDialogoDecision = std::make_unique<sf::Text>(m_font);
    m_textoDialogoDecision->setString(
        "         === DECISION FINAL ===\n\n"
        "Has demostrado tu valia ante los alienigenas.\n\n"
        "Ahora tienes una decision crucial que tomar:"
    );
    m_textoDialogoDecision->setCharacterSize(20);
    m_textoDialogoDecision->setFillColor(sf::Color::White);
    m_textoDialogoDecision->setOutlineThickness(1.5f);
    m_textoDialogoDecision->setOutlineColor(sf::Color::Black);
    m_textoDialogoDecision->setStyle(sf::Text::Bold);
    
    m_textoOpcion1 = std::make_unique<sf::Text>(m_font);
    m_textoOpcion1->setString("    > ESCAPAR DEL LABORATORIO <");
    m_textoOpcion1->setCharacterSize(24);
    m_textoOpcion1->setFillColor(sf::Color::Yellow);
    m_textoOpcion1->setOutlineThickness(1.5f);
    m_textoOpcion1->setOutlineColor(sf::Color::Black);
    
    m_textoOpcion2 = std::make_unique<sf::Text>(m_font);
    m_textoOpcion2->setString("    > UNIRSE A EXPERIMENTOS HUMANOS < ");
    m_textoOpcion2->setCharacterSize(24);
    m_textoOpcion2->setFillColor(sf::Color(150, 150, 150));
    m_textoOpcion2->setOutlineThickness(1.5f);
    m_textoOpcion2->setOutlineColor(sf::Color::Black);
}

void Centinela2State::handleDecisionInput()
{
    static bool upPressed = false;
    static bool downPressed = false;
    static bool enterPressed = false;
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up)) {
        if (!upPressed) {
            upPressed = true;
            m_opcionSeleccionada = 0;
            // Actualizar colores visuales
            if (m_textoOpcion1) m_textoOpcion1->setFillColor(sf::Color::Yellow);
            if (m_textoOpcion2) m_textoOpcion2->setFillColor(sf::Color(150, 150, 150));
        }
    } else {
        upPressed = false;
    }
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)) {
        if (!downPressed) {
            downPressed = true;
            m_opcionSeleccionada = 1;
            // Actualizar colores visuales
            if (m_textoOpcion1) m_textoOpcion1->setFillColor(sf::Color(150, 150, 150));
            if (m_textoOpcion2) m_textoOpcion2->setFillColor(sf::Color::Magenta);
        }
    } else {
        downPressed = false;
    }
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Enter)) {
        if (!enterPressed) {
            enterPressed = true;
            m_dialogoDecisionActivo = false;
            
            LevelTree& levelTree = game->getLevelTree();
            
            if (m_opcionSeleccionada == 0) {
                // FINAL BUENO
                if (levelTree.jumpToNode("final_bueno_Centinela2")) {
                    std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                    if (newState) {
                        game->changeState(std::move(newState));
                    }
                }
            } else {
                // FINAL MALO
                if (levelTree.jumpToNode("final_malo_Centinela2")) {
                    std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                    if (newState) {
                        game->changeState(std::move(newState));
                    }
                }
            }
        }
    } else {
        enterPressed = false;
    }
}
void Centinela2State::handleEvent(const sf::Event& event)
{
    // F3 para debug
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::F3) {
            m_debugMode = !m_debugMode;
            CoordenadasDebug::getInstance().setVisible(m_debugMode);
        }
    }
    //  ADMIN MODE: TECLA E PARA VOLVER 
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::E) {
            if (game->isAdminMode() && !m_estanteAbierto && !m_mensajeEmergenteActivo && !m_dialogoDecisionActivo) {
                mostrarMensajeFlotante(" Regresando al nivel anterior...", 2.0f, sf::Color::Cyan);
                game->adminVolverAlNivelAnterior();
                return;
            }
        }
    }
    if (!m_activo) return;
    
    // Diálogo de decisión final
    if (m_dialogoDecisionActivo) {
        handleDecisionInput();
        return;
    }
    
    //  ABRIR ESTANTE 
    if (!m_estanteCerca.empty() && !m_estanteAbierto && !m_mensajeEmergenteActivo && !m_dialogoDecisionActivo) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::F) {
                m_estanteActualNombre = m_estanteCerca;
                m_estanteAbierto = true;
                m_mensajeEmergenteActivo = true;
                
                // Cargar ingredientes CON sus rutas de imagen correctas
                m_ingredientesEstanteActual.clear();
                
                if (m_estanteCerca == "Carnes") {
                    m_ingredientesEstanteActual = {
                        {"Carne de Res", "carne", sf::Color(139, 69, 19), "assets/images/niveles/centinela2/carne.png"},
                        {"Pollo", "carne", sf::Color(255, 228, 196), "assets/images/niveles/centinela2/pollo.png"},
                        {"Cerdo", "carne", sf::Color(255, 182, 193), "assets/images/niveles/centinela2/cerdo.png"},
                        {"Cordero", "carne", sf::Color(160, 82, 45), "assets/images/niveles/centinela2/cordero.png"}
                    };
                } else if (m_estanteCerca == "Verduras") {
                    m_ingredientesEstanteActual = {
                        {"Tomate", "verdura", sf::Color(255, 99, 71), "assets/images/niveles/centinela2/tomate.png"},
                        {"Lechuga", "verdura", sf::Color(124, 252, 0), "assets/images/niveles/centinela2/lechuga.png"},
                        {"Cebolla", "verdura", sf::Color(255, 215, 0), "assets/images/niveles/centinela2/cebolla.png"},
                        {"Zanahoria", "verdura", sf::Color(255, 140, 0), "assets/images/niveles/centinela2/zanahoria.png"},
                        {"Papa", "verdura", sf::Color(222, 184, 135), "assets/images/niveles/centinela2/papa.png"}
                    };
                } else if (m_estanteCerca == "Lacteos y Embutidos") {
                    m_ingredientesEstanteActual = {
                        {"Huevo", "otros", sf::Color(255, 255, 200), "assets/images/niveles/centinela2/huevo.png"},
                        {"Leche", "otros", sf::Color(255, 255, 255), "assets/images/niveles/centinela2/leche.png"},
                        {"Queso", "otros", sf::Color(255, 215, 0), "assets/images/niveles/centinela2/queso.png"},
                        {"Jamon", "otros", sf::Color(255, 182, 193), "assets/images/niveles/centinela2/jamon.png"},
                        {"Salchicha", "otros", sf::Color(205, 92, 92), "assets/images/niveles/centinela2/salchicha.png"},
                        {"Tocino", "otros", sf::Color(165, 42, 42), "assets/images/niveles/centinela2/tocino.png"}
                    };
                }
                return;
            }
        }
    }
    
    //  ESTANTE ABIERTO (prioridad) 
    if (m_estanteAbierto) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            
            // F Cerrar estante
            if (keyPressed->code == sf::Keyboard::Key::F) {
                m_estanteAbierto = false;
                m_mensajeEmergenteActivo = false;
                return;
            }
            
            // Procesar teclas numéricas para tomar ingredientes
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
            
            if (numSeleccionado >= 0 && numSeleccionado < (int)m_ingredientesEstanteActual.size()) {
                const auto& ing = m_ingredientesEstanteActual[numSeleccionado];
                Inventory* inv = m_player.getInventory();
                
                if (inv) {
                    if (inv->tryCollectItem(ing.nombre, ing.color)) {
                        // Buscar el item recién agregado y asignarle la textura
                        for (int i = 0; i < 15; i++) {
                            Item* item = inv->getItem(i);
                            if (item && item->name == ing.nombre && !item->sprite) {
                                item->textura = std::make_shared<sf::Texture>();
                                if (item->textura->loadFromFile(ing.rutaImagen)) {
                                    item->sprite = std::make_unique<sf::Sprite>(*item->textura);
                                }
                                break;
                            }
                        }
                    } else {
                        mostrarMensajeFlotante("Inventario lleno!", 2.0f, sf::Color::Red);
                    }
                }
            }
        }
        return;
    }
    
    //  DIÁLOGO DEL ASCENSOR 
    if (m_mensajeEmergenteActivo) {
        if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            
            if (m_nivelCompletado || m_gameOver) {
                // Modo victoria/derrota: F ejecuta la acción
                if (keyPressed->code == sf::Keyboard::Key::F) {
                    m_mensajeEmergenteActivo = false;
                    
                    if (m_nivelCompletado) {
                        mostrarDialogoDecision();
                    } else if (m_gameOver) {
                        mostrarMensajeFlotante("Regresando al nivel anterior...", 2.0f, sf::Color::Yellow);
                        game->volverDeCentinela();
                    }
                    m_bloqueActualIndex = -1;
                    return;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    m_mensajeEmergenteActivo = false;
                    m_bloqueActualIndex = -1;
                    return;
                }
            } else {
                if (keyPressed->code == sf::Keyboard::Key::F) {
                    m_mensajeEmergenteActivo = false;
                    m_bloqueActualIndex = -1;
                    m_tiempoBloqueoAscensor = 0.3f;
                    return;
                }
            }
        }
        return; 
    }
    //  MENU/RECETARIO 
    if (m_cercaMenu && !m_nivelCompletado && !m_gameOver) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::F) {
                if (m_cocinaMinigame && m_cocinaMinigame->getMiniGame()) {
                    auto* miniGame = m_cocinaMinigame->getMiniGame();
                    std::string platoReq = miniGame->getPlatoRequeridoNombre();
                    std::string descripcion = miniGame->getPlatoRequeridoDescripcion();
                    auto ingredientes = miniGame->getIngredientesRequeridos();
                    
                    std::string textoIngredientes = "Ingredientes: ";
                    for (size_t i = 0; i < ingredientes.size(); i++) {
                        if (i > 0) textoIngredientes += ", ";
                        textoIngredientes += ingredientes[i];
                    }
                    
                    std::string mensaje = "=== PLATO REQUERIDO ===\n\n" + platoReq + "\n\n" + textoIngredientes + "\n\n" + descripcion + "\n\n[REGRESA AQUI SI OLVIDASTE UN INGREDIENTE]";
                    mostrarMensajeFlotante(mensaje, 5.0f, sf::Color::Yellow);
                }
                return;
            }
        }
    }
    
    //  INVENTARIO 
    Inventory* inv = m_player.getInventory();
    if (inv) inv->handleEvent(event, *window);
    
    //  MINIJUEGO DE COCINA 
    if (m_cocinaMinigame && !m_nivelCompletado && !m_gameOver && !m_estanteAbierto) {
        m_cocinaMinigame->handleEvent(event, *window, m_player.getPosition());
    }
}
void Centinela2State::update(float dt)
{
    // Actualizar mensajes
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f) {
        m_msjActual.tiempoRestante -= dt;
        if (m_msjActual.tiempoRestante <= 0.0f) m_textoMensaje->setString("");
    }
    if (m_estanteAbierto) {
        return;  
    }
    if (m_tiempoFlotante > 0.0f) {
        m_tiempoFlotante -= dt;
    }
    
    if (m_dialogoDecisionActivo) return;
    
    if (!m_juegoActivo && !m_nivelCompletado && !m_gameOver) return;
    
    // Actualizar detección de zonas
    sf::Vector2f playerPos = m_player.getPosition();
    m_cercaCocina = m_cocinaMinigame->getMiniGame()->estaCercaCocina(playerPos);
    m_cercaEntrega = m_cocinaMinigame->getMiniGame()->estaCercaEntrega(playerPos);
    m_cercaMenu = m_cocinaMinigame->getMiniGame()->estaCercaMenu(playerPos);
    m_cercaCentinela = m_player.getHurtbox().findIntersection(m_centinelaArea).has_value();
    m_estanteCerca = m_cocinaMinigame->getMiniGame()->getEstanteCerca(playerPos);
  
    // Detectar bloque interactivo (ascensor)
    m_cercaBloqueInteractivo = false;
    int bloqueIndex = -1;
    for (size_t i = 0; i < m_bloquesInteractivos.size(); i++) {
        if (m_player.getHurtbox().findIntersection(m_bloquesInteractivos[i].area).has_value()) {
            m_cercaBloqueInteractivo = true;
            bloqueIndex = i;
            break;
        }
    }
    
    // Actualizar mensaje de interacción contextual (se dibuja en draw)
   static bool rPresionado = false;

    // Decrementar temporizador de bloqueo
    if (m_tiempoBloqueoAscensor > 0.0f) {
        m_tiempoBloqueoAscensor -= dt;
    }

    if (m_cercaBloqueInteractivo && bloqueIndex != -1 && !m_mensajeEmergenteActivo && !m_dialogoDecisionActivo && m_tiempoBloqueoAscensor <= 0.0f) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
            if (!rPresionado) {
                rPresionado = true;
                m_mensajeEmergenteActivo = true;
                m_bloqueActualIndex = bloqueIndex;
            }
        } else {
            rPresionado = false;
        }
    } else {
        rPresionado = false;
    }
    
    // Movimiento del jugador (solo si no hay diálogo activo)
    if (!m_mensajeEmergenteActivo && !m_dialogoDecisionActivo) {
        sf::Vector2f posAnterior = m_player.getPosition();
        Inventory *inv = m_player.getInventory();
        if (!inv || !inv->isOpen()) {
            sf::Vector2f movimiento(0.f, 0.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) movimiento.y -= 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) movimiento.y += 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) movimiento.x -= 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) movimiento.x += 1.f;
            
            if (movimiento.x != 0.f || movimiento.y != 0.f) {
                float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
                movimiento /= length;
            }
            m_player.move(movimiento, dt);
        }
        
        m_player.update(dt);
        
        // Colisiones
        for (const auto &obj : m_mapaFisico) {
            if (m_player.getHurtbox().findIntersection(obj.getBounds()).has_value()) {
                m_player.setPosition(posAnterior.x, posAnterior.y);
                break;
            }
        }
        
        // Actualizar minijuego
        if (m_cocinaMinigame && !m_nivelCompletado && !m_gameOver) {
            m_cocinaMinigame->update(dt, playerPos);
            verificarEntregaPlato();
        }
        
        // Interacción con centinela (recordar plato)
        static bool rCentinelaPresionado = false;
        if (m_cercaCentinela && !m_nivelCompletado && !m_gameOver) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
                if (!rCentinelaPresionado) {
                    rCentinelaPresionado = true;
                    if (m_cocinaMinigame) {
                        std::string platoReq = m_cocinaMinigame->getMiniGame()->getPlatoRequeridoNombre();
                        mostrarMensajeFlotante("Necesitas cocinar: " + platoReq, 3.0f, sf::Color::Yellow);
                    }
                }
            } else rCentinelaPresionado = false;
        }
        
        actualizarTimer(dt);
        
        // Cámara
        float halfWidth = 640.f, halfHeight = 360.f;
        sf::Vector2f cameraPos = playerPos;
        if (cameraPos.x < halfWidth) cameraPos.x = halfWidth;
        if (cameraPos.x > m_worldSize.x - halfWidth) cameraPos.x = m_worldSize.x - halfWidth;
        if (cameraPos.y < halfHeight) cameraPos.y = halfHeight;
        if (cameraPos.y > m_worldSize.y - halfHeight) cameraPos.y = m_worldSize.y - halfHeight;
        m_camera.setCenter(cameraPos);
        
        if (m_debugMode) CoordenadasDebug::getInstance().actualizar(window, m_camera, playerPos);
        
        // Pausa
        static bool escapeProcesado = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
            if (!escapeProcesado && !m_mensajeEmergenteActivo && !m_dialogoDecisionActivo) {
                escapeProcesado = true;
                game->pushState(std::make_unique<PauseState>(window, game));
            }
        } else {
            escapeProcesado = false;
        }
    }
}

void Centinela2State::draw()
{
    if (!window) return;
    
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    
    // Dibujar el mundo con la cámara
    window->setView(m_camera);
    
    if (m_background) window->draw(*m_background);
    else {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(50, 30, 50));
        window->draw(fallback);
    }
    
    m_player.draw(*window);
    
    // Áreas interactivas (solo debug)
    if (m_debugMode) {

        sf::RectangleShape estanteCarnes(sf::Vector2f(m_areaEstanteCarnes.size.x, m_areaEstanteCarnes.size.y));
        estanteCarnes.setPosition(m_areaEstanteCarnes.position);
        estanteCarnes.setFillColor(sf::Color(139, 0, 0, 100));
        estanteCarnes.setOutlineThickness(2.f);
        estanteCarnes.setOutlineColor(sf::Color::Red);
        window->draw(estanteCarnes);
        
        sf::RectangleShape estanteVerduras(sf::Vector2f(m_areaEstanteVerduras.size.x, m_areaEstanteVerduras.size.y));
        estanteVerduras.setPosition(m_areaEstanteVerduras.position);
        estanteVerduras.setFillColor(sf::Color(0, 100, 0, 100));
        estanteVerduras.setOutlineThickness(2.f);
        estanteVerduras.setOutlineColor(sf::Color::Green);
        window->draw(estanteVerduras);
        
        sf::RectangleShape estanteOtros(sf::Vector2f(m_areaEstanteOtros.size.x, m_areaEstanteOtros.size.y));
        estanteOtros.setPosition(m_areaEstanteOtros.position);
        estanteOtros.setFillColor(sf::Color(139, 139, 0, 100));
        estanteOtros.setOutlineThickness(2.f);
        estanteOtros.setOutlineColor(sf::Color::Yellow);
        window->draw(estanteOtros);
        
        sf::RectangleShape cocinaArea(sf::Vector2f(m_areaCocina.size.x, m_areaCocina.size.y));
        cocinaArea.setPosition(m_areaCocina.position);
        cocinaArea.setFillColor(sf::Color(0, 200, 0, 100));
        cocinaArea.setOutlineThickness(2.f);
        cocinaArea.setOutlineColor(sf::Color::Green);
        window->draw(cocinaArea);
        
        sf::RectangleShape entregaArea(sf::Vector2f(m_areaEntrega.size.x, m_areaEntrega.size.y));
        entregaArea.setPosition(m_areaEntrega.position);
        entregaArea.setFillColor(sf::Color(0, 0, 200, 100));
        entregaArea.setOutlineThickness(2.f);
        entregaArea.setOutlineColor(sf::Color::Blue);
        window->draw(entregaArea);
        
        sf::RectangleShape menuArea(sf::Vector2f(m_areaMenuPlato.size.x, m_areaMenuPlato.size.y));
        menuArea.setPosition(m_areaMenuPlato.position);
        menuArea.setFillColor(sf::Color(100, 100, 200, 100));
        menuArea.setOutlineThickness(2.f);
        menuArea.setOutlineColor(sf::Color::Yellow);
        window->draw(menuArea);
        
        for (const auto &bloque : m_bloquesInteractivos) {
            sf::RectangleShape ascensorRect(sf::Vector2f(bloque.area.size.x, bloque.area.size.y));
            ascensorRect.setPosition(bloque.area.position);
            ascensorRect.setFillColor(sf::Color(0, 255, 255, 100));
            ascensorRect.setOutlineThickness(2.f);
            ascensorRect.setOutlineColor(sf::Color::Yellow);
            window->draw(ascensorRect);
        }
        
        for (const auto &obj : m_mapaFisico) {
            sf::RectangleShape colision;
            colision.setPosition(obj.getBounds().position);
            colision.setSize(obj.getBounds().size);
            colision.setFillColor(sf::Color(255, 0, 0, 100));
            colision.setOutlineThickness(1.f);
            colision.setOutlineColor(sf::Color::Red);
            window->draw(colision);
        }
    }
    
    if (m_cocinaMinigame) m_cocinaMinigame->draw(*window);
    
    // Cambiar a vista por defecto para UI
    window->setView(window->getDefaultView());
    
    // UI superior (timer, contador, plato actual)
    if (m_juegoActivo && !m_nivelCompletado && !m_gameOver && m_fontLoaded) {
        sf::RectangleShape uiBg(sf::Vector2f(winW, 70.f));
        uiBg.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(uiBg);
        
        int minutos = static_cast<int>(m_tiempoRestante) / 60;
        int segundos = static_cast<int>(m_tiempoRestante) % 60;
        char buffer[10];
        snprintf(buffer, sizeof(buffer), "%02d:%02d", minutos, segundos);
        m_textoTimer->setString(buffer);
        m_textoTimer->setPosition(sf::Vector2f(20.f, 10.f));
        window->draw(*m_textoTimer);
        
        m_timerBarra.setPosition(sf::Vector2f(20.f, 45.f));
        window->draw(m_timerBarra);
        
        m_textoContador->setString("Platos: " + std::to_string(m_platosEntregados) + "/" + std::to_string(m_platosRequeridos));
        sf::FloatRect contadorBounds = m_textoContador->getLocalBounds();
        m_textoContador->setPosition(sf::Vector2f(winW - contadorBounds.size.x - 20.f, 15.f));
        window->draw(*m_textoContador);
        
        if (m_cocinaMinigame && m_cocinaMinigame->getMiniGame()) {
            std::string platoActual = m_cocinaMinigame->getMiniGame()->getPlatoRequeridoNombre();
            m_textoPlatoActual->setString("Siguiente plato: " + platoActual);
            sf::FloatRect bounds = m_textoPlatoActual->getLocalBounds();
            m_textoPlatoActual->setPosition(sf::Vector2f(winW / 2.f - bounds.size.x / 2.f, 15.f));
            window->draw(*m_textoPlatoActual);
        }
    }
    
    // Texto de interacción contextual 
    if (m_fontLoaded && m_textoInteraccion && m_juegoActivo && !m_nivelCompletado && !m_gameOver) {
        std::string textoActual;
        if (m_cercaCocina) textoActual = "COCINA - F para cocinar";
        else if (m_cercaEntrega) textoActual = "ENTREGA - F para entregar el plato";
        else if (m_cercaMenu) textoActual = "RECETARIO - F para ver plato requerido";
        else if (!m_estanteCerca.empty()) textoActual = m_estanteCerca + " - F para tomar";
        else if (m_cercaCentinela) textoActual = "CENTINELA - F para recordar el plato";
        else if (m_cercaBloqueInteractivo) textoActual = "ASCENSOR - F para interactuar";
        
        if (!textoActual.empty()) {
            m_textoInteraccion->setString(textoActual);
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 80.f));
            window->draw(*m_textoInteraccion);
        }
    } else if (m_fontLoaded && m_textoInteraccion && (m_nivelCompletado || m_gameOver) && m_cercaBloqueInteractivo) {
        std::string textoActual;
        if (m_nivelCompletado) textoActual = "ASCENSOR - F para tomar tu decision";
        else if (m_gameOver) textoActual = "ASCENSOR - E para regresar al nivel anterior";
        
        if (!textoActual.empty()) {
            m_textoInteraccion->setString(textoActual);
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 80.f));
            window->draw(*m_textoInteraccion);
        }
    }
    
    // Mensaje flotante centrado (con fondo)
    if (m_tiempoFlotante > 0.0f && m_mensajeFlotante && !m_mensajeFlotante->getString().isEmpty() && !m_mensajeEmergenteActivo)
    {
        sf::Vector2f posPantalla(winW / 2.f, winH / 2.f);
        
        sf::FloatRect textBounds = m_mensajeFlotante->getLocalBounds();
        
        sf::RectangleShape fondoRect(sf::Vector2f(textBounds.size.x + 40.f, textBounds.size.y + 30.f));
        fondoRect.setFillColor(sf::Color(0, 0, 0, 220));
        fondoRect.setOutlineColor(m_mensajeFlotante->getFillColor());
        fondoRect.setOutlineThickness(2.f);
        fondoRect.setOrigin(sf::Vector2f(fondoRect.getSize().x / 2.f, fondoRect.getSize().y / 2.f));
        fondoRect.setPosition(posPantalla);
        window->draw(fondoRect);
        
        m_mensajeFlotante->setPosition(posPantalla);
        window->draw(*m_mensajeFlotante);
    }
    
    if (m_debugMode) CoordenadasDebug::getInstance().dibujar(*window);
    
    // Mensaje temporal
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f && !m_textoMensaje->getString().isEmpty() && !m_mensajeEmergenteActivo) {
        m_textoMensaje->setPosition(sf::Vector2f(winW / 2.f, winH * 0.15f));
        window->draw(*m_textoMensaje);
    }
    
    // DIÁLOGO EMERGENTE DEL ASCENSOR 
    if (m_mensajeEmergenteActivo && m_bloqueActualIndex >= 0 && m_bloqueActualIndex < (int)m_bloquesInteractivos.size() && m_fontLoaded)
    {
        sf::RectangleShape overlay(sf::Vector2f(winW, winH));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);
        
        float dialogWidth = 700.f;
        float dialogHeight = 450.f;
        float dialogX = winW / 2.f - dialogWidth / 2.f;
        float dialogY = winH / 2.f - dialogHeight / 2.f;
        
        // Sombra
        sf::RectangleShape shadow(sf::Vector2f(dialogWidth + 8.f, dialogHeight + 8.f));
        shadow.setFillColor(sf::Color(0, 0, 0, 150));
        shadow.setPosition(sf::Vector2f(dialogX + 4.f, dialogY + 4.f));
        window->draw(shadow);
        
        // Caja de diálogo
        sf::RectangleShape dialogBox(sf::Vector2f(dialogWidth, dialogHeight));
        dialogBox.setFillColor(sf::Color(25, 25, 35, 245));
        dialogBox.setOutlineThickness(3.f);
        
        if (m_nivelCompletado)
            dialogBox.setOutlineColor(sf::Color(0, 255, 0, 255));
        else if (m_gameOver)
            dialogBox.setOutlineColor(sf::Color(255, 0, 0, 255));
        else
            dialogBox.setOutlineColor(sf::Color(0, 255, 255, 255));
            
        dialogBox.setPosition(sf::Vector2f(dialogX, dialogY));
        window->draw(dialogBox);
        
        // Barra superior
        sf::RectangleShape topBar(sf::Vector2f(dialogWidth, 50.f));
        
        if (m_nivelCompletado)
            topBar.setFillColor(sf::Color(0, 80, 0, 220));
        else if (m_gameOver)
            topBar.setFillColor(sf::Color(80, 0, 0, 220));
        else
            topBar.setFillColor(sf::Color(0, 80, 80, 220));
            
        topBar.setPosition(sf::Vector2f(dialogX, dialogY));
        window->draw(topBar);
        
        // Línea decorativa
        sf::RectangleShape accentLine(sf::Vector2f(dialogWidth - 40.f, 2.f));
        
        if (m_nivelCompletado)
            accentLine.setFillColor(sf::Color(0, 255, 0, 255));
        else if (m_gameOver)
            accentLine.setFillColor(sf::Color(255, 0, 0, 255));
        else
            accentLine.setFillColor(sf::Color(0, 255, 255, 255));
            
        accentLine.setPosition(sf::Vector2f(dialogX + 20.f, dialogY + 48.f));
        window->draw(accentLine);
        
        // Título
        sf::Text tituloText(m_font);
        if (m_nivelCompletado)
            tituloText.setString("=== VICTORIA ===");
        else if (m_gameOver)
            tituloText.setString("=== DERROTA ===");
        else
            tituloText.setString("=== ASCENSOR ===");
            
        tituloText.setCharacterSize(22);
        tituloText.setStyle(sf::Text::Bold);
        tituloText.setFillColor(sf::Color::White);
        sf::FloatRect tituloBounds = tituloText.getLocalBounds();
        tituloText.setOrigin(sf::Vector2f(tituloBounds.size.x / 2.f, tituloBounds.size.y / 2.f));
        tituloText.setPosition(sf::Vector2f(winW / 2.f, dialogY + 25.f));
        window->draw(tituloText);
        
        // Mensaje del bloque
        sf::Text mensajeText(m_font);
        mensajeText.setString(m_bloquesInteractivos[m_bloqueActualIndex].mensaje);
        mensajeText.setCharacterSize(16);
        mensajeText.setFillColor(sf::Color(240, 240, 255, 255));
        mensajeText.setOutlineColor(sf::Color(0, 0, 0, 150));
        mensajeText.setOutlineThickness(1.5f);
        mensajeText.setOrigin(sf::Vector2f(0.f, 0.f));
        mensajeText.setPosition(sf::Vector2f(dialogX + 40.f, dialogY + 80.f));
        window->draw(mensajeText);
        
        // Panel de instrucciones
        sf::RectangleShape instruccionPanel(sf::Vector2f(dialogWidth - 40.f, 65.f));
        instruccionPanel.setFillColor(sf::Color(15, 15, 25, 200));
        instruccionPanel.setOutlineThickness(1.f);
        instruccionPanel.setOutlineColor(sf::Color(100, 100, 120, 100));
        instruccionPanel.setPosition(sf::Vector2f(dialogX + 20.f, dialogY + dialogHeight - 85.f));
        window->draw(instruccionPanel);
        
        sf::Text instruccionText(m_font);
        instruccionText.setCharacterSize(16);
        instruccionText.setOutlineThickness(0.5f);
        instruccionText.setOutlineColor(sf::Color::Black);
        
       if (m_nivelCompletado)
            instruccionText.setString("[ F ] Tomar decision final     |     [ ESC ] Cerrar");
        else if (m_gameOver)
            instruccionText.setString("[ F ] Regresar al nivel anterior     |     [ ESC ] Cerrar");
        else
            instruccionText.setString("[ F ] Cerrar     |     Sigue cocinando para completar la mision");
                    
        instruccionText.setFillColor(sf::Color(200, 200, 100, 255));
        sf::FloatRect instrBounds = instruccionText.getLocalBounds();
        instruccionText.setOrigin(sf::Vector2f(instrBounds.size.x / 2.f, instrBounds.size.y / 2.f));
        instruccionText.setPosition(sf::Vector2f(winW / 2.f, dialogY + dialogHeight - 52.f));
        window->draw(instruccionText);
        
        // Esquinas decorativas
        sf::CircleShape cornerTL(6.f, 4);
        cornerTL.setFillColor(sf::Color(200, 150, 100, 180));
        cornerTL.setPosition(sf::Vector2f(dialogX + 3.f, dialogY + 3.f));
        window->draw(cornerTL);
        
        sf::CircleShape cornerTR(6.f, 4);
        cornerTR.setFillColor(sf::Color(200, 150, 100, 180));
        cornerTR.setPosition(sf::Vector2f(dialogX + dialogWidth - 9.f, dialogY + 3.f));
        window->draw(cornerTR);
        
        sf::CircleShape cornerBL(6.f, 4);
        cornerBL.setFillColor(sf::Color(200, 150, 100, 180));
        cornerBL.setPosition(sf::Vector2f(dialogX + 3.f, dialogY + dialogHeight - 9.f));
        window->draw(cornerBL);
        
        sf::CircleShape cornerBR(6.f, 4);
        cornerBR.setFillColor(sf::Color(200, 150, 100, 180));
        cornerBR.setPosition(sf::Vector2f(dialogX + dialogWidth - 9.f, dialogY + dialogHeight - 9.f));
        window->draw(cornerBR);
    }
        // DIÁLOGO DEL ESTANTE 
    if (m_estanteAbierto && m_fontLoaded)
    {
        sf::RectangleShape overlay(sf::Vector2f(winW, winH));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);
        
        float dialogWidth = 550.f;
        float dialogHeight = 450.f;
        float dialogX = winW / 2.f - dialogWidth / 2.f;
        float dialogY = winH / 2.f - dialogHeight / 2.f;
        
        // Sombra
        sf::RectangleShape shadow(sf::Vector2f(dialogWidth + 8.f, dialogHeight + 8.f));
        shadow.setFillColor(sf::Color(0, 0, 0, 150));
        shadow.setPosition(sf::Vector2f(dialogX + 4.f, dialogY + 4.f));
        window->draw(shadow);
        
        // Caja de diálogo
        sf::RectangleShape dialogBox(sf::Vector2f(dialogWidth, dialogHeight));
        dialogBox.setFillColor(sf::Color(25, 25, 35, 245));
        dialogBox.setOutlineThickness(3.f);
        dialogBox.setOutlineColor(sf::Color(200, 150, 100, 255));
        dialogBox.setPosition(sf::Vector2f(dialogX, dialogY));
        window->draw(dialogBox);
        
        // Título
        sf::Text tituloText(m_font);
        tituloText.setString("=== " + m_estanteActualNombre + " ===");
        tituloText.setCharacterSize(28);
        tituloText.setStyle(sf::Text::Bold);
        tituloText.setFillColor(sf::Color(255, 215, 0));
        sf::FloatRect tituloBounds = tituloText.getLocalBounds();
        tituloText.setOrigin(sf::Vector2f(tituloBounds.size.x / 2.f, tituloBounds.size.y / 2.f));
        tituloText.setPosition(sf::Vector2f(winW / 2.f, dialogY + 35.f));
        window->draw(tituloText);
        
        // Línea decorativa
        sf::RectangleShape accentLine(sf::Vector2f(dialogWidth - 40.f, 2.f));
        accentLine.setFillColor(sf::Color(200, 150, 100, 255));
        accentLine.setPosition(sf::Vector2f(dialogX + 20.f, dialogY + 60.f));
        window->draw(accentLine);
        
        // Lista de ingredientes
        std::string contenido;
        for (size_t i = 0; i < m_ingredientesEstanteActual.size() && i < 9; i++) {
            contenido += std::to_string(i + 1) + ". " + m_ingredientesEstanteActual[i].nombre + "\n";
        }
        contenido += "\n[ESC] Cerrar";
        
        sf::Text mensajeText(m_font);
        mensajeText.setString(contenido);
        mensajeText.setCharacterSize(20);
        mensajeText.setFillColor(sf::Color(240, 240, 255, 255));
        mensajeText.setOutlineColor(sf::Color(0, 0, 0, 150));
        mensajeText.setOutlineThickness(1.5f);
        mensajeText.setPosition(sf::Vector2f(dialogX + 50.f, dialogY + 90.f));
        window->draw(mensajeText);
        
        // Instrucciones
        sf::Text instruccionText(m_font);
        instruccionText.setString("Presionar [1-9] para tomar ingrediente |     [ F ] Cerrar");
        instruccionText.setCharacterSize(16);
        instruccionText.setFillColor(sf::Color(200, 200, 100, 255));
        sf::FloatRect instrBounds = instruccionText.getLocalBounds();
        instruccionText.setOrigin(sf::Vector2f(instrBounds.size.x / 2.f, instrBounds.size.y / 2.f));
        instruccionText.setPosition(sf::Vector2f(winW / 2.f, dialogY + dialogHeight - 40.f));
        window->draw(instruccionText);
    }
    // Diálogo de decisión final
    if (m_dialogoDecisionActivo && m_fontLoaded) {
        sf::RectangleShape overlay(sf::Vector2f(winW, winH));
        overlay.setFillColor(sf::Color(0, 0, 0, 220));
        window->draw(overlay);
        
        float dialogWidth = 700.f;
        float dialogHeight = 400.f;
        float dialogX = winW / 2.f - dialogWidth / 2.f;
        float dialogY = winH / 2.f - dialogHeight / 2.f;
        
        sf::RectangleShape dialogBox(sf::Vector2f(dialogWidth, dialogHeight));
        dialogBox.setFillColor(sf::Color(25, 25, 35, 245));
        dialogBox.setOutlineThickness(3.f);
        dialogBox.setOutlineColor(sf::Color(150, 100, 200, 255));
        dialogBox.setPosition(sf::Vector2f(dialogX, dialogY));
        window->draw(dialogBox);
        
        m_textoDialogoDecision->setOrigin(sf::Vector2f(0.f, 0.f));
        m_textoDialogoDecision->setPosition(sf::Vector2f(dialogX + 50.f, dialogY + 60.f));
        window->draw(*m_textoDialogoDecision);
        
        if (m_opcionSeleccionada == 0) {
            m_textoOpcion1->setFillColor(sf::Color::Yellow);
            m_textoOpcion1->setString("> ESCAPAR DEL LABORATORIO <");
        } else {
            m_textoOpcion1->setFillColor(sf::Color(150, 150, 150));
            m_textoOpcion1->setString("ESCAPAR DEL LABORATORIO");
        }
        m_textoOpcion1->setOrigin(sf::Vector2f(0.f, 0.f));
        m_textoOpcion1->setPosition(sf::Vector2f(dialogX + 80.f, dialogY + 200.f));
        window->draw(*m_textoOpcion1);
        
        if (m_opcionSeleccionada == 1) {
            m_textoOpcion2->setFillColor(sf::Color::Magenta);
            m_textoOpcion2->setString("> UNIRSE A EXPERIMENTOS HUMANOS <");
        } else {
            m_textoOpcion2->setFillColor(sf::Color(150, 150, 150));
            m_textoOpcion2->setString("UNIRSE A EXPERIMENTOS HUMANOS");
        }
        m_textoOpcion2->setOrigin(sf::Vector2f(0.f, 0.f));
        m_textoOpcion2->setPosition(sf::Vector2f(dialogX + 80.f, dialogY + 270.f));
        window->draw(*m_textoOpcion2);
        
        sf::Text instrucciones(m_font);
        instrucciones.setString("FLECHAS  [Up][Down] |  ENTER para seleccionar");
        instrucciones.setCharacterSize(16);
        instrucciones.setFillColor(sf::Color(150, 150, 150));
        instrucciones.setOrigin(sf::Vector2f(instrucciones.getLocalBounds().size.x / 2.f, 0.f));
        instrucciones.setPosition(sf::Vector2f(winW / 2.f, dialogY + dialogHeight - 40.f));
        window->draw(instrucciones);
    }
    
    // Inventario
    Inventory *inv = m_player.getInventory();
    if (inv) inv->draw(*window);
    
    if (m_debugMode) CoordenadasDebug::getInstance().dibujar(*window);
}
void Game::adminVolverAlNivelAnterior()
{
    LevelNode* currentNode = levelTree.getCurrentNode();
    if (!currentNode) return;
    
    std::string nivelAnteriorId;
    
    // Mapeo manual de centinelas a sus niveles anteriores
    if (currentNode->id == "centinela2") {
        nivelAnteriorId = "nivel6";
    } else if (currentNode->id == "centinela1") {
        nivelAnteriorId = "nivel3";
    } else if (currentNode->id == "centinela3") {
        nivelAnteriorId = "nivel7";
    } else {
        // Si no está mapeado, no hacer nada
        return;
    }
    
    if (levelTree.jumpToNode(nivelAnteriorId)) {
        auto newState = levelTree.createCurrentState(window.get(), this);
        if (newState) {
            changeState(std::move(newState));
        }
    }
}
void Centinela2State::cargarGameOver()
{
    window->setView(window->getDefaultView());
    
    bool modoSupervivencia = false;
    
    if (game->tienePartidaActiva()) {
        GameProgressData& progress = game->getSaveManager().getCurrentProgress();
        modoSupervivencia = (progress.modoElegido == GameProgressData::ModoJuego::CAMINO_CON_CONSECUENCIAS);
    }
    
    // MODO SUPERVIVENCIA: Ir directamente al final malo
    if (modoSupervivencia) {
        cargarFinalMalo();
    } 
    // MODO HISTORIA: Mostrar menú de opciones
    else {
        auto gameOverState = std::make_unique<CentinelaGameOverState>(
            window, game, false, "centinela2"
        );
        game->changeState(std::move(gameOverState));
    }
}

void Centinela2State::cargarFinalMalo()
{
    window->setView(window->getDefaultView());
    
    LevelTree& levelTree = game->getLevelTree();
    if (levelTree.jumpToNode("final_malo_centinela2")) {
        std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
        if (newState) {
            game->changeState(std::move(newState));
        }
    } else {
        std::cerr << "Error: No se pudo cargar final_malo_centinela2" << std::endl;
        game->returnToMenu();
    }
}

void Centinela2State::reiniciarCentinela()
{
    
    // Simplemente recrear el mismo estado del centinela
    auto nuevoEstado = std::make_unique<Centinela2State>(window, game);
    if (nuevoEstado) {
        game->changeState(std::move(nuevoEstado));
    } else {
        game->returnToMenu();
    }
}