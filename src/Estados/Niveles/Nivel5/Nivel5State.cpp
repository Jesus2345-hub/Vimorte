#include "Estados/Niveles/Nivel5/Nivel5State.hpp"
#include "Estados/PauseState.hpp"
#include "Estados/MuerteCentinelaState.hpp"
#include "Configuracion/CoordenadasDebug.hpp"
#include "Estados/GameOverState.hpp"
#include <cmath>
#include <algorithm>


// CONSTRUCTOR

Nivel5State::Nivel5State(sf::RenderWindow *window, Game *game)
    : State(window, game),
      m_background(nullptr),
      m_casoResuelto(false),
      m_nivelCompletado(false),
      m_setActualCaso(-1),
      m_textoInteraccion(nullptr),
      m_mostrarPuertaSalida(true),
      m_cercaPuertaSalida(false),
      m_escapeConsumed(false),
      m_mostrarTutorial(false),
      m_mostrarTutorialPorTecla(false),
      m_msjActual(),
      m_fontLoaded(false),
      m_tiempoFlotante(0.0f) ,
      m_mensajeFlotante(nullptr),
      m_estrellaUsada(false)
{
    m_cercaBloqueInteractivo = false;
    m_mensajeEmergenteActivo = false;
    m_bloqueActualIndex = -1;
    m_msjActual.texto = "";
    m_msjActual.tiempoRestante = 0.0f;
    m_msjActual.color = sf::Color::Yellow;

    m_player.loadAssets();
    m_player.setPosition(315.f, 435.f);
    m_player.setSpeed(300.0f);

    // Verificar si es la primera vez para mostrar el tutorial
    if (game->tienePartidaActiva())
    {
        const auto &items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
        auto it = std::find(items.begin(), items.end(), "TutorialNivel5Visto");

        if (it == items.end())
        {
            m_mostrarTutorial = true;
            game->getSaveManager().addItemRecolectado("TutorialNivel5Visto");
            std::cout << "Primer ingreso a Nivel5: Mostrando tutorial" << std::endl;
        }
    }

    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel5/background.png"))
    {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x),
                                   static_cast<float>(textureSize.y));
        std::cout << "Nivel5 cargado. Tamaño: " << m_worldSize.x << "x" << m_worldSize.y << std::endl;
    }
    else
    {
        std::cout << "Error: No se pudo cargar background.jpg para Nivel5" << std::endl;
        m_worldSize = sf::Vector2f(1754.f, 1587.f);
    }

    sf::Vector2u windowSize = window->getSize();

    // Cámara con tamaño
    float fixedWidth = 1280.f;
    float fixedHeight = 720.f;
    m_camera = sf::View(
        sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
        sf::Vector2f(fixedWidth, fixedHeight));
    m_lastWindowSize = windowSize;

    // Áreas de interacción
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(243.f, 264.f), sf::Vector2f(120.f, 180.f));

    configurarColisiones();
    configurarMinijuegoCriminal();
    configurarBloquesInteractivos();
    
    // Carga de fuente
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (!m_fontLoaded)
    {
        std::cout << "ERROR en Nivel5State: No se pudo cargar la fuente" << std::endl;
    }

    if (m_fontLoaded)
    {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setString("Pesiona F");
        m_textoInteraccion->setCharacterSize(20);
        m_textoInteraccion->setFillColor(sf::Color::White);
        m_textoInteraccion->setPosition(sf::Vector2f(windowSize.x / 2.f, windowSize.y - 70.f));

        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));

        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(28);  
        m_textoMensaje->setFillColor(sf::Color::Yellow);
        m_textoMensaje->setOutlineColor(sf::Color::Black); 
        m_textoMensaje->setOutlineThickness(2.0f);
        m_textoMensaje->setStyle(sf::Text::Bold);
        
        
        // INICIALIZAR MENSAJE FLOTANTE
        
        m_mensajeFlotante = std::make_unique<sf::Text>(m_font);
        m_mensajeFlotante->setCharacterSize(24);
        m_mensajeFlotante->setFillColor(sf::Color::Red);
        m_mensajeFlotante->setOutlineColor(sf::Color::Black);
        m_mensajeFlotante->setOutlineThickness(2.5f);
        m_mensajeFlotante->setStyle(sf::Text::Bold);
    }
    else
    {
        m_textoInteraccion = nullptr;
        m_textoMensaje = nullptr;
    }

    // Guardado automatico
    if (game->tienePartidaActiva())
    {
        game->getSaveManager().setNivelActual(5, 1);
        game->guardarPartidaActual();
        std::cout << "Partida guardada automáticamente en Nivel5" << std::endl;
    }

    std::cout << "Nivel5State inicializado correctamente" << std::endl;
    game->setIsInLevel(true);
    CoordenadasDebug::getInstance().setVisible(false);
    
    m_criminalMinigame.setOnCompleteCallback([this](bool exito) {
    if (exito && !m_criminalGameCompleted) 
    {
        m_criminalGameCompleted = true;
        m_setActualCaso = m_criminalMinigame.getSetActual();
        std::cout << "CASO RESUELTO! Set actual: " << m_setActualCaso << std::endl;
        mostrarMensaje("CASO RESUELTO. Has encontrado todas las pistas y al culpable.\nEntregale las cosas a Andrea", 5.0f);
    }
    else if (!exito)
    {
        m_setActualCaso = m_criminalMinigame.getSetActual();
        std::cout << "CASO FALLIDO! Nuevo caso generado. Set actual: " << m_setActualCaso << std::endl;
    }
    });

    m_criminalMinigame.setDebugMode(true);
    
  
}


// MOSTRAR MENSAJE FLOTANTE

void Nivel5State::mostrarMensajeFlotante(const std::string& texto, float duracion, sf::Color color)
{
    if (!m_fontLoaded) return;
    
    m_mensajeFlotante->setString(texto);
    m_mensajeFlotante->setFillColor(color);
    
    // Centrar el texto
    sf::FloatRect bounds = m_mensajeFlotante->getLocalBounds();
    m_mensajeFlotante->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    
    m_tiempoFlotante = duracion;
    m_clockFlotante.restart();
    
    std::cout << "MENSAJE FLOTANTE: " << texto << std::endl;
}


// REAJUSTAR MINIJUEGO (MANTENIENDO ESTADO)

void Nivel5State::reajustarMinijuegoCriminalManteniendoEstado()
{
    if (!m_criminalMinigame.isActive()) return;
    
    sf::Vector2u windowSize = window->getSize();
    float minijuegoW = windowSize.x * 0.85f;
    float minijuegoH = windowSize.y * 0.85f;
    float minijuegoX = (windowSize.x - minijuegoW) / 2.f;
    float minijuegoY = (windowSize.y - minijuegoH) / 2.f;
    
    m_criminalMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
    m_criminalMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    
    m_criminalMinigame.cargarFondoOnly("assets/images/niveles/nivel5/criminalCase.png");
}


// CONFIGURAR BLOQUES INTERACTIVOS

void Nivel5State::configurarBloquesInteractivos()
{
    m_bloquesInteractivos.clear();
    
    m_bloquesInteractivos.push_back({
        sf::FloatRect(sf::Vector2f(280.f, 904.f), sf::Vector2f(100.f, 100.f)),
        "Andrea esta desesperada.\nSus joyas mas preciosas fueron robadas...\nAndrea : Ayudame a encontrar al culpable, vi a alguien,\nmerodeando entre las sillas.\n\n[Presiona R para entregar los objetos si has resuelto el caso]"
    });
      m_bloquesInteractivos.push_back({
        sf::FloatRect(sf::Vector2f(1123.f, 1096.f), sf::Vector2f(50.f, 50.f)), 
        "Una estrella brillante te llama...\nTe atreves a seguirla?\n\n[Presiona F para ir al Centinela]"
    });

}


// MANEJAR EVENTOS

void Nivel5State::handleEvent(const sf::Event &event)
{
    
    // Prioridad: si hay mensaje emergente
    if (m_mensajeEmergenteActivo) {
        if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            
            // F Cerrar mensaje 
            if (keyPressed->code == sf::Keyboard::Key::F) {
                m_mensajeEmergenteActivo = false;
                m_bloqueActualIndex = -1;
                return;
            }
            
            //  BLOQUE ESTRELLA  - saltar al centinela 2
            if (m_bloqueActualIndex == 1 && !m_estrellaUsada) {
                // Con F  cerramos, con R teletransportamos
                if (keyPressed->code == sf::Keyboard::Key::R) {  
                    m_estrellaUsada = true;
                    m_mensajeEmergenteActivo = false;
                    
                    LevelTree& levelTree = game->getLevelTree();
                    if (levelTree.jumpToNode("centinela2")) {
                        std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                        if (newState) {
                            std::cout << "Saltando a Centinela 2 por la estrella" << std::endl;
                            game->changeState(std::move(newState));
                        }
                    }
                    return;
                }
            }
            
            //  BLOQUE ANDREA - Entregar objetos 
            if (m_bloqueActualIndex == 0) {
                if (keyPressed->code == sf::Keyboard::Key::R && !m_nivelCompletado) 
                {
                    if (m_criminalGameCompleted) 
                    {
                        Inventory* inv = m_player.getInventory();
                        bool tieneTodosLosObjetos = true;
                        
                        int casoCompletado = m_setActualCaso;
                        
                        if (casoCompletado >= 0 && casoCompletado < (int)m_todosLosObjetos.size()) 
                        {
                            std::cout << "Verificando objetos del caso " << casoCompletado << std::endl;
                            for (const auto& objRequerido : m_todosLosObjetos[casoCompletado]) 
                            {
                                bool encontrado = false;
                                if (inv) 
                                {
                                    for (int i = 0; i < 20; i++) 
                                    {
                                        Item* item = inv->getItem(i);
                                        if (item && !item->name.empty() && item->name == objRequerido.nombre) 
                                        {
                                            encontrado = true;
                                            std::cout << "o Encontrado: " << objRequerido.nombre << std::endl;
                                            break;
                                        }
                                    }
                                }
                                if (!encontrado) 
                                {
                                    std::cout << "X FALTA: " << objRequerido.nombre << std::endl;
                                    tieneTodosLosObjetos = false;
                                    break;  
                                }
                            }
                        } 
                        else 
                        {
                            std::cout << "ERROR: casoCompletado inválido: " << casoCompletado << std::endl;
                            tieneTodosLosObjetos = false;
                        }
                        
                        if (tieneTodosLosObjetos) 
                        {
                            if (inv && casoCompletado >= 0 && casoCompletado < (int)m_todosLosObjetos.size()) 
                            {
                                for (const auto& objRequerido : m_todosLosObjetos[casoCompletado]) 
                                {
                                    for (int i = 0; i < 20; i++) 
                                    {
                                        Item* item = inv->getItem(i);
                                        if (item && item->name == objRequerido.nombre) 
                                        {
                                            inv->removeItem(i);
                                            std::cout << "Objeto entregado y eliminado: " << objRequerido.nombre << std::endl;
                                            break;
                                        }
                                    }
                                }
                            }
                            
                            m_casoResuelto = true;
                            m_nivelCompletado = true;
                            
                            m_bloquesInteractivos[m_bloqueActualIndex].mensaje = 
                                "GRACIAS! Has recuperado todas mis joyas.\n"
                                "Eres un heroe...\n\n"
                                "Ahora dirigete al ASCENSOR\n"
                                "y presiona F para avanzar al siguiente nivel.";
                            
                            std::cout << "OBJETOS ENTREGADOS CORRECTAMENTE! Nivel completado." << std::endl;
                        } 
                        else 
                        {
                            m_mensajeEmergenteActivo = false;
                            mostrarMensajeFlotante("Aun no has encontrado todas las joyas. Sigue investigando\n en la escena del crimen.", 3.0f, sf::Color::Yellow);
                        }
                    } 
                    else 
                    {
                        m_mensajeEmergenteActivo = false;
                        mostrarMensajeFlotante("Aun no has resuelto el caso.\nInvestiga la escena del crimen y encuentra\ntodas las pistas y al culpable.", 3.0f, sf::Color::Yellow);
                    }
                }
            }
        }
        return;
    }

    // Teclas globales (solo si NO hay mensaje activo)
    if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>())
    {

        // M Abrir/Cerrar tutorial
        if (keyPressed->code == sf::Keyboard::Key::M)
        {
            if (m_mostrarTutorial || m_mostrarTutorialPorTecla) {
                m_mostrarTutorial = false;
                m_mostrarTutorialPorTecla = false;
            } else {
                std::cout << "M presionada - Activando tutorial" << std::endl;
                if (game->tienePartidaActiva())
                {
                    const auto &items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
                    auto it = std::find(items.begin(), items.end(), "TutorialNivel5Visto");
                    if (it != items.end())
                    {
                        m_mostrarTutorialPorTecla = true;
                    }
                    else
                    {
                        m_mostrarTutorial = true;
                    }
                }
                else
                {
                    m_mostrarTutorialPorTecla = true;
                }
            }
            return;
        }
        
        // ESCAPE Solo para pausa 
        if (keyPressed->code == sf::Keyboard::Key::Escape)
        {
            if (!m_mostrarTutorial && !m_mostrarTutorialPorTecla && !m_mensajeEmergenteActivo)
            {
                game->pushState(std::make_unique<PauseState>(window, game));
            }
            return;
        }
    }

    Inventory *inv = m_player.getInventory();
    if (inv)
    {
        inv->handleEvent(event, *window);
    }
    
    // Manejar eventos del minijuego criminal
    if (m_criminalMinigame.isActive()) {
        m_criminalMinigame.handleEvent(event, *window);
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::F) {
                m_criminalMinigame.deactivate();
                inv->clear();
                return;
            }
        }
        return;
    }
}
// CONFIGURAR MINIJUEGO CRIMINAL

void Nivel5State::configurarMinijuegoCriminal()
{
    m_criminalArea = sf::FloatRect(sf::Vector2f(800.f, 600.f), sf::Vector2f(150.f, 150.f));
    m_cercaCriminalArea = false;
    m_criminalGameCompleted = false;
    
    m_criminalMinigame.limpiarPools();
    
    m_todosLosObjetos.clear();
    m_todosLosSospechosos.clear();
    m_todosLosDialogos.clear();
    
    // Bloque 1
    std::vector<ObjetoBuscar> objetosBloque1;

    objetosBloque1.emplace_back("Collar", sf::FloatRect(sf::Vector2f(560.f, 426.f), sf::Vector2f(36.f, 40.f)), " ", "assets/images/niveles/nivel5/collar.png");
    objetosBloque1.emplace_back("Carta Mojada", sf::FloatRect(sf::Vector2f(191.f, 438.f), sf::Vector2f(75.f, 38.f)), " ", "assets/images/niveles/nivel5/cartaMojada.png");
    objetosBloque1.emplace_back("Reloj Arena", sf::FloatRect(sf::Vector2f(665.f, 357.f), sf::Vector2f(25.f, 70.f)), " ", "assets/images/niveles/nivel5/relojArena.png");  
    objetosBloque1.emplace_back("Medalla", sf::FloatRect(sf::Vector2f(116.f, 164.f), sf::Vector2f(20.f, 23.f)), " ", "assets/images/niveles/nivel5/medalla.png");
    objetosBloque1.emplace_back("Botella", sf::FloatRect(sf::Vector2f(104.f, 359.f), sf::Vector2f(25.f, 24.f)), " ", "assets/images/niveles/nivel5/botellaVidrio.png");
    objetosBloque1.emplace_back("Diario", sf::FloatRect(sf::Vector2f(524.f, 496.f), sf::Vector2f(35.f, 30.f)), " ", "assets/images/niveles/nivel5/diario.png");
    objetosBloque1.emplace_back("Anillo", sf::FloatRect(sf::Vector2f(504.f, 414.f), sf::Vector2f(21.f, 25.f)), " ", "assets/images/niveles/nivel5/anillo.png");
    objetosBloque1.emplace_back("Foto", sf::FloatRect(sf::Vector2f(744.f, 240.f), sf::Vector2f(34.f, 54.f)), " ", "assets/images/niveles/nivel5/foto.png");
    objetosBloque1.emplace_back("Cuchillo", sf::FloatRect(sf::Vector2f(348.f, 370.f), sf::Vector2f(36.f, 25.f)), " ", "assets/images/niveles/nivel5/cuchillo.png");
    objetosBloque1.emplace_back("Bolso", sf::FloatRect(sf::Vector2f(408.f, 473.f), sf::Vector2f(109.f, 80.f)), " ", "assets/images/niveles/nivel5/bolso.png"); 
    objetosBloque1.emplace_back("Trapo Viejo", sf::FloatRect(sf::Vector2f(766.f, 455.f), sf::Vector2f(26.f, 15.f)), " ", "assets/images/niveles/nivel5/trapoViejo.png");

    
    std::vector<Sospechoso> sospechososBloque1;
    sospechososBloque1.emplace_back("Capitan Rodrigo", sf::FloatRect(sf::Vector2f(300.f, 650.f), sf::Vector2f(80.f, 100.f)), "Capitan del barco. Acceso a todo,\npero jamas ha fallado a su tripulacion.\nPor que arriesgaria su reputacion?", false);
    sospechososBloque1.emplace_back("Isabella la Adivina", sf::FloatRect(sf::Vector2f(750.f, 620.f), sf::Vector2f(70.f, 90.f)), "Sintio que algo malo pasaria.\nPero... si realmente lo vio todo,\npor que no lo impidio?", false);
    sospechososBloque1.emplace_back("Don Julio el Pescador", sf::FloatRect(sf::Vector2f(550.f, 680.f), sf::Vector2f(80.f, 90.f)), "Pescador que siempre esta en el muelle.\nDice que vio todo desde su bote...\nDemasiado cerca del lugar del robo.\nY siempre fue el primero en llegar\na la escena.", true);
    
    std::vector<DialogoNarrativo> dialogosBloque1;
    DialogoNarrativo dialogoDonJulio("Don Julio el Pescador", 
        "Estaba en mi bote como cada noche.\nEl barco estaba en calma...\nluego vi a alguien moviendose\nsigilosamente cerca de las pertenencias.\nNo pude ver bien la cara,\nestaba muy oscuro.",
        "assets/images/niveles/nivel5/JulioPescador.png");
    dialogosBloque1.push_back(std::move(dialogoDonJulio));
    
    DialogoNarrativo dialogoCapitan("Capitan Rodrigo", 
        "Conozco a mi tripulacion.\nTodos son honorables.\nLo unico raro fue Isabella...\nestuvo despierta toda la noche.\nPero ella es asi, nerviosa a veces.\nJulio, en cambio... lleva semanas\npreguntando por el valor de las joyas.",
        "assets/images/niveles/nivel5/capitanRodrigo.png");
    dialogosBloque1.push_back(std::move(dialogoCapitan));
    
    DialogoNarrativo dialogoIsabella("Isabella la Adivina", 
        "Tuve una vision, lo admito.\nVi una figura encapuchada robando.\nPero no quiero decir quien era...\npor miedo a represalias.\nEra alguien en quien\ntodos confian.",
        "assets/images/niveles/nivel5/isabellaAdivina.png");
    dialogosBloque1.push_back(std::move(dialogoIsabella));
    
    DialogoNarrativo dialogoTestigo("Testigo", 
        "Recuerdo algo importante.\nJusto antes del robo,\nvi a Alguien revisando las pertenencias\nhoras antes, cuando estaban desatendidas.\nDijo que era para 'revisar sus aparejos'\npero no llevaba aparejos.\nEstaba encapuchado",
        "assets/images/niveles/nivel5/anonimo.jpg");
    dialogosBloque1.push_back(std::move(dialogoTestigo));

    // Bloque 2
    std::vector<ObjetoBuscar> objetosBloque2;

    objetosBloque2.emplace_back("Camaleon", sf::FloatRect(sf::Vector2f(235.f, 400.f), sf::Vector2f(50.f, 17.f)), " ", "assets/images/niveles/nivel5/camaleon.png");
    objetosBloque2.emplace_back("Cofre", sf::FloatRect(sf::Vector2f(753.f, 409.f), sf::Vector2f(40.f, 40.f)), " ", "assets/images/niveles/nivel5/cofre.png");
    objetosBloque2.emplace_back("Mapa", sf::FloatRect(sf::Vector2f(591.f, 471.f), sf::Vector2f(131.f, 59.f)), " ", "assets/images/niveles/nivel5/mapa.png");
    objetosBloque2.emplace_back("Reloj Antiguo", sf::FloatRect(sf::Vector2f(604.f, 390.f), sf::Vector2f(24.f, 34.f)), " ", "assets/images/niveles/nivel5/relojAntiguo.png");
    objetosBloque2.emplace_back("Microscopio", sf::FloatRect(sf::Vector2f(161.f, 197.f), sf::Vector2f(15.f, 53.f)), " ", "assets/images/niveles/nivel5/microscopio.png");
    objetosBloque2.emplace_back("Binocular", sf::FloatRect(sf::Vector2f(362.f, 485.f), sf::Vector2f(33.f, 15.f)), " ", "assets/images/niveles/nivel5/binocular.png");  
    objetosBloque2.emplace_back("Cortina", sf::FloatRect(sf::Vector2f(380.f, 200.f), sf::Vector2f(34.f, 85.f)), " ", "assets/images/niveles/nivel5/cortinas.png");
    objetosBloque2.emplace_back("Juego de Llaves", sf::FloatRect(sf::Vector2f(287.f, 507.f), sf::Vector2f(32.f, 42.f)), " ", "assets/images/niveles/nivel5/llaves.png");
    objetosBloque2.emplace_back("Periodico", sf::FloatRect(sf::Vector2f(619.f, 161.f), sf::Vector2f(43.f, 49.f)), " ", "assets/images/niveles/nivel5/periodico.png");
    objetosBloque2.emplace_back("Red", sf::FloatRect(sf::Vector2f(694.f, 95.f), sf::Vector2f(69.f, 98.f)), " ", "assets/images/niveles/nivel5/redes.png");

    std::vector<Sospechoso> sospechososBloque2;
    sospechososBloque2.emplace_back("Sebastian el Guardabosques", sf::FloatRect(sf::Vector2f(300.f, 600.f), sf::Vector2f(80.f, 100.f)), "Conoce cada rincon del bosque,\nsabe donde esconder cosas.\nParece honesto, pero...\nquien mejor que el para ocultar un robo?", false);
    sospechososBloque2.emplace_back("Valentina la Arqueologa", sf::FloatRect(sf::Vector2f(550.f, 620.f), sf::Vector2f(75.f, 90.f)), "Especialista en tesoros antiguos.\nSabe exactamente que hay dentro\nde cada cofre sin abrirlo.\nTiene el conocimiento...\ny el motivo.", true);
    sospechososBloque2.emplace_back("Don Mateo el Herrero", sf::FloatRect(sf::Vector2f(750.f, 650.f), sf::Vector2f(85.f, 85.f)), "Fabrica candados y llaves.\nPodria abrir cualquier cerradura.\nPero por que robaria algo\nque el mismo puede fabricar?", false);
    
    std::vector<DialogoNarrativo> dialogosBloque2;
    DialogoNarrativo dialogoSebastian("Sebastian el Guardabosques", 
        "Patrullo este bosque hace 3 siglos.\nLa noche anterior al robo,\nvi a alguien con una linterna\ncerca del cofre. No vi el rostro,\npero llevaba botas sucias y llenas de agua.",
        "assets/images/niveles/nivel5/sebastian.png");
    dialogosBloque2.push_back(std::move(dialogoSebastian));
    
    DialogoNarrativo dialogoMateo("Don Mateo el Herrero", 
        "Me pidieron copiar una llave antigua.\nEl patron era identico al candado\ndel cofre. No se quien fue,\npero la persona sabia exactamente\nque medidas pedir.",
        "assets/images/niveles/nivel5/mateo.png");
    dialogosBloque2.push_back(std::move(dialogoMateo));
    
    DialogoNarrativo dialogoValentina("Valentina la Arqueologa", 
        "Ese cofre contiene piezas unicas.\nLlevo mucho tiempo por estas tierras\nbuscando objetos antiguos.\nComo podria forzarlo?,\njamas lo robaria...\nSolo estudio para mi investigacion.",
        "assets/images/niveles/nivel5/valentina.png");
    dialogosBloque2.push_back(std::move(dialogoValentina));
    
    DialogoNarrativo dialogoAnonimo("Testigo Anonimo", 
        "Escuche una discusion acalorada\ncerca del campamento.\nAlguien gritaba: 'Ese tesoro\nes parte de mi,\nme pertenece por derecho'.",
        "assets/images/niveles/nivel5/anonimo.jpg");
    dialogosBloque2.push_back(std::move(dialogoAnonimo));

    // Bloque 3
   std::vector<ObjetoBuscar> objetosBloque3;

    objetosBloque3.emplace_back("Calaveras", sf::FloatRect(sf::Vector2f(697.f, 262.f), sf::Vector2f(45.f, 48.f)), " ", "assets/images/niveles/nivel5/calaveras.png");
    objetosBloque3.emplace_back("Bote", sf::FloatRect(sf::Vector2f(508.f, 254.f), sf::Vector2f(75.f, 35.f)), " ", "assets/images/niveles/nivel5/bote.png");
    objetosBloque3.emplace_back("Rueda", sf::FloatRect(sf::Vector2f(219.f, 184.f), sf::Vector2f(67.f, 67.f)), " ", "assets/images/niveles/nivel5/rueda.png");
    objetosBloque3.emplace_back("Botella de Vino", sf::FloatRect(sf::Vector2f(324.f, 312.f), sf::Vector2f(34.f, 51.f)), " ", "assets/images/niveles/nivel5/botellaVino.png");
    objetosBloque3.emplace_back("Cangrejo", sf::FloatRect(sf::Vector2f(711.f, 446.f), sf::Vector2f(44.f, 24.f)), " ", "assets/images/niveles/nivel5/cangrejo.png");
    objetosBloque3.emplace_back("Juguete de Pirata", sf::FloatRect(sf::Vector2f(370.f, 421.f), sf::Vector2f(24.f, 44.f)), " ", "assets/images/niveles/nivel5/muñeco.png");
    objetosBloque3.emplace_back("Maletin Oculto", sf::FloatRect(sf::Vector2f(345.f, 321.f), sf::Vector2f(55.f, 19.f)), " ", "assets/images/niveles/nivel5/maletinOculto.png");
    objetosBloque3.emplace_back("Dado", sf::FloatRect(sf::Vector2f(658.f, 529.f), sf::Vector2f(14.f, 19.f)), " ", "assets/images/niveles/nivel5/dado.png");
    objetosBloque3.emplace_back("Comida para Gato", sf::FloatRect(sf::Vector2f(379.f, 533.f), sf::Vector2f(34.f, 29.f)), " ", "assets/images/niveles/nivel5/comidaGato.png");
    objetosBloque3.emplace_back("Caja de Madera", sf::FloatRect(sf::Vector2f(280.f, 345.f), sf::Vector2f(46.f, 19.f)), " ", "assets/images/niveles/nivel5/cajaMadera.png");

    std::vector<Sospechoso> sospechososBloque3;
    sospechososBloque3.emplace_back("El Viejo Marino", sf::FloatRect(sf::Vector2f(300.f, 650.f), sf::Vector2f(80.f, 100.f)), "Vive en la costa desde hace decadas.\nConoce cada naufragio.", false);
    sospechososBloque3.emplace_back("Carmen la Buzo", sf::FloatRect(sf::Vector2f(550.f, 620.f), sf::Vector2f(75.f, 90.f)), "Bucea en busca de tesoros", false);
    sospechososBloque3.emplace_back("Misterioso Comerciante", sf::FloatRect(sf::Vector2f(750.f, 650.f), sf::Vector2f(85.f, 85.f)), "Siempre aparece justo\ndespues de los naufragios.\nNadie sabe de donde viene.", true);

    std::vector<DialogoNarrativo> dialogosBloque3;
    DialogoNarrativo dialogoMarino("El Viejo Marino", 
        "Yo conozco cada barco que ha naufragado\nen estas costas. Este ultimo...\nfue sabotaje. Vi a alguien nadando\nhacia la costa con un maletin\nla noche del accidente.",
        "assets/images/niveles/nivel5/viejoMarino.png");
    dialogosBloque3.push_back(std::move(dialogoMarino));

    DialogoNarrativo dialogoComerciante("Misterioso Comerciante", 
        "Yo solo compro lo que encuentro\nen la playa. Es mi negocio.\nCarmen siempre bucea en los pecios.\nElla sabe mas de lo que dice.",
        "assets/images/niveles/nivel5/comerciante.png");
    dialogosBloque3.push_back(std::move(dialogoComerciante));

    DialogoNarrativo dialogoBuzo("Carmen la Buzo", 
        "Es cierto que buceo en busca\nde tesoros, pero jamas robaria nada.\nYo fui quien encontre el maletin...\nalguien me lo robo de mi escondite.\nSolo el Comerciante sabia donde estaba.",
        "assets/images/niveles/nivel5/carmenBuzo.png");
    dialogosBloque3.push_back(std::move(dialogoBuzo));

    DialogoNarrativo dialogoTestigo2("Pescador Anonimo", 
        "La noche del naufragio vi a alguien\ncon una linterna cerca del muelle.\nNo era Carmen, ella bucea de dia.\nEra alguien que nunca se moja...\ncomo el Comerciante.",
        "assets/images/niveles/nivel5/anonimo.jpg");
    dialogosBloque3.push_back(std::move(dialogoTestigo2));

    // Guardar en vectores miembro
    m_todosLosObjetos.push_back(objetosBloque1);
    m_todosLosObjetos.push_back(objetosBloque2);
    m_todosLosObjetos.push_back(objetosBloque3);
    
    m_todosLosSospechosos.push_back(sospechososBloque1);
    m_todosLosSospechosos.push_back(sospechososBloque2);
    m_todosLosSospechosos.push_back(sospechososBloque3);
    
    m_todosLosDialogos.push_back(dialogosBloque1);
    m_todosLosDialogos.push_back(dialogosBloque2);
    m_todosLosDialogos.push_back(dialogosBloque3);
    
    // Agregar sets a los pools
    for (const auto& objSet : m_todosLosObjetos) {
        m_criminalMinigame.agregarSetObjetos(objSet);
    }
    for (const auto& sosSet : m_todosLosSospechosos) {
        m_criminalMinigame.agregarSetSospechosos(sosSet);
    }
    for (const auto& diaSet : m_todosLosDialogos) {
        m_criminalMinigame.agregarSetDialogos(diaSet);
    }
    
    m_objetosCriminal = objetosBloque1;  
    m_sospechososCriminal = sospechososBloque1;
    
    m_criminalMinigame.setInventory(m_player.getInventory());
    m_criminalMinigame.setBaseSize(sf::Vector2f(800.f, 600.f));
    
    sf::Vector2u windowSize = window->getSize();
    float minijuegoW = windowSize.x * 0.85f;
    float minijuegoH = windowSize.y * 0.85f;
    float minijuegoX = (static_cast<float>(windowSize.x) - minijuegoW) / 2.f;
    float minijuegoY = (static_cast<float>(windowSize.y) - minijuegoH) / 2.f;
    
    m_criminalMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    m_criminalMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
    
    m_criminalMinigame.init("assets/images/niveles/nivel5/criminalCase.png", 
                            m_objetosCriminal, 
                            m_sospechososCriminal);
    
    m_criminalMinigame.setDebugMode(true);
    
    m_criminalMinigame.setOnCompleteCallback([this](bool exito) {
    if (exito && !m_criminalGameCompleted) 
    {
        m_criminalGameCompleted = true;
        std::cout << "CASO RESUELTO! Set actual: " << m_setActualCaso << std::endl;
        mostrarMensaje("CASO RESUELTO. Has encontrado todas las pistas y al culpable.\nEntregale las cosas a Andrea", 5.0f);
    }
    });
    
    m_criminalMinigame.cargarFondoOnly("assets/images/niveles/nivel5/criminalCase.png");
    m_criminalMinigame.generarNuevoCaso(); 
    m_setActualCaso = 0;  
}


// REAJUSTAR MINIJUEGO CRIMINAL

void Nivel5State::reajustarMinijuegoCriminal()
{
    if (!m_criminalMinigame.isActive()) return;
    
    sf::Vector2u windowSize = window->getSize();
    float minijuegoW = windowSize.x * 0.85f;
    float minijuegoH = windowSize.y * 0.85f;
    float minijuegoX = (windowSize.x - minijuegoW) / 2.f;
    float minijuegoY = (windowSize.y - minijuegoH) / 2.f;
    
    m_criminalMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
    m_criminalMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    
    m_criminalMinigame.init("assets/images/niveles/nivel5/criminalCase.png", 
                            m_objetosCriminal, m_sospechososCriminal);
    m_criminalMinigame.setDebugMode(true);
}



// VERIFICAR SALIDA DEL NIVEL

void Nivel5State::verificarSalidaNivel() {
    m_cercaPuertaSalida = m_player.getHurtbox().findIntersection(m_puertaSalidaArea).has_value();
    
    static bool fPresionado = false;
    if (m_cercaPuertaSalida && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
        if (!fPresionado) {
            fPresionado = true;
            
            if (m_nivelCompletado) {
                std::cout << "Saliendo del nivel 5..." << std::endl;
                game->avanzarNivel();
            } else {
                mostrarMensajeFlotante("Debes resolver el caso criminal y\nentregar los objetos a Andrea primero.\nHabla con Andrea presionando F", 4.0f, sf::Color::Yellow);
            }
        }
    } else {
        fPresionado = false;
    }
}


// UPDATE

void Nivel5State::update(float dt)
{
    // Actualizar temporizador del mensaje flotante
    if (m_tiempoFlotante > 0.0f) {
        m_tiempoFlotante -= dt;
    }
    
    static sf::Vector2u lastWindowSize = window->getSize();
    sf::Vector2u currentWindowSize = window->getSize();

    if (currentWindowSize != lastWindowSize) {
        lastWindowSize = currentWindowSize;
        
        float minijuegoW = currentWindowSize.x * 0.85f;
        float minijuegoH = currentWindowSize.y * 0.85f;
        float minijuegoX = (currentWindowSize.x - minijuegoW) / 2.f;
        float minijuegoY = (currentWindowSize.y - minijuegoH) / 2.f;
        
        m_criminalMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
        m_criminalMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
        
        m_criminalMinigame.cargarFondoOnly("assets/images/niveles/nivel5/criminalCase.png");
        
        if (m_criminalMinigame.isActive()) 
        {
            std::cout << "Minijuego activo - reescalado aplicado" << std::endl;
        }
    }
    
    // Si hay mensaje emergente, no actualizar movimiento
    if (m_mensajeEmergenteActivo)
    {
        return;
    }
    
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f)
    {
        m_msjActual.tiempoRestante -= dt;
        if (m_msjActual.tiempoRestante <= 0.0f)
        {
            m_textoMensaje->setString("");
        }
    }

    sf::Vector2f posAnterior = m_player.getPosition();

    /// Deteccion de bloques interactivos
    m_cercaBloqueInteractivo = false;
    int bloqueIndex = -1;

    for (size_t i = 0; i < m_bloquesInteractivos.size(); i++) {
        if (m_player.getHurtbox().findIntersection(m_bloquesInteractivos[i].area).has_value()) {
            m_cercaBloqueInteractivo = true;
            bloqueIndex = i;
            break;
        }
    }

    m_bloqueActualIndex = bloqueIndex;   
    
    // Detección de área del minijuego criminal
    m_cercaCriminalArea = m_player.getHurtbox().findIntersection(m_criminalArea).has_value();

    static bool cCriminalPresionado = false;
    if (m_cercaCriminalArea && !m_criminalGameCompleted && !m_criminalMinigame.isActive() 
    && !m_mensajeEmergenteActivo) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
            if (!cCriminalPresionado) {
                cCriminalPresionado = true;
                
                m_criminalMinigame.limpiarPools();
                
                for (const auto& objSet : m_todosLosObjetos) {
                    m_criminalMinigame.agregarSetObjetos(objSet);
                }
                for (const auto& sosSet : m_todosLosSospechosos) {
                    m_criminalMinigame.agregarSetSospechosos(sosSet);
                }
                for (const auto& diaSet : m_todosLosDialogos) {
                    m_criminalMinigame.agregarSetDialogos(diaSet);
                }
                
                m_criminalMinigame.generarNuevoCaso();
                m_setActualCaso = m_criminalMinigame.getSetActualObjetos();

                sf::Vector2u winSize = window->getSize();
                float minijuegoW = winSize.x * 0.85f;
                float minijuegoH = winSize.y * 0.85f;
                float minijuegoX = (winSize.x - minijuegoW) / 2.f;
                float minijuegoY = (winSize.y - minijuegoH) / 2.f;
                
                m_criminalMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
                m_criminalMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
                m_criminalMinigame.cargarFondoOnly("assets/images/niveles/nivel5/criminalCase.png");
                
                m_criminalMinigame.activate();
                std::cout << "Minijuego activado con nuevo caso" << std::endl;
            }
        } else {
            cCriminalPresionado = false;
        }
    }
    
    // Manejar minijuego criminal activo
    if (m_criminalMinigame.isActive()) {
        m_criminalMinigame.update(dt);
        return;
    }
    
    // Interaccion con bloques (abrir mensaje con F)
    static bool fPresionado = false;
    if (m_cercaBloqueInteractivo && bloqueIndex != -1 && !m_mensajeEmergenteActivo && !m_nivelCompletado) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) { 
            if (!fPresionado) {
                fPresionado = true;
                m_mensajeEmergenteActivo = true;
                m_bloqueActualIndex = bloqueIndex;
            }
        } else {
            fPresionado = false;
        }
    } else {
        fPresionado = false;
    }

    // Movimiento
    Inventory *inv = m_player.getInventory();
    if (!inv || !inv->isOpen())
    {
        sf::Vector2f movimiento(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            movimiento.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            movimiento.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            movimiento.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            movimiento.x += 1.f;

        if (movimiento.x != 0.f || movimiento.y != 0.f)
        {
            float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
            movimiento /= length;
        }

        m_player.move(movimiento, dt);
    }

    m_player.update(dt);

    // Colisiones
    for (const auto &obj : m_mapaFisico)
    {
        if (m_player.getHurtbox().findIntersection(obj.getBounds()).has_value())
        {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }
    
    CoordenadasDebug::getInstance().actualizar(window, m_camera, m_player.getPosition());
    
    // Cámara
    sf::Vector2f playerPos = m_player.getPosition();
    sf::Vector2f cameraPos = playerPos;

    float halfWidth = 1280.f / 2.f;
    float halfHeight = 720.f / 2.f;

    if (halfWidth * 2.f >= m_worldSize.x)
    {
        cameraPos.x = m_worldSize.x / 2.f;
    }
    else
    {
        if (cameraPos.x < halfWidth)
            cameraPos.x = halfWidth;
        if (cameraPos.x > m_worldSize.x - halfWidth)
            cameraPos.x = m_worldSize.x - halfWidth;
    }

    if (halfHeight * 2.f >= m_worldSize.y)
    {
        cameraPos.y = m_worldSize.y / 2.f;
    }
    else
    {
        if (cameraPos.y < halfHeight)
            cameraPos.y = halfHeight;
        if (cameraPos.y > m_worldSize.y - halfHeight)
            cameraPos.y = m_worldSize.y - halfHeight;
    }

    m_camera.setCenter(cameraPos);

    verificarSalidaNivel();

    // Pausa
    if (!m_mostrarTutorial && !m_mostrarTutorialPorTecla && !m_mensajeEmergenteActivo)
    {
        static bool escapeProcesado_ = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
        {
            if (!escapeProcesado_)
            {
                escapeProcesado_ = true;
                game->pushState(std::make_unique<PauseState>(window, game));
            }
        }
        else
        {
            escapeProcesado_ = false;
        }
    }

    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
    {
        m_escapeConsumed = false;
    }
}


// DRAW

void Nivel5State::draw()
{
    if (!window)
        return;

    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);

    window->setView(m_camera);

    if (m_background)
    {
        window->draw(*m_background);
    }
    else
    {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(50, 30, 30));
        window->draw(fallback);
    }

    m_player.draw(*window);

    // Dibujar estrella (si no ha sido usada)
    if (!m_estrellaUsada && m_bloquesInteractivos.size() > 1) {
        sf::Texture estrellaTex;
        if (estrellaTex.loadFromFile("assets/images/niveles/nivel5/estrella.png")) {
            sf::Sprite estrellaSprite(estrellaTex);
            sf::FloatRect area = m_bloquesInteractivos[1].area;
            
            // Escalar la estrella al tamaño del área
            sf::Vector2u texSize = estrellaTex.getSize();
            float scaleX = area.size.x / texSize.x;
            float scaleY = area.size.y / texSize.y;
            estrellaSprite.setScale(sf::Vector2f(scaleX, scaleY));
            estrellaSprite.setPosition(sf::Vector2f(area.position.x, area.position.y));
            
            window->draw(estrellaSprite);
        }
    }
    // Debug: colisiones
    for (const auto &obj : m_mapaFisico)
    {
        sf::RectangleShape colision;
        colision.setPosition(sf::Vector2f(obj.getBounds().position.x, obj.getBounds().position.y));
        colision.setSize(sf::Vector2f(obj.getBounds().size.x, obj.getBounds().size.y));
        colision.setFillColor(sf::Color(255, 0, 0, 100));
        colision.setOutlineThickness(2.f);
        colision.setOutlineColor(sf::Color::Red);
        
    }
    
    // Dibujar bloques interactivos (modo debug)
    for (const auto &bloque : m_bloquesInteractivos)
    {
        sf::RectangleShape bloqueDebug(sf::Vector2f(bloque.area.size.x, bloque.area.size.y));
        bloqueDebug.setPosition(sf::Vector2f(bloque.area.position.x, bloque.area.position.y));
        bloqueDebug.setFillColor(sf::Color(0, 255, 255, 100));
        bloqueDebug.setOutlineThickness(2.f);
        bloqueDebug.setOutlineColor(sf::Color::Cyan);
    
    }
    
    if (m_mostrarPuertaSalida && m_nivelCompletado)
    {
        sf::RectangleShape salidaDebug(sf::Vector2f(m_puertaSalidaArea.size.x, m_puertaSalidaArea.size.y));
        salidaDebug.setPosition(sf::Vector2f(m_puertaSalidaArea.position.x, m_puertaSalidaArea.position.y));
        salidaDebug.setFillColor(sf::Color(0, 255, 0, 50));
        salidaDebug.setOutlineThickness(3.f);
        salidaDebug.setOutlineColor(sf::Color::Green);
    }

    window->setView(window->getDefaultView());

    CoordenadasDebug::getInstance().dibujar(*window);

    // Mensaje emergente (dialogo de Andrea)
    if (m_mensajeEmergenteActivo && m_bloqueActualIndex >= 0 && m_bloqueActualIndex < (int)m_bloquesInteractivos.size())
    {
        sf::Vector2u winSize = window->getSize();
        float winW2 = static_cast<float>(winSize.x);
        float winH2 = static_cast<float>(winSize.y);
        
        sf::RectangleShape overlay(sf::Vector2f(winW2, winH2));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);
        
        if (m_fontLoaded)
        {
            float dialogWidth = 800.f;
            float dialogHeight = 520.f;
            float dialogX = winW2 / 2.f - dialogWidth / 2.f;
            float dialogY = winH2 / 2.f - dialogHeight / 2.f;
            
            sf::RectangleShape shadow(sf::Vector2f(dialogWidth + 8.f, dialogHeight + 8.f));
            shadow.setFillColor(sf::Color(0, 0, 0, 150));
            shadow.setPosition(sf::Vector2f(dialogX + 4.f, dialogY + 4.f));
            window->draw(shadow);
            
            sf::RectangleShape dialogBox(sf::Vector2f(dialogWidth, dialogHeight));
            dialogBox.setFillColor(sf::Color(25, 25, 35, 245));
            dialogBox.setOutlineThickness(3.f);
            
            if (m_nivelCompletado)
                dialogBox.setOutlineColor(sf::Color(100, 255, 100, 255));
            else if (m_criminalGameCompleted)
                dialogBox.setOutlineColor(sf::Color(255, 215, 0, 255));
            else
                dialogBox.setOutlineColor(sf::Color(150, 150, 200, 255));
                
            dialogBox.setPosition(sf::Vector2f(dialogX, dialogY));
            window->draw(dialogBox);
            
            sf::RectangleShape topBar(sf::Vector2f(dialogWidth, 50.f));
            
            if (m_nivelCompletado)
                topBar.setFillColor(sf::Color(30, 80, 30, 220));
            else if (m_criminalGameCompleted)
                topBar.setFillColor(sf::Color(80, 70, 30, 220));
            else
                topBar.setFillColor(sf::Color(50, 40, 60, 220));
                
            topBar.setPosition(sf::Vector2f(dialogX, dialogY));
            window->draw(topBar);
            
            sf::RectangleShape accentLine(sf::Vector2f(dialogWidth - 40.f, 2.f));
            
            if (m_nivelCompletado)
                accentLine.setFillColor(sf::Color(100, 255, 100, 255));
            else if (m_criminalGameCompleted)
                accentLine.setFillColor(sf::Color(255, 215, 0, 255));
            else
                accentLine.setFillColor(sf::Color(200, 150, 100, 255));
                
            accentLine.setPosition(sf::Vector2f(dialogX + 20.f, dialogY + 48.f));
            window->draw(accentLine);
            
            // Dentro del daálogo emergente

            sf::Text tituloText(m_font);

            //verificar indice
            if (m_bloqueActualIndex == 1 && !m_estrellaUsada) {
                tituloText.setString("ESTRELLA MISTERIOSA ");
                tituloText.setFillColor(sf::Color(255, 215, 0, 255));  // Dorado
            } 
            else if (m_nivelCompletado) {
                tituloText.setString("o ANDREA o");
                tituloText.setFillColor(sf::Color(100, 255, 100, 255));
            }
            else if (m_criminalGameCompleted) {
                tituloText.setString("+ ANDREA +");
                tituloText.setFillColor(sf::Color(255, 215, 0, 255));
            }
            else {
                tituloText.setString("- ANDREA -");
                tituloText.setFillColor(sf::Color(255, 220, 150, 255));
            }
                            
            tituloText.setCharacterSize(24);
            tituloText.setStyle(sf::Text::Bold);
            
            if (m_nivelCompletado)
                tituloText.setFillColor(sf::Color(100, 255, 100, 255));
            else if (m_criminalGameCompleted)
                tituloText.setFillColor(sf::Color(255, 215, 0, 255));
            else
                tituloText.setFillColor(sf::Color(255, 220, 150, 255));
                
            sf::FloatRect tituloBounds = tituloText.getLocalBounds();
            tituloText.setOrigin(sf::Vector2f(tituloBounds.size.x / 2.f, tituloBounds.size.y / 2.f));
            tituloText.setPosition(sf::Vector2f(winW2 / 2.f, dialogY + 25.f));
            window->draw(tituloText);
            
            sf::Text mensajeText(m_font);

            std::string mensajeCompleto;
            if (m_bloqueActualIndex >= 0 && m_bloqueActualIndex < (int)m_bloquesInteractivos.size()) {
                mensajeCompleto = m_bloquesInteractivos[m_bloqueActualIndex].mensaje;
            }

            mensajeText.setString(mensajeCompleto);
            mensajeText.setCharacterSize(21);

            if (m_nivelCompletado)
                mensajeText.setFillColor(sf::Color(150, 255, 150, 255));
            else
                mensajeText.setFillColor(sf::Color(240, 240, 255, 255));

            mensajeText.setOutlineColor(sf::Color(0, 0, 0, 150));
            mensajeText.setOutlineThickness(1.5f);

            mensajeText.setOrigin(sf::Vector2f(0.f, 0.f));

            float textStartX = dialogX + 30.f;
            float textStartY = dialogY + 80.f;

            mensajeText.setPosition(sf::Vector2f(textStartX, textStartY));
            window->draw(mensajeText);
            
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

            // Verificar si es el bloque estrella
            if (m_bloqueActualIndex == 1 && !m_estrellaUsada) {
                instruccionText.setString("[ R ] Ir al Centinela      |     [ F ] Cerrar");
                instruccionText.setFillColor(sf::Color(255, 215, 0, 255));
            }
            else if (m_nivelCompletado) {
                instruccionText.setString("[ F ] Cerrar    ->    Ve al ascensor y presiona [ F ]");
                instruccionText.setFillColor(sf::Color(100, 255, 100, 255));
            }
            else if (m_criminalGameCompleted) {
                instruccionText.setString("[ R ] Entregar objetos a Andrea     |     [ F ] Cerrar");
                instruccionText.setFillColor(sf::Color(255, 215, 0, 255));
            }
            else {
                instruccionText.setString("[ F ] Cerrar     |     Resuelve el caso criminal primero");
                instruccionText.setFillColor(sf::Color(200, 150, 100, 255));
            }
            
            sf::FloatRect instrBounds = instruccionText.getLocalBounds();
            instruccionText.setOrigin(sf::Vector2f(instrBounds.size.x / 2.f, instrBounds.size.y / 2.f));
            instruccionText.setPosition(sf::Vector2f(winW2 / 2.f, dialogY + dialogHeight - 52.f));
            window->draw(instruccionText);
            
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
    }
    
   // Textos de interacción
    if (m_fontLoaded && m_textoInteraccion)
    {
        if (m_cercaCriminalArea && !m_criminalGameCompleted && !m_criminalMinigame.isActive()) {
            m_textoInteraccion->setString("Presiona F para investigar el crimen");
            m_textoInteraccion->setOutlineColor(sf::Color::Black);      
            m_textoInteraccion->setOutlineThickness(2.0f);
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 90.f));
            window->draw(*m_textoInteraccion);
        }
        if (m_cercaPuertaSalida && m_nivelCompletado)
        {
            m_textoInteraccion->setString("Presiona F para ir al siguiente nivel");
            m_textoInteraccion->setOutlineColor(sf::Color::Black);
            m_textoInteraccion->setOutlineThickness(2.0f);
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 90.f));
            window->draw(*m_textoInteraccion);
        }
        
        if (m_cercaBloqueInteractivo && !m_mensajeEmergenteActivo)
        {
            
            if (m_bloqueActualIndex == 0) {
                m_textoInteraccion->setString("Presiona F. Andrea quiere decirte algo");
            } 
            else if (m_bloqueActualIndex == 1 && !m_estrellaUsada) {
                m_textoInteraccion->setString("Presiona F para seguir la estrella");
            }
            
            m_textoInteraccion->setOutlineColor(sf::Color::Black);      
            m_textoInteraccion->setOutlineThickness(2.0f);
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 100.f));
            window->draw(*m_textoInteraccion);
        }
        
    }

    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f && !m_textoMensaje->getString().isEmpty() && !m_mensajeEmergenteActivo)
    {
        sf::Vector2u winSize = window->getSize();
        
        float posX = static_cast<float>(winSize.x) * 0.5f;   
        float posY = static_cast<float>(winSize.y) * 0.85f;  
        
        m_textoMensaje->setCharacterSize(22);  
        m_textoMensaje->setOutlineThickness(1.5f);
        
        sf::FloatRect bounds = m_textoMensaje->getLocalBounds();
        m_textoMensaje->setOrigin(sf::Vector2f(0.f, 0.f));
        m_textoMensaje->setPosition(sf::Vector2f(posX - bounds.size.x / 2.f, posY));
        window->draw(*m_textoMensaje);
    }

    
    // MENSAJE FLOTANTE CENTRADO 
    
    if (m_tiempoFlotante > 0.0f && !m_mensajeFlotante->getString().isEmpty() && !m_mensajeEmergenteActivo)
    {
        sf::Vector2u winSize = window->getSize();
        float centerX = winSize.x / 2.f;
        float centerY = winSize.y / 2.f;
        
        sf::FloatRect textBounds = m_mensajeFlotante->getLocalBounds();
        
        sf::RectangleShape fondoRect(sf::Vector2f(textBounds.size.x + 40, textBounds.size.y + 30));
        fondoRect.setFillColor(sf::Color(0, 0, 0, 220));
        fondoRect.setOutlineColor(m_mensajeFlotante->getFillColor());
        fondoRect.setOutlineThickness(2.f);
        fondoRect.setOrigin(sf::Vector2f(fondoRect.getSize().x / 2.f, fondoRect.getSize().y / 2.f));
        fondoRect.setPosition(sf::Vector2f(centerX, centerY));
        window->draw(fondoRect);
        
        m_mensajeFlotante->setPosition(sf::Vector2f(centerX, centerY));
        window->draw(*m_mensajeFlotante);
    }

    // Tutorial
    if (m_mostrarTutorial || m_mostrarTutorialPorTecla)
    {
        sf::RectangleShape overlay(sf::Vector2f(window->getSize().x, window->getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);

        if (m_fontLoaded)
        {
            sf::Text tutorialText(m_font);
           tutorialText.setString(
            "EL SECRETO DEL PUERTO\n\n"
            "Un robo ha sacudido la tranquilidad del muelle.\n"
            "Andrea confia en ti para resolver el caso, acercate a Andrea.\n\n"
            "Investiga cada rincon, reune las pistas y\n"
            "descubre la verdad antes de que sea tarde.\n\n"
            "[M] Ayuda/Cerrar");
            tutorialText.setCharacterSize(20);
            tutorialText.setFillColor(sf::Color::White);
            sf::FloatRect textBounds = tutorialText.getLocalBounds();
            tutorialText.setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            tutorialText.setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y / 2.f));
            window->draw(tutorialText);
        }
    }

    Inventory *inv = m_player.getInventory();
    if (inv)
    {
        inv->draw(*window);
    }
    
    if (m_criminalMinigame.isActive()) {
        m_criminalMinigame.draw(*window);
    }
}


// CONFIGURAR COLISIONES

void Nivel5State::configurarColisiones()
{
    m_mapaFisico.clear();

    float mapWidth = m_worldSize.x;
    float mapHeight = m_worldSize.y;

    m_mapaFisico.emplace_back(0.f, 0.f, mapWidth, 250.f);
    m_mapaFisico.emplace_back(0.f, mapHeight - 30.f, mapWidth, 30.f);
    m_mapaFisico.emplace_back(0.f, 0.f, 30.f, mapHeight);
    m_mapaFisico.emplace_back(mapWidth - 30.f, 0.f, 30.f, mapHeight);
    m_mapaFisico.emplace_back(32.f,250.f,161.f,171.f);
    m_mapaFisico.emplace_back(153.f,250.f,273.f,160.f);
    m_mapaFisico.emplace_back(195.f,423.f, 65.f,37.f);
    m_mapaFisico.emplace_back(365.f,423.f, 65.f,37.f);
    m_mapaFisico.emplace_back(32.f,420.f, 111.f,728.f);
    m_mapaFisico.emplace_back(160.f,857.f, 70.f,117.f);
    m_mapaFisico.emplace_back(231.f,900.f, 65.f,74.f);
    m_mapaFisico.emplace_back(147.f,1062.f, 143.f,86.f);
    m_mapaFisico.emplace_back(997.f,254.f, 300.f,155.f);
    m_mapaFisico.emplace_back(1108.f,414.f, 60.f,82.f);
    m_mapaFisico.emplace_back(1176.f,487.f, 122.f,605.f);
    //silla de descanso
    m_mapaFisico.emplace_back(984.f,534.f, 72.f,121.f);
    m_mapaFisico.emplace_back(908.f,615.f, 91.f,40.f);
    m_mapaFisico.emplace_back(854.f,658.f, 88.f,104.f);
    //mesa con agua de coco
    m_mapaFisico.emplace_back(961.f,664.f, 131.f,149.f);
    //silla 2 de descanso
    m_mapaFisico.emplace_back(1134.f,729.f, 72.f,121.f);
    m_mapaFisico.emplace_back(1048.f,814.f, 97.f,102.f);
    m_mapaFisico.emplace_back(963.f,873.f, 88.f,104.f);
    //estrella
    m_mapaFisico.emplace_back(1123.f, 1096.f, 10.f,10.f);

    //ajustes
    m_mapaFisico.emplace_back(1135.f,1125.f, 98.f, 20.f);
    

    m_mapaFisico.emplace_back(424.f,250.f, 600.f,61.f);
    std::cout << "Colisiones configuradas" << std::endl;
}


// JUGADOR HA MUERTO

void Nivel5State::jugadorHaMuerto()
{
    LevelNode *currentNode = game->getLevelTree().getCurrentNode();
    if (currentNode && currentNode->type == LevelType::CENTINELA)
    {
        GameProgressData &progress = game->getSaveManager().getCurrentProgress();
        game->getSaveManager().addMuerte();
        if (progress.modoElegido == GameProgressData::ModoJuego::CAMINO_AGRADABLE)
        {
            std::cout << "Muerte en centinela (modo agradable)" << std::endl;
        }
        else
        {
            std::cout << "Muerte en centinela (modo consecuencias)" << std::endl;
        }
    }
    else
    {
        game->pushState(std::make_unique<PauseState>(window, game));
    }
}


// MOSTRAR MENSAJE 

void Nivel5State::mostrarMensaje(const std::string &texto, float duracion, sf::Color color)
{
    if (!m_textoMensaje)
        return;

    m_msjActual.texto = texto;
    m_msjActual.tiempoRestante = duracion;
    m_msjActual.color = color;

    m_textoMensaje->setString(texto);
    m_textoMensaje->setFillColor(color);

    sf::FloatRect bounds = m_textoMensaje->getLocalBounds();
    m_textoMensaje->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));

    std::cout << "MENSAJE: " << texto << std::endl;
}